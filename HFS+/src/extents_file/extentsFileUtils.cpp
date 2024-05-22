#include "string.h"

#include "../include/disk.h"
#include "../../include/hfsStructures.h"
#include "../../include/utils.h"
#include "../../include/hfs.h"
#include "../../include/hfsFunctionUtils.h"
#include "../../include/extents_file/extentsFileUtils.h"
#include "../../include/extents_file/codes/extentsFileResponseCodes.h"

int32_t eof_compareKeys(ExtentsFileCatalogKey * key1, ExtentsFileCatalogKey * key2)
{

    if((int) key1->fileId > (int) key2->fileId)
        return EOF_KEY_1_HIGHER;
    else if((int) key1->fileId < (int) key2->fileId)
        return EOF_KEY_2_HIGHER;

    if((int) key1->extentOverflowIndex > (int) key2->extentOverflowIndex)
        return EOF_KEY_1_HIGHER;
    else if((int) key1->extentOverflowIndex < (int) key2->extentOverflowIndex)
        return EOF_KEY_2_HIGHER;

    return EOF_KEYS_EQUAL;
}

ExtentsFileHeaderNode* eof_updateNodeOccupiedInHeaderNodeMapRecord(ExtentsFileHeaderNode* extentsFileHeaderNode, uint32_t nodeNumber, uint8_t newValue)
{
    ExtentsFileHeaderNode* updatedExtentsFileHeaderNode = new ExtentsFileHeaderNode ();
    memcpy(updatedExtentsFileHeaderNode, extentsFileHeaderNode, sizeof(ExtentsFileHeaderNode));

    uint32_t byteIndexForNodeNumber = nodeNumber / 8;
    uint32_t bitIndexForNodeNumber = nodeNumber % 8;

    uint8_t newByteValueForNodes = hfs_changeBitValue(byteIndexForNodeNumber, bitIndexForNodeNumber, newValue);
    updatedExtentsFileHeaderNode->mapRecordAndOffsets[byteIndexForNodeNumber] = newByteValueForNodes;

    return updatedExtentsFileHeaderNode;
}

ExtentsDirectoryRecord* eof_createDirectoryRecord(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, uint32_t extentOverflowIndex)
{
    ExtentsDirectoryRecord* record = new ExtentsDirectoryRecord();
    record->catalogData.folderID = volumeHeader->nextExtentsID;
    record->catalogKey.extentOverflowIndex = extentOverflowIndex;

    //we added a new catalog record so we gave it the next catalog id so we need to increase it now
    HFSPlusVolumeHeader* updatedVolumeHeader = new HFSPlusVolumeHeader();
    memcpy(updatedVolumeHeader, volumeHeader, sizeof(HFSPlusVolumeHeader));
    updatedVolumeHeader->nextExtentsID++;
    updateVolumeHeaderNodeOnDisk(diskInfo, volumeHeader, updatedVolumeHeader);
    memcpy(volumeHeader, updatedVolumeHeader, sizeof(HFSPlusVolumeHeader));

    return record;
}

uint32_t eof_getNumberOfBlocksPerNode(HFSPlusVolumeHeader* volumeHeader)
{
    return getExtentsOverflowFileNodeSize(volumeHeader) / volumeHeader->blockSize;
}

uint32_t eof_getNumberOfSectorsPerNode(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader)
{
    return eof_getNumberOfBlocksPerNode(volumeHeader) * getNumberOfSectorsPerBlock(diskInfo, volumeHeader);
}

uint32_t eof_getFirstBlockForGivenNodeIndex(HFSPlusVolumeHeader* volumeHeader, uint32_t nodeNumber)
{
    return volumeHeader->extentsFile.extents[0].startBlock + (nodeNumber + 1) * eof_getNumberOfBlocksPerNode(volumeHeader); //we add 1 because root is nodeNumber 0 but there is also header node
}

uint32_t eof_getSectorForGivenNodeIndex(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, uint32_t nodeNumber)
{
    return eof_getFirstBlockForGivenNodeIndex(volumeHeader, nodeNumber) * getNumberOfSectorsPerBlock(diskInfo, volumeHeader);
}
