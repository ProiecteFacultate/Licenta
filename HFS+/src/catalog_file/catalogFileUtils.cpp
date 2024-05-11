#include "string.h"
#include "windows.h"

#include "../include/disk.h"
#include "../../include/structures.h"
#include "../../include/utils.h"
#include "../../include/hfsFunctionUtils.h"
#include "../../include/catalog_file/catalogFileUtils.h"
#include "../../include/catalog_file/codes/catalogFileResponseCodes.h"

int32_t compareKeys(HFSPlusCatalogKey* key1, HFSPlusCatalogKey* key2)
{
    if(key1->parentID > key2->parentID)
        return KEY_1_HIGHER;

    if(key1->parentID < key2->parentID)
        return KEY_2_HIGHER;

    //else we have equality in parentId so we compare based on nodeName
    char* name1 = new char[key1->nodeName.length + 1];
    memcpy(name1, key1->nodeName.chars, key1->nodeName.length);
    name1[key1->nodeName.length] = 0;

    char* name2 = new char[key2->nodeName.length + 1];
    memcpy(name2, key2->nodeName.chars, key2->nodeName.length);
    name2[key2->nodeName.length] = 0;

    if(strcmp(name1, name2) > 0)
        return KEY_1_HIGHER;
    else if(strcmp(name1, name2) < 0)
        return KEY_2_HIGHER;

    return KEYS_EQUAL;
}

CatalogFileHeaderNode* updateNodeOccupiedInHeaderNodeMapRecord(CatalogFileHeaderNode* catalogFileHeaderNode, uint32_t nodeNumber, uint8_t newValue)
{
    CatalogFileHeaderNode* updatedCatalogFileHeaderNode = new CatalogFileHeaderNode();
    memcpy(updatedCatalogFileHeaderNode, catalogFileHeaderNode, sizeof(CatalogFileHeaderNode));

    uint32_t byteIndexForNodeNumber = nodeNumber / 8;
    uint32_t bitIndexForNodeNumber = nodeNumber % 8;

    uint8_t newByteValueForNodes = changeBitValue(byteIndexForNodeNumber, bitIndexForNodeNumber, newValue);
    updatedCatalogFileHeaderNode->mapRecordAndOffsets[byteIndexForNodeNumber] = newByteValueForNodes;

    return updatedCatalogFileHeaderNode;
}

uint32_t createDirectoryRecord(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogDirectoryRecord* parentRecord, CatalogDirectoryRecord* createdRecord,
                                char* directoryName, int16_t fileType)
{
    uint32_t nameLen = strlen(directoryName);
    createdRecord->catalogKey.keyLength = nameLen;
    createdRecord->catalogKey.parentID = parentRecord->catalogData.folderID;
    createdRecord->catalogKey.nodeName.length = nameLen;
    memcpy(&createdRecord->catalogKey.nodeName.chars, directoryName, nameLen);
    createdRecord->catalogKey.nodeName.chars[nameLen] = 0;

    SYSTEMTIME time;
    GetSystemTime(&time);

    createdRecord->catalogData.recordType = fileType;
    createdRecord->catalogData.folderID = volumeHeader->nextCatalogID;
    //high 7 bits represent how many years since 1900, next 4 for month, next 5 for day and the low 16 represent the second in that day with a granularity of 2 (see in fat)
    createdRecord->catalogData.createDate = ((time.wYear - 1900) << 25) | (time.wMonth << 21) | (time.wDay << 16) | ((time.wHour * 3600 + time.wMinute * 60 + time.wSecond) / 2);
    createdRecord->catalogData.contentModDate = createdRecord->catalogData.createDate;
    createdRecord->catalogData.accessDate = createdRecord->catalogData.createDate;

    return CREATE_DIRECTORY_RECORD_SUCCESS;
    //TODO PREALLOCATE EXTENTS?
}

uint32_t getNumberOfBlocksPerNode(HFSPlusVolumeHeader* volumeHeader)
{
    return getCatalogFileNodeSize() / volumeHeader->blockSize;
}

uint32_t getNumberOfSectorsPerNode(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader)
{
    return getNumberOfBlocksPerNode(volumeHeader) * getNumberOfSectorsPerBlock(diskInfo, volumeHeader);
}

uint32_t getFirstBlockForGivenNodeIndex(HFSPlusVolumeHeader* volumeHeader, uint32_t nodeNumber)
{
    return volumeHeader->catalogFile.extents[0].startBlock + (nodeNumber + 1) * getNumberOfBlocksPerNode(volumeHeader); //we add 1 because root is nodeNumber 0 but there is also header node
}

uint32_t getSectorForGivenNodeIndex(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, uint32_t nodeNumber)
{
    return getFirstBlockForGivenNodeIndex(volumeHeader, nodeNumber) * getNumberOfSectorsPerBlock(diskInfo, volumeHeader);
}
