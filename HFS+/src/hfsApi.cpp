#include "string.h"
#include "vector"

#include "../include/disk.h"
#include "../include/diskCodes.h"
#include "../include/structures.h"
#include "../include/utils.h"
#include "../include/hfsFunctionUtils.h"
#include "../include/hfs.h"
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
    uint32_t nodeOfNewRecord;
    uint32_t findCatalogDirectoryRecordResult = cf_findCatalogDirectoryRecordByFullPath(diskInfo, volumeHeader, catalogFileHeaderNode, directoryParentPath,
                                                                                     &actualCatalogDirectoryRecord, nodeOfNewRecord);

    if(findCatalogDirectoryRecordResult == CF_SEARCH_RECORD_IN_GIVEN_DATA_KEY_DO_NOT_EXIST_IN_TREE)
        return DIRECTORY_CREATION_PARENT_DO_NOT_EXIST;
    else if(findCatalogDirectoryRecordResult == CF_SEARCH_RECORD_IN_GIVEN_DATA_FAILED_FOR_OTHER_REASON)
        return DIRECTORY_CREATION_FAILED_FOR_OTHER_REASON;

    if(catalogFileHeaderNode->headerRecord.freeNodes < catalogFileHeaderNode->headerRecord.totalNodes - 1) //otherwise root node is not yet instantiated
    {
        uint32_t nodeOfNewRecord;
        uint32_t parentAlreadyContainsDirectoryWithGivenName = cf_searchDirectoryRecordByDirectoryNameBeingGivenParentDirectoryRecord(diskInfo, volumeHeader, catalogFileHeaderNode,
                                                                                                                               actualCatalogDirectoryRecord,
                                                                                                                               newDirectoryName,
                                                                                                                               new CatalogDirectoryRecord(),
                                                                                                                               nodeOfNewRecord);

        if(actualCatalogDirectoryRecord->catalogData.recordType != DIRECTORY_TYPE_FOLDER)
            return DIRECTORY_CREATION_PARENT_NOT_A_FOLDER;

        if(parentAlreadyContainsDirectoryWithGivenName == CF_SEARCH_DIRECTORY_RECORD_BY_DIRECTORY_NAME_BEING_GIVEN_PARENT_RECORD_SUCCESS)
            return DIRECTORY_CREATION_NEW_NAME_ALREADY_EXISTS;
    }

    CatalogDirectoryRecord* recordToInsert = new CatalogDirectoryRecord();
    cf_createDirectoryRecord(volumeHeader, actualCatalogDirectoryRecord, recordToInsert, newDirectoryName, newDirectoryType);
    uint32_t insertRecordInTreeResult = cf_insertRecordInTree(diskInfo, volumeHeader, catalogFileHeaderNode, recordToInsert);

    return (insertRecordInTreeResult == CF_INSERT_RECORD_IN_TREE_SUCCESS) ? DIRECTORY_CREATION_SUCCESS : DIRECTORY_CREATION_PARENT_FAILED_TO_INSERT_RECORD_IN_CATALOG_TREE;
}

uint32_t getSubDirectoriesByParentPath(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode,
                                       char* directoryPath, std::vector<CatalogDirectoryRecord*>& subDirectories)
{
    CatalogDirectoryRecord * givenDirectoryRecord = nullptr;
    uint32_t nodeOfNewRecord;
    uint32_t searchParentInodeResult = cf_findCatalogDirectoryRecordByFullPath(diskInfo, volumeHeader, catalogFileHeaderNode, directoryPath, &givenDirectoryRecord,
                                                                            nodeOfNewRecord);

    if(searchParentInodeResult != CF_SEARCH_RECORD_IN_GIVEN_DATA_SUCCESS)
        return GET_SUBDIRECTORIES_GIVEN_DIRECTORY_DO_NOT_EXIST;

    if(givenDirectoryRecord->catalogData.recordType != DIRECTORY_TYPE_FOLDER)
        return GET_SUBDIRECTORIES_GIVEN_DIRECTORY_NOT_A_FOLDER;

    uint32_t traverseResult = cf_traverseSubtree(diskInfo, volumeHeader, catalogFileHeaderNode->headerRecord.rootNode,
                                              givenDirectoryRecord->catalogData.folderID, subDirectories);

    return (traverseResult == CF_TRAVERSE_SUBTREE_SUCCESS) ? GET_SUBDIRECTORIES_SUCCESS : GET_SUBDIRECTORIES_FAILED_FOR_OTHER_REASON;
}

uint32_t write(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, ExtentsFileHeaderNode* extentsFileHeaderNode,
               char* directoryPath, char* dataBuffer, uint32_t maxBytesToWrite, uint32_t& numberOfBytesWritten, uint32_t writeAttribute, uint32_t& reasonForIncompleteWrite)
{
    if(strcmp(directoryPath, "Root\0") == 0) //you can't write directly to root
        return WRITE_BYTES_TO_FILE_CAN_NOT_WRITE_GIVEN_FILE;

    CatalogDirectoryRecord* actualCatalogDirectoryRecord = nullptr;
    uint32_t nodeOfNewRecord, writeResult;
    uint32_t findCatalogDirectoryRecordResult = cf_findCatalogDirectoryRecordByFullPath(diskInfo, volumeHeader, catalogFileHeaderNode, directoryPath,
                                                                                     &actualCatalogDirectoryRecord, nodeOfNewRecord);

    if(findCatalogDirectoryRecordResult == CF_SEARCH_RECORD_IN_GIVEN_DATA_KEY_DO_NOT_EXIST_IN_TREE)
        return WRITE_BYTES_TO_FILE_DIRECTORY_DO_NOT_EXIST;
    else if(findCatalogDirectoryRecordResult == CF_SEARCH_RECORD_IN_GIVEN_DATA_FAILED_FOR_OTHER_REASON)
        return WRITE_BYTES_TO_FILE_FAILED_FOR_OTHER_REASON;

    //TODO what if you try to write to a file (that dont exist) when root node is not created yet? make a chekc like in createDir

    if(actualCatalogDirectoryRecord->catalogData.recordType != DIRECTORY_TYPE_FOLDER)
        return WRITE_BYTES_TO_FILE_CAN_NOT_WRITE_GIVEN_FILE;

    std::vector<HFSPlusExtentDescriptor*> foundExtents;
    if(writeAttribute == WRITE_WITH_TRUNCATE)
        writeResult = writeBytesToFileWithTruncate(diskInfo, volumeHeader, actualCatalogDirectoryRecord, catalogFileHeaderNode, extentsFileHeaderNode, dataBuffer,
                                                   maxBytesToWrite, numberOfBytesWritten, reasonForIncompleteWrite, nodeOfNewRecord, foundExtents);

    if(writeResult == WRITE_BYTES_TO_FILE_SUCCESS)
    {
        if(writeAttribute == WRITE_WITH_TRUNCATE)
        {
            std::vector<HFSPlusExtentDescriptor*> foundExtents;
            CatalogDirectoryRecord* updatedRecord = new CatalogDirectoryRecord();
            memcpy(updatedRecord, actualCatalogDirectoryRecord, sizeof(CatalogDirectoryRecord));
            updatedRecord->catalogData.fileSize = numberOfBytesWritten;
            updatedRecord->catalogData.hfsPlusForkData.totalBlocks = numberOfBytesWritten / volumeHeader->blockSize + 1;
            if(numberOfBytesWritten % volumeHeader->blockSize == 0)
                updatedRecord->catalogData.hfsPlusForkData.totalBlocks--;

            for(uint32_t i = 0; i < updatedRecord->catalogData.hfsPlusForkData.totalBlocks && i < 8; i++)
                updatedRecord->catalogData.hfsPlusForkData.extents[i] = *foundExtents[i];

            uint32_t updateRecordOnDiskResult = cf_aupdateRecordOnDisk(diskInfo, volumeHeader, actualCatalogDirectoryRecord, updatedRecord,
                                                                   nodeOfNewRecord);

            //if this is a fail we will have written and occupied blocks, but untracked by any record (trash blocks)
            return (updateRecordOnDiskResult == CF_UPDATE_RECORD_ON_DISK_SUCCESS) ? WRITE_BYTES_TO_FILE_SUCCESS : WRITE_BYTES_TO_FILE_FAILED_FOR_OTHER_REASON;
        }
    }
}