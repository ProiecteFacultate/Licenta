#include "../include/disk.h"
#include "../include/structures.h"

#ifndef HFS__HFSFUNCTIONUTILS_H
#define HFS__HFSFUNCTIONUTILS_H

uint32_t getMaximumNumberOfRecordsPerCatalogFileNode();

uint32_t getNumberOfSectorsPerBlock(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader);

///////////////

uint32_t getFirstSectorForGivenBlock(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, uint32_t block);

//////////////////

uint32_t getFirstBlockForAllocationFile(HFSPlusVolumeHeader* volumeHeader);

#endif
