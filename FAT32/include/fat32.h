#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include/diskUtils.h"
#include "../include/fat32Init.h"

#ifndef FAT32_FAT32_H
#define FAT32_FAT32_H

int getNextSector(DiskInfo* diskInfo, BootSector* bootSector, uint32_t actualCluster, uint32_t& nextCluster);
int createDirectory(DiskInfo* diskInfo, BootSector* bootSector, char* directoryParentPath, char* directoryName);

static int findDirectoryEntry(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* parentDirectoryEntry, char* searchedDirectoryName, DirectoryEntry** searchedDirectoryEntry);
static int findDirectoryEntryInCluster(BootSector* bootSector, char* clusterData, char* dirName, DirectoryEntry** directoryEntry);
static int searchEmptyCluster(DiskInfo* diskInfo, BootSector* bootSector, uint32_t& emptyClusterNumber);

#endif
