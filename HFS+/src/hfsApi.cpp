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
#include "../include/extents_file/bTreeCatalog.h"
#include "../include/extents_file/extentsFileOperations.h"
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

    updateCatalogDirectoryRecordCreatedDateAndTime(diskInfo, volumeHeader, actualCatalogDirectoryRecord, nodeOfNewRecord);
    updateCatalogDirectoryRecordLastAccessedDateAndTime(diskInfo, volumeHeader, actualCatalogDirectoryRecord, nodeOfNewRecord);
    updateCatalogDirectoryRecordLastModifiedDateAndTime(diskInfo, volumeHeader, actualCatalogDirectoryRecord, nodeOfNewRecord);

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
    uint32_t nodeOfRecord, writeResult;
    uint32_t findCatalogDirectoryRecordResult = cf_findCatalogDirectoryRecordByFullPath(diskInfo, volumeHeader, catalogFileHeaderNode, directoryPath,
                                                                                        &fileRecord, nodeOfRecord);

    if(findCatalogDirectoryRecordResult == CF_SEARCH_RECORD_IN_GIVEN_DATA_KEY_DO_NOT_EXIST_IN_TREE)
        return WRITE_BYTES_TO_FILE_GIVEN_FILE_DO_NOT_EXIST_OR_SEARCH_FAIL;
    else if(findCatalogDirectoryRecordResult == CF_SEARCH_RECORD_IN_GIVEN_DATA_FAILED_FOR_OTHER_REASON)
        return WRITE_BYTES_TO_FILE_FAILED_FOR_OTHER_REASON;

    if(fileRecord->catalogData.recordType != DIRECTORY_TYPE_FILE)
        return WRITE_BYTES_TO_FILE_CAN_NOT_WRITE_GIVEN_FILE;

    std::vector<HFSPlusExtentDescriptor*> foundExtents;
    uint32_t numberOfAlreadyExistingExtents;

    if(writeAttribute == WRITE_WITH_TRUNCATE)
        writeResult = writeBytesToFileWithTruncate(diskInfo, volumeHeader, fileRecord, extentsFileHeaderNode, dataBuffer, maxBytesToWrite, numberOfBytesWritten,
                                                   reasonForIncompleteWrite, foundExtents);
    else
        writeResult = writeBytesToFileWithAppend(diskInfo, volumeHeader, fileRecord, extentsFileHeaderNode, dataBuffer, maxBytesToWrite, numberOfBytesWritten,
                                                   reasonForIncompleteWrite, foundExtents, numberOfAlreadyExistingExtents);

    if(writeResult == WRITE_BYTES_TO_FILE_SUCCESS)
    {
        if(writeAttribute == WRITE_WITH_TRUNCATE)
        {
            uint32_t setExtentsResult = setExtentsForDirectoryRecord(diskInfo, volumeHeader, extentsFileHeaderNode, fileRecord, foundExtents,
                                                                     nodeOfRecord);

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
                                                                      nodeOfRecord);

            if(updateRecordOnDiskResult == CF_UPDATE_RECORD_ON_DISK_SUCCESS)
                memcpy(fileRecord, updatedRecord, sizeof(CatalogDirectoryRecord));

            updateCatalogDirectoryRecordLastAccessedDateAndTime(diskInfo, volumeHeader, fileRecord, nodeOfRecord);
            updateCatalogDirectoryRecordLastModifiedDateAndTime(diskInfo, volumeHeader, fileRecord, nodeOfRecord);
            //if this is a fail we will have written and occupied blocks, but untracked by any record (trash blocks)
            return (updateRecordOnDiskResult == CF_UPDATE_RECORD_ON_DISK_SUCCESS) ? WRITE_BYTES_TO_FILE_SUCCESS : WRITE_BYTES_TO_FILE_FAILED_FOR_OTHER_REASON;
        }
        else
        {
            uint32_t setExtentsResult = addExtentsToDirectoryRecord(diskInfo, volumeHeader, extentsFileHeaderNode, fileRecord, foundExtents,
                                                                    nodeOfRecord, numberOfAlreadyExistingExtents);

            if(setExtentsResult == ADD_EXTENTS_TO_DIRECTORY_RECORD_FAILED)
                return WRITE_BYTES_TO_FILE_FAILED_FOR_OTHER_REASON;

            //now update the record file size and number of blocks
            CatalogDirectoryRecord* updatedRecord = new CatalogDirectoryRecord();
            memcpy(updatedRecord, fileRecord, sizeof(CatalogDirectoryRecord));
            updatedRecord->catalogData.fileSize += numberOfBytesWritten;
            for(uint32_t i = 0; i < foundExtents.size(); i++)
                updatedRecord->catalogData.hfsPlusForkData.totalBlocks += foundExtents[i]->blockCount;

            uint32_t updateRecordOnDiskResult = cf_updateRecordOnDisk(diskInfo, volumeHeader, fileRecord, updatedRecord,
                                                                      nodeOfRecord);

            if(updateRecordOnDiskResult == CF_UPDATE_RECORD_ON_DISK_SUCCESS)
                memcpy(fileRecord, updatedRecord, sizeof(CatalogDirectoryRecord));

            updateCatalogDirectoryRecordLastAccessedDateAndTime(diskInfo, volumeHeader, fileRecord, nodeOfRecord);
            updateCatalogDirectoryRecordLastModifiedDateAndTime(diskInfo, volumeHeader, fileRecord, nodeOfRecord);
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
    std::vector<ExtentsDirectoryRecord*> dummy;
    uint32_t getAllExtentsForRecordResult = getAllExtentsForGivenDirectoryRecord(diskInfo, volumeHeader, extentsFileHeaderNode, fileRecord, extents, dummy);

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
            startBlockInExtentIndex = blockLocalIndex - localIndex;
            break;
        }

        localIndex += extents[i]->blockCount;
        i++;
    }

    //read the first extent (the one that is incomplete)
    uint32_t numOfBlocksRemainedInThisExtent = extents[actualExtentIndex]->blockCount - startBlockInExtentIndex;

    uint32_t startBlockToReadLocalIndex = startingPosition / volumeHeader->blockSize;
    uint32_t lastBlockToReadLocalIndex = (startingPosition + maxBytesToRead - 1) / volumeHeader->blockSize;
    numOfBlocksRemainedToRead = lastBlockToReadLocalIndex - startBlockToReadLocalIndex + 1;
    numOfBlocksToReadFromThisExtent = std::min(numOfBlocksRemainedToRead, numOfBlocksRemainedInThisExtent);
    uint32_t readResult = readDiskSectors(diskInfo, numOfBlocksToReadFromThisExtent * sectorsPerBlock,
                                          (extents[actualExtentIndex]->startBlock + startBlockInExtentIndex) * sectorsPerBlock, readBuffer, numOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        delete fileRecord;
        return READ_BYTES_FROM_FILE_FAILED_FOR_OTHER_REASON;
    }

    extentSize = (extents[0]->blockCount) * volumeHeader->blockSize;
    numberOfBytesRead = std::min(fileRecord->catalogData.fileSize - startingPosition, std::min(extentSize - startingPosition, maxBytesToRead));
    uint32_t startingByteInFirstBlock = startingPosition % volumeHeader->blockSize;
    memcpy(readBuffer, readBuffer + startingByteInFirstBlock, numberOfBytesRead);

    while(numberOfBytesRead < maxBytesToRead)
    {
        actualExtentIndex++;
        if(actualExtentIndex == extents.size())
        {
            updateCatalogDirectoryRecordLastAccessedDateAndTime(diskInfo, volumeHeader, fileRecord, nodeOfNewRecord);
            reasonForIncompleteRead = INCOMPLETE_BYTES_READ_DUE_TO_NO_FILE_NOT_LONG_ENOUGH;
            delete fileRecord;
            return READ_BYTES_FROM_FILE_SUCCESS;
        }

        numOfBytesRemainedToRead = maxBytesToRead - numberOfBytesRead;
        numOfBlocksRemainedToRead = numOfBytesRemainedToRead / volumeHeader->blockSize + 1;
        if(numOfBytesRemainedToRead % volumeHeader->blockSize == 0)
            numOfBlocksRemainedToRead--;

        numOfBlocksToReadFromThisExtent = std::min((int32_t) numOfBlocksRemainedToRead, (int32_t) extents[actualExtentIndex]->blockCount);

        readResult = readDiskSectors(diskInfo, numOfBlocksToReadFromThisExtent * sectorsPerBlock,
                                     extents[actualExtentIndex]->startBlock * sectorsPerBlock, readBuffer + numberOfBytesRead, numOfSectorsRead);

        if(readResult != EC_NO_ERROR)
        {
            updateCatalogDirectoryRecordLastAccessedDateAndTime(diskInfo, volumeHeader, fileRecord, nodeOfNewRecord);
            reasonForIncompleteRead = INCOMPLETE_BYTES_READ_DUE_TO_UNKNOWN;
            delete fileRecord;
            return READ_BYTES_FROM_FILE_SUCCESS;
        }

        extentSize = (extents[actualExtentIndex]->blockCount) * volumeHeader->blockSize;
        numOfBytesReadFromThisBlock = std::min(fileRecord->catalogData.fileSize - numberOfBytesRead, std::min(extentSize, maxBytesToRead - numberOfBytesRead));
        numberOfBytesRead += numOfBytesReadFromThisBlock;
    }

    updateCatalogDirectoryRecordLastAccessedDateAndTime(diskInfo, volumeHeader, fileRecord, nodeOfNewRecord);
    delete fileRecord;
    return READ_BYTES_FROM_FILE_SUCCESS;
}

uint32_t truncate(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, ExtentsFileHeaderNode* extentsFileHeaderNode,
                  char* filePath, uint32_t newSize)
{
    uint32_t indexOfLastExtentToRemain = 0, blocksAlreadyCounted = 0, numberOfBlocksToRemainInLastExtent, startBlockOfLastExtent;
//    if(extentsFileHeaderNode->headerRecord.freeNodes == extentsFileHeaderNode->headerRecord.totalNodes - 1) //root node is not yet instantiated
//        return READ_BYTES_FROM_FILE_GIVEN_FILE_DO_NOT_EXIST_OR_SEARCH_FAIL;

    CatalogDirectoryRecord* fileRecord = nullptr;
    uint32_t nodeOfRecord;
    uint32_t findCatalogDirectoryRecordResult = cf_findCatalogDirectoryRecordByFullPath(diskInfo, volumeHeader, catalogFileHeaderNode, filePath,
                                                                                        &fileRecord, nodeOfRecord);

    if(findCatalogDirectoryRecordResult == CF_SEARCH_RECORD_IN_GIVEN_DATA_KEY_DO_NOT_EXIST_IN_TREE)
        return TRUNCATE_FILE_GIVEN_FILE_DO_NOT_EXIST_OR_SEARCH_FAIL;
    else if(findCatalogDirectoryRecordResult == CF_SEARCH_RECORD_IN_GIVEN_DATA_FAILED_FOR_OTHER_REASON)
        return TRUNCATE_FILE_FAILED_FOR_OTHER_REASON;

    if(fileRecord->catalogData.recordType != DIRECTORY_TYPE_FILE)
    {
        delete fileRecord;
        return TRUNCATE_FILE_CAN_NOT_TRUNCATE_GIVEN_FILE_TYPE;
    }

    if(newSize >= fileRecord->catalogData.fileSize)
        return TRUNCATE_FILE_NEW_SIZE_GREATER_THAN_ACTUAL_SIZE;

    std::vector<HFSPlusExtentDescriptor*> extentsDescriptors;
    std::vector<ExtentsDirectoryRecord*> extentsRecords;
    uint32_t getAllExtentsForRecordResult = getAllExtentsForGivenDirectoryRecord(diskInfo, volumeHeader, extentsFileHeaderNode, fileRecord, extentsDescriptors, extentsRecords);

    if(getAllExtentsForRecordResult == GET_ALL_EXTENTS_FOR_DIRECTORY_RECORD_FAILED)
        return TRUNCATE_FILE_FAILED_FOR_OTHER_REASON;

    uint32_t numberOfBlocksToRemain = newSize / volumeHeader->blockSize + 1;
    if(newSize % volumeHeader->blockSize == 0)
        numberOfBlocksToRemain--;

    for(uint32_t i = 0; i < extentsDescriptors.size(); i++)
    {
        if(blocksAlreadyCounted + extentsDescriptors[i]->blockCount >= numberOfBlocksToRemain)
        {
            indexOfLastExtentToRemain = i;
            startBlockOfLastExtent = extentsDescriptors[i]->startBlock;
            numberOfBlocksToRemainInLastExtent = numberOfBlocksToRemain - blocksAlreadyCounted;
            if(numberOfBlocksToRemain == 0 && indexOfLastExtentToRemain != 0)
                indexOfLastExtentToRemain--;

            break;
        }

        blocksAlreadyCounted += extentsDescriptors[i]->blockCount;
    }

    //delete blocks of extentsDescriptors in surplus AND LAST EXTENT EVEN IF IT WILL STILL CONTAIN SOME BLOCKS
    for (uint32_t i = indexOfLastExtentToRemain; i < extentsDescriptors.size(); i++)
    {
        //if this fails we will have trash blocks
        for (uint32_t j = 0; j < extentsDescriptors[i]->blockCount; j++)
            changeBlockAllocationInAllocationFile(diskInfo, volumeHeader, extentsDescriptors[i]->startBlock + j, (uint8_t) 0);
    }

    //remove extentsDescriptors overflow (if the new number of extentsDescriptors <= 8, we don't need to do anything in fork)
    if(extentsRecords.size() != 0)
        for (uint32_t i = indexOfLastExtentToRemain - 8; i < extentsRecords.size(); i++)
            eof_removeRecordFromTree(diskInfo, volumeHeader, extentsFileHeaderNode, extentsRecords[i]); //if this fails, we will have trash extentsDescriptors dir entries

    //mark the surplus blocks in last record to remain as free
    if(numberOfBlocksToRemainInLastExtent != 0)
    {
        HFSPlusExtentDescriptor* extent = new HFSPlusExtentDescriptor();
        uint32_t createExtentResult = createExtentWithGivenStartBlockAndNumberOfBlocks(diskInfo, volumeHeader, startBlockOfLastExtent,
                                                                                        numberOfBlocksToRemainInLastExtent, extent);

        if(createExtentResult == CREATE_EXTENT_WITH_GIVEN_BLOCKS_FAILED) //if it fails we will lose last part of file so it will be compromised
            return TRUNCATE_FILE_FAILED_FOR_OTHER_REASON;

        std::vector<HFSPlusExtentDescriptor*> extentAsVector;
        extentAsVector.push_back(extent);
        uint32_t dummy;
        uint32_t setExtentsResult = addExtentsToDirectoryRecord(diskInfo, volumeHeader, extentsFileHeaderNode, fileRecord, extentAsVector,
                                                                nodeOfRecord, dummy);

        if(setExtentsResult == ADD_EXTENTS_TO_DIRECTORY_RECORD_FAILED)
            return TRUNCATE_FILE_FAILED_FOR_OTHER_REASON;
    }

    //now update the record file size and number of blocks
    CatalogDirectoryRecord* updatedRecord = new CatalogDirectoryRecord();
    memcpy(updatedRecord, fileRecord, sizeof(CatalogDirectoryRecord));
    updatedRecord->catalogData.fileSize = newSize;
    updatedRecord->catalogData.hfsPlusForkData.totalBlocks = numberOfBlocksToRemain;
    updatedRecord->catalogData.totalNumOfExtents = indexOfLastExtentToRemain + 1; //the index is indexed from 0 so that's why we add 1
    if(newSize == 0)
        updatedRecord->catalogData.totalNumOfExtents = 0;

    uint32_t updateRecordOnDiskResult = cf_updateRecordOnDisk(diskInfo, volumeHeader, fileRecord, updatedRecord, nodeOfRecord);

    if(updateRecordOnDiskResult == CF_UPDATE_RECORD_ON_DISK_SUCCESS)
        memcpy(fileRecord, updatedRecord, sizeof(CatalogDirectoryRecord));

    updateCatalogDirectoryRecordLastAccessedDateAndTime(diskInfo, volumeHeader, fileRecord, nodeOfRecord);
    updateCatalogDirectoryRecordLastModifiedDateAndTime(diskInfo, volumeHeader, fileRecord, nodeOfRecord);
    return (updateRecordOnDiskResult == CF_UPDATE_RECORD_ON_DISK_SUCCESS) ? TRUNCATE_FILE_SUCCESS : TRUNCATE_FILE_FAILED_FOR_OTHER_REASON;
}

uint32_t deleteDirectoryByPath(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, ExtentsFileHeaderNode* extentsFileHeaderNode,
                               char* directoryPath)
{
    if(strcmp(directoryPath, "Root\0") == 0)
        return DELETE_DIRECTORY_CAN_NOT_DELETE_ROOT;

    CatalogDirectoryRecord* directoryRecord = nullptr;
    uint32_t nodeOfNewRecord;
    uint32_t findCatalogDirectoryRecordResult = cf_findCatalogDirectoryRecordByFullPath(diskInfo, volumeHeader, catalogFileHeaderNode, directoryPath,
                                                                                        &directoryRecord, nodeOfNewRecord);

    if(findCatalogDirectoryRecordResult == CF_SEARCH_RECORD_IN_GIVEN_DATA_KEY_DO_NOT_EXIST_IN_TREE)
        return DELETE_DIRECTORY_DIRECTORY_DO_NOT_EXIST_OR_SEARCH_FAIL;
    else if(findCatalogDirectoryRecordResult == CF_SEARCH_RECORD_IN_GIVEN_DATA_FAILED_FOR_OTHER_REASON)
        return DELETE_DIRECTORY_FAILED_FOR_OTHER_REASON;

    uint32_t deleteDirectoryResult = deleteDirectoryAndChildren(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, directoryRecord);
    return (deleteDirectoryResult == DELETE_DIRECTORY_RECORD_AND_RELATED_DATA_SUCCESS) ? DELETE_DIRECTORY_SUCCESS : DELETE_DIRECTORY_FAILED_FOR_OTHER_REASON;
}

uint32_t getDirectoryDisplayableAttributes(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode,
                                           char* directoryPath, DirectoryDisplayableAttributes* attributes)
{
//    if(extentsFileHeaderNode->headerRecord.freeNodes == extentsFileHeaderNode->headerRecord.totalNodes - 1) //root node is not yet instantiated
//        return READ_BYTES_FROM_FILE_GIVEN_FILE_DO_NOT_EXIST_OR_SEARCH_FAIL;

    CatalogDirectoryRecord *directoryRecord = nullptr;
    uint32_t nodeOfRecord;
    uint32_t findCatalogDirectoryRecordResult = cf_findCatalogDirectoryRecordByFullPath(diskInfo, volumeHeader,
                                                                                        catalogFileHeaderNode, directoryPath,
                                                                                        &directoryRecord, nodeOfRecord);

    if (findCatalogDirectoryRecordResult == CF_SEARCH_RECORD_IN_GIVEN_DATA_KEY_DO_NOT_EXIST_IN_TREE)
        return DIRECTORY_GET_DISPLAYABLE_ATTRIBUTES_FAILED_GIVEN_DIRECTORY_DO_NOT_EXIST_OR_SEARCH_ERROR;
    else if (findCatalogDirectoryRecordResult == CF_SEARCH_RECORD_IN_GIVEN_DATA_FAILED_FOR_OTHER_REASON)
        return DIRECTORY_GET_DISPLAYABLE_ATTRIBUTES_FAILED_FOR_OTHER_REASON;

    uint32_t sizeOnDisk = directoryRecord->catalogData.hfsPlusForkData.totalBlocks * volumeHeader->blockSize;
    uint32_t size = directoryRecord->catalogData.fileSize;

    attributes->FileSizeOnDisk = sizeOnDisk;
    attributes->FileSize = size;

    attributes->CreatedYear = (directoryRecord->catalogData.createDate >> 25) + 1900;
    attributes->CreatedMonth = (directoryRecord->catalogData.createDate >> 21) & 0x0000000F;
    attributes->CreatedDay = (directoryRecord->catalogData.createDate >> 16) & 0x0000001F;
    attributes->CreatedHour = (directoryRecord->catalogData.createDate & 0x0000FFFF) * 2 / 3600;
    attributes->CreatedMinute = (directoryRecord->catalogData.createDate & 0x0000FFFF) * 2 % 3600 / 60;
    attributes->CreatedSecond = (directoryRecord->catalogData.createDate & 0x0000FFFF) * 2 % 3600 % 60;

    attributes->LastAccessedYear = (directoryRecord->catalogData.accessDate >> 25) + 1900;
    attributes->LastAccessedMonth = (directoryRecord->catalogData.accessDate >> 21) & 0x0000000F;
    attributes->LastAccessedDay = (directoryRecord->catalogData.accessDate >> 16) & 0x0000001F;
    attributes->LastAccessedHour = (directoryRecord->catalogData.accessDate & 0x0000FFFF) * 2 / 3600;
    attributes->LastAccessedMinute = (directoryRecord->catalogData.accessDate & 0x0000FFFF) * 2 % 3600 / 60;
    attributes->LastAccessedSecond = (directoryRecord->catalogData.accessDate & 0x0000FFFF) * 2 % 3600 % 60;

    attributes->LastChangeYear = (directoryRecord->catalogData.contentModDate >> 25) + 1900;
    attributes->LastChangeMonth = (directoryRecord->catalogData.contentModDate >> 21) & 0x0000000F;
    attributes->LastChangeDay = (directoryRecord->catalogData.contentModDate >> 16) & 0x0000001F;
    attributes->LastChangeHour = (directoryRecord->catalogData.contentModDate & 0x0000FFFF) * 2 / 3600;
    attributes->LastChangeMinute = (directoryRecord->catalogData.contentModDate & 0x0000FFFF) * 2 % 3600 / 60;
    attributes->LastChangeSecond = (directoryRecord->catalogData.contentModDate & 0x0000FFFF) * 2 % 3600 % 60;

    updateCatalogDirectoryRecordLastAccessedDateAndTime(diskInfo, volumeHeader, directoryRecord, nodeOfRecord);

    return DIRECTORY_GET_DISPLAYABLE_ATTRIBUTES_SUCCESS;
}