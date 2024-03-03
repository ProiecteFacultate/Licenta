#include "windows.h"
#include "string"
#include "string.h"
#include "iostream"

#include "../include/disk.h"
#include "../include/diskUtils.h"
#include "../include/diskCodes.h"
#include "../include/Utils.h"
#include "../include/fat32FunctionUtils.h"
#include "../include/fat32Codes.h"
#include "../include/fat32Init.h"


bool checkBootSectorsInitialized(DiskInfo* diskInfo)
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
{//TODO use bootsector structure instead of bytes
    BootSector* bootSectorData = new BootSector();

    //BPB
    char* bootJumpInstruction = "\xEB\x0C\x90";
    memcpy(bootSectorData->BootJumpInstruction, bootJumpInstruction, 3);
    memcpy(bootSectorData->OemIdentifier, "MSWIN4.1", 8);
    bootSectorData->BytesPerSector = 512;
    bootSectorData->SectorsPerCluster = 16;
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