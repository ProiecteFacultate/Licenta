#include "windows.h"
#include "string.h"
#include "cstdint"

#include "../include/disk.h"
#include "../include/diskCodes.h"
#include "../include/structures.h"
#include "../include/hfsFunctionUtils.h"

uint32_t getNumberOfSectorsPerBlock(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader)
{
    return volumeHeader->blockSize / diskInfo->diskParameters.sectorSizeBytes;
}

uint32_t getFirstSectorForGivenBlock(HFSPlusVolumeHeader* volumeHeader, uint32_t block)
{
    return block * volumeHeader->blockSize;
}

uint32_t getFirstBlockForAllocationFile(HFSPlusVolumeHeader* volumeHeader)
{
    return 1024 / volumeHeader->blockSize + 1;
}