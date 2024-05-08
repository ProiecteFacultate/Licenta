#include "string.h"
#include "vector"

#include "../include/disk.h"
#include "../include/diskCodes.h"
#include "../include/structures.h"
#include "../include/utils.h"
#include "../include/hfsFunctionUtils.h"
#include "../include/codes/bTreeResponseCodes.h"
#include "../include/hfs.h"
#include "../include/bTreeCatalog.h"

uint32_t findCatalogDirectoryRecordByFullPath(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode,
                                              char* directoryPath, CatalogDirectoryRecord** catalogDirectoryRecord)
{
    char* actualDirectoryName = strtok(directoryPath, "/");
    if(strcmp(actualDirectoryName, "Root\0") != 0)
        return SEARCH_INODE_BY_FULL_PATH_PARENT_DO_NOT_EXIST;

    *catalogDirectoryRecord = nullptr;
    CatalogDirectoryRecord* searchedCatalogDirectoryRecord = new CatalogDirectoryRecord();

    while(actualDirectoryName != nullptr)
    {
        uint32_t searchedCatalogDirectoryRecordResult = searchDirectoryRecordByDirectoryNameBeingGivenParentDirectoryRecord(diskInfo, volumeHeader, *catalogDirectoryRecord,
                                                                                           actualDirectoryName, searchedCatalogDirectoryRecord);

        if(searchedCatalogDirectoryRecordResult == SEARCH_INODE_BY_DIRECTORY_NAME_IN_PARENT_SUCCESS)
            *catalogDirectoryRecord = searchedCatalogDirectoryRecord;
        else
        {
            delete searchedCatalogDirectoryRecord;

            if(searchedCatalogDirectoryRecordResult == SEARCH_INODE_BY_DIRECTORY_NAME_IN_PARENT_INODE_DO_NOT_EXIST)
                return SEARCH_INODE_BY_FULL_PATH_PARENT_DO_NOT_EXIST;

            return SEARCH_INODE_BY_FULL_PATH_FAILED;
        }

        actualDirectoryName = strtok(nullptr, "/");
    }

    return SEARCH_INODE_BY_FULL_PATH_SUCCESS;
}

uint32_t searchDirectoryRecordByDirectoryNameBeingGivenParentDirectoryRecord(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogDirectoryRecord* parentDirectoryRecord,
                                                                             char* searchedDirectoryName, CatalogDirectoryRecord* searchedDirectoryRecord)
{
    if(parentDirectoryRecord == nullptr) //if we are looking for root, which is not an actual directory
    {
        memset(searchedDirectoryRecord, 0, sizeof(CatalogDirectoryRecord));
        return SEARCH_DIRECTORY_RECORD_BY_DIRECTORY_NAME_BEING_GIVEN_PARENT_RECORD_SUCCESS;
    }

    uint32_t actualNodeNumber = 0;
    char* nodeBuffer = new char[getCatalogFileNodeSize()];

    uint32_t readNodeFromDiskResult = readNodeFromDisk(diskInfo, volumeHeader, nodeBuffer, actualNodeNumber);
    if(readNodeFromDiskResult == READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeBuffer;
        return SEARCH_DIRECTORY_RECORD_BY_DIRECTORY_NAME_BEING_GIVEN_PARENT_RECORD_FAILED_FOR_OTHER_REASON;
    }

    uint32_t searchRecordResult = searchNextNodeForGivenNodeDataAndParentRecord(diskInfo, volumeHeader, parentDirectoryRecord, nodeBuffer, searchedDirectoryRecord);
}

uint32_t readNodeFromDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, char* nodeBuffer, uint32_t nodeNumber)
{
    uint32_t numberOfSectorsRead;
    uint32_t nodeSize = getCatalogFileNodeSize();
    uint32_t numOfSectorsPerNode = getNumberOfBlocksPerNode(volumeHeader) * getNumberOfSectorsPerBlock(diskInfo, volumeHeader);
    uint32_t firstBlockForNode = getFirstBlockForGivenNodeIndex(volumeHeader, nodeNumber); //root node
    uint32_t firstSector = getFirstSectorForGivenBlock(diskInfo, volumeHeader, firstBlockForNode);

    uint32_t readResult = readDiskSectors(diskInfo, numOfSectorsPerNode, firstSector, nodeBuffer, numberOfSectorsRead);

    return (readResult == EC_NO_ERROR) ? READ_NODE_FROM_DISK_SUCCESS : READ_NODE_FROM_DISK_FAILED;
}

uint32_t searchRecordForGivenNodeDataAndParentRecord(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogDirectoryRecord* parentRecord, char* nodeData,
                                                               CatalogDirectoryRecord* searchedRecord)
{
    BTNodeDescriptor* nodeDescriptor = (BTNodeDescriptor*)&nodeData[0];
    uint32_t recordFirstByte = sizeof(BTNodeDescriptor);
    CatalogDirectoryRecord* record = (CatalogDirectoryRecord*)&nodeData[recordFirstByte];

    uint32_t recordIndex = 0;
    while(recordIndex < nodeDescriptor->numRecords && compareKeys(&parentRecord->catalogKey, &record->catalogKey) > 0)
    {
        recordIndex++;
        recordFirstByte += sizeof(CatalogDirectoryRecord);
        record = (CatalogDirectoryRecord*)&nodeData[recordFirstByte];
    }

    if(recordIndex < nodeDescriptor->numRecords && compareKeys(&parentRecord->catalogKey, &record->catalogKey) == 0)
    {
        memcpy(searchedRecord, record, sizeof(CatalogDirectoryRecord));
        return SEARCH_RECORD_IN_GIVEN_DATA_SUCCESS;
    }

    NextNodeInfo* nextNodeInfo = (NextNodeInfo*)&nodeData[getCatalogFileNodeSize() - (recordIndex + 1) * sizeof(NextNodeInfo)];
    //TODO check if children nodes exist

    uint32_t readNodeFromDiskResult = readNodeFromDisk(diskInfo, volumeHeader, nodeData, nextNodeInfo->nodeNumber);
    if(readNodeFromDiskResult == READ_NODE_FROM_DISK_FAILED)
        return SEARCH_RECORD_IN_GIVEN_DATA_FAILED_FOR_OTHER_REASON;

    return searchNextNodeForGivenNodeDataAndParentRecord(diskInfo, volumeHeader, parentRecord, nodeData, searchedRecord);
}
uint32_t updateHeaderNodeOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* updatedCatalogFileHeaderNode)
{
    uint32_t numberOfSectorsWritten;
    uint32_t numOfSectorsToWrite = getNumberOfBlocksPerNode(volumeHeader) * getNumberOfSectorsPerBlock(diskInfo, volumeHeader);
    uint32_t firstSectorForCatalogFile = volumeHeader->catalogFile.extents[0].startBlock * getNumberOfSectorsPerBlock(diskInfo, volumeHeader);
    char* nodeBuffer = new char[getCatalogFileNodeSize()];
    memcpy(nodeBuffer, updatedCatalogFileHeaderNode, sizeof(CatalogFileHeaderNode));

    uint32_t writeResult = writeDiskSectors(diskInfo, numOfSectorsToWrite, firstSectorForCatalogFile, nodeBuffer, numberOfSectorsWritten);
    delete[] nodeBuffer;

    return (writeResult == EC_NO_ERROR) ? UPDATE_HEADER_NODE_ON_DISK_SUCCESS : UPDATE_HEADER_NODE_ON_DISK_FAILED;
}


////////////////////////////////////

static int32_t compareKeys(HFSPlusCatalogKey* key1, HFSPlusCatalogKey* key2)
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

static uint32_t getNumberOfBlocksPerNode(HFSPlusVolumeHeader* volumeHeader)
{
    return getCatalogFileNodeSize() / volumeHeader->blockSize;
}

static uint32_t getFirstBlockForGivenNodeIndex(HFSPlusVolumeHeader* volumeHeader, uint32_t nodeNumber)
{
    return volumeHeader->catalogFile.extents[0].startBlock + (nodeNumber + 1) * getNumberOfBlocksPerNode(volumeHeader); //we add 1 because root is nodeNumber 0 but there is also header node
}

