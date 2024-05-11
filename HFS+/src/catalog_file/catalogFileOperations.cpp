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

uint32_t findCatalogDirectoryRecordByFullPath(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode,
                                              char* directoryPath, CatalogDirectoryRecord** catalogDirectoryRecord)
{
    char* actualDirectoryName = strtok(directoryPath, "/");
    if(strcmp(actualDirectoryName, "Root\0") != 0)
        return SEARCH_RECORD_BY_FULL_PATH_PARENT_DO_NOT_EXIST;

    *catalogDirectoryRecord = nullptr;
    CatalogDirectoryRecord* searchedCatalogDirectoryRecord = new CatalogDirectoryRecord();

    while(actualDirectoryName != nullptr)
    {
        uint32_t searchedCatalogDirectoryRecordResult = searchDirectoryRecordByDirectoryNameBeingGivenParentDirectoryRecord(diskInfo, volumeHeader, *catalogDirectoryRecord,
                                                                                                                             actualDirectoryName, searchedCatalogDirectoryRecord);

        if(searchedCatalogDirectoryRecordResult == SEARCH_DIRECTORY_RECORD_BY_DIRECTORY_NAME_BEING_GIVEN_PARENT_RECORD_SUCCESS)
            *catalogDirectoryRecord = searchedCatalogDirectoryRecord;
        else
        {
            delete searchedCatalogDirectoryRecord;

            if(searchedCatalogDirectoryRecordResult == SEARCH_DIRECTORY_RECORD_BY_DIRECTORY_NAME_BEING_GIVEN_PARENT_RECORD_KEY_DO_NOT_EXIT_IN_TREE)
                return SEARCH_RECORD_BY_FULL_PATH_PARENT_DO_NOT_EXIST;

            return SEARCH_RECORD_BY_FULL_PATH_FAILED_FOR_OTHER_REASON;
        }

        actualDirectoryName = strtok(nullptr, "/");
    }

    return SEARCH_RECORD_BY_FULL_PATH_SUCCESS;
}

static uint32_t searchDirectoryRecordByDirectoryNameBeingGivenParentDirectoryRecord(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogDirectoryRecord* parentDirectoryRecord,
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

    HFSPlusCatalogKey* searchedKey = new HFSPlusCatalogKey();
    searchedKey->nodeName.length = strlen(searchedDirectoryName) + 1;
    memcpy(&searchedKey->nodeName.chars, searchedDirectoryName, searchedKey->nodeName.length);
    searchedKey->nodeName.chars[searchedKey->nodeName.length] = 0;

    uint32_t searchRecordResult = searchRecordForGivenNodeDataAndSearchedKey(diskInfo, volumeHeader, searchedKey, nodeBuffer, searchedDirectoryRecord);

    switch (searchRecordResult) {
        case SEARCH_RECORD_IN_GIVEN_DATA_KEY_DO_NOT_EXIST_IN_TREE:
            return SEARCH_DIRECTORY_RECORD_BY_DIRECTORY_NAME_BEING_GIVEN_PARENT_RECORD_KEY_DO_NOT_EXIT_IN_TREE;
            break;
        case SEARCH_RECORD_IN_GIVEN_DATA_FAILED_FOR_OTHER_REASON:
            return SEARCH_DIRECTORY_RECORD_BY_DIRECTORY_NAME_BEING_GIVEN_PARENT_RECORD_FAILED_FOR_OTHER_REASON;
            break;
        default: //success
            return SEARCH_DIRECTORY_RECORD_BY_DIRECTORY_NAME_BEING_GIVEN_PARENT_RECORD_SUCCESS;
    }
}

//////////////////////////////

void updateHeaderNodeOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* updatedCatalogFileHeaderNode)
{
    uint32_t numberOfSectorsWritten, retryWriteCount = 2;
    uint32_t numOfSectorsToWrite = getNumberOfBlocksPerNode(volumeHeader) * getNumberOfSectorsPerBlock(diskInfo, volumeHeader);
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

void updateNodeOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, char* updatedNodeData, uint32_t nodeNumber)
{
    uint32_t numberOfSectorsWritten, retryWriteCount = 2;
    uint32_t numOfSectorsToWrite = getNumberOfBlocksPerNode(volumeHeader) * getNumberOfSectorsPerBlock(diskInfo, volumeHeader);
    uint32_t firstBlockForNode = getFirstBlockForGivenNodeIndex(volumeHeader, nodeNumber);
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