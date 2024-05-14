#include "string.h"
#include "windows.h"

#include "../include/disk.h"
#include "../../include/structures.h"
#include "../../include/utils.h"
#include "../../include/hfsFunctionUtils.h"
#include "../../include/hfs.h"
#include "../../include/catalog_file/catalogFileUtils.h"
#include "../../include/catalog_file/codes/catalogFileResponseCodes.h"

int32_t cf_compareKeys(HFSPlusCatalogKey* key1, HFSPlusCatalogKey* key2)
{
    if(key1->parentID > key2->parentID)
        return CF_KEY_1_HIGHER;

    if(key1->parentID < key2->parentID)
        return CF_KEY_2_HIGHER;

    //else we have equality in parentId so we compare based on nodeName
    char* name1 = new char[key1->nodeName.length + 1];
    memcpy(name1, key1->nodeName.chars, key1->nodeName.length);
    name1[key1->nodeName.length] = 0;

    char* name2 = new char[key2->nodeName.length + 1];
    memcpy(name2, key2->nodeName.chars, key2->nodeName.length);
    name2[key2->nodeName.length] = 0;

    if(strcmp(name1, name2) > 0)
        return CF_KEY_1_HIGHER;
    else if(strcmp(name1, name2) < 0)
        return CF_KEY_2_HIGHER;

    return CF_KEYS_EQUAL;
}

CatalogFileHeaderNode* cf_updateNodeOccupiedInHeaderNodeMapRecord(CatalogFileHeaderNode* catalogFileHeaderNode, uint32_t nodeNumber, uint8_t newValue)
{
    CatalogFileHeaderNode* updatedCatalogFileHeaderNode = new CatalogFileHeaderNode();
    memcpy(updatedCatalogFileHeaderNode, catalogFileHeaderNode, sizeof(CatalogFileHeaderNode));

    uint32_t byteIndexForNodeNumber = nodeNumber / 8;
    uint32_t bitIndexForNodeNumber = nodeNumber % 8;

    uint8_t newByteValueForNodes = changeBitValue(byteIndexForNodeNumber, bitIndexForNodeNumber, newValue);
    updatedCatalogFileHeaderNode->mapRecordAndOffsets[byteIndexForNodeNumber] = newByteValueForNodes;

    return updatedCatalogFileHeaderNode;
}

CatalogDirectoryRecord* cf_createDirectoryRecord(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogDirectoryRecord* parentRecord, char* directoryName,
                                                 int16_t fileType)
{
    CatalogDirectoryRecord* record = new CatalogDirectoryRecord();

    uint32_t nameLen = strlen(directoryName);
    record->catalogKey.keyLength = nameLen;
    record->catalogKey.parentID = parentRecord->catalogData.folderID;
    record->catalogKey.nodeName.length = nameLen;
    memcpy(&record->catalogKey.nodeName.chars, directoryName, nameLen);
    record->catalogKey.nodeName.chars[nameLen] = 0;

    SYSTEMTIME time;
    GetSystemTime(&time);

    record->catalogData.recordType = fileType;
    record->catalogData.folderID = volumeHeader->nextCatalogID;
    //high 7 bits represent how many years since 1900, next 4 for month, next 5 for day and the low 16 represent the second in that day with a granularity of 2 (see in fat)
    record->catalogData.createDate = ((time.wYear - 1900) << 25) | (time.wMonth << 21) | (time.wDay << 16) | ((time.wHour * 3600 + time.wMinute * 60 + time.wSecond) / 2);
    record->catalogData.contentModDate = record->catalogData.createDate;
    record->catalogData.accessDate = record->catalogData.createDate;

    //we attribute a new catalog record so we gave it the next catalog id so we need to increase it now
    HFSPlusVolumeHeader* updatedVolumeHeader = new HFSPlusVolumeHeader();
    memcpy(updatedVolumeHeader, volumeHeader, sizeof(HFSPlusVolumeHeader));
    updatedVolumeHeader->nextCatalogID++;
    updateVolumeHeaderNodeOnDisk(diskInfo, volumeHeader, updatedVolumeHeader);
    memcpy(volumeHeader, updatedVolumeHeader, sizeof(HFSPlusVolumeHeader));

    return record;
    //TODO PREALLOCATE EXTENTS?
}

uint32_t cf_getNumberOfBlocksPerNode(HFSPlusVolumeHeader* volumeHeader)
{
    return getCatalogFileNodeSize() / volumeHeader->blockSize;
}

uint32_t cf_getNumberOfSectorsPerNode(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader)
{
    return cf_getNumberOfBlocksPerNode(volumeHeader) * getNumberOfSectorsPerBlock(diskInfo, volumeHeader);
}

uint32_t cf_getFirstBlockForGivenNodeIndex(HFSPlusVolumeHeader* volumeHeader, uint32_t nodeNumber)
{
    return volumeHeader->catalogFile.extents[0].startBlock + (nodeNumber + 1) * cf_getNumberOfBlocksPerNode(volumeHeader); //we add 1 because root is nodeNumber 0 but there is also header node
}

uint32_t cf_getSectorForGivenNodeIndex(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, uint32_t nodeNumber)
{
    return cf_getFirstBlockForGivenNodeIndex(volumeHeader, nodeNumber) * getNumberOfSectorsPerBlock(diskInfo, volumeHeader);
}
