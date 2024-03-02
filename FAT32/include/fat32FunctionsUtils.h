#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include/diskUtils.h"
#include "../include/fat32Init.h"

#ifndef FAT32_FAT32FUNCTIONSUTILS_H
#define FAT32_FAT32FUNCTIONSUTILS_H

//Minor helper function

bool checkDirectoryNameValidity(const char* directoryName);

bool compareDirectoryNames(char* expected, const char* actual);

void formatDirectoryName(const char* directoryName, char* formattedName);

uint16_t getFirstFatSector(BootSector* bootSector);

uint32_t getFirstSectorForCluster(BootSector* bootSector, uint32_t cluster);

//Gets number of DATA clusters ignoring the fact that root sector is 2nd sector
uint32_t getTotalNumberOfDataClusters(DiskInfo* diskInfo, BootSector* bootSector);

void copyNewDirectoryTimeToDotDirectoryEntries(DirectoryEntry* newDirectoryEntry, DirectoryEntry* dotDirectoryEntry, DirectoryEntry* dotDotDirectoryEntry);

#endif
