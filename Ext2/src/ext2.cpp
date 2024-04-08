#include "windows.h"
#include "string.h"
#include "string"
#include "vector"

#include "../include/disk.h"
#include "../include/diskCodes.h"
#include "../include/structures.h"
#include "../include/ext2FunctionUtils.h"
#include "../include/ext2Heuristics.h"
#include "../include/codes/ext2Attributes.h"
#include "../include/codes/ext2Codes.h"
#include "../include/codes/ext2ApiResponseCodes.h"
#include "../include/utils.h"
#include "../include/ext2BlocksAllocation.h"
#include "../include/codes/ext2BlocksAllocationCodes.h"
#include "../include/ext2.h"

#define BIG_VALUE 99999999

uint32_t createInode(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* parentInode, ext2_inode* createdInode, uint32_t fileType, bool& isParentRoot)
{
    uint32_t searchInodeResult, inodeGlobalIndex;

    if(fileType == FILE_TYPE_FOLDER)
        searchInodeResult = (isParentRoot ? searchFreeInodeForDirectoryHavingParentRoot(diskInfo, superBlock, inodeGlobalIndex)
                                          : searchFreeInodeForNestedDirectory(diskInfo, superBlock, parentInode, inodeGlobalIndex));
    else if(fileType == FILE_TYPE_REGULAR_FILE)
        searchInodeResult = searchFreeInodeForRegularFile(diskInfo, superBlock, parentInode, inodeGlobalIndex);

    if(searchInodeResult == SEARCH_FREE_INODE_NO_FREE_INODES)
        return CREATE_INODE_FAILED_NO_FREE_INODES;
    else if(searchInodeResult == SEARCH_FREE_INODE_FAILED_FOR_OTHER_REASON)
        return CREATE_INODE_FAILED_FOR_OTHER_REASON;

    uint32_t groupForFoundInode = inodeGlobalIndex / superBlock->s_inodes_per_group;

    SYSTEMTIME time;
    GetSystemTime(&time);

    memset(createdInode, 0, 128);

    createdInode->i_mode = fileType;
    createdInode->i_size = 0;
    //high 7 bits represent how many years since 1900, next 4 for month, next 5 for day and the low 16 represent the second in that day with a granularity of 2 (see in fat)
    createdInode->i_atime = ((time.wYear - 1900) << 25) | (time.wMonth << 21) | (time.wDay << 16) | ((time.wHour * 3600 + time.wMinute * 60 + time.wSecond) / 2);
    createdInode->i_ctime = createdInode->i_atime;
    createdInode->i_mtime = createdInode->i_atime;
    createdInode->i_group = groupForFoundInode;
    createdInode->i_global_index = inodeGlobalIndex;

    return CREATE_INODE_SUCCESS;
}

uint32_t searchAndOccupyMultipleBlocks(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t preferredGroup,  uint32_t numOfBlocks, std::vector<uint32_t>& newBlocks)
{
    uint32_t searchMultipleBlocksResult = searchAndOccupyMultipleBlocksInGivenGroup(diskInfo, superBlock, preferredGroup, numOfBlocks, newBlocks);

    if(searchMultipleBlocksResult == SEARCH_MULTIPLE_DATA_BLOCKS_IN_GROUP_SUCCESS)
        return SEARCH_MULTIPLE_DATA_BLOCKS_SUCCESS;
    else if(searchMultipleBlocksResult == SEARCH_MULTIPLE_DATA_BLOCKS_IN_GROUP_FAILED_FOR_OTHER_REASON)
        return SEARCH_MULTIPLE_DATA_BLOCKS_FAILED_FOR_OTHER_REASON;

    for(uint32_t group = 0; group < getNumberOfGroups(superBlock); group++)
    {
        searchMultipleBlocksResult = searchAndOccupyMultipleBlocksInGivenGroup(diskInfo, superBlock, group, numOfBlocks, newBlocks);
        if(searchMultipleBlocksResult == SEARCH_MULTIPLE_DATA_BLOCKS_IN_GROUP_SUCCESS)
            return SEARCH_MULTIPLE_DATA_BLOCKS_SUCCESS;
        else if(searchMultipleBlocksResult == SEARCH_MULTIPLE_DATA_BLOCKS_IN_GROUP_FAILED_FOR_OTHER_REASON)
            return SEARCH_MULTIPLE_DATA_BLOCKS_FAILED_FOR_OTHER_REASON;
    }

    return SEARCH_MULTIPLE_DATA_BLOCKS_NO_ENOUGH_CONSECUTIVE_FREE_BLOCKS;
}

uint32_t searchAndOccupyMultipleBlocksInGivenGroup(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t group, uint32_t numOfBlocks, std::vector<uint32_t>& newBlocks)
{
    uint32_t firstDataBlockForGivenGroup = getFirstDataBlockForGivenGroup(superBlock, group);
    uint32_t numOfSectorsPerBlock = getNumberOfSectorsPerBlock(diskInfo, superBlock);
    uint32_t dataBitmapBlock = getDataBitmapBlockForGivenGroup(superBlock, group);

    char* blockBuffer = new char[superBlock->s_log_block_size];
    uint32_t numberOfSectorsRead = 0;
    uint32_t readResult = readDiskSectors(diskInfo, numOfSectorsPerBlock, getFirstSectorForGivenBlock(diskInfo, superBlock, dataBitmapBlock),
                                          blockBuffer, numberOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        delete[] blockBuffer;
        return SEARCH_MULTIPLE_DATA_BLOCKS_IN_GROUP_FAILED_FOR_OTHER_REASON;
    }

    uint32_t bitIndex; //we declare it here to be able to jump to bitIndex for the next startingBitIndex (for efficiency to not increase by 1)
    for(uint32_t startingBitIndex = 0; startingBitIndex + numOfBlocks < getNumberOfDataBlocksForGivenGroup(superBlock, group); startingBitIndex = bitIndex + 1)
    {
        bool allBitsZero = true;
        for(bitIndex = startingBitIndex; bitIndex < startingBitIndex + numOfBlocks; bitIndex++)
            if(getBitFromByte(blockBuffer[bitIndex / 8], bitIndex % 8) == 1)
            {
                allBitsZero = false;
                break;
            }

        if(allBitsZero == false)
            continue;

        //if there are some 1 bits, it means we found enough consecutive free blocks
        for(bitIndex = startingBitIndex; bitIndex < startingBitIndex + numOfBlocks; bitIndex++)
            newBlocks.push_back(firstDataBlockForGivenGroup + bitIndex);

        delete[] blockBuffer;

        return SEARCH_MULTIPLE_DATA_BLOCKS_IN_GROUP_SUCCESS;
    }

    delete[] blockBuffer;
    return SEARCH_MULTIPLE_DATA_BLOCKS_IN_GROUP_NO_ENOUGH_CONSECUTIVE_FREE_BLOCKS;
}

uint32_t searchInodeByFullPath(DiskInfo* diskInfo, ext2_super_block* superBlock, char* directoryPath, ext2_inode** inode, bool& isSearchedInodeRoot)
{
    char* actualDirectoryName = strtok(directoryPath, "/");
    if(strcmp(actualDirectoryName, "Root\0") != 0)
        return SEARCH_INODE_BY_FULL_PATH_PARENT_DO_NOT_EXIST;

    *inode = nullptr;
    ext2_inode* searchedInode = new ext2_inode();

    while(actualDirectoryName != nullptr)
    {
        uint32_t searchedInodeResult = searchInodeByDirectoryNameInParent(diskInfo, superBlock, *inode,
                                                                          actualDirectoryName, searchedInode, isSearchedInodeRoot);

        if(searchedInodeResult == SEARCH_INODE_BY_DIRECTORY_NAME_IN_PARENT_SUCCESS)
            *inode = searchedInode;
        else
        {
            delete searchedInode;

            if(searchedInodeResult == SEARCH_INODE_BY_DIRECTORY_NAME_IN_PARENT_INODE_DO_NOT_EXIST)
                return SEARCH_INODE_BY_FULL_PATH_PARENT_DO_NOT_EXIST;

            return SEARCH_INODE_BY_FULL_PATH_FAILED;
        }

        actualDirectoryName = strtok(nullptr, "/");
    }

    return SEARCH_INODE_BY_FULL_PATH_SUCCESS;
}

uint32_t searchInodeByDirectoryNameInParent(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* parentInode, char* searchedDirectoryName, ext2_inode* searchedInode,
                                            bool& isSearchedInodeRoot)
{
    char* blockBuffer = new char[superBlock->s_log_block_size];
    uint32_t numberOfSectorsRead, directoryEntryOffsetInBlock, blockGlobalIndex;

    if(parentInode == nullptr) //it means that searched inode is Root
    {
        uint32_t rootInodeBlock = getFirstInodeTableBlockForGivenGroup(superBlock, 0);

        uint32_t readResult = readDiskSectors(diskInfo, getNumberOfSectorsPerBlock(diskInfo, superBlock),
                                              getFirstSectorForGivenBlock(diskInfo, superBlock, rootInodeBlock), blockBuffer, numberOfSectorsRead);

        if(readResult != EC_NO_ERROR)
        {
            delete[] blockBuffer;
            return SEARCH_INODE_BY_DIRECTORY_NAME_IN_PARENT_FAILED;
        }

        memcpy(searchedInode, blockBuffer, sizeof(ext2_inode));
        isSearchedInodeRoot = true;
        delete[] blockBuffer;
        return SEARCH_INODE_BY_DIRECTORY_NAME_IN_PARENT_SUCCESS;
    }

    for(uint32_t blockLocalIndex = 0; blockLocalIndex < parentInode->i_blocks; blockLocalIndex++)
    {
        uint32_t occupiedBytesInBlock = parentInode->i_size >= superBlock->s_log_block_size * (blockLocalIndex + 1) ? superBlock->s_log_block_size :
                ((parentInode->i_size >= superBlock->s_log_block_size * blockLocalIndex) ? parentInode->i_size % superBlock->s_log_block_size : 0);

        if(occupiedBytesInBlock == 0) //it means that we haven't found the inode, and there isn't any other place to find it
        {
            delete[] blockBuffer;
            return SEARCH_INODE_BY_DIRECTORY_NAME_IN_PARENT_INODE_DO_NOT_EXIST;
        }

        uint32_t getBlockGlobalIndexResult = getDataBlockGlobalIndexByLocalIndexInsideInode(diskInfo, superBlock,
                                                                                            parentInode,
                                                                                            blockLocalIndex,
                                                                                            blockGlobalIndex);
        if(getBlockGlobalIndexResult != GET_DATA_BLOCK_BY_LOCAL_INDEX_SUCCESS)
        {
            delete[] blockBuffer;
            return SEARCH_INODE_BY_DIRECTORY_NAME_IN_PARENT_FAILED;
        }

        uint32_t readResult = readDiskSectors(diskInfo, getNumberOfSectorsPerBlock(diskInfo, superBlock),
                                              getFirstSectorForGivenBlock(diskInfo, superBlock, blockGlobalIndex), blockBuffer, numberOfSectorsRead);

        if(readResult != EC_NO_ERROR)
        {
            delete[] blockBuffer;
            return SEARCH_INODE_BY_DIRECTORY_NAME_IN_PARENT_FAILED;
        }

        ext2_dir_entry* directoryEntry = new ext2_dir_entry();
        uint32_t searchDirectoryInBlockDataResult = searchDirectoryWithGivenNameInGivenBlockData(searchedDirectoryName, blockBuffer, occupiedBytesInBlock,
                                                                                                 directoryEntry, directoryEntryOffsetInBlock);

        if(searchDirectoryInBlockDataResult == SEARCH_DIRECTORY_ENTRY_BY_NAME_IN_GIVEN_BLOCK_DATA_FOUND)
        {
            uint32_t inodeBlockGlobalIndex, inodeOffsetInsideBlock;
            uint32_t getInodeByIndexResult = getInodeByInodeGlobalIndex(diskInfo, superBlock, directoryEntry->inode, searchedInode, inodeBlockGlobalIndex,
                                                                        inodeOffsetInsideBlock);
            delete directoryEntry;
            return (getInodeByIndexResult == GET_INODE_BY_INODE_GLOBAL_INDEX_SUCCESS) ? SEARCH_INODE_BY_DIRECTORY_NAME_IN_PARENT_SUCCESS : SEARCH_INODE_BY_DIRECTORY_NAME_IN_PARENT_FAILED;
        }
    }

    delete[] blockBuffer;
    return SEARCH_INODE_BY_DIRECTORY_NAME_IN_PARENT_INODE_DO_NOT_EXIST;
}

uint32_t searchDirectoryWithGivenNameInGivenBlockData(char* searchedName, char* blockBuffer, uint32_t occupiedBytesInBlock, ext2_dir_entry* searchedDirectoryEntry, uint32_t& directoryEntryOffsetInBlock)
{
    uint32_t offset = 0;
    while(true)
    {
        ext2_dir_entry* directoryEntry = new ext2_dir_entry();
        memcpy(directoryEntry, blockBuffer + offset, sizeof(ext2_dir_entry));

        if(strcmp(searchedName, directoryEntry->name) == 0)
        {
            memcpy(searchedDirectoryEntry, directoryEntry, sizeof(ext2_dir_entry));
            directoryEntryOffsetInBlock = offset;
            delete directoryEntry;
            return SEARCH_DIRECTORY_ENTRY_BY_NAME_IN_GIVEN_BLOCK_DATA_FOUND;
        }

        offset += sizeof(ext2_dir_entry);
        if(offset >= occupiedBytesInBlock)
        {
            delete directoryEntry;
            return SEARCH_DIRECTORY_ENTRY_BY_NAME_IN_GIVEN_BLOCK_DATA_NOT_FOUND;
        }
    }
}

uint32_t writeBytesToFileWithTruncate(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* inode, char* dataBuffer, uint32_t maxBytesToWrite,
                                    uint32_t& numberOfBytesWritten, uint32_t& reasonForIncompleteWrite)
{
    numberOfBytesWritten = 0;
    uint32_t numOfSectorsPerBlock = getNumberOfSectorsPerBlock(diskInfo, superBlock);
    uint32_t blockGlobalIndex, newBlockGlobalIndex, numOfSectorsWritten;
    char* blockBuffer = new char[superBlock->s_log_block_size];

    //in case the number of free space is not enough, add new blocks
    uint32_t totalSpaceInDirectory = superBlock->s_log_block_size * inode->i_blocks;
    uint32_t numberOfBlocksToAddToAddToDirectory = 0;
    if(maxBytesToWrite > totalSpaceInDirectory)
        numberOfBlocksToAddToAddToDirectory = (maxBytesToWrite - totalSpaceInDirectory) / superBlock->s_log_block_size + 1;
    if((maxBytesToWrite - totalSpaceInDirectory) % superBlock->s_log_block_size == 0)
        numberOfBlocksToAddToAddToDirectory--;

    //CAUTION we don't return if the add block fails, so even if it fails, the method will continue, and will add bytes only to the free space available + the blocks added successfully
    for(uint32_t i = 1; i <= numberOfBlocksToAddToAddToDirectory; i++)
    {
        ext2_inode* updatedInode = new ext2_inode();
        memcpy(updatedInode, inode, sizeof(ext2_inode));
        uint32_t addBlockToDirectoryResult = allocateBlockToDirectory(diskInfo, superBlock, inode, newBlockGlobalIndex, updatedInode); //we also update the inode if blocks added
        if(addBlockToDirectoryResult != ADD_BLOCK_TO_DIRECTORY_SUCCESS)
        {
            reasonForIncompleteWrite = INCOMPLETE_BYTES_WRITE_DUE_TO_UNABLE_TO_ADD_NEW_BLOCKS_TO_DIRECTORY;
            break;
        }
        memcpy(inode, updatedInode, sizeof(ext2_inode));
    }

    for(uint32_t blockLocalIndex = 0; blockLocalIndex < inode->i_blocks; blockLocalIndex++)
    {
        uint32_t numOfBytesToWriteToActualBlock = std::min(superBlock->s_log_block_size, maxBytesToWrite - numberOfBytesWritten);

        if(numOfBytesToWriteToActualBlock == 0)
        {
            delete[] blockBuffer;
            return WRITE_BYTES_TO_FILE_SUCCESS;
        }

        uint32_t getBlockGlobalIndexResult = getDataBlockGlobalIndexByLocalIndexInsideInode(diskInfo, superBlock, inode,
                                                                                            blockLocalIndex,
                                                                                            blockGlobalIndex);
        if(getBlockGlobalIndexResult != GET_DATA_BLOCK_BY_LOCAL_INDEX_SUCCESS)
        {
            reasonForIncompleteWrite = INCOMPLETE_BYTES_WRITE_DUE_TO_OTHER;
            delete[] blockBuffer;
            return (numberOfBytesWritten == 0) ? WRITE_BYTES_TO_FILE_FAILED_FOR_OTHER_REASON : WRITE_BYTES_TO_FILE_SUCCESS;
        }

        memcpy(blockBuffer, dataBuffer + numberOfBytesWritten, numOfBytesToWriteToActualBlock);

        uint32_t writeResult = writeDiskSectors(diskInfo , numOfSectorsPerBlock, getFirstSectorForGivenBlock(diskInfo, superBlock, blockGlobalIndex),
                                                blockBuffer, numOfSectorsWritten);

        if(writeResult != EC_NO_ERROR)
        {
            reasonForIncompleteWrite = INCOMPLETE_BYTES_WRITE_DUE_TO_OTHER;
            delete[] blockBuffer;
            return (numberOfBytesWritten == 0) ? WRITE_BYTES_TO_FILE_FAILED_FOR_OTHER_REASON : WRITE_BYTES_TO_FILE_SUCCESS;
        }

        numberOfBytesWritten += numOfBytesToWriteToActualBlock;
        if(numberOfBytesWritten == maxBytesToWrite)
        {
            delete[] blockBuffer;
            return WRITE_BYTES_TO_FILE_SUCCESS;
        }
    }
}

uint32_t writeBytesToFileWithAppend(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* inode, char* dataBuffer, uint32_t maxBytesToWrite,
                                      uint32_t& numberOfBytesWritten, uint32_t& reasonForIncompleteWrite)
{
    numberOfBytesWritten = 0;
    uint32_t numOfSectorsPerBlock = getNumberOfSectorsPerBlock(diskInfo, superBlock);
    uint32_t blockGlobalIndex, newBlockGlobalIndex, numberOfSectorsRead, numOfSectorsWritten;
    char* blockBuffer = new char[superBlock->s_log_block_size];

    //in case the number of free space is not enough, add new blocks
    uint32_t totalSpaceInDirectory = superBlock->s_log_block_size * inode->i_blocks; //total space in blocks of the directory
    uint32_t freeSpaceInDirectory = totalSpaceInDirectory - inode->i_size;
    uint32_t numberOfBlocksToAddToAddToDirectory = 0;
    if(maxBytesToWrite > freeSpaceInDirectory)
        numberOfBlocksToAddToAddToDirectory = (maxBytesToWrite - freeSpaceInDirectory) / superBlock->s_log_block_size + 1;
    if((maxBytesToWrite - freeSpaceInDirectory) % superBlock->s_log_block_size == 0)
        numberOfBlocksToAddToAddToDirectory--;

    //CAUTION we don't return if the add block fails, so even if it fails, the method will continue, and will add bytes only to the free space available + the blocks added successfully
    for(uint32_t i = 1; i <= numberOfBlocksToAddToAddToDirectory; i++)
    {
        ext2_inode* updatedInode = new ext2_inode();
        memcpy(updatedInode, inode, sizeof(ext2_inode));
        uint32_t addBlockToDirectoryResult = allocateBlockToDirectory(diskInfo, superBlock, inode, newBlockGlobalIndex, updatedInode); //we also update the inode if blocks added
        if(addBlockToDirectoryResult != ADD_BLOCK_TO_DIRECTORY_SUCCESS)
        {
            reasonForIncompleteWrite = INCOMPLETE_BYTES_WRITE_DUE_TO_UNABLE_TO_ADD_NEW_BLOCKS_TO_DIRECTORY;
            break;
        }
        memcpy(inode, updatedInode, sizeof(ext2_inode));
    }

    //now we write the bytes to blocks CAUTION first block to write in might already contain bytes, so we won't add bytes from index 0
    uint32_t firstBlockInDirectoryWithFreeSpaceLocalIndex = inode->i_size / superBlock->s_log_block_size;

    for(uint32_t blockLocalIndex = firstBlockInDirectoryWithFreeSpaceLocalIndex; blockLocalIndex < inode->i_blocks; blockLocalIndex++)
    {
        uint32_t occupiedBytesInBlock = inode->i_size >= superBlock->s_log_block_size * (blockLocalIndex + 1) ? superBlock->s_log_block_size :
                ((inode->i_size >= superBlock->s_log_block_size * blockLocalIndex) ? inode->i_size % superBlock->s_log_block_size : 0);

        uint32_t numOfBytesToWriteToActualBlock = std::min(superBlock->s_log_block_size - occupiedBytesInBlock, maxBytesToWrite - numberOfBytesWritten);

        if(numOfBytesToWriteToActualBlock == 0)
        {
            delete[] blockBuffer;
            return WRITE_BYTES_TO_FILE_SUCCESS;
        }

        uint32_t getBlockGlobalIndexResult = getDataBlockGlobalIndexByLocalIndexInsideInode(diskInfo, superBlock, inode,
                                                                                            blockLocalIndex,
                                                                                            blockGlobalIndex);
        if(getBlockGlobalIndexResult != GET_DATA_BLOCK_BY_LOCAL_INDEX_SUCCESS)
        {
            reasonForIncompleteWrite = INCOMPLETE_BYTES_WRITE_DUE_TO_OTHER;
            delete[] blockBuffer;
            return (numberOfBytesWritten == 0) ? WRITE_BYTES_TO_FILE_FAILED_FOR_OTHER_REASON : WRITE_BYTES_TO_FILE_SUCCESS;
        }

        uint32_t readResult = readDiskSectors(diskInfo, numOfSectorsPerBlock,getFirstSectorForGivenBlock(diskInfo, superBlock, blockGlobalIndex),
                                              blockBuffer, numberOfSectorsRead);

        if(readResult != EC_NO_ERROR)
        {
                reasonForIncompleteWrite = INCOMPLETE_BYTES_WRITE_DUE_TO_OTHER;
                delete[] blockBuffer;
                return (numberOfBytesWritten == 0) ? WRITE_BYTES_TO_FILE_FAILED_FOR_OTHER_REASON : WRITE_BYTES_TO_FILE_SUCCESS;
        }

        memcpy(blockBuffer + occupiedBytesInBlock, dataBuffer + numberOfBytesWritten, numOfBytesToWriteToActualBlock);

        uint32_t writeResult = writeDiskSectors(diskInfo , numOfSectorsPerBlock, getFirstSectorForGivenBlock(diskInfo, superBlock, blockGlobalIndex),
                                                blockBuffer, numOfSectorsWritten);

        if(writeResult != EC_NO_ERROR)
        {
            reasonForIncompleteWrite = INCOMPLETE_BYTES_WRITE_DUE_TO_OTHER;
            delete[] blockBuffer;
            return (numberOfBytesWritten == 0) ? WRITE_BYTES_TO_FILE_FAILED_FOR_OTHER_REASON : WRITE_BYTES_TO_FILE_SUCCESS;
        }

        numberOfBytesWritten += numOfBytesToWriteToActualBlock;
        if(numberOfBytesWritten == maxBytesToWrite)
        {
            delete[] blockBuffer;
            return WRITE_BYTES_TO_FILE_SUCCESS;
        }
    }
}