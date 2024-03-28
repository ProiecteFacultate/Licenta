#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <vector>

#include "../include/diskUtils.h"
#include "../include/structures.h"
#include "../include/fat32Init.h"

#ifndef FAT32_FAT32_H
#define FAT32_FAT32_H

//Major functions used by the file system public api


//Being given a directory name to look for, and its parent's directory entry, looks for the directory name in its parent's cluster(S), returning its directory entry
//CAUTION this method looks only in the given parent (don't confuse with findDirectoryEntryByFullPath)
uint32_t findDirectoryEntryByDirectoryName(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* parentDirectoryEntry, char* searchedDirectoryName,
                                           DirectoryEntry* searchedDirectoryEntry);

//Being given a directory name to look for, and a buffer containing a cluster's data, looks for the directory name in that buffer
uint32_t findDirectoryEntryInGivenClusterData(BootSector* bootSector, char* clusterData, char* directoryName, DirectoryEntry* directoryEntry, uint32_t occupiedBytesInCluster,
                                              uint32_t& offset);

//Looks in fat for an empty cluster
uint32_t searchEmptyCluster(DiskInfo* diskInfo, BootSector* bootSector, uint32_t& emptyClusterNumber);

//Changes the value for given cluster in fat
uint32_t updateFat(DiskInfo* diskInfo, BootSector* bootSector, uint32_t clusterNumber, char* value);

//When creating a new directory, we need to add a directory entry for this new directory in its parent cluster(S)
uint32_t addDirectoryEntryToParent(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* parentDirectoryEntry, char* newDirectoryName,
                                                   uint32_t firstEmptyCluster, DirectoryEntry* newDirectoryEntry, uint32_t newDirectoryAttribute);

//For ann existing directory, adds a new free cluster, if found
uint32_t addNewClusterToDirectory(DiskInfo* diskInfo, BootSector* bootSector, uint32_t lastClusterInDirectory, uint32_t& newCluster);

//Add dot & dotdot directory entries when creating a new directory
uint32_t setupFirstClusterInDirectory(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* parentDirectoryEntry, uint32_t clusterNumber, DirectoryEntry* newDirectoryEntry);

//Given a directory entry, updated the entry in its parent, and also all its subdirectories dotdot
uint32_t updateDirectoryEntry(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* givenDirectoryEntry, DirectoryEntry* newDirectoryEntry);

//Given a directory entry, a data buffer, and a max bytes to write, it writes up to max bytes in the cluster, OVERWRITING the existing data (dot & dotdot are not affected by overwrite)
//If the cluster contains more clusters then needed after write, it will set as free in the fat table the no longer used clusters
uint32_t writeBytesToFileWithTruncate(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* directoryEntry, char* dataBuffer, uint32_t maxBytesToWrite,
                                      uint32_t& numberOfBytesWritten, uint32_t& reasonForIncompleteWrite);

//Given a directory entry, a data buffer, and a max bytes to write, it writes up to max bytes in the cluster, APPENDING to the existing data
uint32_t writeBytesToFileWithAppend(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* directoryEntry, char* dataBuffer, uint32_t maxBytesToWrite,
                                    uint32_t& numberOfBytesWritten, uint32_t& reasonForIncompleteWrite);

uint32_t getSubDirectoriesByParentDirectoryEntry(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* parentDirectoryEntry, std::vector<DirectoryEntry*>& subDirectories);

uint32_t deleteDirectoryEntry(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* directoryEntry);

uint32_t deleteDirectoryEntryFromParent(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* givenDirectoryEntry, DirectoryEntry* parentDirectoryEntry);

//Given a directoryEntry, returns the file size & disk size of it and all of its direct & indirect children
//CAUTION it returns the sizes, but actually only for descendants, its size is not count
uint32_t getDirectoryFullByDirectoryEntry(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* directoryEntry, uint32_t& size, uint32_t& sizeOnDisk);

#endif
