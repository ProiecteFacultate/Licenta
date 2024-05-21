#include "vector"
#include "string"

#include "diskUtils.h"
#include "../include/structures.h"

#ifndef EXT2_EXT2TESTAPI_H
#define EXT2_EXT2TESTAPI_H

//type: 0 - Folder, 1 - File
uint32_t ext2_create_directory(DiskInfo* diskInfo, ext2_super_block* superBlock, char* parentPath, char* directoryName, int16_t type, int64_t& timeElapsedMilliseconds);

uint32_t ext2_get_subdirectories(DiskInfo* diskInfo, ext2_super_block* superBlock, char* directoryPath, std::vector<std::pair<ext2_inode*, ext2_dir_entry*>>& subDirectories,
                                int64_t& timeElapsedMilliseconds);

//writeMode: 0 - TRUNCATE, 1 - APPEND
uint32_t ext2_write_file(DiskInfo* diskInfo, ext2_super_block* superBlock, char* filePath, char* buffer, uint32_t bytesToWrite, uint32_t writeMode, uint32_t& numberOfBytesWritten,
                         uint32_t& reasonForIncompleteWrite, int64_t& timeElapsedMilliseconds);

uint32_t ext2_read_file(DiskInfo* diskInfo, ext2_super_block* superBlock, char* filePath, char* buffer, uint32_t bytesToRead, uint32_t startingPosition, uint32_t& numberOfBytesRead,
                        uint32_t& reasonForIncompleteRead, int64_t& timeElapsedMilliseconds);

uint32_t ext2_truncate_file(DiskInfo* diskInfo, ext2_super_block* superBlock, char* filePath, uint32_t newSize, int64_t& timeElapsedMilliseconds);

uint32_t ext2_delete_directory(DiskInfo* diskInfo, ext2_super_block* superBlock, char* directoryPath, int64_t& timeElapsedMilliseconds);

uint32_t ext2_get_directory_attributes(DiskInfo* diskInfo, ext2_super_block* superBlock, char* directoryPath, Ext2DirectoryDisplayableAttributes* attributes,
                                       int64_t& timeElapsedMilliseconds);

#endif
