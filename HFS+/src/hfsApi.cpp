#include "string.h"
#include "vector"

#include "../include/disk.h"
#include "../include/diskCodes.h"
#include "../include/structures.h"
#include "../include/utils.h"
#include "../include/hfsFunctionUtils.h"
#include "../include/codes/hfsAttributes.h"
#include "../include/catalog_file/codes/catalogFileResponseCodes.h"
#include "../include/catalog_file/codes/bTreeResponseCodes.h"
#include "../include/catalog_file/catalogFileOperations.h"
#include "../include/catalog_file/catalogFileUtils.h"
#include "../include/catalog_file/bTreeCatalog.h"
#include "../include/codes/hfsApiResponseCodes.h"
#include "../include/hfsApi.h"

uint32_t createDirectory(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, char* directoryParentPath,
                         char* newDirectoryName, int16_t newDirectoryType)
{
    CatalogDirectoryRecord* actualCatalogDirectoryRecord = nullptr;
    uint32_t findCatalogDirectoryRecordResult = findCatalogDirectoryRecordByFullPath(diskInfo, volumeHeader, catalogFileHeaderNode, directoryParentPath,
                                                                                     &actualCatalogDirectoryRecord);

    if(findCatalogDirectoryRecordResult == SEARCH_RECORD_IN_GIVEN_DATA_KEY_DO_NOT_EXIST_IN_TREE)
        return DIRECTORY_CREATION_PARENT_DO_NOT_EXIST;
    else if(findCatalogDirectoryRecordResult == SEARCH_RECORD_IN_GIVEN_DATA_FAILED_FOR_OTHER_REASON)
        return DIRECTORY_CREATION_FAILED_FOR_OTHER_REASON;

    if(catalogFileHeaderNode->headerRecord.freeNodes < catalogFileHeaderNode->headerRecord.totalNodes - 1) //otherwise root node is not yet instantiated
    {
        uint32_t parentAlreadyContainsDirectoryWithGivenName = searchDirectoryRecordByDirectoryNameBeingGivenParentDirectoryRecord(diskInfo, volumeHeader, catalogFileHeaderNode,
                                                                                                                               actualCatalogDirectoryRecord,
                                                                                                                               newDirectoryName,
                                                                                                                               new CatalogDirectoryRecord());

        if(actualCatalogDirectoryRecord->catalogData.recordType != DIRECTORY_TYPE_FOLDER)
            return DIRECTORY_CREATION_PARENT_NOT_A_FOLDER;

        if(parentAlreadyContainsDirectoryWithGivenName == SEARCH_DIRECTORY_RECORD_BY_DIRECTORY_NAME_BEING_GIVEN_PARENT_RECORD_SUCCESS)
            return DIRECTORY_CREATION_NEW_NAME_ALREADY_EXISTS;
    }

    CatalogDirectoryRecord* recordToInsert = new CatalogDirectoryRecord();
    createDirectoryRecord(diskInfo, volumeHeader, actualCatalogDirectoryRecord, recordToInsert, newDirectoryName, newDirectoryType);
    uint32_t insertRecordInTreeResult = insertRecordInTree(diskInfo, volumeHeader, catalogFileHeaderNode, recordToInsert);

    return (insertRecordInTreeResult == INSERT_RECORD_IN_TREE_SUCCESS) ? DIRECTORY_CREATION_SUCCESS : DIRECTORY_CREATION_PARENT_FAILED_TO_INSERT_RECORD_IN_CATALOG_TREE;
}

uint32_t getSubDirectoriesByParentPath(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode,
                                       char* directoryPath, std::vector<CatalogDirectoryRecord*>& subDirectories)
{
    CatalogDirectoryRecord * givenDirectoryRecord = nullptr;
    uint32_t searchParentInodeResult = findCatalogDirectoryRecordByFullPath(diskInfo, volumeHeader, catalogFileHeaderNode, directoryPath, &givenDirectoryRecord);

    if(searchParentInodeResult != SEARCH_RECORD_IN_GIVEN_DATA_SUCCESS)
        return GET_SUBDIRECTORIES_GIVEN_DIRECTORY_DO_NOT_EXIST;

    if(givenDirectoryRecord->catalogData.recordType != DIRECTORY_TYPE_FOLDER)
        return GET_SUBDIRECTORIES_GIVEN_DIRECTORY_NOT_A_FOLDER;

    uint32_t traverseResult = traverseSubtree(diskInfo, volumeHeader, catalogFileHeaderNode->headerRecord.rootNode,
                                              givenDirectoryRecord->catalogData.folderID, subDirectories);

    return (traverseResult == TRAVERSE_SUBTREE_SUCCESS) ? GET_SUBDIRECTORIES_SUCCESS : GET_SUBDIRECTORIES_FAILED_FOR_OTHER_REASON;
}