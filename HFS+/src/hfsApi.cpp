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
#include "../include/codes/hfsCodes.h"
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

    CatalogDirectoryRecord* recordToInsert = cf_createDirectoryRecord(diskInfo, volumeHeader, actualCatalogDirectoryRecord, newDirectoryName,
                             newDirectoryType);
    uint32_t insertRecordInTreeResult = cf_insertRecordInTree(diskInfo, volumeHeader, catalogFileHeaderNode, recordToInsert);

    return (insertRecordInTreeResult == CF_INSERT_RECORD_IN_TREE_SUCCESS) ? DIRECTORY_CREATION_SUCCESS : DIRECTORY_CREATION_PARENT_FAILED_TO_INSERT_RECORD_IN_CATALOG_TREE;
}

uint32_t getSubDirectoriesByParentPath(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode,
                                       char* directoryPath, std::vector<CatalogDirectoryRecord*>& subDirectories)
{
    if(catalogFileHeaderNode->headerRecord.freeNodes == catalogFileHeaderNode->headerRecord.totalNodes - 1) //root node is not yet instantiated
        return GET_SUBDIRECTORIES_GIVEN_DIRECTORY_DO_NOT_EXIST;

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

//    if(extentsFileHeaderNode->headerRecord.freeNodes == extentsFileHeaderNode->headerRecord.totalNodes - 1) //root node is not yet instantiated
//        return WRITE_BYTES_TO_FILE_GIVEN_FILE_DO_NOT_EXIST_OR_SEARCH_FAIL;

    CatalogDirectoryRecord* fileRecord = nullptr;
    uint32_t nodeOfNewRecord, writeResult;
    uint32_t findCatalogDirectoryRecordResult = cf_findCatalogDirectoryRecordByFullPath(diskInfo, volumeHeader, catalogFileHeaderNode, directoryPath,
                                                                                        &fileRecord, nodeOfNewRecord);

    if(findCatalogDirectoryRecordResult == CF_SEARCH_RECORD_IN_GIVEN_DATA_KEY_DO_NOT_EXIST_IN_TREE)
        return WRITE_BYTES_TO_FILE_GIVEN_FILE_DO_NOT_EXIST_OR_SEARCH_FAIL;
    else if(findCatalogDirectoryRecordResult == CF_SEARCH_RECORD_IN_GIVEN_DATA_FAILED_FOR_OTHER_REASON)
        return WRITE_BYTES_TO_FILE_FAILED_FOR_OTHER_REASON;

    if(fileRecord->catalogData.recordType != DIRECTORY_TYPE_FILE)
        return WRITE_BYTES_TO_FILE_CAN_NOT_WRITE_GIVEN_FILE;

    std::vector<HFSPlusExtentDescriptor*> foundExtents;
    if(writeAttribute == WRITE_WITH_TRUNCATE)
        writeResult = writeBytesToFileWithTruncate(diskInfo, volumeHeader, fileRecord, catalogFileHeaderNode, extentsFileHeaderNode, dataBuffer,
                                                   maxBytesToWrite, numberOfBytesWritten, reasonForIncompleteWrite, nodeOfNewRecord, foundExtents);

    if(writeResult == WRITE_BYTES_TO_FILE_SUCCESS)
    {
        if(writeAttribute == WRITE_WITH_TRUNCATE)
        {
            uint32_t setExtentsResult = setExtentsForDirectoryRecord(diskInfo, volumeHeader, extentsFileHeaderNode, fileRecord, foundExtents,
                                                                     nodeOfNewRecord);

            if(setExtentsResult == SET_EXTENTS_FOR_DIRECTORY_RECORD_FAILED)
                return WRITE_BYTES_TO_FILE_FAILED_FOR_OTHER_REASON;

            //now update the record file size and number of blocks
            CatalogDirectoryRecord* updatedRecord = new CatalogDirectoryRecord();
            memcpy(updatedRecord, fileRecord, sizeof(CatalogDirectoryRecord));
            updatedRecord->catalogData.fileSize = numberOfBytesWritten;
            updatedRecord->catalogData.hfsPlusForkData.totalBlocks = numberOfBytesWritten / volumeHeader->blockSize + 1;
            if(numberOfBytesWritten % volumeHeader->blockSize == 0)
                updatedRecord->catalogData.hfsPlusForkData.totalBlocks--;

            uint32_t updateRecordOnDiskResult = cf_updateRecordOnDisk(diskInfo, volumeHeader, fileRecord, updatedRecord,
                                                                      nodeOfNewRecord);

            //if this is a fail we will have written and occupied blocks, but untracked by any record (trash blocks)
            return (updateRecordOnDiskResult == CF_UPDATE_RECORD_ON_DISK_SUCCESS) ? WRITE_BYTES_TO_FILE_SUCCESS : WRITE_BYTES_TO_FILE_FAILED_FOR_OTHER_REASON;
        }
    }
}

uint32_t read(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, ExtentsFileHeaderNode* extentsFileHeaderNode,
              char* filePath, char* readBuffer, uint32_t startingPosition, uint32_t maxBytesToRead, uint32_t& numberOfBytesRead, uint32_t& reasonForIncompleteRead)
{
    if(strcmp(filePath, "Root\0") == 0) //you can't write directly to root
        return READ_BYTES_FROM_FILE_CAN_NOT_READ_GIVEN_FILE;

//    if(extentsFileHeaderNode->headerRecord.freeNodes == extentsFileHeaderNode->headerRecord.totalNodes - 1) //root node is not yet instantiated
//        return READ_BYTES_FROM_FILE_GIVEN_FILE_DO_NOT_EXIST_OR_SEARCH_FAIL;

    CatalogDirectoryRecord* fileRecord = nullptr;
    uint32_t nodeOfNewRecord;
    uint32_t findCatalogDirectoryRecordResult = cf_findCatalogDirectoryRecordByFullPath(diskInfo, volumeHeader, catalogFileHeaderNode, filePath,
                                                                                        &fileRecord, nodeOfNewRecord);

    if(findCatalogDirectoryRecordResult == CF_SEARCH_RECORD_IN_GIVEN_DATA_KEY_DO_NOT_EXIST_IN_TREE)
        return READ_BYTES_FROM_FILE_GIVEN_FILE_DO_NOT_EXIST_OR_SEARCH_FAIL;
    else if(findCatalogDirectoryRecordResult == CF_SEARCH_RECORD_IN_GIVEN_DATA_FAILED_FOR_OTHER_REASON)
        return READ_BYTES_FROM_FILE_FAILED_FOR_OTHER_REASON;

    if(fileRecord->catalogData.recordType != DIRECTORY_TYPE_FILE)
    {
        delete fileRecord;
        return READ_BYTES_FROM_FILE_CAN_NOT_READ_GIVEN_FILE;
    }

    if(startingPosition > fileRecord->catalogData.fileSize)
    {
        delete fileRecord;
        return READ_BYTES_FROM_FILE_GIVEN_START_EXCEEDS_FILE_SIZE;
    }

    uint32_t numOfSectorsRead, numOfBytesReadFromThisBlock, numOfBytesRemainedToRead, numOfBlocksRemainedToRead, numOfBlocksToReadFromThisExtent, extentSize;
    uint32_t sectorsPerBlock = getNumberOfSectorsPerBlock(diskInfo, volumeHeader);

    std::vector<HFSPlusExtentDescriptor*> extents;
    uint32_t getAllExtentsForRecordResult = getAllExtentsForGivenDirectoryRecord(diskInfo, volumeHeader, extentsFileHeaderNode, fileRecord, extents);

    if(getAllExtentsForRecordResult == GET_ALL_EXTENTS_FOR_DIRECTORY_RECORD_FAILED)
    {
        delete fileRecord;
        return READ_BYTES_FROM_FILE_FAILED_FOR_OTHER_REASON;
    }

    uint32_t blockLocalIndex = startingPosition / volumeHeader->blockSize;

    //look for the extent of the first block we are reading from
    uint32_t localIndex = 0, actualExtentIndex, startBlockInExtentIndex, i = 0;
    while(localIndex < blockLocalIndex)
    {
        if(localIndex + extents[i]->blockCount >= blockLocalIndex)
        {
            actualExtentIndex = i; //when we read the first extent to read from this will pe the start extent index
            startBlockInExtentIndex = blockLocalIndex - localIndex - 1; //we decrease 1 because blocks are considered from 0 in extent
            break;
        }

        localIndex += extents[i]->blockCount;
        i++;
    }

    //read the first extent (the one that is incomplete)
    uint32_t numOfBlocksRemainedInThisExtent = extents[actualExtentIndex]->blockCount - startBlockInExtentIndex;

    numOfBlocksRemainedToRead = maxBytesToRead / volumeHeader->blockSize + 1;
    if(maxBytesToRead % volumeHeader->blockSize == 0)
        numOfBlocksRemainedToRead--;

    numOfBlocksToReadFromThisExtent = std::min(numOfBlocksRemainedToRead, numOfBlocksRemainedInThisExtent);
    uint32_t readResult = readDiskSectors(diskInfo, numOfBlocksToReadFromThisExtent * sectorsPerBlock,
                                          (extents[actualExtentIndex]->startBlock + actualExtentIndex) * sectorsPerBlock, readBuffer, numOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        delete fileRecord;
        return READ_BYTES_FROM_FILE_FAILED_FOR_OTHER_REASON;
    }

    extentSize = (extents[0]->blockCount) * volumeHeader->blockSize;
    numberOfBytesRead = std::min(fileRecord->catalogData.fileSize - startingPosition, std::min(extentSize - startingPosition, maxBytesToRead));
    memcpy(readBuffer, readBuffer + startingPosition, numberOfBytesRead);

    while(numberOfBytesRead < maxBytesToRead)
    {
        actualExtentIndex++;
        if(actualExtentIndex == extents.size())
        {
            reasonForIncompleteRead = INCOMPLETE_BYTES_READ_DUE_TO_NO_FILE_NOT_LONG_ENOUGH;
            delete fileRecord;
            return READ_BYTES_FROM_FILE_SUCCESS;
        }

        numOfBytesRemainedToRead = maxBytesToRead - numberOfBytesRead;
        numOfBlocksRemainedToRead = numOfBytesRemainedToRead / volumeHeader->blockSize + 1;
        if(numOfBytesRemainedToRead % volumeHeader->blockSize == 0)
            numOfBlocksRemainedToRead--;

        numOfBlocksToReadFromThisExtent = std::min(numOfBlocksRemainedToRead, extents[actualExtentIndex]->blockCount);

        readResult = readDiskSectors(diskInfo, numOfBlocksToReadFromThisExtent * sectorsPerBlock,
                                     extents[actualExtentIndex]->startBlock * sectorsPerBlock, readBuffer + numberOfBytesRead, numOfSectorsRead);

        if(readResult != EC_NO_ERROR)
        {
            reasonForIncompleteRead = INCOMPLETE_BYTES_READ_DUE_TO_UNKNOWN;
            delete fileRecord;
            return READ_BYTES_FROM_FILE_SUCCESS;
        }

        extentSize = (extents[0]->blockCount) * volumeHeader->blockSize;
        numOfBytesReadFromThisBlock = std::min(extentSize - startingPosition - numberOfBytesRead, std::min(extentSize, maxBytesToRead - numberOfBytesRead));
        numberOfBytesRead += numOfBytesReadFromThisBlock;
    }

    delete fileRecord;
    return READ_BYTES_FROM_FILE_SUCCESS;
}