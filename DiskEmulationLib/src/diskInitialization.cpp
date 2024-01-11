#include "windows.h"
#include "string"
#include "string.h"
#include "iostream"

#include "../include/diskInitialization.h"

DiskInfo::DiskInfo(const char* diskDirectory, unsigned int sectorsNumber, unsigned int sectorSizeBytes, unsigned long long totalSizeBytes)   //DiskInfo constructor
{
    this->diskDirectory = diskDirectory;
    this->sectorsNumber = sectorsNumber;
    this->sectorSizeBytes = sectorSizeBytes;
    this->totalSizeBytes = totalSizeBytes;
    this->status = 0;
}

DiskInfo initializeDisk(const char* diskDirectory, unsigned int sectorsNumber, unsigned int sectorSize)
{
    for(int sector = 0; sector < sectorsNumber; sector++) {
        char *fullFilePath = buildFilePath(diskDirectory, sector);

        CreateFile(fullFilePath,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_NEW,
                   FILE_ATTRIBUTE_NORMAL,NULL);
    }

    long long totalDiskSizeBytes = sectorsNumber * sectorSize;
    return DiskInfo(diskDirectory, sectorsNumber, sectorSize, totalDiskSizeBytes);
}

//////////////
//
//
//
//
//
//
////////////// DISK SERVICES //////////////

int getDiskStatus(DiskInfo *diskInfo)
{
    return diskInfo->status;
}

int readDiskSectors(DiskInfo *diskInfo, int numOfSectorsToRead, int sector, char* buffer, int &numOfSectorsRead)
{
    memset(buffer, '\0', strlen(buffer));

    for(int sectorNum = 0; sectorNum < numOfSectorsToRead; sectorNum++)
    {
        if(sector + sectorNum > diskInfo->sectorsNumber)
        {
            numOfSectorsRead = sectorNum;
            diskInfo->status = 4;
            return 1;
        }

        char* sectorReadBuffer = new char[diskInfo->sectorSizeBytes + 1];

        int readSectorResult = readSector(diskInfo, sector + sectorNum, sectorReadBuffer);
        if(readSectorResult == 1) //read sector failed
        {
            numOfSectorsRead = sectorNum;
            diskInfo->status = 64;
            return 1;
        }

        strcat(buffer, sectorReadBuffer);
    }

    numOfSectorsRead = numOfSectorsToRead;
    diskInfo->status = 0;
    return 0;
}

int writeDiskSectors(DiskInfo *diskInfo, int numOfSectorsToWrite, int sector, char* buffer, int &numOfSectorsWritten)
{
    for(int sectorNum = 0; sectorNum < numOfSectorsToWrite; sectorNum++)
    {
        if(sector + sectorNum > diskInfo->sectorsNumber)
        {
            numOfSectorsWritten = sectorNum;
            diskInfo->status = 4;
            return 1;
        }

        if(strlen(buffer) == 0)  //we successfully wrote all buffer, but in less sectors than specified
        {
            numOfSectorsWritten = sectorNum;
            diskInfo->status = 0;
            return 0;
        }

        char* sectorWriteBuffer = new char[diskInfo->sectorSizeBytes + 1];
        strncpy(sectorWriteBuffer, buffer, diskInfo->sectorSizeBytes);
        sectorWriteBuffer[diskInfo->sectorSizeBytes] = '\0';
        size_t remainingBufferSize = std::max(static_cast<int>(strlen(buffer) - diskInfo->sectorSizeBytes), 0);
        memmove(buffer, buffer + diskInfo->sectorSizeBytes, remainingBufferSize);
        buffer[remainingBufferSize] = '\0';

        int writeSectorResult = writeSector(diskInfo, sector + sectorNum, sectorWriteBuffer);
        if(writeSectorResult == 1) //write sector failed
        {
            numOfSectorsWritten = sectorNum;
            diskInfo->status = 64;
            return 1;
        }
    }

    numOfSectorsWritten = numOfSectorsToWrite;
    diskInfo->status = 0;
    return 0;
}

//////////////
//
//
//
//
//
//
////////////// HELPER FUNCTIONS //////////////

static char* buildFilePath(const char* diskDirectory, int sector)
{
    size_t fullFilePathLen = strlen(diskDirectory) + 32;
    char* fullFilePath = new char[fullFilePathLen];
    snprintf(fullFilePath, fullFilePathLen, "%s\\sector_%d", diskDirectory, sector);

    return fullFilePath;
}

static int readSector(DiskInfo *diskInfo, int sector, char *buffer) //returns 0 if success, 1 otherwise
{
    char *fullFilePath = buildFilePath(diskInfo->diskDirectory, sector);

    HANDLE fileHandle = CreateFile(fullFilePath,OFN_READONLY,0,NULL,OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL,NULL);

    if(fileHandle == INVALID_HANDLE_VALUE)
    {
        return 1;
    }

    DWORD dwBytesRead = 0;
    bool readFileResult = ReadFile(fileHandle, buffer, diskInfo->sectorSizeBytes, &dwBytesRead, NULL);
    if(!readFileResult)
    {
        return 1;
    }

    buffer[diskInfo->sectorSizeBytes] = '\0';
    CloseHandle(fileHandle);

    return 0;
}

static int writeSector(DiskInfo *diskInfo, int sector, char *buffer) //returns 0 if success, 1 otherwise
{
    char* fullFilePath = buildFilePath(diskInfo->diskDirectory, sector);

    HANDLE fileHandle = CreateFile(fullFilePath,OF_READWRITE,0,NULL,OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL,NULL);

    if(fileHandle == INVALID_HANDLE_VALUE)
    {
        return 1;
    }

    DWORD bytesWritten;
    bool writeFileResult = WriteFile(fileHandle,buffer,strlen(buffer),&bytesWritten,nullptr);
    if(!writeFileResult)
    {
        return 1;
    }

    CloseHandle(fileHandle);

    return  0;
}