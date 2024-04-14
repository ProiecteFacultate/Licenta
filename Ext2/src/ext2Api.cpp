#include "windows.h"
#include "string.h"
#include "vector"

#include "../include/disk.h"
#include "../include/diskCodes.h"
#include "../include/structures.h"
#include "../include/ext2FunctionUtils.h"
#include "../include/ext2BlocksAllocation.h"
#include "../include/codes/ext2Attributes.h"
#include "../include/codes/ext2Codes.h"
#include "../include/codes/ext2ApiResponseCodes.h"
#include "../include/ext2.h"
#include "../include/utils.h"
#include "../include/ext2Api.h"

uint32_t createDirectory(DiskInfo* diskInfo, ext2_super_block* superBlock, char* parentDirectoryPath, char* newDirectoryName, uint32_t newDirectoryType)
{
    ext2_inode* actualInode = nullptr;
    bool isParentRoot;
    uint32_t searchParentInodeResult = searchInodeByFullPath(diskInfo, superBlock, parentDirectoryPath, &actualInode, isParentRoot);

    if(searchParentInodeResult != SEARCH_INODE_BY_FULL_PATH_SUCCESS)
        return DIRECTORY_CREATION_PARENT_DO_NOT_EXIST;

    uint32_t parentAlreadyContainsDirectoryWithGivenName = searchInodeByDirectoryNameInParent(diskInfo, superBlock, actualInode,
                                                                                              newDirectoryName, new ext2_inode(), isParentRoot);

    if(actualInode->i_mode != FILE_TYPE_FOLDER)
        return DIRECTORY_CREATION_PARENT_NOT_A_FOLDER;

    if(parentAlreadyContainsDirectoryWithGivenName == SEARCH_INODE_BY_DIRECTORY_NAME_IN_PARENT_SUCCESS)
        return DIRECTORY_CREATION_NEW_NAME_ALREADY_EXISTS;

    //create the new inode (without preallocate blocks yet)
    ext2_inode* newInode = new ext2_inode();
    uint32_t createInodeResult = createInode(diskInfo, superBlock, actualInode, newInode, newDirectoryType, isParentRoot);

    if(createInodeResult == CREATE_INODE_FAILED_NO_FREE_INODES)
        return DIRECTORY_CREATION_NO_FREE_INODES;
    else if(createInodeResult == CREATE_INODE_FAILED_FOR_OTHER_REASON)
        return DIRECTORY_CREATION_FAILED_FOR_OTHER_REASON;

    //mark the inode as occupied in the inode bitmap
    uint32_t updateValueInInodeBitmapResult = updateValueInInodeBitmap(diskInfo, superBlock, newInode->i_global_index, 1);

    if(updateValueInInodeBitmapResult == UPDATE_VALUE_IN_INODE_BITMAP_FAILED)
        return DIRECTORY_CREATION_FAILED_FOR_OTHER_REASON;

    //now we look for blocks to preallocate
    uint32_t preallocateNumberOfBlocks;
    if(newDirectoryType == FILE_TYPE_FOLDER)
        preallocateNumberOfBlocks = superBlock->s_prealloc_dir_blocks;
    else if(newDirectoryType == FILE_TYPE_REGULAR_FILE)
        preallocateNumberOfBlocks = superBlock->s_prealloc_blocks;

    std::vector<uint32_t> preallocateBlocks;
    uint32_t searchPreallocateBlocksResult;
    do {
        searchPreallocateBlocksResult = searchAndOccupyMultipleBlocks(diskInfo, superBlock, newInode->i_group, preallocateNumberOfBlocks, preallocateBlocks);
        preallocateNumberOfBlocks /= 2;
    } while (searchPreallocateBlocksResult != SEARCH_MULTIPLE_DATA_BLOCKS_SUCCESS && preallocateNumberOfBlocks > 0);

    if(searchPreallocateBlocksResult == SEARCH_MULTIPLE_DATA_BLOCKS_NO_ENOUGH_CONSECUTIVE_FREE_BLOCKS)
        return DIRECTORY_CREATION_NO_FREE_DATA_BLOCKS;
    else  if(searchPreallocateBlocksResult == SEARCH_MULTIPLE_DATA_BLOCKS_FAILED_FOR_OTHER_REASON)
        return DIRECTORY_CREATION_FAILED_FOR_OTHER_REASON;

    //mark the preallocate blocks as occupied in the data blocks bitmap
    for(int i = 0; i < preallocateBlocks.size(); i++)
    {
        uint32_t updateDataBlockBitmapResult = updateValueInDataBlockBitmap(diskInfo, superBlock, preallocateBlocks[i], 1);
        if(updateDataBlockBitmapResult == UPDATE_VALUE_IN_DATA_BLOCK_BITMAP_FAILED)
            return DIRECTORY_CREATION_FAILED_FOR_OTHER_REASON;
    }

    newInode->i_blocks = preallocateBlocks.size();
    for(int i = 0; i < preallocateBlocks.size(); i++)
        newInode->i_block[i] = preallocateBlocks[i];

    //now add the inode to the inode table
    uint32_t addInodeToInodeTableResult = addInodeToInodeTable(diskInfo, superBlock, newInode);

    if(addInodeToInodeTableResult == ADD_INODE_TO_INODE_TABLE_FAILED)
        return DIRECTORY_CREATION_FAILED_FOR_OTHER_REASON;

    //update inode's group and preallocate blocks' group groups descriptor
    uint32_t preallocateBlocksGroup = preallocateBlocks[0] / superBlock->s_blocks_per_group;
    uint32_t inodeGroupDescriptorUpdate = updateGroupDescriptor(diskInfo, superBlock, newInode->i_group, -1, 0);
    uint32_t preallocateBlocksDescriptorUpdate = updateGroupDescriptor(diskInfo, superBlock, preallocateBlocksGroup, 0, -preallocateBlocks.size());

    if(inodeGroupDescriptorUpdate == UPDATE_GROUP_DESCRIPTOR_FAILED || preallocateBlocksDescriptorUpdate == UPDATE_GROUP_DESCRIPTOR_FAILED)
        return DIRECTORY_CREATION_FAILED_FOR_OTHER_REASON;

    uint32_t addDirectoryEntryToParentResult = addDirectoryEntryToParent(diskInfo, superBlock, actualInode, newInode, newDirectoryName);

    return (addDirectoryEntryToParentResult == ADD_DIRECTORY_ENTRY_TO_PARENT_SUCCESS) ? DIRECTORY_CREATION_SUCCESS : DIRECTORY_CREATION_FAILED_FOR_OTHER_REASON;
}

uint32_t getSubDirectoriesByParentPath(DiskInfo* diskInfo, ext2_super_block* superBlock, char* directoryPath, std::vector<std::pair<ext2_inode*, ext2_dir_entry*>>& subDirectories)
{
    ext2_inode* givenDirectoryInode = nullptr;
    bool isParentRoot;
    uint32_t searchParentInodeResult = searchInodeByFullPath(diskInfo, superBlock, directoryPath, &givenDirectoryInode, isParentRoot);

    if(searchParentInodeResult != SEARCH_INODE_BY_FULL_PATH_SUCCESS)
        return GET_SUBDIRECTORIES_GIVEN_DIRECTORY_DO_NOT_EXIST;

    if(givenDirectoryInode->i_mode != FILE_TYPE_FOLDER)
        return GET_SUBDIRECTORIES_GIVEN_DIRECTORY_NOT_A_FOLDER;

    return getSubDirectoriesByParentInode(diskInfo, superBlock, givenDirectoryInode, subDirectories);
}

uint32_t write(DiskInfo* diskInfo, ext2_super_block* superBlock, char* directoryPath, char* dataBuffer, uint32_t maxBytesToWrite, uint32_t& numberOfBytesWritten, uint32_t writeAttribute,
               uint32_t& reasonForIncompleteWrite)
{
    if(strcmp(directoryPath, "Root\0") == 0) //you can't write directly to root
        return WRITE_BYTES_TO_FILE_CAN_NOT_WRITE_GIVEN_FILE;

    ext2_inode *actualInode = nullptr;
    bool isParentRoot;
    uint32_t findDirectoryEntryResult = searchInodeByFullPath(diskInfo, superBlock, directoryPath,
                                                                     &actualInode, isParentRoot);
    if(findDirectoryEntryResult != SEARCH_INODE_BY_FULL_PATH_SUCCESS)
        return WRITE_BYTES_TO_FILE_GIVEN_FILE_DO_NOT_EXIST_OR_SEARCH_FAIL;

    if(actualInode->i_mode != FILE_TYPE_REGULAR_FILE)
    {
        delete actualInode;
        return WRITE_BYTES_TO_FILE_CAN_NOT_WRITE_GIVEN_FILE;
    }

    uint32_t writeResult;

    if(writeAttribute == WRITE_WITH_TRUNCATE)
        writeResult = writeBytesToFileWithTruncate(diskInfo, superBlock, actualInode, dataBuffer, maxBytesToWrite, numberOfBytesWritten,
                                                   reasonForIncompleteWrite);
    else
        writeResult = writeBytesToFileWithAppend(diskInfo, superBlock, actualInode, dataBuffer, maxBytesToWrite, numberOfBytesWritten,
                                                 reasonForIncompleteWrite);

    if(writeResult == WRITE_BYTES_TO_FILE_SUCCESS)
    {
        if(writeAttribute == WRITE_WITH_TRUNCATE)
            actualInode->i_size = numberOfBytesWritten;
        else
            actualInode->i_size = actualInode->i_size + numberOfBytesWritten;

        //CAUTION we don't query the result of inode update, so it might fail, but the bytes were written, so it's still considered a success
        updateInode(diskInfo, superBlock, actualInode, actualInode);

        delete actualInode;
        return WRITE_BYTES_TO_FILE_SUCCESS;
    }

    delete actualInode;
    return WRITE_BYTES_TO_FILE_FAILED_FOR_OTHER_REASON;
}

uint32_t read(DiskInfo* diskInfo, ext2_super_block* superBlock, char* directoryPath, char* readBuffer, uint32_t startingPosition, uint32_t maxBytesToRead, uint32_t& numberOfBytesRead,
              uint32_t& reasonForIncompleteRead)
{
    numberOfBytesRead = 0;
    uint32_t blockGlobalIndex, numOfBytesReadFromThisBlock, readResult, numberOfSectorsRead;
    uint32_t numOfSectorsPerBlock = getNumberOfSectorsPerBlock(diskInfo, superBlock);

    if(strcmp(directoryPath, "Root\0") == 0) //you can't read directly to root
        return READ_BYTES_FROM_FILE_CAN_NOT_READ_GIVEN_FILE;

    ext2_inode* actualInode = nullptr;
    bool isParentRoot;
    uint32_t findDirectoryEntryResult = searchInodeByFullPath(diskInfo, superBlock, directoryPath,
                                                                     &actualInode, isParentRoot);

    if(findDirectoryEntryResult != SEARCH_INODE_BY_FULL_PATH_SUCCESS)
        return READ_BYTES_FROM_FILE_GIVEN_FILE_DO_NOT_EXIST_OR_SEARCH_FAIL;

    if(actualInode->i_mode != FILE_TYPE_REGULAR_FILE)
    {
        delete actualInode;
        return READ_BYTES_FROM_FILE_CAN_NOT_READ_GIVEN_FILE;
    }

    if(startingPosition > actualInode->i_size)
    {
        delete actualInode;
        return READ_BYTES_FROM_FILE_GIVEN_START_EXCEEDS_FILE_SIZE;
    }

    uint32_t blockLocalIndex = startingPosition / superBlock->s_log_block_size;
    uint32_t startingPositionOffsetInBlock = startingPosition % superBlock->s_log_block_size;

    uint32_t getBlockGlobalIndexResult = getDataBlockGlobalIndexByLocalIndexInsideInode(diskInfo, superBlock,
                                                                                        actualInode, blockLocalIndex,
                                                                                        blockGlobalIndex);
    if(getBlockGlobalIndexResult != GET_DATA_BLOCK_BY_LOCAL_INDEX_SUCCESS)
    {
        delete actualInode;
        return READ_BYTES_FROM_FILE_FAILED_FOR_OTHER_REASON;
    }

    readResult = readDiskSectors(diskInfo, numOfSectorsPerBlock,getFirstSectorForGivenBlock(diskInfo, superBlock, blockGlobalIndex),
                                 readBuffer, numberOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        delete actualInode;
        return READ_BYTES_FROM_FILE_FAILED_FOR_OTHER_REASON;
    }

    numberOfBytesRead = std::min(actualInode->i_size - startingPosition, std::min(superBlock->s_log_block_size - startingPositionOffsetInBlock, maxBytesToRead));
    memcpy(readBuffer, readBuffer + startingPositionOffsetInBlock, numberOfBytesRead);

    while(numberOfBytesRead < maxBytesToRead)
    {
        blockLocalIndex++;
        if(blockLocalIndex >= actualInode->i_blocks)
        {
            reasonForIncompleteRead = INCOMPLETE_BYTES_READ_DUE_TO_NO_FILE_NOT_LONG_ENOUGH;
            delete actualInode;
            return READ_BYTES_FROM_FILE_SUCCESS;
        }

        getBlockGlobalIndexResult = getDataBlockGlobalIndexByLocalIndexInsideInode(diskInfo, superBlock, actualInode,
                                                                                   blockLocalIndex, blockGlobalIndex);

        if(getBlockGlobalIndexResult != GET_DATA_BLOCK_BY_LOCAL_INDEX_SUCCESS)
        {
            reasonForIncompleteRead = INCOMPLETE_BYTES_READ_DUE_TO_OTHER;
            delete actualInode;
            return READ_BYTES_FROM_FILE_FAILED_FOR_OTHER_REASON;
        }

        readResult = readDiskSectors(diskInfo, numOfSectorsPerBlock,getFirstSectorForGivenBlock(diskInfo, superBlock, blockGlobalIndex),
                                     readBuffer + numberOfBytesRead, numberOfSectorsRead);

        if(readResult != EC_NO_ERROR)
        {
            reasonForIncompleteRead = INCOMPLETE_BYTES_READ_DUE_TO_OTHER;
            delete actualInode;
            return READ_BYTES_FROM_FILE_FAILED_FOR_OTHER_REASON;
        }

        numOfBytesReadFromThisBlock = std::min(actualInode->i_size - startingPosition - numberOfBytesRead, std::min(superBlock->s_log_block_size, maxBytesToRead - numberOfBytesRead));
        numberOfBytesRead += numOfBytesReadFromThisBlock;
    }

    delete actualInode;
    return READ_BYTES_FROM_FILE_SUCCESS;
}

uint32_t truncateFile(DiskInfo* diskInfo, ext2_super_block* superBlock, char* directoryPath, uint32_t newSize)
{
    ext2_inode *actualInode = nullptr;
    bool isParentRoot;
    uint32_t findDirectoryEntryResult = searchInodeByFullPath(diskInfo, superBlock, directoryPath, &actualInode, isParentRoot);

    if(findDirectoryEntryResult != SEARCH_INODE_BY_FULL_PATH_SUCCESS)
        return TRUNCATE_FILE_GIVEN_FILE_DO_NOT_EXIST_OR_SEARCH_FAIL;

    if(actualInode->i_mode != FILE_TYPE_REGULAR_FILE)
    {
        delete actualInode;
        return TRUNCATE_FILE_CAN_NOT_TRUNCATE_GIVEN_FILE_TYPE;
    }

    if(newSize >= actualInode->i_size)
        return TRUNCATE_FILE_NEW_SIZE_GREATER_THAN_ACTUAL_SIZE;

    uint32_t numberOfBlocksOccupied = actualInode->i_size / superBlock->s_log_block_size + 1;
    if(actualInode->i_size % superBlock->s_log_block_size == 0)
        numberOfBlocksOccupied--;

    uint32_t numberOfBlocksToRemainOccupied = newSize / superBlock->s_log_block_size + 1;
    if(newSize % superBlock->s_log_block_size == 0)
        numberOfBlocksToRemainOccupied--;

    ext2_inode* updatedParentInode = new ext2_inode();
    memcpy(updatedParentInode, actualInode, sizeof(ext2_inode));
    updatedParentInode->i_size = newSize;
    updatedParentInode->i_blocks = numberOfBlocksToRemainOccupied;
    uint32_t updateParentInodeResult = updateInode(diskInfo, superBlock, actualInode, updatedParentInode);

    if(updateParentInodeResult == UPDATE_INODE_FAILED)
        return TRUNCATE_FILE_FAILED_FOR_OTHER_REASON;

    ext2_inode* inodeCopy = new ext2_inode();
    memcpy(inodeCopy, actualInode, sizeof(ext2_inode));
    for(uint32_t blockLocalIndex = 1; blockLocalIndex <= numberOfBlocksOccupied - numberOfBlocksToRemainOccupied; blockLocalIndex++)
    {
        deallocateLastBlockInDirectory(diskInfo, superBlock, inodeCopy); //we don't query the deallocation result, so it might fail to deallocate, so we will have trash blocks
        inodeCopy->i_blocks--;
    }

    return TRUNCATE_FILE_SUCCESS;
}

uint32_t deleteDirectoryByPath(DiskInfo* diskInfo, ext2_super_block* superBlock, char* directoryPath, std::string& warning)
{

    if(strcmp(directoryPath, "Root\0") == 0) //you can't delete the root
        return DELETE_DIRECTORY_CAN_NOT_DELETE_ROOT;

    char* parentPath = new char[strlen(directoryPath)];
    extractParentPathFromPath(directoryPath, parentPath);

    bool isParentRoot;
    ext2_inode *actualInode = nullptr;
    uint32_t findDirectoryEntryResult = searchInodeByFullPath(diskInfo, superBlock, directoryPath, &actualInode, isParentRoot);

    if(findDirectoryEntryResult != SEARCH_INODE_BY_FULL_PATH_SUCCESS)
        return DELETE_DIRECTORY_FAILED_TO_FIND_GIVEN_DIRECTORY;

    ext2_inode *parentInode = nullptr;
    findDirectoryEntryResult = searchInodeByFullPath(diskInfo, superBlock, parentPath, &parentInode, isParentRoot);

    if(findDirectoryEntryResult != SEARCH_INODE_BY_FULL_PATH_SUCCESS)
        return DELETE_DIRECTORY_FAILED_FOR_OTHER_REASON; // if we found the directory, parent also exist, so if we have a fails here is for another reason ,not because parent do not exist

    uint32_t deleteDirectoryEntryFromParentResult = deleteDirectoryEntryFromParent(diskInfo, superBlock, actualInode, parentInode);

    if(deleteDirectoryEntryFromParentResult == DELETE_DIRECTORY_ENTRY_FROM_PARENT_FAILED)
        return DELETE_DIRECTORY_FAILED_TO_DELETE_DIRECTORY_ENTRY_FROM_PARENT;

    uint32_t deleteInodeResult = deleteInodeOfDirectoryAndChildren(diskInfo, superBlock, actualInode);
 //   uint32_t deleteInodeResult = deleteInode(diskInfo, superBlock, actualInode);

    if(deleteInodeResult == DELETE_ALL_DIRECTORY_AND_CHILDREN_INODES_FAILED)
        return DELETE_DIRECTORY_FAILED_TO_DELETE_INODE;

    uint32_t freeClustersOfDirectoryAndParentResult = freeBlocksOfDirectoryAndChildren(diskInfo, superBlock, actualInode, warning); //we don't return, but give a warning

    return (freeClustersOfDirectoryAndParentResult == FREE_ALL_DIRECTORY_AND_CHILDREN_BLOCKS_SUCCESS) ? DELETE_DIRECTORY_SUCCESS : DELETE_DIRECTORY_FAILED_TO_FREE_BLOCKS;
}