#include "windows.h"
#include "string"
#include "string.h"
#include "iostream"

#include "../include/disk.h"
#include "../include/diskUtils.h"
#include "../include/diskCodes.h"
#include "../include/utils.h"
#include "../include/fat32FunctionUtils.h"
#include "../include/structures.h"
#include "../include/codes/fat32Codes.h"
#include "../include/fat32Init.h"
#include "../include/fat32Attributes.h"

void fat32Startup(char* diskDirectory, DiskInfo** diskInfo, BootSector** bootSector, FsInfo** fsInfo, uint32_t sectorsNumber, uint32_t sectorSize)
{
    if(checkDiskInitialization(diskDirectory) == false)
        initializeDisk(diskDirectory, diskInfo, 1024, 512);
    else
        *diskInfo = getDisk(diskDirectory);

    bool fat32AlreadyInitialized = true;
    if(checkFat32FileSystemInitialization(*diskInfo) == false)
    {
        initializeBootSectors(*diskInfo);
        fat32AlreadyInitialized = false;
        std::cout << "Boot sectors initialized\n";
    }

    *bootSector = readBootSector(*diskInfo);
    *fsInfo = readFsInfo(*diskInfo, *bootSector);

    if(fat32AlreadyInitialized == false)
    {
        initializeFat(*diskInfo, *bootSector);
        std::cout << "File allocation table initialized\n";
    }
}

bool checkDiskInitialization(char* diskDirectory)
{
    return !(getDisk(diskDirectory) == nullptr);
}

void initializeDisk(char* diskDirectory, DiskInfo** diskInfo, uint32_t sectorsNumber, uint32_t sectorSize)
{
    *diskInfo = initializeDisk(diskDirectory, sectorsNumber, sectorSize);
    uint32_t batchSize = 1000;
    fillDiskInitialMemory(*diskInfo, batchSize);
    std::cout << "Disk initialized\n";
}

bool checkFat32FileSystemInitialization(DiskInfo* diskInfo)
{
    char* firstBootSectorBuffer = new char[diskInfo->diskParameters.sectorSizeBytes];

    uint32_t numberOfSectorsRead = 0;
    int retryReadCount = 2;
    int readResult = readDiskSectors(diskInfo, 1, 0, firstBootSectorBuffer, numberOfSectorsRead);
    while(readResult != EC_NO_ERROR && retryReadCount > 0)
    {
        readResult = readDiskSectors(diskInfo, 1, 0, firstBootSectorBuffer, numberOfSectorsRead);
        retryReadCount--;
    }

    if(retryReadCount == 0)
    {
        throw std::runtime_error("Failed to check boot sector initialization");
    }

    uint16_t bootSignature = *(uint16_t*)&firstBootSectorBuffer[510];

    delete[] firstBootSectorBuffer;

    return bootSignature == 43605;   //0xAA55
}

void initializeBootSectors(DiskInfo* diskInfo)
{
    BootSector* bootSectorData = new BootSector();

    //BPB
    char* bootJumpInstruction = "\xEB\x0C\x90";
    memcpy(bootSectorData->BootJumpInstruction, bootJumpInstruction, 3);
    memcpy(bootSectorData->OemIdentifier, "MSWIN4.1", 8);
    bootSectorData->BytesPerSector = 512;
    bootSectorData->SectorsPerCluster = 4;   //TODO CHANGE
    bootSectorData->ReservedSectors = 32;
    bootSectorData->FatCount = 2;
    bootSectorData->RootDirEntryCount = 0;
    bootSectorData->TotalSectors = 0;
    bootSectorData->MediaDescriptorType = 99;
    bootSectorData->X_SectorsPerFat = 0;
    bootSectorData->SectorsPerTrack = 99;
    bootSectorData->Heads = 99;
    bootSectorData->HiddenSectors = 0;
    bootSectorData->LargeSectorCount = 0;

    //EBPB
    bootSectorData->SectorsPerFat = 36;
    bootSectorData->Flags = 20224; //40xF00
    bootSectorData->FatVersion =0;
    bootSectorData->RootDirCluster = 2;
    bootSectorData->FsInfoSector = 1;
    bootSectorData->BackupBootSector = 6;
    memset(bootSectorData->Reserved_1, 0, 12);
    bootSectorData->Drive = 0;
    bootSectorData->FlagsWinNT = 0;
    bootSectorData->Signature = 99;
    bootSectorData->VolumeId = 0;
    memcpy(bootSectorData->VolumeLabel, "LABEL      ", 11);
    memcpy(bootSectorData->SystemId, "FAT32   ", 8);
    bootSectorData->BootSignature = 43605; //(0xAA55)

    char* firstBootSectorBuffer = new char[diskInfo->diskParameters.sectorSizeBytes];
    memcpy(firstBootSectorBuffer, bootSectorData, 90);
    memcpy(firstBootSectorBuffer + 510, "\x55\xAA", 2);

    uint32_t numberOfSectorsWritten = 0;
    int retryWriteCount = 2;
    int writeResult = writeDiskSectors(diskInfo, 1, 0, firstBootSectorBuffer, numberOfSectorsWritten);
    while(writeResult != EC_NO_ERROR && retryWriteCount > 0)
    {
        writeResult = writeDiskSectors(diskInfo, 1, 0, firstBootSectorBuffer, numberOfSectorsWritten);
        retryWriteCount--;
    }

    if(retryWriteCount == 0)
    {
        throw std::runtime_error("Failed to initialize first boot sector");
    }

    char* SecondBootSectorBuffer = new char[diskInfo->diskParameters.sectorSizeBytes];
    char* secondBootSectorData = new char[28];  //FsInfo
    memcpy(secondBootSectorData, "\x72\x72\x41\x61\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x55\xAA", 28);
    memcpy(SecondBootSectorBuffer, "\x52\x52\x61\x41", 4);
    memcpy(SecondBootSectorBuffer + 484, secondBootSectorData, 28);

    numberOfSectorsWritten = 0;
    writeDiskSectors(diskInfo, 1, 1, SecondBootSectorBuffer, numberOfSectorsWritten);

    retryWriteCount = 2;
    writeResult =  writeDiskSectors(diskInfo, 1, 1, SecondBootSectorBuffer, numberOfSectorsWritten);
    while(writeResult != EC_NO_ERROR && retryWriteCount > 0)
    {
        writeResult =  writeDiskSectors(diskInfo, 1, 1, SecondBootSectorBuffer, numberOfSectorsWritten);
        retryWriteCount--;
    }

    if(retryWriteCount == 0)
    {
        throw std::runtime_error("Failed to initialize second boot sector");
    }

    char* rootFileName = new char[11];
    memset(rootFileName, ' ', 11);
    memcpy(rootFileName, "Root", 4); //in root its name 'Root' is in first dir entry, while in root's children will be in dot dot
    DirectoryEntry* rootDirectoryEntry = new DirectoryEntry();
    memset(rootDirectoryEntry, 0, 32); //to have 0 for values that are not set below
    memcpy(rootDirectoryEntry->FileName, rootFileName, 11);
    rootDirectoryEntry->FileSize = 64; //root dir does not contain dot & dotdot entries, but we consider that they exist for symmetry with other clusters
    rootDirectoryEntry->FirstClusterLow = bootSectorData->RootDirCluster;
    rootDirectoryEntry->FirstClusterHigh = bootSectorData->RootDirCluster >> 16;
    rootDirectoryEntry->Attributes = ATTR_DIRECTORY;
    char* rootFirstSectorData = new char[bootSectorData->BytesPerSector];
    memset(rootFirstSectorData, 0, bootSectorData->BytesPerSector);
    memcpy(rootFirstSectorData, rootDirectoryEntry, 32);

    retryWriteCount = 2;
    numberOfSectorsWritten = 0;
    writeResult =  writeDiskSectors(diskInfo, 1, getFirstSectorForCluster(bootSectorData, bootSectorData->RootDirCluster),
                                    rootFirstSectorData, numberOfSectorsWritten);
    while(writeResult != EC_NO_ERROR && retryWriteCount > 0)
    {
        writeResult =  writeDiskSectors(diskInfo, 1, getFirstSectorForCluster(bootSectorData, bootSectorData->RootDirCluster),
                                        rootFirstSectorData, numberOfSectorsWritten);
        retryWriteCount--;
    }

    if(retryWriteCount == 0)
    {
        throw std::runtime_error("Failed to write root directory entry");
    }

    delete[] firstBootSectorBuffer;
    delete[] SecondBootSectorBuffer;
    delete[] secondBootSectorData;
}

void initializeFat(DiskInfo* diskInfo, BootSector* bootSector)
{
    char* firstFatSectorData = new char[bootSector->BytesPerSector];
    memset(firstFatSectorData, '\0', bootSector->BytesPerSector);

    *(uint32_t*)&firstFatSectorData[0] = FAT_VALUE_RESERVED_1;
    *(uint32_t*)&firstFatSectorData[4] = FAT_VALUE_RESERVED_1;
    *(uint32_t*)&firstFatSectorData[8] = FAT_VALUE_EOC; //root first cluster (root starts from cluster 2 so the 2 previous ones are ignored)

    uint32_t numberOfSectorsWritten = 0;
    writeDiskSectors(diskInfo, 1, getFirstFatSector(bootSector), firstFatSectorData, numberOfSectorsWritten);
}

BootSector* readBootSector(DiskInfo* diskInfo)
{
    char* readBuffer = new char[diskInfo->diskParameters.sectorSizeBytes];
    uint32_t numberOfSectorsRead = 0;
    readDiskSectors(diskInfo, 1, 0, readBuffer, numberOfSectorsRead);

    BootSector* bootSector = (BootSector*)&readBuffer[0];
    memcpy((char*)&bootSector->BootSignature, &readBuffer[510], 2);

    return bootSector;
}

FsInfo* readFsInfo(DiskInfo* diskInfo, BootSector* bootSector)
{
    char* readBuffer = new char[diskInfo->diskParameters.sectorSizeBytes];
    uint32_t numberOfSectorsRead = 0;
    readDiskSectors(diskInfo, 1, bootSector->FsInfoSector, readBuffer, numberOfSectorsRead);

    FsInfo * fsInfo = (FsInfo*)&readBuffer[480];
    memcpy((char*)&fsInfo->LeadSignature, &readBuffer[0], 4);

    return fsInfo;
}