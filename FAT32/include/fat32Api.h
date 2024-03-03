#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <vector>

#include "../include/diskUtils.h"
#include "../include/fat32Init.h"

#ifndef FAT32_FAT32API_H
#define FAT32_FAT32API_H

//This is the public api for the file system

uint32_t createDirectory(DiskInfo* diskInfo, BootSector* bootSector, char* directoryParentPath, char* newDirectoryName);

//Being given a directory path, return all its subdirectories (folders and files)
uint32_t getSubDirectories(DiskInfo* diskInfo, BootSector* bootSector, char* directoryPath);

#endif
