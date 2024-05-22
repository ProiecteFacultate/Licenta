#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include/fat32Structures.h"

#ifndef FAT32INIT_FAT32INIT_H
#define FAT32INIT_FAT32INIT_H

//this does all the check required (checks if disk is initialized with all its sectors, if fat32 file system is initialized), and if not, it initializes them.
//the given sectorsNumber & sectorSize arguments are for the case where the disk is not initialized, and we need to initialize it. If you know for sure it is already init, you can
//set these values to anything
void fat32Startup(char* diskDirectory, DiskInfo** diskInfo, BootSector** bootSector, FsInfo** fsInfo, uint32_t sectorsNumber, uint32_t sectorSize, uint32_t sectorsPerCluster,
                  bool printSteps);

static bool checkDiskInitialization(char* diskDirectory);
static void initializeDisk(char* diskDirectory, DiskInfo** diskInfo, uint32_t sectorsNumber, uint32_t sectorSize, bool printSteps);

bool checkFat32FileSystemInitialization(DiskInfo* diskInfo);

BootSector* readBootSector(DiskInfo* diskInfo);
FsInfo * readFsInfo(DiskInfo* diskInfo, BootSector* bootSector);

///////

static void initializeBootSectors(DiskInfo* diskInfo, uint32_t sectorsPerCluster);
//This initializes only the file allocation table NOT THE WHOLE FAT32 FILE SYSTEM!
static void initializeFat(DiskInfo* diskInfo, BootSector* bootSector);

#endif