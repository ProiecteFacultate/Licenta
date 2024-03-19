#ifndef EXT2_EXT2INIT_H
#define EXT2_EXT2INIT_H

bool checkDiskInitialization(char* diskDirectory);
void initializeDisk(char* diskDirectory, DiskInfo** diskInfo, uint32_t sectorsNumber, uint32_t sectorSize);

bool checkExt2FileSystemInitialization(DiskInfo* diskInfo);

////
//Creates main superblock
static void initializeFirstSectors(DiskInfo* diskInfo);

#endif
