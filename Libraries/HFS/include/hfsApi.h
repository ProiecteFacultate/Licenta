#include "vector"

#include "../include/disk.h"
#include "../include/hfsStructures.h"

#ifndef HFS__HFSAPI_H
#define HFS__HFSAPI_H

uint32_t createDirectory(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, char* directoryParentPath,
                         char* newDirectoryName, int16_t newDirectoryType);

uint32_t getSubDirectoriesByParentPath(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode,
                                       char* directoryPath, std::vector<CatalogDirectoryRecord*>& subDirectories);

uint32_t write(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, ExtentsFileHeaderNode* extentsFileHeaderNode,
               char* directoryPath, char* dataBuffer, uint32_t maxBytesToWrite, uint32_t& numberOfBytesWritten, uint32_t writeAttribute, uint32_t& reasonForIncompleteWrite);

uint32_t read(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, ExtentsFileHeaderNode* extentsFileHeaderNode,
              char* filePath, char* readBuffer, uint32_t startingPosition, uint32_t maxBytesToRead, uint32_t& numberOfBytesRead, uint32_t& reasonForIncompleteRead);

uint32_t truncate(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, ExtentsFileHeaderNode* extentsFileHeaderNode,
                  char* filePath, uint32_t newSize);

uint32_t deleteDirectoryByPath(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, ExtentsFileHeaderNode* extentsFileHeaderNode,
                               char* directoryPath);

uint32_t getDirectoryDisplayableAttributes(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode,
                                           char* directoryPath, HFSDirectoryDisplayableAttributes* attributes);

#endif
