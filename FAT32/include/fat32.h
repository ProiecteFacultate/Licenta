#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include/diskUtils.h"
#include "../include/fat32Init.h"

#ifndef FAT32_FAT32_H
#define FAT32_FAT32_H

//Major functions used by the file system public api

//Being given a cluster number, looks into fat for the next cluster (if any), in chain
uint32_t getNextCluster(DiskInfo* diskInfo, BootSector* bootSector, uint32_t actualCluster, uint32_t& nextCluster);

//Being given a directory name to look for, and its parent's directory entry, looks for the directory name in its parent's cluster(S)
uint32_t findDirectoryEntryByDirectoryName(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* parentDirectoryEntry, char* searchedDirectoryName,
                                                  DirectoryEntry** searchedDirectoryEntry);

//Being given a directory name to look for, and a buffer containing a cluster's data, looks for the directory name in that buffer
uint32_t findDirectoryEntryInGivenClusterData(BootSector* bootSector, char* clusterData, char* directoryName, DirectoryEntry** directoryEntry);

//Looks in fat for an empty cluster
uint32_t searchEmptyCluster(DiskInfo* diskInfo, BootSector* bootSector, uint32_t& emptyClusterNumber);

//Changes the value for given cluster in fat
uint32_t updateFat(DiskInfo* diskInfo, BootSector* bootSector, uint32_t clusterNumber, char* value);

//Being given a buffer containing a cluster's data, find the first place where we could place a new directory entry
uint32_t findFirstFreeOffsetInGivenClusterData(BootSector* bootSector, char* clusterData, uint32_t& firstFreeOffset);

//Populates a given directory entry with data at creation
void createDirectoryEntry(char* directoryName, uint8_t directoryAttribute, uint32_t firstCluster, DirectoryEntry* directoryEntry);

//When creating a new directory, we need to add a directory entry for this new directory in its parent cluster(S)
uint32_t addDirectoryEntryToParent(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* parentDirectoryEntry, char* newDirectoryName,
                                                   uint32_t firstEmptyCluster, DirectoryEntry* newDirectoryEntry);

//For ann existing directory, adds a new free cluster, if found
uint32_t addNewClusterToDirectory(DiskInfo* diskInfo, BootSector* bootSector, uint32_t lastClusterInDirectory, uint32_t& newCluster);

//Add dot & dotdot directory entries when creating a new directory
uint32_t setupFirstClusterInDirectory(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* parentDirectoryEntry, uint32_t clusterNumber, DirectoryEntry* newDirectoryEntry);


#endif
