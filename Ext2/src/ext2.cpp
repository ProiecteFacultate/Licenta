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
#include "../include/ext2.h"

#define BIG_VALUE 99999999

uint32_t createInode(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* parentInode, ext2_inode* createdInode, uint32_t fileType, bool& isParentRoot)
{
    uint32_t searchInodeResult, inodeGlobalIndex;

    if(fileType == FILE_TYPE_DIRECTORY)
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

    for(uint32_t startingBitIndex = 0; startingBitIndex + numOfBlocks < getNumberOfDataBlocksForGivenGroup(superBlock, group); startingBitIndex += numOfBlocks)
    {
        for(uint32_t bitIndex = startingBitIndex; bitIndex < startingBitIndex + numOfBlocks; bitIndex++)
            if(getBitFromByte(blockBuffer[bitIndex / 8], bitIndex % 8) == 1)
                continue;

        //if not continue, it means we found enough consecutive free blocks
        for(uint32_t bitIndex = startingBitIndex; bitIndex < bitIndex + numOfBlocks; bitIndex++)
            newBlocks.push_back(firstDataBlockForGivenGroup + bitIndex);

        delete[] blockBuffer;

        return SEARCH_MULTIPLE_DATA_BLOCKS_IN_GROUP_SUCCESS;
    }

    delete[] blockBuffer;
    return SEARCH_MULTIPLE_DATA_BLOCKS_IN_GROUP_NO_ENOUGH_CONSECUTIVE_FREE_BLOCKS;
}

uint32_t searchAndOccupyFreeDataBlock(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t preferredGroup, uint32_t previousBlockGlobalIndex, uint32_t& newDataBlock)
{
    uint32_t previousBlockGroup = previousBlockGlobalIndex / superBlock->s_blocks_per_group;
    uint32_t previousBlockLocalIndex = previousBlockGlobalIndex % superBlock->s_blocks_per_group;

    uint32_t searchFreeBlockResult = searchAndOccupyFreeDataBlockInGivenGroup(diskInfo, superBlock, previousBlockGroup, previousBlockLocalIndex, newDataBlock);

    if(searchFreeBlockResult == SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_SUCCESS)
        return SEARCH_EMPTY_DATA_BLOCK_SUCCESS;
    else if(searchFreeBlockResult == SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_FAILED_FOR_OTHER_REASON)
        return SEARCH_EMPTY_DATA_BLOCK_FAILED_FOR_OTHER_REASON;

    for(uint32_t group = 0; group < getNumberOfGroups(superBlock); group++)
    {
        searchFreeBlockResult = searchAndOccupyFreeDataBlockInGivenGroup(diskInfo, superBlock, group, BIG_VALUE, newDataBlock);
        if(searchFreeBlockResult == SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_SUCCESS)
            return SEARCH_EMPTY_DATA_BLOCK_SUCCESS;
        else if(searchFreeBlockResult == SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_FAILED_FOR_OTHER_REASON)
            return SEARCH_EMPTY_DATA_BLOCK_FAILED_FOR_OTHER_REASON;
    }

    return SEARCH_EMPTY_DATA_BLOCK_NO_FREE_BLOCKS;
}

uint32_t searchAndOccupyFreeDataBlockInGivenGroup(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t group, uint32_t previousBlockLocalIndex, uint32_t& newDataBlock)
{
    uint32_t numOfSectorsPerBlock = getNumberOfSectorsPerBlock(diskInfo, superBlock);
    uint32_t dataBitmapBlock = getDataBitmapBlockForGivenGroup(superBlock, group);

    char* blockBuffer = new char[superBlock->s_log_block_size];
    uint32_t numberOfSectorsRead = 0;
    uint32_t readResult = readDiskSectors(diskInfo, numOfSectorsPerBlock, getFirstSectorForGivenBlock(diskInfo, superBlock, dataBitmapBlock),
                                          blockBuffer, numberOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        delete[] blockBuffer;
        return SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_FAILED_FOR_OTHER_REASON;
    }

    //checks the block after the given previousBlock
    uint32_t firstFreeDataBlockInGroupAfterGivenPrevious;
    previousBlockLocalIndex++;
    if(previousBlockLocalIndex != BIG_VALUE && getBitFromByte(blockBuffer[previousBlockLocalIndex / 8], previousBlockLocalIndex % 8) == 0)
    {
        firstFreeDataBlockInGroupAfterGivenPrevious = previousBlockLocalIndex;
        newDataBlock = getFirstDataBlockForGivenGroup(superBlock, group) + firstFreeDataBlockInGroupAfterGivenPrevious;
        delete[] blockBuffer;

        return SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_SUCCESS;
    }


    for(uint32_t bitIndex = 0; bitIndex < getNumberOfDataBlocksForGivenGroup(superBlock, group); bitIndex++)
        if(getBitFromByte(blockBuffer[bitIndex / 8], bitIndex % 8) == 0)
        {
            firstFreeDataBlockInGroupAfterGivenPrevious = bitIndex;
            uint8_t newByteValue = changeBitValue(blockBuffer[bitIndex / 8], bitIndex % 8, 1);
            memset(blockBuffer + bitIndex / 8, newByteValue, 1);

            uint32_t numOfSectorsWritten;
            uint32_t writeResult = writeDiskSectors(diskInfo , numOfSectorsPerBlock, getFirstSectorForGivenBlock(diskInfo, superBlock, dataBitmapBlock),
                                                    blockBuffer, numOfSectorsWritten);

            newDataBlock = getFirstDataBlockForGivenGroup(superBlock, group) + firstFreeDataBlockInGroupAfterGivenPrevious;
            delete[] blockBuffer;

            return (writeResult != EC_NO_ERROR) ? SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_FAILED_FOR_OTHER_REASON : SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_SUCCESS;
        }

    return SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_NO_FREE_BLOCKS;
}





/////////////////////////////////////
////////////////////////////////////////






uint32_t addInodeToGroup(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t group, uint32_t fileType)
{
    ext2_group_desc* groupDescriptor = new ext2_group_desc();
    uint32_t groupDescriptorBlock;
    uint32_t groupDescriptorOffsetInsideBlock;
    uint32_t getGroupDescriptorResult = getGroupDescriptorOfGivenGroup(diskInfo, superBlock, group, groupDescriptor, groupDescriptorBlock,
                                                                       groupDescriptorOffsetInsideBlock);

    if(getGroupDescriptorResult == GET_GROUP_DESCRIPTOR_FOR_GIVEN_GROUP_FAILED)
        return ADD_INODE_TO_GROUP_FAILED_FOR_OTHER_REASON;

    if(groupDescriptor->bg_free_inodes_count < 1)
        return ADD_INODE_TO_GROUP_NO_FREE_INODES_IN_GROUP;

    uint32_t preallocNumberOfBlocks;
    if(fileType == FILE_TYPE_DIRECTORY)
        preallocNumberOfBlocks = superBlock->s_prealloc_dir_blocks;
    else if(fileType == FILE_TYPE_REGULAR_FILE)
        preallocNumberOfBlocks = superBlock->s_prealloc_blocks;

    if(groupDescriptor->bg_free_blocks_count < preallocNumberOfBlocks)
        return ADD_INODE_TO_GROUP_NO_ENOUGH_FREE_DATA_BLOCKS_IN_GROUP;

    std::vector<uint32_t> dataBlocks;

    for(uint32_t preallocBlockIndex = 0; preallocBlockIndex < preallocNumberOfBlocks; preallocBlockIndex++)
    {
        uint32_t firstFreeDataBlock;
        uint32_t searchFreeDataBlockResult = searchAndOccupyFirstFreeDataBlockInGroup(diskInfo, superBlock, group, firstFreeDataBlock);

        if(searchFreeDataBlockResult == SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_FAILED_FOR_OTHER_REASON)
            return ADD_INODE_TO_GROUP_FAILED_FOR_OTHER_REASON;

        dataBlocks.push_back(firstFreeDataBlock);
    }

    uint32_t blockWithFirstFreeInodeInInodeTable;
    uint32_t offsetOfFirstFreeInode;
    uint32_t searchEmptyInodeInGroupResult = searchAndOccupyEmptyInodeInGroup(diskInfo, superBlock, group,
                                                                              blockWithFirstFreeInodeInInodeTable,
                                                                              offsetOfFirstFreeInode);

    if(searchEmptyInodeInGroupResult != SEARCH_EMPTY_INODE_IN_GROUP_SUCCESS)
        return ADD_INODE_TO_GROUP_FAILED_FOR_OTHER_REASON;

    ext2_inode* newInode = new ext2_inode();
    createNewInode(superBlock, newInode, fileType, dataBlocks);

    char* blockBuffer = new char[superBlock->s_log_block_size];
    uint32_t numberOfSectorsRead = 0;
    uint32_t readResult = readDiskSectors(diskInfo, getNumberOfSectorsPerBlock(diskInfo, superBlock),
                                          getFirstSectorForGivenBlock(diskInfo, superBlock, blockWithFirstFreeInodeInInodeTable), blockBuffer, numberOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        delete[] blockBuffer;
        return ADD_INODE_TO_GROUP_FAILED_FOR_OTHER_REASON;
    }

    memcpy(blockBuffer + offsetOfFirstFreeInode, newInode, sizeof(ext2_inode));
    uint32_t numOfSectorsWritten;
    uint32_t writeResult = writeDiskSectors(diskInfo , getNumberOfSectorsPerBlock(diskInfo, superBlock),
                                            getFirstSectorForGivenBlock(diskInfo, superBlock, blockWithFirstFreeInodeInInodeTable), blockBuffer, numOfSectorsWritten);

    delete[] blockBuffer;
    if(writeResult != EC_NO_ERROR)
        return ADD_INODE_TO_GROUP_FAILED_FOR_OTHER_REASON;

    ext2_group_desc* newGroupDescriptor = new ext2_group_desc();
    memcpy(newGroupDescriptor, groupDescriptor, sizeof(ext2_group_desc));
    newGroupDescriptor->bg_free_inodes_count--;
    newGroupDescriptor->bg_free_blocks_count -= preallocNumberOfBlocks;
    //CAUTION we don't query the result for this, so we could get add inode success, but fail on updating the group descriptor
    updateMainGroupDescriptor(diskInfo, superBlock, newGroupDescriptor, groupDescriptorBlock, groupDescriptorOffsetInsideBlock);

    return ADD_INODE_TO_GROUP_SUCCESS;
}

uint32_t searchAndOccupyFirstFreeDataBlockInGroup(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t group, uint32_t& firstFreeDataBlock)
{
    uint32_t numOfSectorsPerBlock = getNumberOfSectorsPerBlock(diskInfo, superBlock);
    uint32_t dataBitmapBlock = getDataBitmapBlockForGivenGroup(superBlock, group);

    char* blockBuffer = new char[superBlock->s_log_block_size];
    uint32_t numberOfSectorsRead = 0;
    uint32_t readResult = readDiskSectors(diskInfo, numOfSectorsPerBlock, getFirstSectorForGivenBlock(diskInfo, superBlock, dataBitmapBlock),
                                          blockBuffer, numberOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        delete[] blockBuffer;
        return SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_FAILED_FOR_OTHER_REASON;
    }

    uint32_t firstFreeDataBlockInGroup = BIG_VALUE;
    for(uint32_t bitIndex = 0; bitIndex < getNumberOfDataBlocksForGivenGroup(superBlock, group); bitIndex++)
        if(getBitFromByte(blockBuffer[bitIndex / 8], bitIndex % 8) == 0)
        {
            firstFreeDataBlockInGroup = bitIndex;
            uint8_t newByteValue = changeBitValue(blockBuffer[bitIndex / 8], bitIndex % 8, 1);
            memset(blockBuffer + bitIndex / 8, newByteValue, 1);

            uint32_t numOfSectorsWritten;
            uint32_t writeResult = writeDiskSectors(diskInfo , numOfSectorsPerBlock, getFirstSectorForGivenBlock(diskInfo, superBlock, dataBitmapBlock),
                                                    blockBuffer, numOfSectorsWritten);

            delete[] blockBuffer;

            if(writeResult != EC_NO_ERROR)
                SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_FAILED_FOR_OTHER_REASON;

            break;
        }

    firstFreeDataBlock = getFirstDataBlockForGivenGroup(superBlock, group) + firstFreeDataBlockInGroup;

    return SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_SUCCESS;
}

uint32_t searchAndOccupyEmptyInodeInGroup(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t group, uint32_t& blockWithFirstFreeInodeInInodeTable, uint32_t& offsetOfFirstFreeInode)
{
    uint32_t numOfSectorsPerBlock = getNumberOfSectorsPerBlock(diskInfo, superBlock);
    uint32_t inodeBitmapBlock = getInodeBitmapBlockForGivenGroup(superBlock, group);

    char* blockBuffer = new char[superBlock->s_log_block_size];
    uint32_t numberOfSectorsRead = 0;
    uint32_t readResult = readDiskSectors(diskInfo, numOfSectorsPerBlock, getFirstSectorForGivenBlock(diskInfo, superBlock, inodeBitmapBlock),
                                          blockBuffer, numberOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        delete[] blockBuffer;
        return SEARCH_EMPTY_INODE_IN_GROUP_FAILED;
    }

    uint32_t firstFreeInodeInGroup = BIG_VALUE;
    for(uint32_t bitIndex = 0; bitIndex < getNumberOfInodesForGivenGroup(superBlock, group); bitIndex++)
        if(getBitFromByte(blockBuffer[bitIndex / 8], bitIndex % 8) == 0)
        {
            firstFreeInodeInGroup = bitIndex;
            uint8_t newByteValue = changeBitValue(blockBuffer[bitIndex / 8], bitIndex % 8, 1);
            memset(blockBuffer + bitIndex / 8, newByteValue, 1);

            uint32_t numOfSectorsWritten;
            uint32_t writeResult = writeDiskSectors(diskInfo , numOfSectorsPerBlock, getFirstSectorForGivenBlock(diskInfo, superBlock, inodeBitmapBlock),
                                                    blockBuffer, numOfSectorsWritten);

            delete[] blockBuffer;

            if(writeResult != EC_NO_ERROR)
                SEARCH_EMPTY_INODE_IN_GROUP_FAILED;

            break;
        }

    blockWithFirstFreeInodeInInodeTable = getFirstInodeTableBlockForGivenGroup(superBlock, group) + (firstFreeInodeInGroup * 128) / superBlock->s_log_block_size;
    if(firstFreeInodeInGroup != 0 && (firstFreeInodeInGroup * sizeof(ext2_inode)) % superBlock->s_log_block_size == 0)
        blockWithFirstFreeInodeInInodeTable--;

    offsetOfFirstFreeInode = (firstFreeInodeInGroup * sizeof(ext2_inode )) % superBlock->s_log_block_size;

    return SEARCH_EMPTY_INODE_IN_GROUP_SUCCESS;
}

void createNewInode(ext2_super_block* superBlock, ext2_inode* newInode, uint32_t fileType, std::vector<uint32_t>& blocks)
{
    SYSTEMTIME time;
    GetSystemTime(&time);

    memset(newInode, 0, sizeof(ext2_inode));

    newInode->i_mode = fileType;
    newInode->i_size = 0;
    //high 7 bits represent how many years since 1900, next 4 for month, next 5 for day and the low 16 represent the second in that day with a granularity of 2 (see in fat)
    newInode->i_atime = ((time.wYear - 1900) << 25) | (time.wMonth << 21) | (time.wDay << 16) | ((time.wHour * 3600 + time.wMinute * 60 + time.wSecond) / 2);
    newInode->i_ctime = newInode->i_atime;
    newInode->i_mtime = newInode->i_atime;

    if(fileType == FILE_TYPE_DIRECTORY)
        newInode->i_blocks = superBlock->s_prealloc_dir_blocks;
    else if(fileType == FILE_TYPE_REGULAR_FILE)
        newInode->i_blocks = superBlock->s_prealloc_blocks;

    for(uint32_t blockIndex = 0; blockIndex < newInode->i_blocks; blockIndex++)
        newInode->i_block[blockIndex] = blocks[blockIndex];

    newInode->i_file_acl = 0;
    newInode->i_dir_acl = 0;
    newInode->i_faddr = 0;
}

uint32_t updateMainGroupDescriptor(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_group_desc* newGroupDescriptor, uint32_t groupDescriptorBlock, uint32_t groupDescriptorOffset)
{
    char* blockBuffer = new char[superBlock->s_log_block_size];
    uint32_t numberOfSectorsRead = 0;
    uint32_t readResult = readDiskSectors(diskInfo, getNumberOfSectorsPerBlock(diskInfo, superBlock),
                                          getFirstSectorForGivenBlock(diskInfo, superBlock, groupDescriptorBlock), blockBuffer, numberOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        delete[] blockBuffer;
        return UPDATE_MAIN_GROUP_DESCRIPTOR_FAILED;
    }

    memcpy(blockBuffer + groupDescriptorOffset, newGroupDescriptor, sizeof(ext2_group_desc));
    uint32_t numOfSectorsWritten;
    uint32_t writeResult = writeDiskSectors(diskInfo , getNumberOfSectorsPerBlock(diskInfo, superBlock),
                                            getFirstSectorForGivenBlock(diskInfo, superBlock, groupDescriptorBlock), blockBuffer, numOfSectorsWritten);

    delete[] blockBuffer;

    return (writeResult == EC_NO_ERROR) ? UPDATE_MAIN_GROUP_DESCRIPTOR_SUCCESS : UPDATE_MAIN_GROUP_DESCRIPTOR_FAILED;
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
    uint32_t numberOfSectorsRead;
    uint32_t blockGlobalIndex;
    uint32_t searchedInodeGlobalIndex;

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
                parentInode->i_size % superBlock->s_log_block_size;

        if(occupiedBytesInBlock == 0) //it means that we haven't found the inode, and there isn't any other place to find it
        {
            delete[] blockBuffer;
            return SEARCH_INODE_BY_DIRECTORY_NAME_IN_PARENT_INODE_DO_NOT_EXIST;
        }

        uint32_t getBlockGlobalIndexResult = getDataBlockGlobalIndexByLocalIndex(diskInfo, superBlock, parentInode, blockLocalIndex, blockGlobalIndex);
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

        uint32_t searchDirectoryInBlockDataResult = searchDirectoryWithGivenNameInGivenBlockData(searchedDirectoryName, blockBuffer, occupiedBytesInBlock, searchedInodeGlobalIndex);

        if(searchDirectoryInBlockDataResult == SEARCH_DIRECTORY_ENTRY_BY_NAME_IN_GIVEN_BLOCK_DATA_FOUND)
        {
            uint32_t getInodeByIndexResult = getInodeByInodeGlobalIndex(diskInfo, superBlock, searchedInodeGlobalIndex, searchedInode);
            return (getInodeByIndexResult == GET_INODE_BY_INODE_GLOBAL_INDEX_SUCCESS) ? SEARCH_INODE_BY_DIRECTORY_NAME_IN_PARENT_SUCCESS : SEARCH_INODE_BY_DIRECTORY_NAME_IN_PARENT_FAILED;
        }
    }

    delete[] blockBuffer;
    return SEARCH_INODE_BY_DIRECTORY_NAME_IN_PARENT_INODE_DO_NOT_EXIST;
}

uint32_t searchDirectoryWithGivenNameInGivenBlockData(char* searchedName, char* blockBuffer, uint32_t occupiedBytesInBlock, uint32_t& searchedInodeGlobalIndex)
{
    uint32_t offset = 0;
    while(true)
    {
        uint32_t directoryEntryLength = *(uint16_t*)&blockBuffer[offset + 4];
        ext2_dir_entry* directoryEntry = new ext2_dir_entry();
        memcpy(directoryEntry, blockBuffer + offset, directoryEntryLength);

        if(strcmp(searchedName, directoryEntry->name) == 0)
        {
            searchedInodeGlobalIndex = directoryEntry->inode;
            delete directoryEntry;
            return SEARCH_DIRECTORY_ENTRY_BY_NAME_IN_GIVEN_BLOCK_DATA_FOUND;
        }

        offset += directoryEntryLength;
        if(offset >= occupiedBytesInBlock)
        {
            delete directoryEntry;
            return SEARCH_DIRECTORY_ENTRY_BY_NAME_IN_GIVEN_BLOCK_DATA_NOT_FOUND;
        }
    }
}