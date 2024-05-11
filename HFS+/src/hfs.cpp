#include "iostream"
#include "string.h"

#include "disk.h"
#include "diskCodes.h"
#include "../include/structures.h"
#include "../include/utils.h"
#include "../include/hfsFunctionUtils.h"
#include "../include/hfs.h"

void updateVolumeHeaderNodeOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, HFSPlusVolumeHeader* updatedVolumeHeader)
{
    uint32_t numberOfSectorsWritten, retryWriteCount = 2;
    uint32_t sector = getFirstSectorForGivenBlock(diskInfo, volumeHeader, getFirstBlockForVolumeHeader(volumeHeader->blockSize));
    char* blockBuffer = new char[volumeHeader->blockSize];

    if(diskInfo->diskParameters.sectorSizeBytes <= volumeHeader->blockSize) //if blockSize is 512 or 1024 we write from the beginning of block
        memcpy(blockBuffer, updatedVolumeHeader, sizeof(HFSPlusVolumeHeader));
    else
        memcpy(blockBuffer + 1024, updatedVolumeHeader, sizeof(HFSPlusVolumeHeader));

    uint32_t writeResult = writeDiskSectors(diskInfo, 1, sector, blockBuffer, numberOfSectorsWritten);

    while(writeResult != EC_NO_ERROR && retryWriteCount > 0)
    {
        writeResult = writeDiskSectors(diskInfo, 1, sector, blockBuffer, numberOfSectorsWritten);
        retryWriteCount--;
    }

    if(retryWriteCount == 0)
        throw std::runtime_error("Failed to initialize volume header!");

    delete[] blockBuffer;
}