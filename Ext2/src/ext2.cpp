#include "windows.h"
#include "string.h"
#include "vector"

#include "../include/disk.h"
#include "../include/diskCodes.h"
#include "../include/structures.h"
#include "../include/ext2FunctionUtils.h"
#include "../include/ext2Attributes.h"
#include "../include/codes/ext2Codes.h"
#include "../include/ext2.h"

uint32_t addInodeToGroup(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* inode, uint32_t group, uint32_t fileType)
{
    ext2_group_desc* groupDescriptor = new ext2_group_desc();
    uint32_t getGroupDescriptorResult = getGroupDescriptorOfGivenGroup(diskInfo, superBlock, group, groupDescriptor);

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

        if(searchFreeDataBlockResult == SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_FAILED)
            return ADD_INODE_TO_GROUP_FAILED_FOR_OTHER_REASON;

        dataBlocks.push_back(firstFreeDataBlock);
    }

    uint32_t blockWithFirstFreeInode;
    uint32_t offsetOfFirstFreeInode;
    uint32_t searchEmptyInodeInGroupResult = searchEmptyInodeInGroup(diskInfo, superBlock, group, blockWithFirstFreeInode, offsetOfFirstFreeInode);

    if(searchEmptyInodeInGroupResult != SEARCH_EMPTY_INODE_IN_GROUP_SUCCESS)
        return ADD_INODE_TO_GROUP_FAILED_FOR_OTHER_REASON;

    ext2_inode* newInode = new ext2_inode();
    createNewInode(superBlock, newInode, fileType, dataBlocks);

    char* blockBuffer = new char[superBlock->s_log_block_size];
    uint32_t numberOfSectorsRead = 0;
    uint32_t readResult = readDiskSectors(diskInfo, getNumberOfSectorsPerBlock(diskInfo, superBlock),
                                          getFirstSectorForGivenBlock(diskInfo, superBlock, blockWithFirstFreeInode), blockBuffer, numberOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        delete[] blockBuffer;
        return ADD_INODE_TO_GROUP_FAILED_FOR_OTHER_REASON;
    }

    memcpy(blockBuffer + offsetOfFirstFreeInode, newInode, sizeof(ext2_inode));
    uint32_t numOfSectorsWritten;
    uint32_t writeResult = writeDiskSectors(diskInfo , getNumberOfSectorsPerBlock(diskInfo, superBlock),
                                            getFirstSectorForGivenBlock(diskInfo, superBlock, blockWithFirstFreeInode), blockBuffer, numOfSectorsWritten);

    delete[] blockBuffer;
    if(writeResult != EC_NO_ERROR)
        return ADD_INODE_TO_GROUP_FAILED_FOR_OTHER_REASON;

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
        return SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_FAILED;
    }

    uint32_t firstFreeDataBlockInGroup = 99999;
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
                SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_FAILED;

            break;
        }

    firstFreeDataBlock = getFirstDataBlockForGivenGroup(superBlock, group) + firstFreeDataBlockInGroup;

    return SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_SUCCESS;
}

uint32_t searchEmptyInodeInGroup(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t group, uint32_t& blockWithFirstFreeInode, uint32_t& offsetOfFirstFreeInode)
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
        return SEARCH_EMPTY_INODE_IN_GROUP_OTHER_ERROR;
    }

    uint32_t firstFreeInodeInGroup = 99999;
    for(uint32_t bitIndex = 0; bitIndex < getNumberOfInodesForGivenGroup(superBlock, group); bitIndex++)
        if(getBitFromByte(blockBuffer[bitIndex / 8], bitIndex % 8) == 0)
        {
            firstFreeInodeInGroup = bitIndex;
            break;
        }

    blockWithFirstFreeInode = getInodeBlockForInodeIndexInGroup(superBlock, group, firstFreeInodeInGroup);
    offsetOfFirstFreeInode = (firstFreeInodeInGroup * 128) % superBlock->s_log_block_size;

    delete[] blockBuffer;
    return SEARCH_EMPTY_INODE_IN_GROUP_SUCCESS;
}

uint32_t createNewInode(ext2_super_block* superBlock, ext2_inode* newInode, uint32_t fileType, std::vector<uint32_t>& blocks)
{
    SYSTEMTIME time;
    GetSystemTime(&time);

    memset(newInode, 0, 128);

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

//    if(parentDirectoryInode == nullptr) //it means this we are creating the newInode for root, which doesn't have a parent
//    {
//        uint32_t firstGroupBlock = getFirstDataBlockForGivenGroup(superBlock, 0);
//        for(uint32_t preallocBlock = firstGroupBlock, index = 0; preallocBlock < firstGroupBlock + superBlock->s_prealloc_dir_blocks; preallocBlock++, index++)
//            newInode->i_block[index] = preallocBlock;
//    }

    newInode->i_file_acl = 0;
    newInode->i_dir_acl = 0;
    newInode->i_faddr = 0;
}