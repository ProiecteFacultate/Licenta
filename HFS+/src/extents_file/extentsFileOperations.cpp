#include "iostream"
#include "string.h"

#include "../include/disk.h"
#include "../include/diskCodes.h"
#include "../../include/structures.h"
#include "../../include/utils.h"
#include "../../include/hfsFunctionUtils.h"
#include "../../include/extents_file/codes/bTreeResponseCodes.h"
#include "../../include/extents_file/codes/extentsFileResponseCodes.h"
#include "../../include/codes/hfsAttributes.h"
#include "../../include/extents_file/bTreeCatalog.h"
#include "../../include/extents_file/extentsFileUtils.h"
#include "../../include/extents_file/extentsFileOperations.h"

void eof_updateExtentsHeaderNodeOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsFileHeaderNode* updatedExtentsFileHeaderNode)
{
    uint32_t numberOfSectorsWritten, retryWriteCount = 2;
    uint32_t numOfSectorsToWrite = eof_getNumberOfBlocksPerNode(volumeHeader) * getNumberOfSectorsPerBlock(diskInfo, volumeHeader);
    uint32_t firstSectorForExtentsFile = volumeHeader->extentsFile.extents[0].startBlock * getNumberOfSectorsPerBlock(diskInfo, volumeHeader);
    char* nodeBuffer = new char[getExtentsOverflowFileNodeSize()];
    memcpy(nodeBuffer, updatedExtentsFileHeaderNode, sizeof(ExtentsFileHeaderNode));

    uint32_t writeResult = writeDiskSectors(diskInfo, numOfSectorsToWrite, firstSectorForExtentsFile, nodeBuffer, numberOfSectorsWritten);

    while(writeResult != EC_NO_ERROR && retryWriteCount > 0)
    {
        writeResult = writeDiskSectors(diskInfo, numOfSectorsToWrite, firstSectorForExtentsFile, nodeBuffer, numberOfSectorsWritten);
        retryWriteCount--;
    }

    if(retryWriteCount == 0)
        throw std::runtime_error("FATAL ERROR: Update extents header node on disk failed!");

    delete[] nodeBuffer;
}

void eof_updateNodeOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, char* updatedNodeData, uint32_t nodeNumber)
{
    uint32_t numberOfSectorsWritten, retryWriteCount = 2;
    uint32_t numOfSectorsToWrite = eof_getNumberOfBlocksPerNode(volumeHeader) * getNumberOfSectorsPerBlock(diskInfo, volumeHeader);
    uint32_t firstBlockForNode = eof_getFirstBlockForGivenNodeIndex(volumeHeader, nodeNumber);
    uint32_t firstSectorForGivenNode = getFirstSectorForGivenBlock(diskInfo, volumeHeader, firstBlockForNode);

    uint32_t writeResult = writeDiskSectors(diskInfo, numOfSectorsToWrite, firstSectorForGivenNode, updatedNodeData, numberOfSectorsWritten);

    while(writeResult != EC_NO_ERROR && retryWriteCount > 0)
    {
        writeResult = writeDiskSectors(diskInfo, numOfSectorsToWrite, firstSectorForGivenNode, updatedNodeData, numberOfSectorsWritten);
        retryWriteCount--;
    }

    if(retryWriteCount == 0)
        throw std::runtime_error("FATAL ERROR: Update extents node on disk failed!");
}

uint32_t eof_updateRecordOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsDirectoryRecord* directoryRecord,
                            ExtentsDirectoryRecord* updatedDirectoryRecord, uint32_t nodeNumberOfRecord)
{
    char* nodeData = new char[getExtentsOverflowFileNodeSize()];
    uint32_t readNodeFromDiskResult = eof_readNodeFromDisk(diskInfo, volumeHeader, nodeData, nodeNumberOfRecord);

    if(readNodeFromDiskResult == EOF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData;
        return EOF_UPDATE_RECORD_ON_DISK_FAILED;
    }

    BTNodeDescriptor* nodeDescriptor = (BTNodeDescriptor*)&nodeData[sizeof(BTNodeDescriptor)];

    for(uint32_t i = 0; i < nodeDescriptor->numRecords; i++)
    {
        uint32_t firstByteOfRecord = sizeof(BTNodeDescriptor) + i * sizeof(ExtentsDirectoryRecord);
        ExtentsDirectoryRecord* record = (ExtentsDirectoryRecord*)&nodeData[firstByteOfRecord];

        if(eof_compareKeys(&record->catalogKey, &directoryRecord->catalogKey) == 0)
        {
            memcpy(nodeData + firstByteOfRecord, updatedDirectoryRecord, sizeof(ExtentsDirectoryRecord));

            try {
                eof_updateNodeOnDisk(diskInfo, volumeHeader, nodeData, nodeNumberOfRecord);
            }
            catch(std::runtime_error& error){
                delete[] nodeData;
                return EOF_UPDATE_RECORD_ON_DISK_FAILED;
            }

            delete[] nodeData;
            return EOF_UPDATE_RECORD_ON_DISK_SUCCESS;
        }
    }
}