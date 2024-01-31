#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include/diskUtils.h"
#include "../include/fat32Init.h"

#ifndef FAT32_FAT32_H
#define FAT32_FAT32_H

uint32_t getNextSector(DiskInfo* diskInfo, BootSector* bootSector, uint32_t actualCluster);
int createFolder(DiskInfo* diskInfo, BootSector* bootSector, char* directoryParentPath, char* directoryName);

static DirectoryEntry* findDirectoryEntry(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* parentDirEntry, char* searchedDir);
static DirectoryEntry* findDirectoryEntryInCluster(BootSector* bootSector, char* clusterData, char* dirName);

#endif
