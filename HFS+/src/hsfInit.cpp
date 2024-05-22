#include "string.h"
#include "vector"
#include "iostream"
#include "cstdint"

#include "../include/disk.h"
#include "../include/diskCodes.h"
#include "../include/hfsStructures.h"
#include "../include/hfsInit.h"
#include "../include/utils.h"
#include "../include/codes/hfsAttributes.h"
#include "../include/hfsFunctionUtils.h"

void hfsStartup(char* diskDirectory, DiskInfo** diskInfo, uint32_t sectorsNumber, uint32_t sectorSize, uint32_t blockSize, bool printSteps)
{
    if(checkDiskInitialization(diskDirectory) == false)
        initializeDisk(diskDirectory, diskInfo, sectorsNumber, sectorSize, printSteps);
    else
        *diskInfo = getDisk(diskDirectory);

    bool hfsAlreadyInitialized = true;
    if(checkHfsFileSystemInitialization(*diskInfo) == false)
    {
        initializeVolumeHeader(*diskInfo, blockSize);
        hfsAlreadyInitialized = false;
        if(printSteps == true)
            std::cout << "Volume header initialized\n";
    }

    HFSPlusVolumeHeader* volumeHeader = readVolumeHeader(*diskInfo);

    if(hfsAlreadyInitialized == false)
    {
        initializeAllocationFile(*diskInfo, volumeHeader);
        initializeExtentsOverflowFile(*diskInfo, volumeHeader);
        initializeCatalogFile(*diskInfo, volumeHeader);
        if(printSteps == true)
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
    uint32_t numberOfSectorsRead, nodeSize = getExtentsOverflowFileNodeSize(volumeHeader);
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
    uint32_t numberOfSectorsRead, nodeSize = getCatalogFileNodeSize(volumeHeader);
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

static void initializeDisk(char* diskDirectory, DiskInfo** diskInfo, uint32_t sectorsNumber, uint32_t sectorSize, bool printSteps)
{
    *diskInfo = initializeDisk(diskDirectory, sectorsNumber, sectorSize);
    uint32_t batchSize = 1000;
    fillDiskInitialMemory(*diskInfo, batchSize);
    if(printSteps == true)
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

static void initializeVolumeHeader(DiskInfo* diskInfo, uint32_t blockSize)
{
    //initialize volume header
    HFSPlusVolumeHeader* hfsVolumeHeader = new HFSPlusVolumeHeader();
    memset(hfsVolumeHeader, 0, sizeof(HFSPlusVolumeHeader));

    uint64_t totalBytesOnDisk = (uint64_t) diskInfo->diskParameters.sectorsNumber * (uint64_t) diskInfo->diskParameters.sectorSizeBytes; //the first 1024 are also part of block(s)
    uint32_t clumpBlocks = 8;

    hfsVolumeHeader->signature = 11080;
    hfsVolumeHeader->version = 4;
    hfsVolumeHeader->attributes = 8;
    hfsVolumeHeader->lastMountedVersion = 942551344; //'8.10'
    hfsVolumeHeader->journalInfoBlock = 0;
    hfsVolumeHeader->createDate = 999;
    hfsVolumeHeader->modifyDate = 999;
    hfsVolumeHeader->backupDate  = 999;
    hfsVolumeHeader->checkedDate = 999;
    hfsVolumeHeader->fileCount = 0;
    hfsVolumeHeader->folderCount = 0;
    hfsVolumeHeader->blockSize = blockSize;
    hfsVolumeHeader->totalBlocks = totalBytesOnDisk / hfsVolumeHeader->blockSize; //if the last block is incomplete, we ignore it
    hfsVolumeHeader->nextAllocation = 0;
    hfsVolumeHeader->rsrcClumpSize = 0;
    hfsVolumeHeader->dataClumpSize = hfsVolumeHeader->blockSize * clumpBlocks;
    hfsVolumeHeader->nextCatalogID = 1; //first record will have id 1, and its parent is root (which is not an actual record) so 0; any record directly under root will have parent 0
    hfsVolumeHeader->nextExtentsID = 1; //same as for nextCatalogID
    hfsVolumeHeader->encodingsBitmap = 0;
    // hfsVolumeHeader->finderInfo -> this array stays at 0 for all fields

    //initialize allocation file & extents overflow file & catalog file; we need this here in order to add forkDatas to volume header
    uint32_t totalNodesPerBTree = 1024; //for extents overflow file btree & catalog file btree

    HFSPlusForkData* allocationFileForkData = new HFSPlusForkData();
    initializeAllocationFileForkData(diskInfo, allocationFileForkData, hfsVolumeHeader->blockSize);
    hfsVolumeHeader->allocationFile = *allocationFileForkData;

    HFSPlusForkData* extentsOverflowFileForkData = new HFSPlusForkData();
    initializeExtentsOverflowFileForkData(extentsOverflowFileForkData, hfsVolumeHeader->blockSize, totalNodesPerBTree, allocationFileForkData);
    hfsVolumeHeader->extentsFile = * extentsOverflowFileForkData;

    HFSPlusForkData* catalogFileForkData = new HFSPlusForkData();
    initializeCatalogFileForkData(catalogFileForkData, hfsVolumeHeader->blockSize, totalNodesPerBTree, extentsOverflowFileForkData);
    hfsVolumeHeader->catalogFile = *catalogFileForkData;

    //now write the volume header on disk
    uint32_t numberOfSectorsWritten, retryWriteCount = 2;
    char* blockBuffer = new char[blockSize];
    memset(blockBuffer, 0, blockSize);

    if(blockSize <= 1024)  //if blockSize is 512 or 1024 we write from the beginning of block
        memcpy(blockBuffer, hfsVolumeHeader, sizeof(HFSPlusVolumeHeader));
    else
        memcpy(blockBuffer + 1024, hfsVolumeHeader, sizeof(HFSPlusVolumeHeader));

    uint32_t sector = getFirstSectorForGivenBlock(diskInfo, hfsVolumeHeader, getFirstBlockForVolumeHeader(blockSize));
    int writeResult = writeDiskSectors(diskInfo, getNumberOfSectorsPerBlock(diskInfo, hfsVolumeHeader), sector, blockBuffer, numberOfSectorsWritten);
    while(writeResult != EC_NO_ERROR && retryWriteCount > 0)
    {
        writeResult = writeDiskSectors(diskInfo, 1, sector, blockBuffer, numberOfSectorsWritten);
        retryWriteCount--;
    }

    if(retryWriteCount == 0)
        throw std::runtime_error("Failed to initialize volume header!");

    delete[] blockBuffer, delete allocationFileForkData, delete extentsOverflowFileForkData, delete catalogFileForkData;
}

//in documentation the allocation file represents all blocks (including the ones for structures). we make to to include only the data blocks so first bit in allocation file represents
//first data block. However the allocation file size is enough to represent all blocks (including the ones for structures) not only the fata blocks
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

static void initializeExtentsOverflowFileForkData(HFSPlusForkData* forkData, uint32_t blockSize, uint32_t totalNodes, HFSPlusForkData * allocationFileForkData)
{

    uint32_t nodeSize = blockSize; //if it is 1024 then standard. else as a block
    forkData->totalBlocks = totalNodes * (nodeSize / blockSize);
    forkData->logicalSize = forkData->totalBlocks * blockSize;
    forkData->clumpSize = (nodeSize / blockSize); //we read one node at a time
    forkData->extents[0].blockCount = forkData->totalBlocks;
    forkData->extents[0].startBlock = getFirstBlockForExtentsOverflowFile(allocationFileForkData);
}

static void initializeCatalogFileForkData(HFSPlusForkData* forkData, uint32_t blockSize, uint32_t totalNodes, HFSPlusForkData * extentsOverflowFileForkData)
{
    uint32_t nodeSize;
    if(blockSize >= 4096)
        nodeSize = blockSize;
    else
        nodeSize = 4096;

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
        byte = hfs_changeBitValue(byte, bitIndex, 1);

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
    uint32_t nodeSize = getExtentsOverflowFileNodeSize(volumeHeader);

    BTNodeDescriptor* headerNodeDescriptor = new BTNodeDescriptor();
    headerNodeDescriptor->fLink = 0;
    headerNodeDescriptor->bLink = 0;
    headerNodeDescriptor->kind = kBTHeaderNode;
    headerNodeDescriptor->height = 0;
    headerNodeDescriptor->numRecords = 0;
    headerNodeDescriptor->isLeaf = NODE_IS_LEAF;

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
    uint8_t byteValueForOccupiedFirstNode = hfs_changeBitValue(0, 0, 1);
    uint32_t mapRecordIndex = sizeof(BTNodeDescriptor) + sizeof(BTHeaderRec) + 128;
    nodeBuffer[mapRecordIndex] = byteValueForOccupiedFirstNode;

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
    uint32_t nodeSize = getCatalogFileNodeSize(volumeHeader);

    //create and write on disk the header node
    BTNodeDescriptor* headerNodeDescriptor = new BTNodeDescriptor();
    headerNodeDescriptor->fLink = 0;
    headerNodeDescriptor->bLink = 0;
    headerNodeDescriptor->kind = kBTHeaderNode;
    headerNodeDescriptor->height = 0;
    headerNodeDescriptor->numRecords = 0;
    headerNodeDescriptor->isLeaf = NODE_IS_LEAF;

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
    //mark first node and second in tree as occupied (in map record)
    uint8_t byteValueForHeaderNode = hfs_changeBitValue(0, 0, 1); //header node
    uint32_t mapRecordIndex = sizeof(BTNodeDescriptor) + sizeof(BTHeaderRec) + 128;
    nodeBuffer[mapRecordIndex] = byteValueForHeaderNode;

    //now write the extent overflow file header node on disk
    uint32_t numberOfSectorsWritten, retryWriteCount = 2, numOfSectorsToWrite = nodeSize / diskInfo->diskParameters.sectorSizeBytes;
    uint32_t firstSectorForCatalogFile = getFirstBlockForCatalogFile(&volumeHeader->extentsFile) * getNumberOfSectorsPerBlock(diskInfo, volumeHeader);

    uint32_t writeResult = writeDiskSectors(diskInfo, numOfSectorsToWrite, firstSectorForCatalogFile, nodeBuffer, numberOfSectorsWritten);
    while(writeResult != EC_NO_ERROR && retryWriteCount > 0)
    {
        writeResult = writeDiskSectors(diskInfo, numOfSectorsToWrite, firstSectorForCatalogFile, nodeBuffer, numberOfSectorsWritten);
        retryWriteCount--;
    }

    if(retryWriteCount == 0)
        throw std::runtime_error("Failed to initialize catalog file node header!");
}

////////////////////////////////

static uint32_t getFirstBlockForExtentsOverflowFile(HFSPlusForkData* allocationFileForkData)
{
    return allocationFileForkData->extents[0].startBlock + allocationFileForkData->extents[0].blockCount;
}

static uint32_t getFirstBlockForCatalogFile(HFSPlusForkData* extentsFileForkData)
{
    return extentsFileForkData->extents[0].startBlock + extentsFileForkData->extents[0].blockCount;
}