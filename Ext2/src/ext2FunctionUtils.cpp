#include "windows.h"
#include "string.h"
#include "cstdint"

#include "../include/disk.h"
#include "../include/diskCodes.h"
#include "../include/ext2Structures.h"
#include "../include/ext2.h"
#include "../include/codes/ext2Codes.h"
#include "../include/utils.h"
#include "../include/ext2BlocksAllocation.h"
#include "../include/codes/ext2BlocksAllocationCodes.h"
#include "../include/ext2FunctionUtils.h"

#define VALUE_ENTRY sizeof(uint32_t) //we define this as a macro to be easier to replace in testing

uint32_t getNumberOfGroups(ext2_super_block* superBlock)
{
    uint32_t numOfGroups = superBlock->s_block_count / superBlock->s_blocks_per_group + 1;
    if(superBlock->s_block_count % superBlock->s_blocks_per_group == 0)
        numOfGroups--;

    return numOfGroups;
}

uint32_t getNumberOfGroupDescriptorsBlocksInFullGroup(ext2_super_block* superBlock)
{
    uint32_t numOfGroupDescriptorsBlocks = (getNumberOfGroups(superBlock) * sizeof(ext2_group_desc)) / superBlock->s_log_block_size + 1;
    if(getNumberOfGroups(superBlock) * sizeof(ext2_group_desc) % superBlock->s_log_block_size == 0)
        numOfGroupDescriptorsBlocks--;

    return numOfGroupDescriptorsBlocks;
}

uint32_t getNumberOfInodesBlocksInFullGroup(ext2_super_block* superBlock)
{
    uint32_t numOfInodeTables = (superBlock->s_inodes_per_group * sizeof(ext2_inode)) / superBlock->s_log_block_size + 1;
    if(superBlock->s_inodes_per_group * sizeof(ext2_inode) % superBlock->s_log_block_size == 0)
        numOfInodeTables--;

    return numOfInodeTables;
}

uint32_t getNumberOfDataBlocksInFullGroup(ext2_super_block* superBlock)
{
    return superBlock->s_blocks_per_group - getNumberOfGroupDescriptorsBlocksInFullGroup(superBlock) - getNumberOfInodesBlocksInFullGroup(superBlock) - 3;
}

uint32_t getNumberOfInodesInFullGroup(ext2_super_block* superBlock)
{
    uint32_t inodesPerInodeTableBlock = superBlock->s_log_block_size / sizeof(ext2_inode);
    return getNumberOfInodesBlocksInFullGroup(superBlock) * inodesPerInodeTableBlock;
}

//////////////////////////

uint32_t getNumberOfBlocksForGivenGroup(ext2_super_block* superBlock, uint32_t group)
{
    if(group < getNumberOfGroups(superBlock) - 1)
        return superBlock->s_blocks_per_group;

    //this is the case for the last group which is most probably incomplete
    uint32_t totalNumberOfBlocksOccupiedByRestOfGroups = (getNumberOfGroups(superBlock) - 1) * superBlock->s_blocks_per_group;
    return superBlock->s_block_count - totalNumberOfBlocksOccupiedByRestOfGroups;
}

uint32_t getFirstBlockForGivenGroup(ext2_super_block* superBlock, uint32_t group)
{
    return superBlock->s_blocks_per_group * group; //first block of the first group is considered to be block index 0 globally, even if the book says from 1
}

uint32_t getNumberOfDataBlocksForGivenGroup(ext2_super_block* superBlock, uint32_t group)
{
    if(group < getNumberOfGroups(superBlock) - 1)
        return getNumberOfDataBlocksInFullGroup(superBlock);

    //this is the case for the last group which is most probably incomplete
    uint32_t numOfNonDataBlocksInFullBlock =getNumberOfGroupDescriptorsBlocksInFullGroup(superBlock) + getNumberOfInodesBlocksInFullGroup(superBlock) + 3;
    if(getNumberOfBlocksForGivenGroup(superBlock, group) <= numOfNonDataBlocksInFullBlock) //we don't use max instead because we have uint and will underflow
        return 0;

    return getNumberOfBlocksForGivenGroup(superBlock, group) - numOfNonDataBlocksInFullBlock;
}

uint32_t getFirstDataBlockForGivenGroup(ext2_super_block* superBlock, uint32_t group)
{
    if(getNumberOfDataBlocksForGivenGroup(superBlock, group) == 0) //it means this is the first group, and may be incomplete, and if 0 it means it has no data blocks
        return 0;

    uint32_t numOfNonDataBlocksInGivenGroup = superBlock->s_blocks_per_group - getNumberOfDataBlocksInFullGroup(superBlock);
    return getFirstBlockForGivenGroup(superBlock, group) + numOfNonDataBlocksInGivenGroup;
}

uint32_t getNumberOfGroupDescriptorsBlocksForGivenGroup(ext2_super_block* superBlock, uint32_t group)
{
    if(group < getNumberOfGroups(superBlock) - 1)
        return getNumberOfGroupDescriptorsBlocksInFullGroup(superBlock);

    //this is the case for the last group which is most probably incomplete
    if(getNumberOfBlocksForGivenGroup(superBlock, group) < 2) //it means that this group contains only the super block
        return 0;

    if(getNumberOfBlocksForGivenGroup(superBlock, group) < getNumberOfGroupDescriptorsBlocksInFullGroup(superBlock) + 1) //we got some group descriptors blocks but not all of them
        return getNumberOfBlocksForGivenGroup(superBlock, group) - 1;

    return getNumberOfGroupDescriptorsBlocksInFullGroup(superBlock);
}

uint32_t getFirstGroupDescriptorsBlockForGivenGroup(ext2_super_block* superBlock, uint32_t group)
{
    if(getNumberOfGroupDescriptorsBlocksForGivenGroup(superBlock, group) == 0)
        return 0;

    return getFirstBlockForGivenGroup(superBlock, group) + 1;
}

uint32_t getNumberOfInodesBlocksForGivenGroup(ext2_super_block* superBlock, uint32_t group)
{
    if(group < getNumberOfGroups(superBlock) - 1)
        return getNumberOfInodesBlocksInFullGroup(superBlock);

    //this is the case for the last group which is most probably incomplete
    if(getNumberOfBlocksForGivenGroup(superBlock, group) < getNumberOfGroupDescriptorsBlocksInFullGroup(superBlock) + 3)
        return 0;

    if(getNumberOfBlocksForGivenGroup(superBlock, group) <
            getNumberOfGroupDescriptorsBlocksInFullGroup(superBlock) + getNumberOfInodesBlocksInFullGroup(superBlock) + 3)
        return getNumberOfBlocksForGivenGroup(superBlock, group) - getNumberOfInodesBlocksInFullGroup(superBlock) - 3;

    return getNumberOfInodesBlocksInFullGroup(superBlock);
}

uint32_t getFirstInodeBlockForGivenGroup(ext2_super_block* superBlock, uint32_t group)
{
    if(getNumberOfInodesBlocksForGivenGroup(superBlock, group) == 0)
        return 0;

    return getFirstBlockForGivenGroup(superBlock, group) + getNumberOfGroupDescriptorsBlocksInFullGroup(superBlock) + 3;
}

uint32_t getDataBitmapBlockForGivenGroup(ext2_super_block* superBlock, uint32_t group)
{
    if(getNumberOfBlocksForGivenGroup(superBlock, group) < getNumberOfGroupDescriptorsBlocksForGivenGroup(superBlock, group) + 2)
        return 0;

    return getFirstBlockForGivenGroup(superBlock, group) + getNumberOfGroupDescriptorsBlocksInFullGroup(superBlock) + 1;
}

uint32_t getInodeBitmapBlockForGivenGroup(ext2_super_block* superBlock, uint32_t group)
{
    if(getNumberOfBlocksForGivenGroup(superBlock, group) < getNumberOfGroupDescriptorsBlocksForGivenGroup(superBlock, group) + 3)
        return 0;

    return getFirstBlockForGivenGroup(superBlock, group) + getNumberOfGroupDescriptorsBlocksInFullGroup(superBlock) + 2;
}

uint32_t getFirstInodeTableBlockForGivenGroup(ext2_super_block* superBlock, uint32_t group)
{
    if(getNumberOfBlocksForGivenGroup(superBlock, group) < getNumberOfGroupDescriptorsBlocksForGivenGroup(superBlock, group) + 4)
        return 0;

    return getFirstBlockForGivenGroup(superBlock, group) + getNumberOfGroupDescriptorsBlocksInFullGroup(superBlock) + 3;
}

///////////////////

uint32_t getNumberOfInodesForGivenGroup(ext2_super_block* superBlock, uint32_t group)
{
    if(group < getNumberOfGroups(superBlock) - 1)
        return getNumberOfInodesInFullGroup(superBlock);

    return superBlock->s_inodes_count - (getNumberOfGroups(superBlock) - 1) * getNumberOfInodesInFullGroup(superBlock);
}

///////////////////

uint32_t getNumberOfSectorsPerBlock(DiskInfo* diskInfo, ext2_super_block* superBlock)
{
    return superBlock->s_log_block_size / diskInfo->diskParameters.sectorSizeBytes;
}

uint32_t getFirstSectorForGivenBlock(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t block)
{
    return (1024 / diskInfo->diskParameters.sectorSizeBytes) + getNumberOfSectorsPerBlock(diskInfo, superBlock) * block; //we index blocks from 0, even if the standard is from 1
}

uint32_t getFirstSectorForGivenGroup(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t group)
{
    return getFirstSectorForGivenBlock(diskInfo, superBlock, getFirstBlockForGivenGroup(superBlock, group));
}

uint32_t getGroupNumberForGivenBlockGlobalIndex(ext2_super_block* superBlock, uint32_t blockGlobalIndex)
{
    return blockGlobalIndex / superBlock->s_blocks_per_group; //blocks are indexed from 0 globally, same for groups
}

///////////////////////

uint32_t getGlobalIndexOfInode(ext2_super_block* superBlock, uint32_t group, uint32_t localInodeIndex)
{
    return group * superBlock->s_inodes_per_group + localInodeIndex;
}

void getInodeBlockAndOffsetForInodeIndexInGroup(ext2_super_block* superBlock, uint32_t group, uint32_t localInodeIndex, uint32_t& blockGlobalIndex, uint32_t& offsetInsideBlock)
{
    uint32_t localIndexOfInodeBlockForGivenInodeIndex = (localInodeIndex * sizeof(ext2_inode)) / superBlock->s_log_block_size; //indexing is done from 0
    if(localInodeIndex * sizeof(ext2_inode) % superBlock->s_log_block_size == 0)
        localIndexOfInodeBlockForGivenInodeIndex--;

    if(getNumberOfInodesBlocksForGivenGroup(superBlock, group) < localIndexOfInodeBlockForGivenInodeIndex + 1)
        blockGlobalIndex = 0;
    else
        blockGlobalIndex =  getFirstInodeBlockForGivenGroup(superBlock, group) + localIndexOfInodeBlockForGivenInodeIndex;

    offsetInsideBlock = (localInodeIndex * sizeof(ext2_inode)) % superBlock->s_log_block_size;
}

/////////////////////////

uint32_t getGroupDescriptorOfGivenGroup(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t group, ext2_group_desc* searchedGroupDescriptor, uint32_t& groupDescriptorBlock,
                                        uint32_t& groupDescriptorOffsetInsideBlock)
{
    //when we want to read a group descriptor we don't read from the given group's group descriptors, but from the first one
    groupDescriptorBlock = getFirstGroupDescriptorsBlockForGivenGroup(superBlock, 0) + (group * sizeof(ext2_group_desc)) / superBlock->s_log_block_size;
    groupDescriptorOffsetInsideBlock = (group * sizeof(ext2_group_desc)) % superBlock->s_log_block_size;

    char* blockBuffer = new char[superBlock->s_log_block_size];
    uint32_t numberOfSectorsRead;
    uint32_t readResult = readDiskSectors(diskInfo, getNumberOfSectorsPerBlock(diskInfo, superBlock),
                                          getFirstSectorForGivenBlock(diskInfo, superBlock, groupDescriptorBlock),blockBuffer, numberOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        delete[] blockBuffer;
        return GET_GROUP_DESCRIPTOR_FOR_GIVEN_GROUP_FAILED;
    }

    memcpy(searchedGroupDescriptor, &blockBuffer[groupDescriptorOffsetInsideBlock], sizeof(ext2_group_desc));

    delete[] blockBuffer;
    return GET_GROUP_DESCRIPTOR_FOR_GIVEN_GROUP_SUCCESS;
}

//////////////////////////////////////////////

uint32_t getDataBlockGlobalIndexByLocalIndexInsideInode(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* inode, uint32_t searchedBlockLocalIndexInInode, uint32_t& searchedBlockGlobalIndex)
{
    if(searchedBlockLocalIndexInInode < 12) //searched block is in a direct block
    {
        searchedBlockGlobalIndex = inode->i_block[searchedBlockLocalIndexInInode];
        return GET_DATA_BLOCK_BY_LOCAL_INDEX_SUCCESS;
    }

    char* blockBuffer = new char[superBlock->s_log_block_size];
    uint32_t blockSize = superBlock->s_log_block_size;
    uint32_t numberOfSectorsRead, readResult;

    if(searchedBlockLocalIndexInInode <= blockSize / VALUE_ENTRY + 11) //searched block is in a second order block
    {
        readResult = readDiskSectors(diskInfo, getNumberOfSectorsPerBlock(diskInfo, superBlock),
                                              getFirstSectorForGivenBlock(diskInfo, superBlock, inode->i_block[12]),blockBuffer, numberOfSectorsRead);

        if(readResult != EC_NO_ERROR)
        {
            delete[] blockBuffer;
            return GET_DATA_BLOCK_BY_LOCAL_INDEX_FAILED;
        }

        uint32_t indexInSecondOrderArray = (searchedBlockLocalIndexInInode - 12) * VALUE_ENTRY;
        searchedBlockGlobalIndex = *(uint32_t*)&blockBuffer[indexInSecondOrderArray];

        delete[] blockBuffer;
        return GET_DATA_BLOCK_BY_LOCAL_INDEX_SUCCESS;
    }

    if(searchedBlockLocalIndexInInode <= blockSize * blockSize / (VALUE_ENTRY * VALUE_ENTRY) + blockSize / VALUE_ENTRY + 11)
    {
        readResult = readDiskSectors(diskInfo, getNumberOfSectorsPerBlock(diskInfo, superBlock),
                                     getFirstSectorForGivenBlock(diskInfo, superBlock, inode->i_block[13]),blockBuffer, numberOfSectorsRead);

        if(readResult != EC_NO_ERROR)
        {
            delete[] blockBuffer;
            return GET_DATA_BLOCK_BY_LOCAL_INDEX_FAILED;
        }

        uint32_t indexInSecondOrderArray = ((searchedBlockLocalIndexInInode - blockSize / VALUE_ENTRY - 12) / (blockSize / VALUE_ENTRY)) * VALUE_ENTRY;
        uint32_t thirdOrderArrayBlock = *(uint32_t*)&blockBuffer[indexInSecondOrderArray];

        readResult = readDiskSectors(diskInfo, getNumberOfSectorsPerBlock(diskInfo, superBlock),
                                     getFirstSectorForGivenBlock(diskInfo, superBlock, thirdOrderArrayBlock),blockBuffer, numberOfSectorsRead);

        if(readResult != EC_NO_ERROR)
        {
            delete[] blockBuffer;
            return GET_DATA_BLOCK_BY_LOCAL_INDEX_FAILED;
        }

        uint32_t indexInThirdOrderArray = ((searchedBlockLocalIndexInInode - blockSize / VALUE_ENTRY - 12) % (blockSize / VALUE_ENTRY)) * VALUE_ENTRY;
        searchedBlockGlobalIndex = *(uint32_t*)&blockBuffer[indexInThirdOrderArray];
        delete[] blockBuffer;
        return GET_DATA_BLOCK_BY_LOCAL_INDEX_SUCCESS;
    }

    //ELSE there is in 4 level

    readResult = readDiskSectors(diskInfo, getNumberOfSectorsPerBlock(diskInfo, superBlock),
                                 getFirstSectorForGivenBlock(diskInfo, superBlock, inode->i_block[14]),blockBuffer, numberOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        delete[] blockBuffer;
        return GET_DATA_BLOCK_BY_LOCAL_INDEX_FAILED;
    }

    uint32_t indexInSecondOrderArray = ((searchedBlockLocalIndexInInode - blockSize * blockSize / (VALUE_ENTRY * VALUE_ENTRY) - blockSize / VALUE_ENTRY - 12) / (blockSize * blockSize / (VALUE_ENTRY * VALUE_ENTRY))) * VALUE_ENTRY;
    uint32_t thirdOrderArrayBlock = *(uint32_t*)&blockBuffer[indexInSecondOrderArray];

    readResult = readDiskSectors(diskInfo, getNumberOfSectorsPerBlock(diskInfo, superBlock),
                                 getFirstSectorForGivenBlock(diskInfo, superBlock, thirdOrderArrayBlock),blockBuffer, numberOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        delete[] blockBuffer;
        return GET_DATA_BLOCK_BY_LOCAL_INDEX_FAILED;
    }

    uint32_t indexInThirdOrderArray = ((searchedBlockLocalIndexInInode - blockSize * blockSize / (VALUE_ENTRY * VALUE_ENTRY) - blockSize / VALUE_ENTRY - 12) % (blockSize * blockSize / (VALUE_ENTRY * VALUE_ENTRY))) / (blockSize / VALUE_ENTRY) * VALUE_ENTRY;
    uint32_t forthOrderArrayBlock = *(uint32_t*)&blockBuffer[indexInThirdOrderArray];

    readResult = readDiskSectors(diskInfo, getNumberOfSectorsPerBlock(diskInfo, superBlock),
                                 getFirstSectorForGivenBlock(diskInfo, superBlock, forthOrderArrayBlock),blockBuffer, numberOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        delete[] blockBuffer;
        return GET_DATA_BLOCK_BY_LOCAL_INDEX_FAILED;
    }

    uint32_t indexInForthOrderArray = ((searchedBlockLocalIndexInInode - blockSize * blockSize / (VALUE_ENTRY * VALUE_ENTRY) - blockSize / VALUE_ENTRY - 12) % (blockSize * blockSize / (VALUE_ENTRY * VALUE_ENTRY))) % (blockSize / VALUE_ENTRY) * VALUE_ENTRY;
    searchedBlockGlobalIndex =  *(uint32_t*)&blockBuffer[indexInForthOrderArray];
    delete[] blockBuffer;
    return GET_DATA_BLOCK_BY_LOCAL_INDEX_SUCCESS;
}

uint32_t getDataBlockLocalIndexInLocalListOfDataBlocksByGlobalIndex(ext2_super_block* superBlock, uint32_t dataBlockGlobalIndex)
{
    uint32_t blockIndexInGroup = dataBlockGlobalIndex % superBlock->s_blocks_per_group;
    return blockIndexInGroup - getNumberOfGroupDescriptorsBlocksInFullGroup(superBlock) - getNumberOfInodesBlocksInFullGroup(superBlock) - 3;
}

uint32_t getInodeByInodeGlobalIndex(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t inodeGlobalIndex, ext2_inode* searchedInode, uint32_t& inodeBlock,
                                    uint32_t& inodeOffsetInsideBlock)
{
    uint32_t groupOfTheInode = inodeGlobalIndex / superBlock->s_inodes_per_group; //inodes global index is counted from 0
    uint32_t inodesPerBlock = superBlock->s_log_block_size / sizeof(ext2_inode);
    inodeBlock = getFirstInodeTableBlockForGivenGroup(superBlock, groupOfTheInode) + (inodeGlobalIndex % superBlock->s_inodes_per_group) / inodesPerBlock;

    char* blockBuffer = new char[superBlock->s_log_block_size];
    uint32_t numberOfSectorsRead;
    uint32_t readResult = readDiskSectors(diskInfo, getNumberOfSectorsPerBlock(diskInfo, superBlock),
                                 getFirstSectorForGivenBlock(diskInfo, superBlock, inodeBlock),blockBuffer, numberOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        delete[] blockBuffer;
        return GET_INODE_BY_INODE_GLOBAL_INDEX_FAILED;
    }

    inodeOffsetInsideBlock = (inodeGlobalIndex % inodesPerBlock) * sizeof(ext2_inode);
    memcpy(searchedInode, blockBuffer + inodeOffsetInsideBlock, sizeof(ext2_inode));

    delete[] blockBuffer;
    return GET_INODE_BY_INODE_GLOBAL_INDEX_SUCCESS;
}

uint32_t getNumberOfOccupiedInodesInGroup(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t group, uint32_t& numberOfOccupiedInodes)
{
    numberOfOccupiedInodes = 0;
    uint32_t inodeBitmapBlock = getInodeBitmapBlockForGivenGroup(superBlock, group);
    char* blockBuffer = new char[superBlock->s_log_block_size];
    uint32_t numberOfSectorsRead = 0;
    uint32_t readResult = readDiskSectors(diskInfo, getNumberOfSectorsPerBlock(diskInfo, superBlock),
                                          getFirstSectorForGivenBlock(diskInfo, superBlock, inodeBitmapBlock), blockBuffer, numberOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        delete[] blockBuffer;
        return GET_NUMBER_OF_OCCUPIED_INODE_IN_BLOCK_FAILED;
    }

    while(ext2_getBitFromByte(blockBuffer[numberOfOccupiedInodes / 8], numberOfOccupiedInodes % 8) == 1)
        numberOfOccupiedInodes++;

    delete[] blockBuffer;
    return GET_NUMBER_OF_OCCUPIED_INODE_IN_BLOCK_SUCCESS;
}

uint32_t updateGroupDescriptor(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t group, int32_t freeInodesChange, int32_t freeDataBlocksChange)
{
    uint32_t numOfSectorsPerBlock = getNumberOfSectorsPerBlock(diskInfo, superBlock);
    ext2_group_desc* groupDescriptor = new ext2_group_desc();
    uint32_t groupDescriptorBlock;
    uint32_t groupDescriptorOffsetInsideBlock;
    uint32_t getGroupDescriptorResult = getGroupDescriptorOfGivenGroup(diskInfo, superBlock, group, groupDescriptor, groupDescriptorBlock,
                                                                       groupDescriptorOffsetInsideBlock);

    if(getGroupDescriptorResult == GET_GROUP_DESCRIPTOR_FOR_GIVEN_GROUP_FAILED)
    {
        delete groupDescriptor;
        return UPDATE_GROUP_DESCRIPTOR_FAILED;
    }

    groupDescriptor->bg_free_inodes_count += freeInodesChange;
    groupDescriptor->bg_free_blocks_count += freeDataBlocksChange;

    char* blockBuffer = new char[superBlock->s_log_block_size];
    uint32_t numOfSectorsRead;
    uint32_t readResult = readDiskSectors(diskInfo , numOfSectorsPerBlock, getFirstSectorForGivenBlock(diskInfo, superBlock, groupDescriptorBlock),
                                           blockBuffer, numOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        delete[] blockBuffer;
        UPDATE_GROUP_DESCRIPTOR_FAILED;
    }

    memcpy(blockBuffer + groupDescriptorOffsetInsideBlock, groupDescriptor, sizeof(ext2_group_desc));

    uint32_t numOfSectorsWritten;
    uint32_t writeResult = writeDiskSectors(diskInfo , numOfSectorsPerBlock, getFirstSectorForGivenBlock(diskInfo, superBlock, groupDescriptorBlock),
                                            blockBuffer, numOfSectorsWritten);

    delete[] blockBuffer;
    return (writeResult == EC_NO_ERROR) ? UPDATE_GROUP_DESCRIPTOR_SUCCESS : UPDATE_GROUP_DESCRIPTOR_FAILED;
}

uint32_t addInodeToInodeTable(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* inode)
{
    uint32_t numberOfOccupiedInodesInGroup;
    uint32_t getNumberOfOccupiedInodesResult = getNumberOfOccupiedInodesInGroup(diskInfo, superBlock, inode->i_group, numberOfOccupiedInodesInGroup);
    if(getNumberOfOccupiedInodesResult == GET_NUMBER_OF_OCCUPIED_INODE_IN_BLOCK_FAILED)
        return ADD_INODE_TO_INODE_TABLE_FAILED;

    uint32_t inodesPerBlock = superBlock->s_log_block_size / sizeof(ext2_inode);
    uint32_t inodeBlockGlobalIndex = getFirstInodeTableBlockForGivenGroup(superBlock, inode->i_group) + (inode->i_global_index % superBlock->s_inodes_per_group) / inodesPerBlock;
    uint32_t offsetInsideInodeTableBlock = (inode->i_global_index * sizeof(ext2_inode)) % superBlock->s_log_block_size;
    char* blockBuffer = new char[superBlock->s_log_block_size];
    uint32_t numberOfSectorsRead = 0;
    uint32_t readResult = readDiskSectors(diskInfo, getNumberOfSectorsPerBlock(diskInfo, superBlock),
                                          getFirstSectorForGivenBlock(diskInfo, superBlock, inodeBlockGlobalIndex), blockBuffer, numberOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        delete[] blockBuffer;
        return ADD_INODE_TO_INODE_TABLE_FAILED;
    }

    memcpy(blockBuffer + offsetInsideInodeTableBlock, inode, sizeof(ext2_inode));
    uint32_t numOfSectorsWritten;
    uint32_t writeResult = writeDiskSectors(diskInfo , getNumberOfSectorsPerBlock(diskInfo, superBlock),
                                            getFirstSectorForGivenBlock(diskInfo, superBlock, inodeBlockGlobalIndex), blockBuffer, numOfSectorsWritten);

    delete[] blockBuffer;

    return (writeResult == EC_NO_ERROR) ? ADD_INODE_TO_INODE_TABLE_SUCCESS : ADD_INODE_TO_INODE_TABLE_FAILED;
}

uint32_t updateValueInInodeBitmap(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t inodeGlobalIndex, uint8_t newValue)
{
    uint32_t numOfSectorsPerBlock = getNumberOfSectorsPerBlock(diskInfo, superBlock);
    uint32_t groupOfInode = inodeGlobalIndex / superBlock->s_inodes_per_group;
    uint32_t inodeBitmapBlock = getInodeBitmapBlockForGivenGroup(superBlock, groupOfInode);

    char* blockBuffer = new char[superBlock->s_log_block_size];
    uint32_t numberOfSectorsRead = 0;
    uint32_t readResult = readDiskSectors(diskInfo, numOfSectorsPerBlock, getFirstSectorForGivenBlock(diskInfo, superBlock, inodeBitmapBlock),
                                          blockBuffer, numberOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        delete[] blockBuffer;
        return UPDATE_VALUE_IN_INODE_BITMAP_FAILED;
    }

    uint32_t inodeLocalIndex = inodeGlobalIndex % superBlock->s_inodes_per_group;
    uint8_t newByteValue = ext2_changeBitValue(blockBuffer[inodeLocalIndex / 8], inodeLocalIndex % 8, newValue);
    memset(blockBuffer + inodeLocalIndex / 8, newByteValue, 1);

    uint32_t numOfSectorsWritten;
    uint32_t writeResult = writeDiskSectors(diskInfo , numOfSectorsPerBlock, getFirstSectorForGivenBlock(diskInfo, superBlock, inodeBitmapBlock),
                                            blockBuffer, numOfSectorsWritten);

    delete[] blockBuffer;
    return (writeResult == EC_NO_ERROR) ? UPDATE_VALUE_IN_INODE_BITMAP_SUCCESS : UPDATE_VALUE_IN_INODE_BITMAP_FAILED;
}

uint32_t updateValueInDataBlockBitmap(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t dataBlockGlobalIndex, uint8_t newValue)
{
    uint32_t numOfSectorsPerBlock = getNumberOfSectorsPerBlock(diskInfo, superBlock);
    uint32_t groupOfBlock = dataBlockGlobalIndex / superBlock->s_blocks_per_group;
    uint32_t dataBitmapBlock = getDataBitmapBlockForGivenGroup(superBlock, groupOfBlock);

    char* blockBuffer = new char[superBlock->s_log_block_size];
    uint32_t numberOfSectorsRead = 0;
    uint32_t readResult = readDiskSectors(diskInfo, numOfSectorsPerBlock, getFirstSectorForGivenBlock(diskInfo, superBlock, dataBitmapBlock),
                                          blockBuffer, numberOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        delete[] blockBuffer;
        return UPDATE_VALUE_IN_DATA_BLOCK_BITMAP_FAILED;
    }

    uint32_t firstDataBlockForGivenGroup = getFirstDataBlockForGivenGroup(superBlock, groupOfBlock);
 //   uint32_t dataBlockLocalIndex = dataBlockGlobalIndex % getNumberOfBlocksForGivenGroup(superBlock, 0) - firstDataBlockForGivenGroup; //local index in list of data blocks: 0,1,2..
    uint32_t dataBlockLocalIndex = dataBlockGlobalIndex - firstDataBlockForGivenGroup;
    uint8_t newByteValue = ext2_changeBitValue(blockBuffer[dataBlockLocalIndex / 8], dataBlockLocalIndex % 8, newValue);
    memset(blockBuffer + dataBlockLocalIndex / 8, newByteValue, 1);

    uint32_t numOfSectorsWritten;
    uint32_t writeResult = writeDiskSectors(diskInfo , numOfSectorsPerBlock, getFirstSectorForGivenBlock(diskInfo, superBlock, dataBitmapBlock),
                                            blockBuffer, numOfSectorsWritten);

    if(writeResult != EC_NO_ERROR)
    {
        delete[] blockBuffer;
        return UPDATE_VALUE_IN_DATA_BLOCK_BITMAP_FAILED;
    }

    return UPDATE_VALUE_IN_DATA_BLOCK_BITMAP_SUCCESS;
}

uint32_t updateInode(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* inode, ext2_inode* newInode)
{
    uint32_t inodeBlockGlobalIndex, inodeOffsetInsideBlock, numberOfSectorsRead, numOfSectorsWritten;
    ext2_inode* dummy = new ext2_inode();
    uint32_t getInodeByIndexResult = getInodeByInodeGlobalIndex(diskInfo, superBlock, inode->i_global_index, dummy, inodeBlockGlobalIndex,
                                                                inodeOffsetInsideBlock);

    if(getInodeByIndexResult == GET_INODE_BY_INODE_GLOBAL_INDEX_FAILED)
        return UPDATE_INODE_FAILED;

    char* blockBuffer = new char[superBlock->s_log_block_size];
    uint32_t readResult = readDiskSectors(diskInfo, getNumberOfSectorsPerBlock(diskInfo, superBlock),
                                          getFirstSectorForGivenBlock(diskInfo, superBlock, inodeBlockGlobalIndex), blockBuffer, numberOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        delete[] blockBuffer;
        return UPDATE_INODE_FAILED;
    }

    memcpy(blockBuffer + inodeOffsetInsideBlock, newInode, sizeof(ext2_inode));
    uint32_t writeResult = writeDiskSectors(diskInfo , getNumberOfSectorsPerBlock(diskInfo, superBlock),
                                            getFirstSectorForGivenBlock(diskInfo, superBlock, inodeBlockGlobalIndex), blockBuffer, numOfSectorsWritten);

    delete[] blockBuffer;
    return (writeResult == EC_NO_ERROR) ? UPDATE_INODE_SUCCESS : UPDATE_INODE_FAILED;
}

uint32_t addDirectoryEntryToParent(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* parentInode, ext2_inode* newInode, char* newDirectoryName)
{
    uint32_t newDirectoryEntryBlockGlobalIndex, parentInodeBlockGlobalIndex, parentInodeOffsetInsideBlock, numberOfSectorsRead, numOfSectorsWritten, dummy;
    char* blockBuffer = new char[superBlock->s_log_block_size];

    ext2_dir_entry* directoryEntry = new ext2_dir_entry();
    memset(directoryEntry, 0, sizeof(ext2_dir_entry));
    directoryEntry->inode = newInode->i_global_index;
    directoryEntry->name_len = strlen(newDirectoryName);
    directoryEntry->file_type = newInode->i_mode;
    memcpy(&directoryEntry->name, newDirectoryName, directoryEntry->name_len);

    uint32_t blockToAddDirectoryEntryLocalIndex = parentInode->i_size / superBlock->s_log_block_size; //the block where is the last data of the directory
    if(parentInode->i_size != 0 && parentInode->i_size % superBlock->s_log_block_size == 0) //it means the data ends at the end of a block
    {
        if(parentInode->i_blocks == blockToAddDirectoryEntryLocalIndex) //it means there aren't anymore preallocate blocks, so we need to add a new one to the directory
        {
            uint32_t addBlockToDirectoryResult = allocateBlockToDirectory(diskInfo, superBlock, parentInode, dummy, parentInode); //we update the parentInode
            if(addBlockToDirectoryResult != ADD_BLOCK_TO_DIRECTORY_SUCCESS)
            {
                delete[] blockBuffer, delete directoryEntry;
                return ADD_DIRECTORY_ENTRY_TO_PARENT_FAILED;
            }
        }
        //else it is the next block after parentInode->i_size / superBlock->s_log_block_size which is already + 1 because the size are equal
    }

    uint32_t getBlockGlobalIndexResult = getDataBlockGlobalIndexByLocalIndexInsideInode(diskInfo, superBlock,
                                                                                        parentInode,
                                                                                        blockToAddDirectoryEntryLocalIndex,
                                                                                        newDirectoryEntryBlockGlobalIndex);
    if(getBlockGlobalIndexResult != GET_DATA_BLOCK_BY_LOCAL_INDEX_SUCCESS)
    {
        delete[] blockBuffer, delete directoryEntry;
        return ADD_DIRECTORY_ENTRY_TO_PARENT_FAILED;
    }

    uint32_t readResult = readDiskSectors(diskInfo, getNumberOfSectorsPerBlock(diskInfo, superBlock),
                                          getFirstSectorForGivenBlock(diskInfo, superBlock, newDirectoryEntryBlockGlobalIndex), blockBuffer, numberOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        delete[] blockBuffer, delete directoryEntry;
        return ADD_DIRECTORY_ENTRY_TO_PARENT_FAILED;
    }

    uint32_t offsetInBlockToAddDirectoryEntry = parentInode->i_size % superBlock->s_log_block_size;
    memcpy(blockBuffer + offsetInBlockToAddDirectoryEntry, directoryEntry, sizeof(ext2_dir_entry));

    uint32_t writeResult = writeDiskSectors(diskInfo , getNumberOfSectorsPerBlock(diskInfo, superBlock),
                                            getFirstSectorForGivenBlock(diskInfo, superBlock, newDirectoryEntryBlockGlobalIndex), blockBuffer, numOfSectorsWritten);

    if(writeResult != EC_NO_ERROR)
    {
        delete[] blockBuffer, delete directoryEntry;
        return ADD_DIRECTORY_ENTRY_TO_PARENT_FAILED;
    }

    ext2_inode* updatedParentInode = new ext2_inode();
    memcpy(updatedParentInode, parentInode, sizeof(ext2_inode));
    updatedParentInode->i_size += sizeof(ext2_inode);
    uint32_t updateParentInodeResult = updateInode(diskInfo, superBlock, parentInode, updatedParentInode);

    return (updateParentInodeResult == UPDATE_INODE_SUCCESS) ? ADD_DIRECTORY_ENTRY_TO_PARENT_SUCCESS : ADD_DIRECTORY_ENTRY_TO_PARENT_FAILED;
}

void updateInodeLastAccessedDataAndTime(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* inode)
{
    uint32_t inodeBlockGlobalIndex, inodeOffsetInsideBlock;
    ext2_inode* updatedInode = new ext2_inode();

    uint32_t getInodeByIndexResult = getInodeByInodeGlobalIndex(diskInfo, superBlock, inode->i_global_index, updatedInode, inodeBlockGlobalIndex,
                                                                inodeOffsetInsideBlock);

    if(getInodeByIndexResult == GET_INODE_BY_INODE_GLOBAL_INDEX_FAILED)
    {
        delete updatedInode;
        return;
    }

    updatedInode->i_atime = ext2_getCurrentTimeDateAndTimeFormatted();
    updateInode(diskInfo, superBlock, inode, updatedInode);
    delete updatedInode;
}

void updateInodeLastChangeDataAndTime(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* inode)
{
    uint32_t inodeBlockGlobalIndex, inodeOffsetInsideBlock;
    ext2_inode* updatedInode = new ext2_inode();

    uint32_t getInodeByIndexResult = getInodeByInodeGlobalIndex(diskInfo, superBlock, inode->i_global_index, updatedInode, inodeBlockGlobalIndex,
                                                                inodeOffsetInsideBlock);

    if(getInodeByIndexResult == GET_INODE_BY_INODE_GLOBAL_INDEX_FAILED)
    {
        delete updatedInode;
        return;
    }

    updatedInode->i_mtime = ext2_getCurrentTimeDateAndTimeFormatted();
    updateInode(diskInfo, superBlock, inode, updatedInode);
    delete updatedInode;
}