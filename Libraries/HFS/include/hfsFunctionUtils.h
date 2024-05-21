#include "../include/disk.h"
#include "../include/structures.h"

#ifndef HFS__HFSFUNCTIONUTILS_H
#define HFS__HFSFUNCTIONUTILS_H

uint32_t getMaximumNumberOfRecordsPerCatalogFileNode();

uint32_t getMaximumNumberOfRecordsPerExtentsFileNode();

uint32_t getNumberOfSectorsPerBlock(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader);

///////////////

uint32_t getFirstSectorForGivenBlock(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, uint32_t block);

//////////////////

uint32_t getFirstBlockForAllocationFile(HFSPlusVolumeHeader* volumeHeader);

uint32_t getFirstBlockForVolumeHeader(uint32_t blockSize);

uint32_t changeBlockAllocationInAllocationFile(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, uint32_t blockGlobalIndex, uint8_t newValue);

void updateCatalogDirectoryRecordCreatedDateAndTime(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogDirectoryRecord* directoryRecord, uint32_t nodeOfRecord);

void updateCatalogDirectoryRecordLastAccessedDateAndTime(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogDirectoryRecord* directoryRecord, uint32_t nodeOfRecord);

void updateCatalogDirectoryRecordLastModifiedDateAndTime(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogDirectoryRecord* directoryRecord, uint32_t nodeOfRecord);

#endif
