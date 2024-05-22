#include "vector"
#include "string"

#include "diskUtils.h"
#include "../include/fat32Structures.h"

#ifndef FAT32_FAT32TESTAPI_H
#define FAT32_FAT32TESTAPI_H

//type: 0 - Folder, 1 - File
uint32_t fat32_create_directory(DiskInfo* diskInfo, BootSector* bootSector, char* parentPath, char* directoryName, int16_t type, int64_t& timeElapsedMilliseconds);

uint32_t fat32_get_subdirectories(DiskInfo* diskInfo, BootSector* bootSector, char* directoryPath, std::vector<std::pair<DirectoryEntry*, DirectorySizeDetails*>>& subDirectories,
                                int64_t& timeElapsedMilliseconds);
//writeMode: 0 - TRUNCATE, 1 - APPEND
uint32_t fat32_write_file(DiskInfo* diskInfo, BootSector* bootSector, char* filePath, char* buffer, uint32_t bytesToWrite, uint32_t writeMode, uint32_t& numberOfBytesWritten,
                        uint32_t& reasonForIncompleteWrite, int64_t& timeElapsedMilliseconds);

uint32_t fat32_read_file(DiskInfo* diskInfo, BootSector* bootSector, char* filePath, char* buffer, uint32_t bytesToRead, uint32_t startingPosition, uint32_t& numberOfBytesRead,
                       uint32_t& reasonForIncompleteRead, int64_t& timeElapsedMilliseconds);

uint32_t fat32_truncate_file(DiskInfo* diskInfo, BootSector* bootSector, char* filePath, uint32_t newSize, int64_t& timeElapsedMilliseconds);

uint32_t fat32_delete_directory(DiskInfo* diskInfo, BootSector* bootSector, char* directoryPath, int64_t& timeElapsedMilliseconds);

uint32_t fat32_get_directory_attributes(DiskInfo* diskInfo, BootSector* bootSector, char* directoryPath, Fat32DirectoryDisplayableAttributes* attributes, int64_t& timeElapsedMilliseconds);

#endif
