#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include/structures.h"

#ifndef FAT32INIT_FAT32INIT_H
#define FAT32INIT_FAT32INIT_H

//Initialization functions of the file system

bool checkBootSectorsInitialized(DiskInfo* diskInfo);
void initializeBootSectors(DiskInfo* diskInfo);
BootSector* readBootSector(DiskInfo* diskInfo);
void initializeFat(DiskInfo* diskInfo, BootSector* bootSector);
FsInfo * readFsInfo(DiskInfo* diskInfo, BootSector* bootSector);

#endif