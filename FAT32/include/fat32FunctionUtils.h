#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include/diskUtils.h"
#include "../include/fat32Init.h"

#ifndef FAT32_FAT32FUNCTIONUTILS_H
#define FAT32_FAT32FUNCTIONUTILS_H


uint16_t getFirstFatSector(BootSector* bootSector);

uint32_t getFirstSectorForCluster(BootSector* bootSector, uint32_t cluster);

//Gets number of DATA clusters ignoring the fact that root sector is 2nd sector
uint32_t getTotalNumberOfDataClusters(DiskInfo* diskInfo, BootSector* bootSector);

//Being given a cluster number, looks into fat for the next cluster (if any), in chain
uint32_t getNextCluster(DiskInfo* diskInfo, BootSector* bootSector, uint32_t actualCluster, uint32_t& nextCluster);

void copyNewDirectoryTimeToDotDirectoryEntries(DirectoryEntry* newDirectoryEntry, DirectoryEntry* dotDirectoryEntry, DirectoryEntry* dotDotDirectoryEntry);

//Being given the first cluster (of a directory), find the n-th cluster (if exists) of that directory
uint32_t findNthClusterInChain(DiskInfo* diskInfo, BootSector* bootSector, uint32_t firstCluster, uint32_t n, uint32_t& foundClusterNumber);

//Populates a given directory entry with data at creation
void createDirectoryEntry(char* directoryName, uint8_t directoryAttribute, uint32_t firstCluster, DirectoryEntry* directoryEntry);

#endif //FAT32_FAT32FUNCTIONUTILS_H
