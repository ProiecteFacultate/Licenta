#include "disk.h"
#include "structures.h"

#ifndef HFS__HFS_H
#define HFS__HFS_H

uint32_t writeBytesToFileWithTruncate(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogDirectoryRecord* fileRecord, ExtentsFileHeaderNode* extentsFileHeaderNode,
                                      char* dataBuffer, uint32_t maxBytesToWrite, uint32_t& numberOfBytesWritten, uint32_t& reasonForIncompleteWrite,
                                      std::vector<HFSPlusExtentDescriptor*>& foundExtents);

uint32_t writeBytesToFileWithAppend(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogDirectoryRecord* fileRecord, ExtentsFileHeaderNode* extentsFileHeaderNode,
                                    char* dataBuffer, uint32_t maxBytesToWrite, uint32_t& numberOfBytesWritten, uint32_t& reasonForIncompleteWrite,
                                    std::vector<HFSPlusExtentDescriptor*>& newExtents, uint32_t& numberOfAlreadyExistingExtents);

//CRITICAL
void updateVolumeHeaderNodeOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, HFSPlusVolumeHeader* updatedVolumeHeader);

/////////////////////////////////////

//Writes data to extent and also marks its blocks as occupied. //CAUTION The provided data block should be the size of FULL SECTORS (in order to prevent overflows)
static uint32_t writeDataToExtent(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, HFSPlusExtentDescriptor* extent, char* data);

static uint32_t searchFreeExtentOfGivenNumberOfBlocks(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, uint32_t numberOfBlocksInExtent, HFSPlusExtentDescriptor* foundExtent);

uint32_t setExtentsForDirectoryRecord(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsFileHeaderNode* extentsFileHeaderNode, CatalogDirectoryRecord* fileRecord,
                                      std::vector<HFSPlusExtentDescriptor*> extents, uint32_t nodeNumberOfRecord);

uint32_t addExtentsToDirectoryRecord(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsFileHeaderNode* extentsFileHeaderNode, CatalogDirectoryRecord* fileRecord,
                                     std::vector<HFSPlusExtentDescriptor*> newExtents, uint32_t nodeNumberOfRecord, uint32_t& numberOfAlreadyExistingExtents);

//extentsDirectoryRecords are returned for extents records that are found in extents overflow file, so their number will be extents - 8
//HIDDEN FEATURE The extents are returned sorted
uint32_t getAllExtentsForGivenDirectoryRecord(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsFileHeaderNode* extentsFileHeaderNode,
                                              CatalogDirectoryRecord* fileRecord, std::vector<HFSPlusExtentDescriptor*>& extents,
                                              std::vector<ExtentsDirectoryRecord*>& extentsDirectoryRecords);

uint32_t deleteDirectoryAndChildren(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode,
                                    ExtentsFileHeaderNode* extentsFileHeaderNode, CatalogDirectoryRecord* catalogDirectoryRecordToDelete);

//This method deletes all extents of a directory, the original 8 ones + the ones in extent overflow (deallocate their blocks & for the ones in extent overflow deletes the record)
//CAUTION this deletes all extents, but doesn't update catalogData.totalNumOfExtents or fileSize of the directory record
static uint32_t clearDirectoryRecordData(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsFileHeaderNode* extentsFileHeaderNode,
                                         CatalogDirectoryRecord* catalogDirectoryRecordToDelete);

//This uses clearDirectoryRecordData for extents clearing & then deletes the directory record (to completely delete the directory)
static uint32_t deleteDirectoryRecordAndAllItsRelatedData(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode,
                                                   ExtentsFileHeaderNode* extentsFileHeaderNode, CatalogDirectoryRecord* catalogDirectoryRecordToDelete);

#endif