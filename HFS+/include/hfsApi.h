#include "../include/disk.h"
#include "../include/structures.h"

#ifndef HFS__HFSAPI_H
#define HFS__HFSAPI_H

uint32_t createDirectory(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, char* directoryParentPath,
                         char* newDirectoryName, int16_t newDirectoryType);

uint32_t getSubDirectoriesByParentPath(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode,
                                       char* directoryPath, std::vector<CatalogDirectoryRecord*>& subDirectories);

#endif
