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


//Being given a directory name to look for, and its parent's directory entry, looks for the directory name in its parent's cluster(S), returning its directory entry
uint32_t findDirectoryEntryByDirectoryName(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* parentDirectoryEntry, char* searchedDirectoryName,
                                                  DirectoryEntry** searchedDirectoryEntry);

//Being given a directory name to look for, and a buffer containing a cluster's data, looks for the directory name in that buffer
uint32_t findDirectoryEntryInGivenClusterData(BootSector* bootSector, char* clusterData, char* directoryName, DirectoryEntry** directoryEntry, uint32_t occupiedBytesInCluster,
                                              uint32_t& offset);

//Looks in fat for an empty cluster
uint32_t searchEmptyCluster(DiskInfo* diskInfo, BootSector* bootSector, uint32_t& emptyClusterNumber);

//Changes the value for given cluster in fat
uint32_t updateFat(DiskInfo* diskInfo, BootSector* bootSector, uint32_t clusterNumber, char* value);

//When creating a new directory, we need to add a directory entry for this new directory in its parent cluster(S)
uint32_t addDirectoryEntryToParent(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* parentDirectoryEntry, char* newDirectoryName,
                                                   uint32_t firstEmptyCluster, DirectoryEntry* newDirectoryEntry);

//For ann existing directory, adds a new free cluster, if found
uint32_t addNewClusterToDirectory(DiskInfo* diskInfo, BootSector* bootSector, uint32_t lastClusterInDirectory, uint32_t& newCluster);

//Add dot & dotdot directory entries when creating a new directory
uint32_t setupFirstClusterInDirectory(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* parentDirectoryEntry, uint32_t clusterNumber, DirectoryEntry* newDirectoryEntry);

//Given a directory entry, updated the entry in its parent, and also all its subdirectories dotdot
uint32_t updateDirectoryEntry(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* givenDirectoryEntry, DirectoryEntry* newDirectoryEntry);

#endif
