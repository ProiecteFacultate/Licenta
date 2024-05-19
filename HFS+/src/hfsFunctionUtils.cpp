#include "windows.h"
#include "string.h"
#include "cstdint"

#include "../include/disk.h"
#include "../include/diskCodes.h"
#include "../include/structures.h"
#include "../include/utils.h"
#include "../include/codes/hfsCodes.h"
#include "../include/hfsFunctionUtils.h"

uint32_t getMaximumNumberOfRecordsPerCatalogFileNode()
{
    uint32_t catalogFileNodeSize = getCatalogFileNodeSize();
    int records = 1;

    while(true)
    {
        if(sizeof(BTNodeDescriptor) + records * sizeof(CatalogDirectoryRecord) + (records + 1) * sizeof(ChildNodeInfo) > catalogFileNodeSize)
            break;

        records++;
    }

    if(records % 2 == 1) //we want an odd number of max records per node
        records--;

    return records - 1;
}

uint32_t getMaximumNumberOfRecordsPerExtentsFileNode()
{
    uint32_t extentsFileNodeSize = getExtentsOverflowFileNodeSize();
    int records = 1;

    while(true)
    {
        if(sizeof(BTNodeDescriptor) + records * sizeof(ExtentsDirectoryRecord) + (records + 1) * sizeof(ChildNodeInfo) > extentsFileNodeSize)
            break;

        records++;
    }

    if(records % 2 == 1) //we want an odd number of max records per node
        records--;

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

uint32_t getFirstBlockForVolumeHeader(uint32_t blockSize)
{
    switch (blockSize) {
        case 512:
            return 2;
        case 1024:
            return 1;
        default:
            return 0;
    }
}

uint32_t changeBlockAllocationInAllocationFile(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, uint32_t blockGlobalIndex, uint8_t newValue)
{
    uint32_t numOfSectorsRead, numOfSectorsWritten;

    uint32_t firstDataBlockGlobalIndex = volumeHeader->catalogFile.extents[0].startBlock + volumeHeader->catalogFile.extents[0].blockCount;
    uint32_t blockLocalIndexInDataBlocks = blockGlobalIndex - firstDataBlockGlobalIndex;

    uint32_t byteIndexInBlock = (blockLocalIndexInDataBlocks / 8) % volumeHeader->blockSize;
    uint32_t bitIndex = blockLocalIndexInDataBlocks % 8;
    uint32_t blockToUpdate = volumeHeader->allocationFile.extents[0].startBlock + blockLocalIndexInDataBlocks / (volumeHeader->blockSize * 8);

    char* blockBuffer = new char[volumeHeader->blockSize];
    uint32_t readResult = readDiskSectors(diskInfo, getNumberOfSectorsPerBlock(diskInfo, volumeHeader),
                                 getFirstSectorForGivenBlock(diskInfo, volumeHeader, blockToUpdate), blockBuffer, numOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        delete[] blockBuffer;
        return CHANGE_BLOCK_ALLOCATION_FAILED;
    }

    uint8_t newByteValue = changeBitValue(blockBuffer[byteIndexInBlock], bitIndex, newValue);
    blockBuffer[byteIndexInBlock] = newByteValue;

    uint32_t writeResult = writeDiskSectors(diskInfo, getNumberOfSectorsPerBlock(diskInfo, volumeHeader),
                                          getFirstSectorForGivenBlock(diskInfo, volumeHeader, blockToUpdate), blockBuffer, numOfSectorsWritten);

    delete[] blockBuffer;
    return (writeResult == EC_NO_ERROR) ? CHANGE_BLOCK_ALLOCATION_SUCCESS : CHANGE_BLOCK_ALLOCATION_FAILED;
}