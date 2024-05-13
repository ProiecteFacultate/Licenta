#ifndef HFS__EXTENTSFILEUTILS_H
#define HFS__EXTENTSFILEUTILS_H

//Returns positive value if first key is higher, or negative if second is higher
int32_t eof_compareKeys(ExtentsFileCatalogKey* key1, ExtentsFileCatalogKey* key2);

ExtentsFileHeaderNode* eof_updateNodeOccupiedInHeaderNodeMapRecord(ExtentsFileHeaderNode* extentsFileHeaderNode, uint32_t nodeNumber, uint8_t newValue);

uint32_t eof_createDirectoryRecord(HFSPlusVolumeHeader* volumeHeader, ExtentsDirectoryRecord* parentRecord, ExtentsDirectoryRecord* createdRecord,
                               char* directoryName, uint32_t extentOverflowIndex);

uint32_t eof_getNumberOfBlocksPerNode(HFSPlusVolumeHeader* volumeHeader);

uint32_t eof_getNumberOfSectorsPerNode(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader);

uint32_t eof_getFirstBlockForGivenNodeIndex(HFSPlusVolumeHeader* volumeHeader, uint32_t nodeNumber);

uint32_t eof_getSectorForGivenNodeIndex(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, uint32_t nodeNumber);

#endif
