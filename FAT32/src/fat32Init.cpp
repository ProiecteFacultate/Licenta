#include "windows.h"
#include "string"
#include "string.h"
#include "iostream"

#include "../include/disk.h"
#include "../include/diskUtils.h"
#include "../include/diskCodes.h"
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
{
    char* firstBootSectorBuffer = new char[diskInfo->diskParameters.sectorSizeBytes];
    char* firstBootSectorData = new char[90];
    memcpy(firstBootSectorData, "\xEB\x0C\x90MSWIN4.1\x00\x02\x40\x20\x00\x02\x00\x00\x00\x00\xFA\x00\x00\x22\x00\x22\x00\x00\x00\x00\x00\x00\x00\x00\x00" // BPB + EBPB
     "\x22\x00\x00\x00\x00\x4F\x00\x00\x02\x00\x00\x00\x01\x00\x06\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x29\x00\x00\x00\x00LABEL      FAT32   ", 90);
    memcpy(firstBootSectorBuffer, firstBootSectorData, 90);
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
    delete[] firstBootSectorData;
    delete[] SecondBootSectorBuffer;
    delete[] secondBootSectorData;
}

void initializeFat(DiskInfo* diskInfo, BootSector* bootSector)
{
    uint32_t firstFatSector = bootSector->ReservedSectors;
    char* firstFatSectorData = new char[bootSector->BytesPerSector];
    memset(firstFatSectorData, '\0', bootSector->BytesPerSector);

    firstFatSectorData[0] = '\x01';
    firstFatSectorData[4] = '\x01';

    uint32_t numberOfSectorsWritten = 0;
    writeDiskSectors(diskInfo, 1, firstFatSector, firstFatSectorData, numberOfSectorsWritten);
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