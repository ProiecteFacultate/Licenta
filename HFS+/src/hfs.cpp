#include "iostream"
#include "string.h"
#include "vector"
#include <algorithm>

#include "disk.h"
#include "diskCodes.h"
#include "../include/structures.h"
#include "../include/utils.h"
#include "../include/hfsFunctionUtils.h"
#include "../include/codes/hfsCodes.h"
#include "../include/catalog_file/catalogFileOperations.h"
#include "../include/catalog_file/bTreeCatalog.h"
#include "../include/catalog_file/codes/catalogFileResponseCodes.h"
#include "../include/catalog_file/codes/bTreeResponseCodes.h"
#include "../include/extents_file/extentsFileOperations.h"
#include "../include/extents_file/bTreeCatalog.h"
#include "../include/extents_file/extentsFileUtils.h"
#include "../include/extents_file/codes/extentsFileResponseCodes.h"
#include "../include/extents_file/codes/bTreeResponseCodes.h"
#include "../include/codes/hfsCodes.h"
#include "../include/codes/hfsApiResponseCodes.h"
#include "../include/codes/hfsAttributes.h"
#include "../include/hfs.h"

uint32_t writeBytesToFileWithTruncate(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogDirectoryRecord* fileRecord, ExtentsFileHeaderNode* extentsFileHeaderNode,
                                      char* dataBuffer, uint32_t maxBytesToWrite, uint32_t& numberOfBytesWritten, uint32_t& reasonForIncompleteWrite,
                                      std::vector<HFSPlusExtentDescriptor*>& foundExtents)
{
    uint32_t searchExtentResult, extentSize, startPositionInBufferToWriteFrom, writeDataToExtentResult;

    //we clear all existing extents, new ones will be added
    uint32_t clearDirectoryRecordDataResult = clearDirectoryRecordData(diskInfo, volumeHeader, extentsFileHeaderNode, fileRecord);

    if(clearDirectoryRecordDataResult == CLEAR_DIRECTORY_RECORD_DATA_FAILED)
        return WRITE_BYTES_TO_FILE_FAILED_FOR_OTHER_REASON;

    if(maxBytesToWrite == 0) //it might be given 0
        return WRITE_BYTES_TO_FILE_SUCCESS;

    uint32_t numberOfBlocksRequired = maxBytesToWrite / volumeHeader->blockSize + 1;
    if(maxBytesToWrite % volumeHeader->blockSize == 0)
        numberOfBlocksRequired--;

    uint32_t foundBlocks = 0;
    //when we don't found an extent the size we want we reduce and look for an extent the size of the biggest extent found previously instead of starting from num of blocks still
    // not found and then decreasing one by one; it is for efficiency
    uint32_t preferableExtentSize = numberOfBlocksRequired;

    //search extents and write to them
    while(foundBlocks < numberOfBlocksRequired)
    {
        preferableExtentSize = std::min(preferableExtentSize, numberOfBlocksRequired - foundBlocks);

        for(extentSize = preferableExtentSize; extentSize >= 1; extentSize--)
        {
            HFSPlusExtentDescriptor* extent = new HFSPlusExtentDescriptor();
            searchExtentResult = searchFreeExtentOfGivenNumberOfBlocks(diskInfo, volumeHeader, extentSize, extent);

            if(searchExtentResult == SEARCH_FREE_EXTENT_FAILED_FOR_OTHER_REASON)
            {
                numberOfBytesWritten = foundBlocks * volumeHeader->blockSize;
                reasonForIncompleteWrite = INCOMPLETE_BYTES_WRITE_DUE_TO_UNKNOWN;

                //if we partially managed to write (more than 0 bytes) it is considered a success
                return (foundBlocks != 0) ? WRITE_BYTES_TO_FILE_SUCCESS : WRITE_BYTES_TO_FILE_FAILED_FOR_OTHER_REASON;
            }
            else if(searchExtentResult == SEARCH_FREE_EXTENT_SUCCESS)
            {
                //write the extent (writeDataToExtent method also marks the blocks as occupied in allocation file)
                startPositionInBufferToWriteFrom = foundBlocks * volumeHeader->blockSize;
                writeDataToExtentResult = writeDataToExtent(diskInfo, volumeHeader, extent, dataBuffer + startPositionInBufferToWriteFrom);

                if(writeDataToExtentResult == WRITE_DATA_TO_EXTENT_FAILED)
                {
                    numberOfBytesWritten = foundBlocks * volumeHeader->blockSize;
                    reasonForIncompleteWrite = INCOMPLETE_BYTES_WRITE_DUE_TO_UNKNOWN;

                    //if we partially managed to write (more than 0 bytes) it is considered a success
                    return (foundBlocks != 0) ? WRITE_BYTES_TO_FILE_SUCCESS : WRITE_BYTES_TO_FILE_FAILED_FOR_OTHER_REASON;
                }

                foundExtents.push_back(extent);
                preferableExtentSize = extent->blockCount;
                foundBlocks += extent->blockCount;
                break;
            }
            //else we haven't found an extent with preferable size so extentSize is decreased
        }

        if(extentSize == 0) //it means there is no free block!!
        {
            numberOfBytesWritten = foundBlocks * volumeHeader->blockSize;
            reasonForIncompleteWrite = INCOMPLETE_BYTES_WRITE_DUE_TO_NO_FREE_BLOCKS;

            //if we partially managed to write (more than 0 bytes) it is considered a success
            return (foundBlocks != 0) ? WRITE_BYTES_TO_FILE_SUCCESS : WRITE_BYTES_TO_FILE_FAILED_DUE_TO_NO_FREE_BLOCKS;
        }

        if(foundBlocks == numberOfBlocksRequired)
            numberOfBytesWritten = maxBytesToWrite;
    }

    return WRITE_BYTES_TO_FILE_SUCCESS;
}

uint32_t writeBytesToFileWithAppend(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogDirectoryRecord* fileRecord, ExtentsFileHeaderNode* extentsFileHeaderNode,
                                    char* dataBuffer, uint32_t maxBytesToWrite, uint32_t& numberOfBytesWritten, uint32_t& reasonForIncompleteWrite,
                                    std::vector<HFSPlusExtentDescriptor*>& newExtents, uint32_t& numberOfAlreadyExistingExtents)
{
    uint32_t searchExtentResult, extentSize, writeDataToExtentResult, startPositionInBufferToWriteFrom, numOfSectorsRead, numOfSectorsWritten;
    uint32_t sectorsPerBlock = getNumberOfSectorsPerBlock(diskInfo, volumeHeader);

    if(maxBytesToWrite == 0) //it might be given 0
        return WRITE_BYTES_TO_FILE_SUCCESS;

    std::vector<HFSPlusExtentDescriptor*> extents;
    std::vector<ExtentsDirectoryRecord*> dummy;
    uint32_t getAllExtentsForRecordResult = getAllExtentsForGivenDirectoryRecord(diskInfo, volumeHeader, extentsFileHeaderNode, fileRecord, extents, dummy);

    if(getAllExtentsForRecordResult == GET_ALL_EXTENTS_FOR_DIRECTORY_RECORD_FAILED)
        return WRITE_BYTES_TO_FILE_FAILED_FOR_OTHER_REASON;
    numberOfAlreadyExistingExtents = extents.size();

    uint32_t lastBlockOfLastExtent = extents[extents.size() - 1]->startBlock + extents[extents.size() - 1]->blockCount - 1; //aka last block of file
    uint32_t numOfBytesWrittenInLastBlock = fileRecord->catalogData.fileSize % volumeHeader->blockSize;
    uint32_t numOfFreeBytesRemainedInLastBlock = volumeHeader->blockSize - numOfBytesWrittenInLastBlock;
    char* blockBuffer = new char[volumeHeader->blockSize];
    uint32_t readResult = readDiskSectors(diskInfo, sectorsPerBlock, lastBlockOfLastExtent * sectorsPerBlock, blockBuffer, numOfSectorsRead);

    if(readResult != EC_NO_ERROR)
        return WRITE_BYTES_TO_FILE_FAILED_FOR_OTHER_REASON;

    memcpy(blockBuffer + numOfBytesWrittenInLastBlock, dataBuffer, std::min(maxBytesToWrite, numOfFreeBytesRemainedInLastBlock));

    uint32_t writeResult = writeDiskSectors(diskInfo, sectorsPerBlock, lastBlockOfLastExtent * sectorsPerBlock, blockBuffer, numOfSectorsWritten);

    if(writeResult != EC_NO_ERROR)
        return WRITE_BYTES_TO_FILE_FAILED_FOR_OTHER_REASON;

    if(maxBytesToWrite <= numOfFreeBytesRemainedInLastBlock)
    {
        numberOfBytesWritten = maxBytesToWrite;
        return WRITE_BYTES_TO_FILE_SUCCESS;
    }

    numberOfBytesWritten = numOfFreeBytesRemainedInLastBlock;
    uint32_t bytesToWriteRemainedAfterFillingLastBlock = maxBytesToWrite - numOfFreeBytesRemainedInLastBlock;
    uint32_t numberOfBlocksRequired = bytesToWriteRemainedAfterFillingLastBlock / volumeHeader->blockSize + 1;

    if(bytesToWriteRemainedAfterFillingLastBlock % volumeHeader->blockSize == 0)
        numberOfBlocksRequired--;

    uint32_t foundBlocks = 0;
    //when we don't found an extent the size we want we reduce and look for an extent the size of the biggest extent found previously instead of starting from num of blocks still
    // not found and then decreasing one by one; it is for efficiency
    uint32_t preferableExtentSize = numberOfBlocksRequired;

    //search extents and write to them
    while(foundBlocks < numberOfBlocksRequired)
    {
        preferableExtentSize = std::min(preferableExtentSize, numberOfBlocksRequired - foundBlocks);

        for(extentSize = preferableExtentSize; extentSize >= 1; extentSize--)
        {
            HFSPlusExtentDescriptor* extent = new HFSPlusExtentDescriptor();
            searchExtentResult = searchFreeExtentOfGivenNumberOfBlocks(diskInfo, volumeHeader, extentSize, extent);

            if(searchExtentResult == SEARCH_FREE_EXTENT_FAILED_FOR_OTHER_REASON)
            {
                numberOfBytesWritten = foundBlocks * volumeHeader->blockSize;
                reasonForIncompleteWrite = INCOMPLETE_BYTES_WRITE_DUE_TO_UNKNOWN;

                //if we partially managed to write (more than 0 bytes) it is considered a success
                return (foundBlocks != 0) ? WRITE_BYTES_TO_FILE_SUCCESS : WRITE_BYTES_TO_FILE_FAILED_FOR_OTHER_REASON;
            }
            else if(searchExtentResult == SEARCH_FREE_EXTENT_SUCCESS)
            {
                //write the extent (writeDataToExtent method also marks the blocks as occupied in allocation file)
                startPositionInBufferToWriteFrom = numOfFreeBytesRemainedInLastBlock + foundBlocks * volumeHeader->blockSize;
                writeDataToExtentResult = writeDataToExtent(diskInfo, volumeHeader, extent, dataBuffer + startPositionInBufferToWriteFrom);

                if(writeDataToExtentResult == WRITE_DATA_TO_EXTENT_FAILED)
                {
                    numberOfBytesWritten = foundBlocks * volumeHeader->blockSize;
                    reasonForIncompleteWrite = INCOMPLETE_BYTES_WRITE_DUE_TO_UNKNOWN;

                    //if we partially managed to write (more than 0 bytes) it is considered a success
                    return (foundBlocks != 0) ? WRITE_BYTES_TO_FILE_SUCCESS : WRITE_BYTES_TO_FILE_FAILED_FOR_OTHER_REASON;
                }

                newExtents.push_back(extent);
                preferableExtentSize = extent->blockCount;
                foundBlocks += extent->blockCount;
                break;
            }
            //else we haven't found an extent with preferable size so extentSize is decreased
        }

        if(extentSize == 0) //it means there is no free block!!
        {
            numberOfBytesWritten = foundBlocks * volumeHeader->blockSize;
            reasonForIncompleteWrite = INCOMPLETE_BYTES_WRITE_DUE_TO_NO_FREE_BLOCKS;

            //if we partially managed to write (more than 0 bytes) it is considered a success
            return (foundBlocks != 0) ? WRITE_BYTES_TO_FILE_SUCCESS : WRITE_BYTES_TO_FILE_FAILED_DUE_TO_NO_FREE_BLOCKS;
        }

        if(foundBlocks == numberOfBlocksRequired)
            numberOfBytesWritten = maxBytesToWrite;
    }

    return WRITE_BYTES_TO_FILE_SUCCESS;
}

void updateVolumeHeaderNodeOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, HFSPlusVolumeHeader* updatedVolumeHeader)
{
    uint32_t numberOfSectorsWritten, retryWriteCount = 2;
    uint32_t sector = getFirstSectorForGivenBlock(diskInfo, volumeHeader, getFirstBlockForVolumeHeader(volumeHeader->blockSize));
    char* blockBuffer = new char[volumeHeader->blockSize];

    if(diskInfo->diskParameters.sectorSizeBytes <= volumeHeader->blockSize) //if blockSize is 512 or 1024 we write from the beginning of block
        memcpy(blockBuffer, updatedVolumeHeader, sizeof(HFSPlusVolumeHeader));
    else
        memcpy(blockBuffer + 1024, updatedVolumeHeader, sizeof(HFSPlusVolumeHeader));

    uint32_t writeResult = writeDiskSectors(diskInfo, 1, sector, blockBuffer, numberOfSectorsWritten);

    while(writeResult != EC_NO_ERROR && retryWriteCount > 0)
    {
        writeResult = writeDiskSectors(diskInfo, 1, sector, blockBuffer, numberOfSectorsWritten);
        retryWriteCount--;
    }

    if(retryWriteCount == 0)
        throw std::runtime_error("Failed to initialize volume header!");

    delete[] blockBuffer;
}

/////////////////////////////////////////

static uint32_t writeDataToExtent(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, HFSPlusExtentDescriptor* extent, char* data)
{
    uint32_t numOfSectorsWritten;
    uint32_t writeResult = writeDiskSectors(diskInfo, extent->blockCount * getNumberOfSectorsPerBlock(diskInfo, volumeHeader),
                                            extent->startBlock * getNumberOfSectorsPerBlock(diskInfo, volumeHeader), data, numOfSectorsWritten);

    if(writeResult != EC_NO_ERROR)
        return WRITE_DATA_TO_EXTENT_FAILED;

    //now mark the blocks of the extent as occupied in allocation file
    for(uint32_t i = 0; i < extent->blockCount; i++)
    {
        uint32_t changeBlockAllocationResult = changeBlockAllocationInAllocationFile(diskInfo, volumeHeader, extent->startBlock + i, (uint8_t) 1);
        if(changeBlockAllocationResult == CHANGE_BLOCK_ALLOCATION_FAILED)
            return WRITE_DATA_TO_EXTENT_FAILED;
    }

    return WRITE_DATA_TO_EXTENT_SUCCESS;
}

static uint32_t searchFreeExtentOfGivenNumberOfBlocks(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, uint32_t numberOfBlocksInExtent, HFSPlusExtentDescriptor* foundExtent)
{
    uint32_t numOfSectorsRead, readResult, blockLocalIndex, blockGlobalIndex, byteIndexInBuffer, bitIndexInByte, readBatchSize = 10; //how many blocks from allocation file to read at once
    foundExtent->startBlock = 0;
    uint32_t firstDataBlockGlobalIndex = volumeHeader->catalogFile.extents[0].startBlock + volumeHeader->catalogFile.extents[0].blockCount; //the first bitIndex in allocation file
    uint32_t totalNumberOfDataBlocks = volumeHeader->totalBlocks - firstDataBlockGlobalIndex + 1;
    uint32_t numOfBlocksRepresentedInAnAllocationBlock = volumeHeader->blockSize * 8;

    char* allocationFileBlocks = new char[readBatchSize * volumeHeader->blockSize];

    for(uint32_t blockIndex = 0; blockIndex < totalNumberOfDataBlocks; blockIndex++) //blockIndex is the same as bitIndex
    {
        byteIndexInBuffer = (blockIndex / 8) % (readBatchSize * volumeHeader->blockSize);
        bitIndexInByte = blockIndex % 8;
        blockLocalIndex = (blockIndex / 8) / volumeHeader->blockSize; //the block inside allocation file that contain the representation for the blockIndex (bitIndex)

        if(blockIndex % (readBatchSize * numOfBlocksRepresentedInAnAllocationBlock) == 0) //we need to read a new batch of allocation file blocks
        {
            blockGlobalIndex = volumeHeader->allocationFile.extents[0].startBlock + blockLocalIndex;
            readResult = readDiskSectors(diskInfo, readBatchSize * getNumberOfSectorsPerBlock(diskInfo, volumeHeader),
                                                  getFirstSectorForGivenBlock(diskInfo, volumeHeader, blockGlobalIndex), allocationFileBlocks, numOfSectorsRead);

            if(readResult != EC_NO_ERROR)
            {
                delete[] allocationFileBlocks;
                return SEARCH_FREE_EXTENT_FAILED_FOR_OTHER_REASON;
            }
        }

        if(getBitFromByte(allocationFileBlocks[byteIndexInBuffer], bitIndexInByte) == 0)
        {
            if(foundExtent->startBlock == 0)
            {
                foundExtent->startBlock = firstDataBlockGlobalIndex + blockIndex;
                foundExtent->blockCount = 1;
            }
            else
                foundExtent->blockCount++;

            if(foundExtent->blockCount == numberOfBlocksInExtent)
            {
                delete[] allocationFileBlocks;
                return SEARCH_FREE_EXTENT_SUCCESS;
            }
        }
        else
            foundExtent->startBlock = 0;
    }

    delete[] allocationFileBlocks;
    return SEARCH_FREE_EXTENT_NO_EXTENT_WITH_DESIRED_NUMBER_OF_BLOCKS;
}

uint32_t setExtentsForDirectoryRecord(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsFileHeaderNode* extentsFileHeaderNode, CatalogDirectoryRecord* fileRecord,
                                      std::vector<HFSPlusExtentDescriptor*> extents, uint32_t nodeNumberOfRecord)
{
    CatalogDirectoryRecord* updatedFileRecord = new CatalogDirectoryRecord();
    memcpy(updatedFileRecord, fileRecord, sizeof(CatalogDirectoryRecord));

    for(uint32_t i = 0; i < extents.size() && i < 8; i++)
    {
        updatedFileRecord->catalogData.hfsPlusForkData.extents[i] = *extents[i];
        updatedFileRecord->catalogData.hfsPlusForkData.totalBlocks += extents[i]->blockCount;
    }

    updatedFileRecord->catalogData.totalNumOfExtents = extents.size();
    uint32_t updateResult = cf_updateRecordOnDisk(diskInfo, volumeHeader, fileRecord, updatedFileRecord, nodeNumberOfRecord);

    if(updateResult == CF_UPDATE_RECORD_ON_DISK_FAILED)
        return SET_EXTENTS_FOR_DIRECTORY_RECORD_FAILED;

    memcpy(fileRecord, updatedFileRecord, sizeof(CatalogDirectoryRecord));

    //now add the extents thar are more than 8
    for(uint32_t i = 8; i < extents.size(); i++)
    {
        ExtentsDirectoryRecord* recordToInsert = eof_createDirectoryRecord(diskInfo, volumeHeader, i + 8);
        uint32_t insertRecordInTreeResult = eof_insertRecordInTree(diskInfo, volumeHeader, extentsFileHeaderNode, recordToInsert);

        if(insertRecordInTreeResult == EOF_INSERT_RECORD_IN_TREE_FAILED)
            return SET_EXTENTS_FOR_DIRECTORY_RECORD_FAILED;
    }

    return SET_EXTENTS_FOR_DIRECTORY_RECORD_SUCCESS;
}

uint32_t addExtentsToDirectoryRecord(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsFileHeaderNode* extentsFileHeaderNode, CatalogDirectoryRecord* fileRecord,
                                      std::vector<HFSPlusExtentDescriptor*> newExtents, uint32_t nodeNumberOfRecord, uint32_t& numberOfAlreadyExistingExtents)
{
    CatalogDirectoryRecord* updatedFileRecord = new CatalogDirectoryRecord();
    memcpy(updatedFileRecord, fileRecord, sizeof(CatalogDirectoryRecord));
    uint32_t numberOfAlreadyExistingExtentsInExtentsOverflow, newExtentIndex = 0;

    if(fileRecord->catalogData.totalNumOfExtents <= 8)
        numberOfAlreadyExistingExtentsInExtentsOverflow = 0;
    else
        numberOfAlreadyExistingExtentsInExtentsOverflow = fileRecord->catalogData.totalNumOfExtents - 8;

    for(uint32_t i = numberOfAlreadyExistingExtents; i < 8 && newExtentIndex < newExtents.size(); i++, newExtentIndex++)
    {
        updatedFileRecord->catalogData.hfsPlusForkData.extents[i] = *newExtents[newExtentIndex];
        updatedFileRecord->catalogData.hfsPlusForkData.totalBlocks += newExtents[newExtentIndex]->blockCount;
    }

    updatedFileRecord->catalogData.totalNumOfExtents += newExtents.size();
    uint32_t updateResult = cf_updateRecordOnDisk(diskInfo, volumeHeader, fileRecord, updatedFileRecord, nodeNumberOfRecord);

    if(updateResult == CF_UPDATE_RECORD_ON_DISK_FAILED)
        return ADD_EXTENTS_TO_DIRECTORY_RECORD_FAILED;

    memcpy(fileRecord, updatedFileRecord, sizeof(CatalogDirectoryRecord));

    //now add the new extents that don;t fit in the 8 limit
    uint32_t newExtentsAddedInOverflowIndex = 0;
    for(; newExtentIndex < newExtents.size(); newExtentIndex++, newExtentsAddedInOverflowIndex++)
    {
        ExtentsDirectoryRecord* recordToInsert = eof_createDirectoryRecord(diskInfo, volumeHeader, numberOfAlreadyExistingExtentsInExtentsOverflow + newExtentsAddedInOverflowIndex);
        uint32_t insertRecordInTreeResult = eof_insertRecordInTree(diskInfo, volumeHeader, extentsFileHeaderNode, recordToInsert);

        if(insertRecordInTreeResult == EOF_INSERT_RECORD_IN_TREE_FAILED)
            return ADD_EXTENTS_TO_DIRECTORY_RECORD_FAILED;
    }

    return ADD_EXTENTS_TO_DIRECTORY_RECORD_SUCCESS;
}

uint32_t getAllExtentsForGivenDirectoryRecord(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsFileHeaderNode* extentsFileHeaderNode,
                                              CatalogDirectoryRecord* fileRecord, std::vector<HFSPlusExtentDescriptor*>& extents,
                                              std::vector<ExtentsDirectoryRecord*>& extentsDirectoryRecords)
{
    //put extents from the record
    for(uint32_t i = 0; i < fileRecord->catalogData.totalNumOfExtents && i < 8; i++)
    {
        HFSPlusExtentDescriptor* extent = new HFSPlusExtentDescriptor();
        memcpy(extent, &fileRecord->catalogData.hfsPlusForkData.extents[i], sizeof(HFSPlusExtentDescriptor));
        extents.push_back(extent);
    }

    std::vector<ExtentsDirectoryRecord*> recordsVector;
    uint32_t traverseResult = eof_traverseSubtree(diskInfo, volumeHeader, extentsFileHeaderNode->headerRecord.rootNode,
                                                 fileRecord->catalogData.folderID, recordsVector);

    if(traverseResult == EOF_TRAVERSE_SUBTREE_FAILED)
        return GET_ALL_EXTENTS_FOR_DIRECTORY_RECORD_FAILED;

    //sort to have the extents ordered
    std::sort(recordsVector.begin(), recordsVector.end(), [](ExtentsDirectoryRecord* record1, ExtentsDirectoryRecord* record2) {
        return record1->catalogKey.extentOverflowIndex < record2->catalogKey.extentOverflowIndex;
    });

    for(uint32_t i = 0; i < recordsVector.size(); i++)
    {
        HFSPlusExtentDescriptor* extent = new HFSPlusExtentDescriptor();
        memcpy(extent, &recordsVector[i]->catalogData.extent, sizeof(HFSPlusExtentDescriptor));
        extents.push_back(extent);
        extentsDirectoryRecords.push_back(recordsVector[i]);
    }

    return GET_ALL_EXTENTS_FOR_DIRECTORY_RECORD_SUCCESS;
}

uint32_t deleteDirectoryAndChildren(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode,
                                                   ExtentsFileHeaderNode* extentsFileHeaderNode, CatalogDirectoryRecord* catalogDirectoryRecordToDelete)
{
    std::vector<CatalogDirectoryRecord*> subDirectoryCatalogRecords;

    if(catalogDirectoryRecordToDelete->catalogData.recordType == DIRECTORY_TYPE_FOLDER)
    {
        uint32_t traverseResult = cf_traverseSubtree(diskInfo, volumeHeader, catalogFileHeaderNode->headerRecord.rootNode,
                                                     catalogDirectoryRecordToDelete->catalogData.folderID, subDirectoryCatalogRecords);

        if(traverseResult == CF_TRAVERSE_SUBTREE_FAILED)
            return DELETE_DIRECTORY_AND_CHILDREN_FAILED;

        for(CatalogDirectoryRecord* childEntry : subDirectoryCatalogRecords)
        {
            uint32_t deleteChildResult = deleteDirectoryAndChildren(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, childEntry);

            if(deleteChildResult == DELETE_DIRECTORY_RECORD_AND_RELATED_DATA_FAILED)
                break;
        }

        for(CatalogDirectoryRecord* entry : subDirectoryCatalogRecords)
            delete entry;
    }

    uint32_t deleteChildResult = deleteDirectoryRecordAndAllItsRelatedData(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode,
                                                                           catalogDirectoryRecordToDelete);

    return (deleteChildResult == DELETE_DIRECTORY_RECORD_AND_RELATED_DATA_SUCCESS) ? DELETE_DIRECTORY_AND_CHILDREN_SUCCESS : DELETE_DIRECTORY_AND_CHILDREN_FAILED;
}

static uint32_t clearDirectoryRecordData(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsFileHeaderNode* extentsFileHeaderNode,
                                         CatalogDirectoryRecord* catalogDirectoryRecordToDelete)
{
    std::vector<HFSPlusExtentDescriptor *> extents;
    std::vector<ExtentsDirectoryRecord *> extentsDirectoryRecords;
    uint32_t getAllExtentsForRecordResult = getAllExtentsForGivenDirectoryRecord(diskInfo, volumeHeader,
                                                                                 extentsFileHeaderNode,
                                                                                 catalogDirectoryRecordToDelete,
                                                                                 extents, extentsDirectoryRecords);

    if (getAllExtentsForRecordResult == GET_ALL_EXTENTS_FOR_DIRECTORY_RECORD_FAILED)
        return CLEAR_DIRECTORY_RECORD_DATA_FAILED;

   //deallocate blocks
    for (uint32_t i = 0; i < extents.size(); i++)
    {
        //if this fails we will have trash blocks
        for (uint32_t j = 0; j < extents[i]->blockCount; j++)
            changeBlockAllocationInAllocationFile(diskInfo, volumeHeader, extents[i]->startBlock + j, (uint8_t) 0);
    }

    //remove extents records from extents overflow file
    for (uint32_t i = 0; i < extentsDirectoryRecords.size(); i++)
        eof_removeRecordFromTree(diskInfo, volumeHeader, extentsFileHeaderNode, extentsDirectoryRecords[i]); //if this fails, we will have trash extents dir entries

    return CLEAR_DIRECTORY_RECORD_DATA_SUCCESS;
}

static uint32_t deleteDirectoryRecordAndAllItsRelatedData(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode,
                                                   ExtentsFileHeaderNode* extentsFileHeaderNode, CatalogDirectoryRecord* catalogDirectoryRecordToDelete)
{
    uint32_t clearDirectoryRecordDataResult = clearDirectoryRecordData(diskInfo, volumeHeader, extentsFileHeaderNode, catalogDirectoryRecordToDelete);

    if(clearDirectoryRecordDataResult == CLEAR_DIRECTORY_RECORD_DATA_FAILED)
        return DELETE_DIRECTORY_RECORD_AND_RELATED_DATA_FAILED;

    uint32_t removeCatalogDirectoryRecordFromTreeResult = cf_removeRecordFromTree(diskInfo, volumeHeader, catalogFileHeaderNode, catalogDirectoryRecordToDelete);

    return (removeCatalogDirectoryRecordFromTreeResult == CF_REMOVE_RECORD_FROM_TREE_SUCCESS) ? DELETE_DIRECTORY_RECORD_AND_RELATED_DATA_SUCCESS :
                                                                                        DELETE_DIRECTORY_RECORD_AND_RELATED_DATA_FAILED;
}