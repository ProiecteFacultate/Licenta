#include "vector"

#include "../include/disk.h"
#include "../include/structures.h"

#ifndef EXT2_EXT2INIT_H
#define EXT2_EXT2INIT_H

void ext2Startup(char* diskDirectory, DiskInfo** diskInfo, uint32_t sectorsNumber, uint32_t sectorSize);

//the super block of the first group (the one that is used, the others are just for backup)
ext2_super_block* readFirstSuperBlock(DiskInfo* diskInfo);

//////////

static bool checkDiskInitialization(char* diskDirectory);
static void initializeDisk(char* diskDirectory, DiskInfo** diskInfo, uint32_t sectorsNumber, uint32_t sectorSize);

static bool checkExt2FileSystemInitialization(DiskInfo* diskInfo);

//Creates main superblock
static void initializeFirstSuperBlockInFirstGroup(DiskInfo* diskInfo);
//initialize super block for all blocks, and also groups descriptors
static void initializeGroups(DiskInfo* diskInfo, ext2_super_block* superBlock);
static void initializeGroupDescriptors(DiskInfo* diskInfo, ext2_super_block* superBlock, std::vector<ext2_group_desc *>& groupDescriptors);

#endif
