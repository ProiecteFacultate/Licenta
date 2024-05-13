#include "iostream"
#include "string.h"
#include "vector"

#include "disk.h"
#include "diskCodes.h"
#include "../include/structures.h"
#include "../include/utils.h"
#include "../include/hfsFunctionUtils.h"
#include "../include/codes/hfsCodes.h"
#include "../include/catalog_file/catalogFileOperations.h"
#include "../include/catalog_file/codes/catalogFileResponseCodes.h"
#include "../include/codes/hfsCodes.h"
#include "../include/hfs.h"

//TODO ADD UPDATE RECORD  IN ALL BRANCHES
uint32_t writeBytesToFileWithTruncate(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogDirectoryRecord* fileRecord, CatalogFileHeaderNode* catalogFileHeaderNode,
                                      ExtentsFileHeaderNode* extentsFileHeaderNode, char* dataBuffer, uint32_t maxBytesToWrite, uint32_t& numberOfBytesWritten,
                                      uint32_t& reasonForIncompleteWrite, uint32_t nodeNumberOfRecord, std::vector<HFSPlusExtentDescriptor*> foundExtents)
{
    uint32_t searchExtentResult, extentSize, startPositionInBufferToWriteFrom, writeDataToExtentResult;
    if(maxBytesToWrite == 0) //it might be given 0
        return WRITE_BYTES_TO_FILE_SUCCESS;

    uint32_t numberOfBlocksRequired = maxBytesToWrite / volumeHeader->blockSize + 1;
    if(maxBytesToWrite % volumeHeader->blockSize == 0)
        numberOfBlocksRequired--;

    uint32_t foundBlocks = 0;
    //when we don't found an extent the size we want we reduce and look for an extent the size of the biggest extent found previously instead of starting from num of blocks still
    // not found and then decreasing one by one; it is for efficiency
    uint32_t preferableExtentSize = numberOfBlocksRequired - foundBlocks;

    //search extents and write to them
    while(foundBlocks < numberOfBlocksRequired)
    {
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
    uint32_t numOfBytesWritten;
    uint32_t writeResult = writeDiskSectors(diskInfo, extent->blockCount * getNumberOfSectorsPerBlock(diskInfo, volumeHeader),
                                            extent->startBlock * getNumberOfSectorsPerBlock(diskInfo, volumeHeader), data, numOfBytesWritten);

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
    uint32_t totalNumberOfDataBlocks = volumeHeader->totalBlocks - firstDataBlockGlobalIndex - 1;
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
        {
            foundExtent->startBlock = 0;
            foundExtent->blockCount = 0;
        }
    }

    delete[] allocationFileBlocks;
    return SEARCH_FREE_EXTENT_NO_EXTENT_WITH_DESIRED_NUMBER_OF_BLOCKS;
}

uint32_t setExtentsForDirectoryRecord(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, CatalogDirectoryRecord* fileRecord,
                                      std::vector<HFSPlusExtentDescriptor*> extents, uint32_t nodeNumberOfRecord)
{
    CatalogDirectoryRecord* updatedFileRecord = new CatalogDirectoryRecord();
    memcpy(updatedFileRecord, fileRecord, sizeof(CatalogDirectoryRecord));

    for(uint32_t i = 0; i < extents.size() && i < 8; i++)
    {
        fileRecord->catalogData.hfsPlusForkData.extents[i] = *extents[i];
        fileRecord->catalogData.hfsPlusForkData.totalBlocks += extents[i]->blockCount;
    }

    uint32_t updateResult = cf_updateRecordOnDisk(diskInfo, volumeHeader, fileRecord, updatedFileRecord, nodeNumberOfRecord);

    if(updateResult == CF_UPDATE_RECORD_ON_DISK_FAILED)
        return SET_EXTENTS_FOR_DIRECTORY_RECORD_FAILED;

    memcpy(fileRecord, updatedFileRecord, sizeof(CatalogDirectoryRecord));
    return SET_EXTENTS_FOR_DIRECTORY_RECORD_SUCCESS;
}