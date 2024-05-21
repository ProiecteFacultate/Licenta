#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include/diskUtils.h"
#include "../include/structures.h"
#include "../include/fat32Init.h"

#ifndef FAT32_FAT32FUNCTIONUTILS_H
#define FAT32_FAT32FUNCTIONUTILS_H

//Helper function that know about the file system (they have access to structures like DiskInfo, BootSector, DirectoryEntry

uint32_t getClusterSize(BootSector* bootSector);

uint16_t getFirstFatSector(BootSector* bootSector);

uint32_t getFirstClusterForDirectory(BootSector* bootSector, DirectoryEntry* directoryEntry);

uint32_t getFirstSectorForCluster(BootSector* bootSector, uint32_t cluster);

//Gets number of DATA clusters ignoring the fact that root sector is 2nd sector
uint32_t getTotalNumberOfDataClusters(DiskInfo* diskInfo, BootSector* bootSector);

//Being given a cluster number, looks into fat for the next cluster (if any), in chain. If a next cluster is found 'nextCluster' parameter will return its value, otherwise will
//return 'actualCluster' value
uint32_t getNextCluster(DiskInfo* diskInfo, BootSector* bootSector, uint32_t actualCluster, uint32_t& nextCluster);

void copyNewDirectoryTimeToDotDirectoryEntries(DirectoryEntry* newDirectoryEntry, DirectoryEntry* dotDirectoryEntry, DirectoryEntry* dotDotDirectoryEntry);

//Being given the first cluster (of a directory), find the n-th cluster (if exists) of that directory. If the n-th cluster exists 'foundClusterNumber' will return its value
//n is indexed from 0, so the first cluster of the directory is 0!
//HIDDEN FEATURE otherwise will return the last value found, so the value of the last cluster of the directory
uint32_t findNthClusterInChain(DiskInfo* diskInfo, BootSector* bootSector, uint32_t firstCluster, uint32_t n, uint32_t& foundClusterNumber);

//Populates a given directory entry with data at creation
void createDirectoryEntry(char* directoryName, uint8_t directoryAttribute, uint32_t firstCluster, DirectoryEntry* directoryEntry);

//Being given a directory entry, traverse all its subdirectories, and update their dotdot entry (used when a perent directory changes its file size and want its subdirectories to know)
uint32_t updateSubDirectoriesDotDotEntries(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* givenDirectoryEntry, DirectoryEntry* newDotDotEntry);

//Being given a directory FULL path, returns its corresponding directory entry
//CAUTION this method requires only the directory path (don't confuse with findDirectoryEntryByDirectoryName)
uint32_t findDirectoryEntryByFullPath(DiskInfo* diskInfo, BootSector* bootSector, char* directoryPath, DirectoryEntry** directoryEntry);

//Being given a cluster, iterate through the chain setting their corresponding fat value to FREE
uint32_t freeClustersInChainStartingWithGivenCluster(DiskInfo* diskInfo, BootSector* bootSector, uint32_t startingClusterNumber);

#endif //FAT32_FAT32FUNCTIONUTILS_H
