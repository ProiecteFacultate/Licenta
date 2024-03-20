#include "../include/disk.h"
#include "../include/structures.h"

#ifndef EXT2_EXT2FUNCTIONUTILS_H
#define EXT2_EXT2FUNCTIONUTILS_H

uint32_t getNumberOfGroups(ext2_super_block* superBlock);
uint32_t getNumberOfGroupDescriptorsTablesPerGroup(ext2_super_block* superBlock);
uint32_t getNumberOfInodesTablesPerGroup(ext2_super_block* superBlock);
uint32_t getNumberOfDataBlocksPerGroup(ext2_super_block* superBlock);
uint32_t getFirstBlockForGroup(ext2_super_block* superBlock, uint32_t group);
uint32_t getFirstSectorForBlock(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t block);
uint32_t getFirstSectorForGroup(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t group);
uint32_t getNumberOfSectorsPerBlock(DiskInfo* diskInfo, ext2_super_block* superBlock);

#endif
