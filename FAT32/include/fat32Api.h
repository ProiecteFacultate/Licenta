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

uint32_t createDirectory(DiskInfo* diskInfo, BootSector* bootSector, char* directoryParentPath, char* newDirectoryName, uint32_t newDirectoryAttribute);

//Being given a directory path, return all its subdirectories (folders and files)
uint32_t getSubDirectories(DiskInfo* diskInfo, BootSector* bootSector, char* directoryPath, std::vector<DirectoryEntry*>& subDirectories);

//Being given a directory path, a buffer with data, and a max number of bytes, writes up to the max number of bytes.
//CAUTION if it writes less than max bytes, it will still return success. This could occur for example when there is insufficient space on disk
uint32_t write(DiskInfo* diskInfo, BootSector* bootSector, char* directoryPath, char* dataBuffer, uint32_t maxBytesToWrite, uint32_t& numberOfBytesWritten, uint32_t writeAttribute,
               uint32_t& reasonForIncompleteWrite);

//Being given a directory path, a buffer to read into, and a max number of bytes, read up to the max number of bytes
uint32_t read(DiskInfo* diskInfo, BootSector* bootSector, char* directoryPath, char* readBuffer, uint32_t maxBytesToRead, uint32_t& numberOfBytesRead, uint32_t& reasonForIncompleteRead);

uint32_t truncateFile(DiskInfo* diskInfo, BootSector* bootSector, char* directoryPath);

#endif
