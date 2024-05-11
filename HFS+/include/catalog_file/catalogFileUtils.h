#include "../include/disk.h"
#include "../../include/structures.h"

#ifndef HFS__CATALOGFILEUTILS_H
#define HFS__CATALOGFILEUTILS_H

//Returns positive value if first key is higher, or negative if second is higher
int32_t compareKeys(HFSPlusCatalogKey* key1, HFSPlusCatalogKey* key2);

//Receiving the actual catalogFileHeaderNode and a node number, it updates the value in the map record bitmap, and returns a pointer to a new CatalogFileHeaderNode containing the new
//updated version CAUTION this method just creates the new updated CatalogFileHeaderNode structure, it does not write it on the disk
CatalogFileHeaderNode* updateNodeOccupiedInHeaderNodeMapRecord(CatalogFileHeaderNode* catalogFileHeaderNode, uint32_t nodeNumber, uint8_t newValue);

uint32_t getNumberOfBlocksPerNode(HFSPlusVolumeHeader* volumeHeader);

uint32_t getNumberOfSectorsPerNode(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader);

uint32_t getFirstBlockForGivenNodeIndex(HFSPlusVolumeHeader* volumeHeader, uint32_t nodeNumber);

uint32_t getSectorForGivenNodeIndex(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, uint32_t nodeNumber);

#endif