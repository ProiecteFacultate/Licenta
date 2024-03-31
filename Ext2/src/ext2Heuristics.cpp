#include "string.h"
#include "string"
#include "vector"
#include "cstdint"
#include "math.h"

#include "../include/disk.h"
#include "../include/diskCodes.h"
#include "../include/structures.h"
#include "../include/ext2FunctionUtils.h"
#include "../include/codes/ext2Codes.h"
#include "../include/codes/ext2Attributes.h"
#include "../include/ext2Heuristics.h"

uint32_t searchFreeInodeForDirectoryHavingParentRoot(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t& foundInodeGlobalIndex)
{
    uint32_t numberOfGroups = getNumberOfGroups(superBlock);
    uint32_t freeInodesAverage = 0, freeBlocksAverage = 0, foundGroup;
    std::vector<std::pair<uint16_t, uint16_t>> groupFreeInodesAndFreeBlocksList;

    uint32_t getFreeInodesAndBlockResult = getGroupsFreeInodesAndFreeBlocks(diskInfo, superBlock, groupFreeInodesAndFreeBlocksList);

    if(getFreeInodesAndBlockResult == GET_GROUPS_FREE_INODES_AND_GROUPS_FAILED)
        return SEARCH_FREE_INODE_FAILED_FOR_OTHER_REASON;

    for(uint32_t group = 0; group < numberOfGroups; group++)
    {
        freeInodesAverage += groupFreeInodesAndFreeBlocksList[group].first;
        freeBlocksAverage += groupFreeInodesAndFreeBlocksList[group].second;
    }

    freeInodesAverage /= numberOfGroups;
    freeBlocksAverage /= numberOfGroups;

    for(uint32_t group = 0; group < numberOfGroups; group++)
        if(groupFreeInodesAndFreeBlocksList[group].first > freeInodesAverage)
        {
            foundGroup = group; //in case there is no group with both free inodes and blocks < average (in book fallback rule 2.c)
            if(groupFreeInodesAndFreeBlocksList[group].second > freeBlocksAverage)
            {
                foundGroup = group;
                break;
            }
        }

    uint32_t searchFirstFreeInodeInGroupResult = searchFirstFreeInodeInGroup(diskInfo, superBlock, foundGroup, foundInodeGlobalIndex);

    switch (searchFirstFreeInodeInGroupResult) {
        case SEARCH_FIRST_EMPTY_INODE_IN_GROUP_NO_FREE_INODE_IN_GROUP:
            return SEARCH_FREE_INODE_NO_FREE_INODES;
        case SEARCH_FIRST_EMPTY_INODE_IN_GROUP_FAILED_FOR_OTHER_REASON:
            return SEARCH_FREE_INODE_FAILED_FOR_OTHER_REASON;
        default:
            return SEARCH_FREE_INODE_SUCCESS;
    }
}

uint32_t searchFreeInodeForNestedDirectory(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* parentInode, uint32_t& foundInodeGlobalIndex)
{
    uint32_t numberOfGroups = getNumberOfGroups(superBlock);
    uint32_t numOfOccupiedInodesInParent;
    uint32_t getParentNumberOfOccupiedInodesResult = getNumberOfOccupiedInodesInGroup(diskInfo, superBlock, parentInode->i_group, numOfOccupiedInodesInParent);
    if(getParentNumberOfOccupiedInodesResult == GET_NUMBER_OF_OCCUPIED_INODE_IN_BLOCK_FAILED)
        return SEARCH_FREE_INODE_FAILED_FOR_OTHER_REASON;

    if(numOfOccupiedInodesInParent < getNumberOfInodesForGivenGroup(superBlock, parentInode->i_group))
    {
        int32_t debt;
        uint32_t calculateGroupDebtResult = calculateGroupDebt(diskInfo, superBlock, parentInode->i_group, debt);
        if(calculateGroupDebtResult == CALCULATE_GROUP_DEBT_FAILED)
            return SEARCH_FREE_INODE_FAILED_FOR_OTHER_REASON;

        //calculate average debt
        int32_t averageDebt = 0;
        for(uint32_t group = 0; group < numberOfGroups; group++)
        {
            int32_t actualGroupDebt;
            calculateGroupDebtResult = calculateGroupDebt(diskInfo, superBlock, group, actualGroupDebt);
            if(calculateGroupDebtResult == CALCULATE_GROUP_DEBT_FAILED)
                return SEARCH_FREE_INODE_FAILED_FOR_OTHER_REASON;

            averageDebt += actualGroupDebt;
        }

        averageDebt /= numberOfGroups;
        if(debt <= averageDebt)
        {
            uint32_t searchFirstFreeInodeInGroupResult = searchFirstFreeInodeInGroup(diskInfo, superBlock, parentInode->i_group, foundInodeGlobalIndex);
            switch (searchFirstFreeInodeInGroupResult) {
                case SEARCH_FIRST_EMPTY_INODE_IN_GROUP_NO_FREE_INODE_IN_GROUP:
                    return SEARCH_FREE_INODE_NO_FREE_INODES;
                case SEARCH_FIRST_EMPTY_INODE_IN_GROUP_FAILED_FOR_OTHER_REASON:
                    return SEARCH_FREE_INODE_FAILED_FOR_OTHER_REASON;
                default:
                    return SEARCH_FREE_INODE_SUCCESS;
            }
        }
    }

    //ELSE it means there are no free inodes in parent group OR that the parent group debt is too high (> than average); in this case we apply fallback rule (case 2.c in book)
    uint32_t foundGroup;
    uint32_t freeInodesAverage = 0;
    std::vector<std::pair<uint16_t, uint16_t>> groupFreeInodesAndFreeBlocksList;

    uint32_t getFreeInodesAndBlockResult = getGroupsFreeInodesAndFreeBlocks(diskInfo, superBlock, groupFreeInodesAndFreeBlocksList);

    if(getFreeInodesAndBlockResult == GET_GROUPS_FREE_INODES_AND_GROUPS_FAILED)
        return SEARCH_FREE_INODE_FAILED_FOR_OTHER_REASON;

    for(uint32_t group = 0; group < numberOfGroups; group++)
        freeInodesAverage += groupFreeInodesAndFreeBlocksList[group].first;

    freeInodesAverage /= numberOfGroups;

    //starts looking for the first group with num of free inodes > average STARTING WITH PARENT GROUP
    if(groupFreeInodesAndFreeBlocksList[parentInode->i_group].second > freeInodesAverage)
        foundGroup = parentInode->i_group;

    for(uint32_t group = 0; group < numberOfGroups; group++)
        if(groupFreeInodesAndFreeBlocksList[group].first > freeInodesAverage)
        {
            foundGroup = group;
            break;
        }

    uint32_t searchFirstFreeInodeInGroupResult = searchFirstFreeInodeInGroup(diskInfo, superBlock, foundGroup, foundInodeGlobalIndex);

    switch (searchFirstFreeInodeInGroupResult) {
        case SEARCH_FIRST_EMPTY_INODE_IN_GROUP_NO_FREE_INODE_IN_GROUP:
            return SEARCH_FREE_INODE_NO_FREE_INODES;
        case SEARCH_FIRST_EMPTY_INODE_IN_GROUP_FAILED_FOR_OTHER_REASON:
            return SEARCH_FREE_INODE_FAILED_FOR_OTHER_REASON;
        default:
            return SEARCH_FREE_INODE_SUCCESS;
    }
}

uint32_t searchFreeInodeForRegularFile(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* parentInode, uint32_t& foundInodeGlobalIndex)
{
    uint32_t numberOfGroups = getNumberOfGroups(superBlock);

    uint32_t searchFirstFreeInodeInGroupResult = searchFirstFreeInodeInGroup(diskInfo, superBlock, parentInode->i_group, foundInodeGlobalIndex);
    if(searchFirstFreeInodeInGroupResult == SEARCH_FIRST_EMPTY_INODE_IN_GROUP_SUCCESS)
        return SEARCH_FREE_INODE_SUCCESS;

    for(uint32_t maxSumPower = 0; pow(2, maxSumPower) <= numberOfGroups; maxSumPower++)
    {
        uint32_t group = parentInode->i_group;
        for(uint32_t sumPower = 0; sumPower <= maxSumPower; sumPower++)
            group += pow(2, sumPower);

        group %= numberOfGroups;
        searchFirstFreeInodeInGroupResult = searchFirstFreeInodeInGroup(diskInfo, superBlock, group, foundInodeGlobalIndex);
        if(searchFirstFreeInodeInGroupResult == SEARCH_FIRST_EMPTY_INODE_IN_GROUP_SUCCESS)
            return SEARCH_FREE_INODE_SUCCESS;
    }

    //ELSE we make a linear search until we find a group with free inode
    for(uint32_t group = 0; group < numberOfGroups; group++)
    {
        searchFirstFreeInodeInGroupResult = searchFirstFreeInodeInGroup(diskInfo, superBlock, group, foundInodeGlobalIndex);
        if(searchFirstFreeInodeInGroupResult == SEARCH_FIRST_EMPTY_INODE_IN_GROUP_SUCCESS)
            return SEARCH_FREE_INODE_SUCCESS;
    }

    return SEARCH_FREE_INODE_NO_FREE_INODES;
}

//////////////////////////

static uint32_t getGroupsFreeInodesAndFreeBlocks(DiskInfo* diskInfo, ext2_super_block* superBlock, std::vector<std::pair<uint16_t, uint16_t>>& groupFreeInodesAndFreeBlocksList)
{
    uint32_t dummy1, dummy2;
    uint32_t numberOfGroups = getNumberOfGroups(superBlock);
    ext2_group_desc* groupDescriptor = new ext2_group_desc();

    for(uint32_t group = 0; group < numberOfGroups; group++)
    {
        uint32_t getGroupDescriptorResult = getGroupDescriptorOfGivenGroup(diskInfo, superBlock, group, groupDescriptor, dummy1, dummy2);
        if(getGroupDescriptorResult == GET_GROUP_DESCRIPTOR_FOR_GIVEN_GROUP_FAILED)
        {
            delete groupDescriptor;
            return GET_GROUPS_FREE_INODES_AND_GROUPS_FAILED;
        }

        //we do this instead of directly passing groupDescriptor value because of a gcc bug which doesn't allow to compile
        uint16_t freeInodesInGroup;
        memcpy(&freeInodesInGroup, &groupDescriptor->bg_free_inodes_count, 2);
        uint16_t freeBlocksInGroup;
        memcpy(&freeBlocksInGroup, &groupDescriptor->bg_free_blocks_count, 2);
        groupFreeInodesAndFreeBlocksList.push_back(std::make_pair(freeInodesInGroup, freeBlocksInGroup));
    }

    delete groupDescriptor;
    return  GET_GROUPS_FREE_INODES_AND_GROUPS_SUCCESS;
}

static uint32_t searchFirstFreeInodeInGroup(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t group, uint32_t& firstFreeInodeInGroupGlobalIndex)
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
        return SEARCH_FIRST_EMPTY_INODE_IN_GROUP_FAILED_FOR_OTHER_REASON;
    }

    for(uint32_t bitIndex = 0; bitIndex < getNumberOfInodesForGivenGroup(superBlock, group); bitIndex++)
        if(getBitFromByte(blockBuffer[bitIndex / 8], bitIndex % 8) == 0)
        {
            firstFreeInodeInGroupGlobalIndex = group * getNumberOfInodesBlocksInFullGroup(superBlock) + bitIndex;
            delete[] blockBuffer;
            return SEARCH_FIRST_EMPTY_INODE_IN_GROUP_SUCCESS;
        }

    return SEARCH_FIRST_EMPTY_INODE_IN_GROUP_NO_FREE_INODE_IN_GROUP;
}

static uint32_t calculateGroupDebt(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t group, int32_t& debt)
{
    debt = 0;
    uint32_t numOfSectorsPerBlock = getNumberOfSectorsPerBlock(diskInfo, superBlock);
    uint32_t inodeBitmapBlock = getInodeBitmapBlockForGivenGroup(superBlock, group);
    uint32_t numberOfInodesPerBlock = superBlock->s_log_block_size / sizeof(ext2_inode);
    char* blockBuffer = new char[superBlock->s_log_block_size];
    uint32_t numberOfSectorsRead = 0;
    uint32_t readResult = readDiskSectors(diskInfo, numOfSectorsPerBlock, getFirstSectorForGivenBlock(diskInfo, superBlock, inodeBitmapBlock),
                                          blockBuffer, numberOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        delete[] blockBuffer;
        return CALCULATE_GROUP_DEBT_FAILED;
    }

    char* blockBuffer_2 = new char[superBlock->s_log_block_size];
    ext2_inode* actualInode = new ext2_inode();

    for(uint32_t inodeIndex = 0; ; inodeIndex++)
    {
        if(getBitFromByte(blockBuffer[inodeIndex / 8], inodeIndex % 8) == 1)
        {
            if(inodeIndex % numberOfInodesPerBlock == 0) //read a new block from inodes table
            {
                uint32_t inodeBlock = getFirstInodeTableBlockForGivenGroup(superBlock, group) + inodeIndex / numberOfInodesPerBlock;
                readResult = readDiskSectors(diskInfo, numOfSectorsPerBlock, getFirstSectorForGivenBlock(diskInfo, superBlock, inodeBlock),
                                             blockBuffer_2, numberOfSectorsRead);

                if(readResult != EC_NO_ERROR)
                {
                    delete[] blockBuffer, delete[] blockBuffer_2, delete actualInode;
                    return CALCULATE_GROUP_DEBT_FAILED;
                }

                actualInode = (ext2_inode*)&blockBuffer_2[(inodeIndex * sizeof(ext2_inode)) % superBlock->s_log_block_size];
                (actualInode->i_mode == FILE_TYPE_DIRECTORY) ? debt++ : debt--;
            }
        }
        else
        {
            delete[] blockBuffer, delete[] blockBuffer_2, delete actualInode;
            return CALCULATE_GROUP_DEBT_SUCCESS;
        }
    }
}