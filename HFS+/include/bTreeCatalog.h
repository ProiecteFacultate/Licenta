#include "../include/disk.h"
#include "../include/structures.h"

#ifndef HFS__BTREECATALOG_H
#define HFS__BTREECATALOG_H

/////////////////////////

uint32_t readNodeFromDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, char* nodeBuffer, uint32_t nodeNumber);

uint32_t searchRecordForGivenNodeDataAndParentRecord(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogDirectoryRecord* parentRecord, char* nodeData,
                                                     CatalogDirectoryRecord* searchedRecord);

uint32_t updateHeaderNodeOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* updatedCatalogFileHeaderNode);

////////////////////////////

//Returns positive value if first key is higher, or negative if second is higher
static int32_t compareKeys(HFSPlusCatalogKey* key1, HFSPlusCatalogKey* key2);

static uint32_t getNumberOfBlocksPerNode(HFSPlusVolumeHeader* volumeHeader);

static uint32_t getFirstBlockForGivenNodeIndex(HFSPlusVolumeHeader* volumeHeader, uint32_t nodeNumber);

#endif
