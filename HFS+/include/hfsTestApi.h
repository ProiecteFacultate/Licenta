#include "vector"
#include "string"

#include "diskUtils.h"
#include "../include/structures.h"

#ifndef HFS__HFSTESTAPI_H
#define HFS__HFSTESTAPI_H

//type: 0 - Folder, 1 - File
uint32_t hfs_create_directory(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, char* parentPath, char* directoryName,
                              int16_t type, int64_t& timeElapsedMilliseconds);

uint32_t hfs_get_subdirectories(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, char* directoryPath,
                                std::vector<CatalogDirectoryRecord*>& subDirectories, int64_t& timeElapsedMilliseconds);

//writeMode: 0 - TRUNCATE, 1 - APPEND
uint32_t hfs_write_file(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, ExtentsFileHeaderNode* extentsFileHeaderNode, char* filePath,
                        char* buffer, uint32_t bytesToWrite, uint32_t writeMode, uint32_t& numberOfBytesWritten, uint32_t& reasonForIncompleteWrite, int64_t& timeElapsedMilliseconds);

uint32_t hfs_read_file(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, ExtentsFileHeaderNode* extentsFileHeaderNode, char* filePath,
                        char* buffer, uint32_t bytesToRead, uint32_t startingPosition, uint32_t& numberOfBytesRead, uint32_t& reasonForIncompleteRead, int64_t& timeElapsedMilliseconds);

uint32_t hfs_truncate_file(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, ExtentsFileHeaderNode* extentsFileHeaderNode, char* filePath,
                           uint32_t newSize, int64_t& timeElapsedMilliseconds);

uint32_t hfs_delete_directory(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, ExtentsFileHeaderNode* extentsFileHeaderNode, char* directoryPath,
                              int64_t& timeElapsedMilliseconds);

uint32_t hfs_get_directory_attributes(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, char* directoryPath, HFSDirectoryDisplayableAttributes* attributes,
                                      int64_t& timeElapsedMilliseconds);

#endif
