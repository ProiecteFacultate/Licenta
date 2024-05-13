#include "iostream"
#include "string.h"

#include "../include/disk.h"
#include "../include/diskCodes.h"
#include "../../include/structures.h"
#include "../../include/utils.h"
#include "../../include/hfsFunctionUtils.h"
#include "../../include/catalog_file/codes/bTreeResponseCodes.h"
#include "../../include/catalog_file/codes/catalogFileResponseCodes.h"
#include "../../include/codes/hfsAttributes.h"
#include "../../include/catalog_file/bTreeCatalog.h"
#include "../../include/catalog_file/catalogFileUtils.h"
#include "../../include/catalog_file/catalogFileOperations.h"

uint32_t cf_findCatalogDirectoryRecordByFullPath(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode,
                                              char* directoryPath, CatalogDirectoryRecord** catalogDirectoryRecord, uint32_t& nodeOfNewRecord)
{
    char* actualDirectoryName = strtok(directoryPath, "/");
    if(strcmp(actualDirectoryName, "Root\0") != 0)
        return CF_SEARCH_RECORD_BY_FULL_PATH_PARENT_DO_NOT_EXIST;

    *catalogDirectoryRecord = nullptr;
    CatalogDirectoryRecord* searchedCatalogDirectoryRecord = new CatalogDirectoryRecord();

    while(actualDirectoryName != nullptr)
    {
        uint32_t searchedCatalogDirectoryRecordResult = cf_searchDirectoryRecordByDirectoryNameBeingGivenParentDirectoryRecord(diskInfo, volumeHeader, catalogFileHeaderNode,
                                                                                                                            *catalogDirectoryRecord,
                                                                                                                             actualDirectoryName,
                                                                                                                             searchedCatalogDirectoryRecord,
                                                                                                                            nodeOfNewRecord);

        if(searchedCatalogDirectoryRecordResult == CF_SEARCH_DIRECTORY_RECORD_BY_DIRECTORY_NAME_BEING_GIVEN_PARENT_RECORD_SUCCESS)
            *catalogDirectoryRecord = searchedCatalogDirectoryRecord;
        else
        {
            delete searchedCatalogDirectoryRecord;

            if(searchedCatalogDirectoryRecordResult == CF_SEARCH_DIRECTORY_RECORD_BY_DIRECTORY_NAME_BEING_GIVEN_PARENT_RECORD_KEY_DO_NOT_EXIT_IN_TREE)
                return CF_SEARCH_RECORD_BY_FULL_PATH_PARENT_DO_NOT_EXIST;

            return CF_SEARCH_RECORD_BY_FULL_PATH_FAILED_FOR_OTHER_REASON;
        }

        actualDirectoryName = strtok(nullptr, "/");
    }

    return CF_SEARCH_RECORD_BY_FULL_PATH_SUCCESS;
}

uint32_t cf_searchDirectoryRecordByDirectoryNameBeingGivenParentDirectoryRecord(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode,
                                                                             CatalogDirectoryRecord* parentDirectoryRecord, char* searchedDirectoryName,
                                                                             CatalogDirectoryRecord* searchedDirectoryRecord, uint32_t& nodeNumberForRecord)
{
    if(parentDirectoryRecord == nullptr) //if we are looking for root, which is not an actual directory
    {
        memset(searchedDirectoryRecord, 0, sizeof(CatalogDirectoryRecord));
        return CF_SEARCH_DIRECTORY_RECORD_BY_DIRECTORY_NAME_BEING_GIVEN_PARENT_RECORD_SUCCESS;
    }

    uint32_t actualNodeNumber = catalogFileHeaderNode->headerRecord.rootNode;
    char* nodeBuffer = new char[getCatalogFileNodeSize()];

    uint32_t readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, nodeBuffer, actualNodeNumber);
    if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeBuffer;
        return CF_SEARCH_DIRECTORY_RECORD_BY_DIRECTORY_NAME_BEING_GIVEN_PARENT_RECORD_FAILED_FOR_OTHER_REASON;
    }

    HFSPlusCatalogKey* searchedKey = new HFSPlusCatalogKey();
    searchedKey->parentID = parentDirectoryRecord->catalogData.folderID;
    searchedKey->nodeName.length = strlen(searchedDirectoryName) + 1;
    memcpy(&searchedKey->nodeName.chars, searchedDirectoryName, searchedKey->nodeName.length);
    searchedKey->nodeName.chars[searchedKey->nodeName.length] = 0;

    uint32_t searchRecordResult = cf_searchRecordForGivenNodeDataAndSearchedKey(diskInfo, volumeHeader, searchedKey, nodeBuffer, searchedDirectoryRecord,
                                                                             nodeNumberForRecord);

    switch (searchRecordResult) {
        case CF_SEARCH_RECORD_IN_GIVEN_DATA_KEY_DO_NOT_EXIST_IN_TREE:
            return CF_SEARCH_DIRECTORY_RECORD_BY_DIRECTORY_NAME_BEING_GIVEN_PARENT_RECORD_KEY_DO_NOT_EXIT_IN_TREE;
        case CF_SEARCH_RECORD_IN_GIVEN_DATA_FAILED_FOR_OTHER_REASON:
            return CF_SEARCH_DIRECTORY_RECORD_BY_DIRECTORY_NAME_BEING_GIVEN_PARENT_RECORD_FAILED_FOR_OTHER_REASON;
        default: //success
            return CF_SEARCH_DIRECTORY_RECORD_BY_DIRECTORY_NAME_BEING_GIVEN_PARENT_RECORD_SUCCESS;
    }
}

//////////////////////////////

void cf_updateCatalogHeaderNodeOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* updatedCatalogFileHeaderNode)
{
    uint32_t numberOfSectorsWritten, retryWriteCount = 2;
    uint32_t numOfSectorsToWrite = cf_getNumberOfBlocksPerNode(volumeHeader) * getNumberOfSectorsPerBlock(diskInfo, volumeHeader);
    uint32_t firstSectorForCatalogFile = volumeHeader->catalogFile.extents[0].startBlock * getNumberOfSectorsPerBlock(diskInfo, volumeHeader);
    char* nodeBuffer = new char[getCatalogFileNodeSize()];
    memcpy(nodeBuffer, updatedCatalogFileHeaderNode, sizeof(CatalogFileHeaderNode));

    uint32_t writeResult = writeDiskSectors(diskInfo, numOfSectorsToWrite, firstSectorForCatalogFile, nodeBuffer, numberOfSectorsWritten);

    while(writeResult != EC_NO_ERROR && retryWriteCount > 0)
    {
        writeResult = writeDiskSectors(diskInfo, numOfSectorsToWrite, firstSectorForCatalogFile, nodeBuffer, numberOfSectorsWritten);
        retryWriteCount--;
    }

    if(retryWriteCount == 0)
        throw std::runtime_error("FATAL ERROR: Update catalog header node on disk failed!");

    delete[] nodeBuffer;
}

void cf_updateNodeOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, char* updatedNodeData, uint32_t nodeNumber)
{
    uint32_t numberOfSectorsWritten, retryWriteCount = 2;
    uint32_t numOfSectorsToWrite = cf_getNumberOfBlocksPerNode(volumeHeader) * getNumberOfSectorsPerBlock(diskInfo, volumeHeader);
    uint32_t firstBlockForNode = cf_getFirstBlockForGivenNodeIndex(volumeHeader, nodeNumber);
    uint32_t firstSectorForGivenNode = getFirstSectorForGivenBlock(diskInfo, volumeHeader, firstBlockForNode);

    uint32_t writeResult = writeDiskSectors(diskInfo, numOfSectorsToWrite, firstSectorForGivenNode, updatedNodeData, numberOfSectorsWritten);

    while(writeResult != EC_NO_ERROR && retryWriteCount > 0)
    {
        writeResult = writeDiskSectors(diskInfo, numOfSectorsToWrite, firstSectorForGivenNode, updatedNodeData, numberOfSectorsWritten);
        retryWriteCount--;
    }

    if(retryWriteCount == 0)
        throw std::runtime_error("FATAL ERROR: Update catalog node on disk failed!");
}

uint32_t cf_updateRecordOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogDirectoryRecord* directoryRecord,
                            CatalogDirectoryRecord* updatedDirectoryRecord, uint32_t nodeNumberOfRecord)
{
    char* nodeData = new char[getCatalogFileNodeSize()];
    uint32_t readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, nodeData, nodeNumberOfRecord);

    if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData;
        return CF_UPDATE_RECORD_ON_DISK_FAILED;
    }

    BTNodeDescriptor* nodeDescriptor = (BTNodeDescriptor*)&nodeData[sizeof(BTNodeDescriptor)];

    for(uint32_t i = 0; i < nodeDescriptor->numRecords; i++)
    {
        uint32_t firstByteOfRecord = sizeof(BTNodeDescriptor) + i * sizeof(CatalogDirectoryRecord);
        CatalogDirectoryRecord* record = (CatalogDirectoryRecord*)&nodeData[firstByteOfRecord];

        if(cf_compareKeys(&record->catalogKey, &directoryRecord->catalogKey) == 0)
        {
            memcpy(nodeData + firstByteOfRecord, updatedDirectoryRecord, sizeof(CatalogDirectoryRecord));

            try {
                cf_updateNodeOnDisk(diskInfo, volumeHeader, nodeData, nodeNumberOfRecord);
            }
            catch(std::runtime_error& error){
                delete[] nodeData;
                return CF_UPDATE_RECORD_ON_DISK_FAILED;
            }

            delete[] nodeData;
            return CF_UPDATE_RECORD_ON_DISK_SUCCESS;
        }
    }
}