#include "disk.h"
#include "structures.h"

#ifndef HFS__HFS_H
#define HFS__HFS_H

uint32_t writeBytesToFileWithTruncate(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogDirectoryRecord* fileRecord, CatalogFileHeaderNode* catalogFileHeaderNode,
                                      ExtentsFileHeaderNode* extentsFileHeaderNode, char* dataBuffer, uint32_t maxBytesToWrite, uint32_t& numberOfBytesWritten,
                                      uint32_t& reasonForIncompleteWrite, uint32_t nodeNumberOfRecord, std::vector<HFSPlusExtentDescriptor*> foundExtents);

//CRITICAL
void updateVolumeHeaderNodeOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, HFSPlusVolumeHeader* updatedVolumeHeader);

/////////////////////////////////////

//Writes data to extent and also marks its blocks as occupied. //CAUTION The provided data block should be the size of FULL SECTORS (in order to prevent overflows)
static uint32_t writeDataToExtent(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, HFSPlusExtentDescriptor* extent, char* data);

static uint32_t searchFreeExtentOfGivenNumberOfBlocks(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, uint32_t numberOfBlocksInExtent, HFSPlusExtentDescriptor* foundExtent);

uint32_t setExtentsForDirectoryRecord(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, CatalogDirectoryRecord* fileRecord,
                                      std::vector<HFSPlusExtentDescriptor*> extents, uint32_t nodeNumberOfRecord);

#endif