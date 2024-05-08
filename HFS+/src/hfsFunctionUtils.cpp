#include "windows.h"
#include "string.h"
#include "cstdint"

#include "../include/disk.h"
#include "../include/diskCodes.h"
#include "../include/structures.h"
#include "../include/utils.h"
#include "../include/hfsFunctionUtils.h"

uint32_t getMaximumNumberOfRecordsPerCatalogFileNode()
{
    uint32_t catalogFileNodeSize = getCatalogFileNodeSize();
    int records = 1;

    while(true)
    {
        if(sizeof(BTNodeDescriptor) + records * sizeof(CatalogDirectoryRecord) + (records + 1) * sizeof(NextNodeInfo) > catalogFileNodeSize)
            break;

        records++;
    }

    return records - 1;
}

uint32_t getNumberOfSectorsPerBlock(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader)
{
    return volumeHeader->blockSize / diskInfo->diskParameters.sectorSizeBytes;
}

uint32_t getFirstSectorForGivenBlock(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, uint32_t block)
{
    uint32_t sectorsPerBlock = volumeHeader->blockSize / diskInfo->diskParameters.sectorSizeBytes;
    return block * sectorsPerBlock;
}

uint32_t getFirstBlockForAllocationFile(HFSPlusVolumeHeader* volumeHeader)
{
    return 1024 / volumeHeader->blockSize + 1;
}