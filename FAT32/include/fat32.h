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
static int changeFatClusterValue(DiskInfo* diskInfo, BootSector* bootSector, uint32_t clusterNumber, char* value);
static int findFirstFreeOffsetInDirectory(BootSector* bootSector, char* clusterData, uint32_t& firstFreeOffset);  //given a directory, find the first place where we could place a new dir entry
static void createDirectoryEntry(BootSector* bootSector, char* directoryName, uint8_t directoryAttribute, uint32_t firstCluster, DirectoryEntry* directoryEntry); //this could be used for any dir entry, both files and directory
static int addDirectoryEntryToParentDirectory(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* parentDirectoryEntry, char* newDirectoryName, uint32_t firstEmptyCluster);
static int setupFirstClusterInDirectory(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* parentDirectoryEntry, uint32_t clusterNumber);

#endif
