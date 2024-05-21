#include "../include/disk.h"
#include "../../include/structures.h"

#ifndef HFS__CATALOGFILEUTILS_H
#define HFS__CATALOGFILEUTILS_H

//Returns positive value if first key is higher, or negative if second is higher
int32_t cf_compareKeys(HFSPlusCatalogKey* key1, HFSPlusCatalogKey* key2);

//Receiving the actual catalogFileHeaderNode and a node number, it updates the value in the map record bitmap, and returns a pointer to a new CatalogFileHeaderNode containing the new
//updated version CAUTION this method just creates the new updated CatalogFileHeaderNode structure, it does not write it on the disk
CatalogFileHeaderNode* cf_updateNodeOccupiedInHeaderNodeMapRecord(CatalogFileHeaderNode* catalogFileHeaderNode, uint32_t nodeNumber, uint8_t newValue);

CatalogDirectoryRecord* cf_createDirectoryRecord(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogDirectoryRecord* parentRecord, char* directoryName,
                                                 int16_t fileType);

uint32_t cf_getNumberOfBlocksPerNode(HFSPlusVolumeHeader* volumeHeader);

uint32_t cf_getNumberOfSectorsPerNode(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader);

uint32_t cf_getFirstBlockForGivenNodeIndex(HFSPlusVolumeHeader* volumeHeader, uint32_t nodeNumber);

uint32_t cf_getSectorForGivenNodeIndex(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, uint32_t nodeNumber);

#endif
