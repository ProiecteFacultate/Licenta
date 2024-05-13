#include "string.h"

#include "../include/disk.h"
#include "../../include/structures.h"
#include "../../include/utils.h"
#include "../../include/hfsFunctionUtils.h"
#include "../../include/extents_file/extentsFileUtils.h"
#include "../../include/extents_file/codes/extentsFileResponseCodes.h"

int32_t eof_compareKeys(ExtentsFileCatalogKey * key1, ExtentsFileCatalogKey * key2)
{
    if(key1->parentID > key2->parentID)
        return EOF_KEY_1_HIGHER;

    if(key1->parentID < key2->parentID)
        return EOF_KEY_2_HIGHER;

    //else we have equality in parentId so we compare based on nodeName
    char* name1 = new char[key1->nodeName.length + 1];
    memcpy(name1, key1->nodeName.chars, key1->nodeName.length);
    name1[key1->nodeName.length] = 0;

    char* name2 = new char[key2->nodeName.length + 1];
    memcpy(name2, key2->nodeName.chars, key2->nodeName.length);
    name2[key2->nodeName.length] = 0;

    if(strcmp(name1, name2) > 0)
        return EOF_KEY_1_HIGHER;
    else if(strcmp(name1, name2) < 0)
        return EOF_KEY_2_HIGHER;
    else
    {
        if(key1->extentOverflowIndex > key2->extentOverflowIndex)
            return EOF_KEY_1_HIGHER;
        else if(key1->extentOverflowIndex < key2->extentOverflowIndex)
            return EOF_KEY_2_HIGHER;
    }

    return EOF_KEYS_EQUAL;
}

ExtentsFileHeaderNode* eof_updateNodeOccupiedInHeaderNodeMapRecord(ExtentsFileHeaderNode* extentsFileHeaderNode, uint32_t nodeNumber, uint8_t newValue)
{
    ExtentsFileHeaderNode* updatedExtentsFileHeaderNode = new ExtentsFileHeaderNode ();
    memcpy(updatedExtentsFileHeaderNode, extentsFileHeaderNode, sizeof(ExtentsFileHeaderNode));

    uint32_t byteIndexForNodeNumber = nodeNumber / 8;
    uint32_t bitIndexForNodeNumber = nodeNumber % 8;

    uint8_t newByteValueForNodes = changeBitValue(byteIndexForNodeNumber, bitIndexForNodeNumber, newValue);
    updatedExtentsFileHeaderNode->mapRecordAndOffsets[byteIndexForNodeNumber] = newByteValueForNodes;

    return updatedExtentsFileHeaderNode;
}

uint32_t eof_createDirectoryRecord(HFSPlusVolumeHeader* volumeHeader, ExtentsDirectoryRecord* parentRecord, ExtentsDirectoryRecord* createdRecord,
                               char* directoryName, uint32_t extentOverflowIndex)
{
    uint32_t nameLen = strlen(directoryName);
    createdRecord->catalogKey.keyLength = nameLen;
    createdRecord->catalogKey.parentID = parentRecord->catalogData.folderID;
    createdRecord->catalogKey.nodeName.length = nameLen;
    createdRecord->catalogKey.extentOverflowIndex = extentOverflowIndex;
    memcpy(&createdRecord->catalogKey.nodeName.chars, directoryName, nameLen);
    createdRecord->catalogKey.nodeName.chars[nameLen] = 0;

    createdRecord->catalogData.folderID = volumeHeader->nextCatalogID;

    return EOF_CREATE_DIRECTORY_RECORD_SUCCESS;
}

uint32_t eof_getNumberOfBlocksPerNode(HFSPlusVolumeHeader* volumeHeader)
{
    return getExtentsOverflowFileNodeSize() / volumeHeader->blockSize;
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
