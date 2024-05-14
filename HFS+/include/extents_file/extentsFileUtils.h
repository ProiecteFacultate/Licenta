#ifndef HFS__EXTENTSFILEUTILS_H
#define HFS__EXTENTSFILEUTILS_H

//Returns positive value if first key is higher, or negative if second is higher
int32_t eof_compareKeys(ExtentsFileCatalogKey* key1, ExtentsFileCatalogKey* key2);

//Receiving the actual extentsFileHeaderNode and a node number, it updates the value in the map record bitmap, and returns a pointer to a new ExtentsFileHeaderNode containing
// the new updated version CAUTION this method just creates the new updated CatalogFileHeaderNode structure, it does not write it on the disk
ExtentsFileHeaderNode* eof_updateNodeOccupiedInHeaderNodeMapRecord(ExtentsFileHeaderNode* extentsFileHeaderNode, uint32_t nodeNumber, uint8_t newValue);

ExtentsDirectoryRecord* eof_createDirectoryRecord(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, uint32_t extentOverflowIndex);

uint32_t eof_getNumberOfBlocksPerNode(HFSPlusVolumeHeader* volumeHeader);

uint32_t eof_getNumberOfSectorsPerNode(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader);

uint32_t eof_getFirstBlockForGivenNodeIndex(HFSPlusVolumeHeader* volumeHeader, uint32_t nodeNumber);

uint32_t eof_getSectorForGivenNodeIndex(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, uint32_t nodeNumber);

#endif
