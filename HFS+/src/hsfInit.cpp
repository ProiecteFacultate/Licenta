#include "string.h"
#include "vector"
#include "iostream"
#include "cstdint"

#include "../include/disk.h"
#include "../include/diskCodes.h"
#include "../include/structures.h"
#include "../include/hfsInit.h"
#include "../include/utils.h"
#include "../include/hfsFunctionUtils.h"

void hfsStartup(char* diskDirectory, DiskInfo** diskInfo, uint32_t sectorsNumber, uint32_t sectorSize)
{
    if(checkDiskInitialization(diskDirectory) == false)
        initializeDisk(diskDirectory, diskInfo, sectorsNumber, sectorSize);
    else
        *diskInfo = getDisk(diskDirectory);

    bool hfsAlreadyInitialized = true;
    if(checkHfsFileSystemInitialization(*diskInfo) == false)
    {
        initializeVolumeHeader(*diskInfo);
        hfsAlreadyInitialized = false;
        std::cout << "Volume header initialized\n";
    }

    HFSPlusVolumeHeader* volumeHeader = readVolumeHeader(*diskInfo);

    if(hfsAlreadyInitialized == false)
    {
        initializeAllocationFile(*diskInfo, volumeHeader);
        initializeExtentsOverflowFile(*diskInfo, volumeHeader);
        initializeCatalogFile(*diskInfo, volumeHeader);
        std::cout << "Groups initialized\n";
    }
}

HFSPlusVolumeHeader* readVolumeHeader(DiskInfo* diskInfo)
{
    uint32_t numberOfSectorsRead = 0;
    uint32_t sector = 1024 / diskInfo->diskParameters.sectorSizeBytes;
    char* readBuffer = new char[diskInfo->diskParameters.sectorSizeBytes];

    uint32_t readResult = readDiskSectors(diskInfo, 1, sector, readBuffer, numberOfSectorsRead);
    if(readResult != EC_NO_ERROR)
        throw std::runtime_error("Failed to read volume header!");

    if(diskInfo->diskParameters.sectorSizeBytes <= 1024)
        return (HFSPlusVolumeHeader*)&readBuffer[0];

    return (HFSPlusVolumeHeader*)&readBuffer[1024];
}

ExtentsFileHeaderNode* readExtentsOverflowFileHeaderNode(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader)
{
    uint32_t numberOfSectorsRead, nodeSize = getExtentsOverflowFileNodeSize();
    uint32_t numOfSectorsOccupied = (diskInfo->diskParameters.sectorSizeBytes <= nodeSize) ? nodeSize / diskInfo->diskParameters.sectorSizeBytes : 1;
    char* readBuffer = new char[numOfSectorsOccupied * diskInfo->diskParameters.sectorSizeBytes];
    uint32_t firstSector = getFirstSectorForGivenBlock(diskInfo, volumeHeader, volumeHeader->extentsFile.extents[0].startBlock);

    uint32_t readResult = readDiskSectors(diskInfo, numOfSectorsOccupied, firstSector, readBuffer, numberOfSectorsRead);

    if(readResult != EC_NO_ERROR)
        throw std::runtime_error("Failed to read extents overflow file header node!");

    return (ExtentsFileHeaderNode*)&readBuffer[0];
}

CatalogFileHeaderNode* readCatalogFileHeaderNode(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader)
{
    uint32_t numberOfSectorsRead, nodeSize = getCatalogFileNodeSize();
    uint32_t numOfSectorsOccupied = (diskInfo->diskParameters.sectorSizeBytes <= nodeSize) ? nodeSize / diskInfo->diskParameters.sectorSizeBytes : 1;
    char* readBuffer = new char[numOfSectorsOccupied * diskInfo->diskParameters.sectorSizeBytes];
    uint32_t firstSector = getFirstSectorForGivenBlock(diskInfo, volumeHeader, volumeHeader->catalogFile.extents[0].startBlock);

    uint32_t readResult = readDiskSectors(diskInfo, numOfSectorsOccupied, firstSector, readBuffer, numberOfSectorsRead);

    if(readResult != EC_NO_ERROR)
        throw std::runtime_error("Failed to read extents catalog file header node!");

    return (CatalogFileHeaderNode*)&readBuffer[0];
}

//////////////////////////////////////

static bool checkDiskInitialization(char* diskDirectory)
{
    return !(getDisk(diskDirectory) == nullptr);
}

static void initializeDisk(char* diskDirectory, DiskInfo** diskInfo, uint32_t sectorsNumber, uint32_t sectorSize)
{
    *diskInfo = initializeDisk(diskDirectory, sectorsNumber, sectorSize);
    uint32_t batchSize = 1000;
    fillDiskInitialMemory(*diskInfo, batchSize);
    std::cout << "Disk initialized\n";
}

static bool checkHfsFileSystemInitialization(DiskInfo* diskInfo)
{
    //The first 1024 are used by the boot block(s) and the VolumeHeader is in the next 512 (from 1024 to 1535)
    uint16_t bootSignature;
    uint32_t numberOfSectorsRead, retryReadCount = 2, sectorToRead;
    char* volumeHeaderBuffer = new char[diskInfo->diskParameters.sectorSizeBytes];

    switch (diskInfo->diskParameters.sectorSizeBytes) {
        case 512:
            sectorToRead = 2;
            break;
        case 1024:
            sectorToRead = 1;
            break;
        default:
            sectorToRead = 0; //for case where it is > 1024 so 2048, 4096 etc
    }

    int readResult = readDiskSectors(diskInfo, 1, sectorToRead, volumeHeaderBuffer, numberOfSectorsRead);
    while(readResult != EC_NO_ERROR && retryReadCount > 0)
    {
        readResult = readDiskSectors(diskInfo, 1, sectorToRead, volumeHeaderBuffer, numberOfSectorsRead);
        retryReadCount--;
    }

    if(retryReadCount == 0)
    {
        delete[] volumeHeaderBuffer;
        throw std::runtime_error("Failed to check boot sector initialization");
    }

    bootSignature = *(uint16_t*)&volumeHeaderBuffer[1024 % diskInfo->diskParameters.sectorSizeBytes];
    delete[] volumeHeaderBuffer;

    return bootSignature == 11080;   //'H+' = 0x482B -> as uint16 0x2B48
}

///////////////////////////////

static void initializeVolumeHeader(DiskInfo* diskInfo)
{
    //initialize volume header
    HFSPlusVolumeHeader* hfsVolumeHeader = new HFSPlusVolumeHeader();
    memset(hfsVolumeHeader, 0, sizeof(HFSPlusVolumeHeader));

    uint64_t totalBytesOnDisk = (uint64_t) diskInfo->diskParameters.sectorsNumber * (uint64_t) diskInfo->diskParameters.sectorSizeBytes; //the first 1024 are also part of block(s)
    uint32_t blockSize = 1024, clumpBlocks = 8;

    hfsVolumeHeader->signature = 11080;
    hfsVolumeHeader->version = 4;
    hfsVolumeHeader->attributes = 8;
    hfsVolumeHeader->lastMountedVersion = 942551344; //'8.10'
    hfsVolumeHeader->journalInfoBlock = 0;
    hfsVolumeHeader->createDate = 999; //TODO
    hfsVolumeHeader->modifyDate = 999; //TODO
    hfsVolumeHeader->backupDate  = 999;
    hfsVolumeHeader->checkedDate = 999;
    hfsVolumeHeader->fileCount = 0;
    hfsVolumeHeader->folderCount = 0;
    hfsVolumeHeader->blockSize = blockSize;
    hfsVolumeHeader->totalBlocks = totalBytesOnDisk / hfsVolumeHeader->blockSize; //if the last block is incomplete, we ignore it
    hfsVolumeHeader->nextAllocation = 0;
    hfsVolumeHeader->rsrcClumpSize = 0;
    hfsVolumeHeader->dataClumpSize = hfsVolumeHeader->blockSize * clumpBlocks;
    hfsVolumeHeader->nextCatalogID = 0;
    hfsVolumeHeader->writeCount = 0;
    hfsVolumeHeader->encodingsBitmap = 0;
    // hfsVolumeHeader->finderInfo -> this array stays at 0 for all fields

    //initialize allocation file & extents overflow file & catalog file; we need this here in order to add forkDatas to volume header
    uint32_t totalNodesPerBTree = 1024; //for extents overflow file btree & catalog file btree

    HFSPlusForkData* allocationFileForkData = new HFSPlusForkData();
    initializeAllocationFileForkData(diskInfo, allocationFileForkData, hfsVolumeHeader->blockSize);
    hfsVolumeHeader->allocationFile = *allocationFileForkData;

    HFSPlusForkData* extentsOverflowFileForkData = new HFSPlusForkData();
    initializeExtentsOverflowFileForkData(extentsOverflowFileForkData, hfsVolumeHeader->blockSize, totalNodesPerBTree, 1024, allocationFileForkData);
    hfsVolumeHeader->extentsFile = * extentsOverflowFileForkData;

    HFSPlusForkData* catalogFileForkData = new HFSPlusForkData();
    initializeCatalogFileForkData(catalogFileForkData, hfsVolumeHeader->blockSize, totalNodesPerBTree, 4096, extentsOverflowFileForkData);
    hfsVolumeHeader->catalogFile = *catalogFileForkData;

    //now write the volume header on disk
    uint32_t numberOfSectorsWritten, retryWriteCount = 2;
    char* blockBuffer = new char[blockSize];
    memset(blockBuffer, 0, blockSize);

    if(diskInfo->diskParameters.sectorSizeBytes <= blockSize) //if blockSize is 512 or 1024 we write from the beginning of block
        memcpy(blockBuffer, hfsVolumeHeader, sizeof(HFSPlusVolumeHeader));
    else
        memcpy(blockBuffer + 1024, hfsVolumeHeader, sizeof(HFSPlusVolumeHeader));

    uint32_t sector = getFirstSectorForGivenBlock(diskInfo, hfsVolumeHeader, getFirstBlockForVolumeHeader(blockSize));
    int writeResult = writeDiskSectors(diskInfo, 1, sector, blockBuffer, numberOfSectorsWritten);
    while(writeResult != EC_NO_ERROR && retryWriteCount > 0)
    {
        writeResult = writeDiskSectors(diskInfo, 1, sector, blockBuffer, numberOfSectorsWritten);
        retryWriteCount--;
    }

    if(retryWriteCount == 0)
        throw std::runtime_error("Failed to initialize volume header!");

    delete[] blockBuffer, delete allocationFileForkData, delete extentsOverflowFileForkData, delete catalogFileForkData;
}

static void initializeAllocationFileForkData(DiskInfo* diskInfo, HFSPlusForkData* forkData, uint32_t blockSize)
{
    uint64_t diskSize = (uint64_t) diskInfo->diskParameters.sectorsNumber * (uint64_t) diskInfo->diskParameters.sectorSizeBytes;
    uint32_t numberOfBlocks = diskSize / blockSize;
    uint32_t numberOfBlocksRepresentedInABitmapBlock = blockSize * 8;

    forkData->totalBlocks = numberOfBlocks / numberOfBlocksRepresentedInABitmapBlock + 1;
    forkData->logicalSize = forkData->totalBlocks * blockSize;
    forkData->clumpSize = 2; //we are reading from allocation file so we probably don't need more than 2 blocks (2 in case the blocks we look for in bitmap are at the end of a block)
    forkData->extents[0].blockCount = forkData->totalBlocks;
    forkData->extents[0].startBlock = getFirstBlockForVolumeHeader(blockSize) + 1;
}

static void initializeExtentsOverflowFileForkData(HFSPlusForkData* forkData, uint32_t blockSize, uint32_t totalNodes, uint32_t nodeSize, HFSPlusForkData * allocationFileForkData)
{
    forkData->totalBlocks = totalNodes * (nodeSize / blockSize);
    forkData->logicalSize = forkData->totalBlocks * blockSize;
    forkData->clumpSize = (nodeSize / blockSize); //we read one node at a time
    forkData->extents[0].blockCount = forkData->totalBlocks;
    forkData->extents[0].startBlock = getFirstBlockForExtentsOverflowFile(allocationFileForkData);
}

static void initializeCatalogFileForkData(HFSPlusForkData* forkData, uint32_t blockSize, uint32_t totalNodes, uint32_t nodeSize, HFSPlusForkData * extentsOverflowFileForkData)
{
    forkData->totalBlocks = totalNodes * (nodeSize / blockSize);
    forkData->logicalSize = forkData->totalBlocks * blockSize;
    forkData->clumpSize = (nodeSize / blockSize); //we read one node at a time
    forkData->extents[0].blockCount = forkData->totalBlocks;
    forkData->extents[0].startBlock = getFirstBlockForCatalogFile(extentsOverflowFileForkData);
}

///////////////////

static void initializeAllocationFile(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader)
{
    uint32_t numOfBlocksOccupiedByStructures = volumeHeader->catalogFile.extents[0].startBlock + volumeHeader->catalogFile.extents[0].blockCount;
    uint32_t numOfBytesToOccupyInBitmap = numOfBlocksOccupiedByStructures / 8 + 1;
    if(numOfBlocksOccupiedByStructures % 8 == 0)
        numOfBytesToOccupyInBitmap--;

    uint32_t numOfSectors = numOfBytesToOccupyInBitmap / diskInfo->diskParameters.sectorSizeBytes + 1;
    if(numOfBytesToOccupyInBitmap % diskInfo->diskParameters.sectorSizeBytes == 0)
        numOfSectors--;

    char* buffer = new char[numOfSectors * diskInfo->diskParameters.sectorSizeBytes];

    for(uint32_t byteIndex = 0; byteIndex < numOfBytesToOccupyInBitmap - 1; byteIndex++)
        buffer[byteIndex] = 0xFF;

    uint32_t numOfBitsRemainedToOccupy = numOfBlocksOccupiedByStructures % 8;
    uint8_t byte = 0;
    for(uint32_t bitIndex = 0; bitIndex < numOfBitsRemainedToOccupy; bitIndex++)
        byte = changeBitValue(byte, bitIndex, 1);

    buffer[numOfBytesToOccupyInBitmap - 1] = byte;

    uint32_t numberOfSectorsWritten, retryWriteCount = 2;
    uint32_t firstSectorOfAllocationFile = getFirstSectorForGivenBlock(diskInfo, volumeHeader, volumeHeader->catalogFile.extents[0].startBlock);
    int writeResult = writeDiskSectors(diskInfo, numOfSectors, firstSectorOfAllocationFile, buffer, numberOfSectorsWritten);
    while(writeResult != EC_NO_ERROR && retryWriteCount > 0)
    {
        writeResult = writeDiskSectors(diskInfo, numOfSectors, firstSectorOfAllocationFile, buffer, numberOfSectorsWritten);
        retryWriteCount--;
    }

    if(retryWriteCount == 0)
        throw std::runtime_error("Failed to initialize allocation file!");
}

static void initializeExtentsOverflowFile(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader)
{
    uint32_t nodeSize = getExtentsOverflowFileNodeSize();

    BTNodeDescriptor* headerNodeDescriptor = new BTNodeDescriptor();
    headerNodeDescriptor->fLink = 0;
    headerNodeDescriptor->bLink = 0;
    headerNodeDescriptor->kind = kBTHeaderNode;
    headerNodeDescriptor->height = 0;
    headerNodeDescriptor->numRecords = 0;
    headerNodeDescriptor->reserved = 0;

    BTHeaderRec* headerRecord = new BTHeaderRec();
    headerRecord->treeDepth = 0;
    headerRecord->rootNode = 0;
    headerRecord->leafRecords = 0;
    headerRecord->firstLeafNode = 0;
    headerRecord->lastLeafNode = 0;
    headerRecord->nodeSize = nodeSize;
    headerRecord->maxKeyLength = sizeof(HFSPlusCatalogKey);
    headerRecord->totalNodes = 1024;
    headerRecord->freeNodes = 1023; //1 is this (the header node)
    headerRecord->reserved1 = 0;
    headerRecord->clumpSize = 0; //ignored
    headerRecord->btreeType = 0;
    headerRecord->keyCompareType = 0;
    headerRecord->attributes = 0;
    memset(&headerRecord->reserved3[0], 0, 64);

    char* nodeBuffer = new char[nodeSize];
    //copy node descriptor in memory
    memcpy(nodeBuffer, headerNodeDescriptor, sizeof(BTNodeDescriptor));
    //copy header record in memory
    memcpy(nodeBuffer + 14, headerRecord, sizeof(BTHeaderRec));
    //mark first node in tree as occupied (in map record)
    uint8_t byteValueForOccupiedFirstNode = changeBitValue(0, 0, 1);
    uint32_t mapRecordIndex = sizeof(BTNodeDescriptor) + sizeof(BTHeaderRec) + 128;
    nodeBuffer[mapRecordIndex] = byteValueForOccupiedFirstNode;

    uint16_t offset0 = 14;
    memcpy(&nodeBuffer[nodeSize - 2], &offset0, 2);
    uint16_t offset1 = 14 + sizeof(BTHeaderRec);
    memcpy(&nodeBuffer[nodeSize - 4], &offset1, 2);
    uint16_t offset2 = 14 + sizeof(BTHeaderRec) + 128;  //128 is hardcoded; the user data record is not used anyway
    memcpy(&nodeBuffer[nodeSize - 6], &offset2, 2);

    //now write the extent overflow file header node on disk
    uint32_t numberOfSectorsWritten, retryWriteCount = 2, numOfSectorsToWrite = nodeSize / diskInfo->diskParameters.sectorSizeBytes;
    uint32_t firstSectorForExtentsOverflowFile = getFirstBlockForExtentsOverflowFile(&volumeHeader->allocationFile) * getNumberOfSectorsPerBlock(diskInfo, volumeHeader);

    int writeResult = writeDiskSectors(diskInfo, numOfSectorsToWrite, firstSectorForExtentsOverflowFile, nodeBuffer, numberOfSectorsWritten);
    while(writeResult != EC_NO_ERROR && retryWriteCount > 0)
    {
        writeResult = writeDiskSectors(diskInfo, numOfSectorsToWrite, firstSectorForExtentsOverflowFile, nodeBuffer, numberOfSectorsWritten);
        retryWriteCount--;
    }

    if(retryWriteCount == 0)
        throw std::runtime_error("Failed to initialize extents overflow file node header!");
}

static void initializeCatalogFile(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader)
{
    uint32_t nodeSize = getCatalogFileNodeSize();

    BTNodeDescriptor* headerNodeDescriptor = new BTNodeDescriptor();
    headerNodeDescriptor->fLink = 0;
    headerNodeDescriptor->bLink = 0;
    headerNodeDescriptor->kind = kBTHeaderNode;
    headerNodeDescriptor->height = 0;
    headerNodeDescriptor->numRecords = 0;
    headerNodeDescriptor->reserved = 0;

    BTHeaderRec* headerRecord = new BTHeaderRec();
    headerRecord->treeDepth = 0;
    headerRecord->rootNode = 0;
    headerRecord->leafRecords = 0;
    headerRecord->firstLeafNode = 0;
    headerRecord->lastLeafNode = 0;
    headerRecord->nodeSize = nodeSize;
    headerRecord->maxKeyLength = sizeof(HFSPlusCatalogKey);
    headerRecord->totalNodes = 1024;
    headerRecord->freeNodes = 1023; //1 is this (the header node)
    headerRecord->reserved1 = 0;
    headerRecord->clumpSize = 0; //ignored
    headerRecord->btreeType = 0;
    headerRecord->keyCompareType = 0;
    headerRecord->attributes = 0;
    memset(&headerRecord->reserved3[0], 0, 64);

    char* nodeBuffer = new char[nodeSize];
    //copy node descriptor in memory
    memcpy(nodeBuffer, headerNodeDescriptor, sizeof(BTNodeDescriptor));
    //copy header record in memory
    memcpy(nodeBuffer + 14, headerRecord, sizeof(BTHeaderRec));
    //mark first node in tree as occupied (in map record)
    uint8_t byteValueForOccupiedFirstNode = changeBitValue(0, 0, 1);
    uint32_t mapRecordIndex = sizeof(BTNodeDescriptor) + sizeof(BTHeaderRec) + 128;
    nodeBuffer[mapRecordIndex] = byteValueForOccupiedFirstNode;

    uint16_t offset0 = 14;
    memcpy(&nodeBuffer[nodeSize - 2], &offset0, 2);
    uint16_t offset1 = 14 + sizeof(BTHeaderRec);
    memcpy(&nodeBuffer[nodeSize - 4], &offset1, 2);
    uint16_t offset2 = 14 + sizeof(BTHeaderRec) + 128;  //128 is hardcoded; the user data record is not used anyway
    memcpy(&nodeBuffer[nodeSize - 6], &offset2, 2);

    //now write the extent overflow file header node on disk
    uint32_t numberOfSectorsWritten, retryWriteCount = 2, numOfSectorsToWrite = nodeSize / diskInfo->diskParameters.sectorSizeBytes;
    uint32_t firstSectorForCatalogFile = getFirstBlockForCatalogFile(&volumeHeader->extentsFile) * getNumberOfSectorsPerBlock(diskInfo, volumeHeader);

    int writeResult = writeDiskSectors(diskInfo, numOfSectorsToWrite, firstSectorForCatalogFile, nodeBuffer, numberOfSectorsWritten);
    while(writeResult != EC_NO_ERROR && retryWriteCount > 0)
    {
        writeResult = writeDiskSectors(diskInfo, numOfSectorsToWrite, firstSectorForCatalogFile, nodeBuffer, numberOfSectorsWritten);
        retryWriteCount--;
    }

    if(retryWriteCount == 0)
        throw std::runtime_error("Failed to initialize catalog file node header!");
}

////////////////////////////////

static uint32_t getFirstBlockForVolumeHeader(uint32_t blockSize)
{
    switch (blockSize) {
        case 512:
            return 2;
        case 1024:
            return 1;
        default:
            return 0;
    }
}

static uint32_t getFirstBlockForExtentsOverflowFile(HFSPlusForkData* allocationFileForkData)
{
    return allocationFileForkData->extents[0].startBlock + allocationFileForkData->extents[0].blockCount;
}

static uint32_t getFirstBlockForCatalogFile(HFSPlusForkData* extentsFileForkData)
{
    return extentsFileForkData->extents[0].startBlock + extentsFileForkData->extents[0].blockCount;
}