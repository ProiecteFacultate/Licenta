#include "../include/disk.h"
#include "../include/structures.h"
#include "../include/ext2FunctionUtils.h"

uint32_t getNumberOfGroups(ext2_super_block* superBlock)
{
    return superBlock->s_block_count / superBlock->s_blocks_per_group;
}

uint32_t getNumberOfGroupDescriptorsTablesPerGroup(ext2_super_block* superBlock)
{
    uint32_t numOfGroupDescriptors = (getNumberOfGroups(superBlock) * sizeof(ext2_group_desc)) / superBlock->s_log_block_size + 1;
    if(getNumberOfGroups(superBlock) * sizeof(ext2_group_desc) % superBlock->s_log_block_size == 0)
        numOfGroupDescriptors--;

    return numOfGroupDescriptors;
}

uint32_t getNumberOfInodesTablesPerGroup(ext2_super_block* superBlock)
{
    uint32_t numOfInodeTables = (superBlock->s_inodes_per_group * 128) / superBlock->s_log_block_size + 1;
    if(superBlock->s_inodes_per_group * 128 % superBlock->s_log_block_size == 0)
        numOfInodeTables--;

    return numOfInodeTables;
}

uint32_t getNumberOfDataBlocksPerGroup(ext2_super_block* superBlock)
{
    return superBlock->s_blocks_per_group - getNumberOfGroupDescriptorsTablesPerGroup(superBlock) - getNumberOfInodesTablesPerGroup(superBlock) - 3;
}

uint32_t getFirstBlockForGroup(ext2_super_block* superBlock, uint32_t group)
{
    return superBlock->s_blocks_per_group * group + 1; //first block of the first group is considered to be block index 1 globally
}

uint32_t getFirstSectorForBlock(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t block)
{
    return (1024 / diskInfo->diskParameters.sectorSizeBytes) + getNumberOfSectorsPerBlock(diskInfo, superBlock) * (block - 1); //-1 because blocks are indexed from 1 globally
}

uint32_t getFirstSectorForGroup(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t group)
{
    getFirstSectorForBlock(diskInfo, superBlock, getFirstBlockForGroup(superBlock, group));
}

uint32_t getNumberOfSectorsPerBlock(DiskInfo* diskInfo, ext2_super_block* superBlock)
{
    return superBlock->s_log_block_size / diskInfo->diskParameters.sectorSizeBytes;
}