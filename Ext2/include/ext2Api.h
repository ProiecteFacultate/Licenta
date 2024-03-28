#include "../include/disk.h"
#include "../include/structures.h"

#ifndef EXT2_EXT2API_H
#define EXT2_EXT2API_H

uint32_t createDirectory(DiskInfo* diskInfo, ext2_super_block* superBlock, char* parentDirectoryPath, char* newDirectoryName, uint32_t newDirectoryType);

#endif
