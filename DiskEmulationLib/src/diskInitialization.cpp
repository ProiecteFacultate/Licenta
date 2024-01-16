#include "windows.h"
#include "string"
#include "string.h"
#include "iostream"

#include "../include/diskInitialization.h"
#include "../include/diskCallsResponse.h"

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
    char* defaultPartitionName = new char[128];
    strcpy(defaultPartitionName, "Partition_1\0");

    char* partitionPath = new char[1024];
    snprintf(partitionPath, 1024, "%s\\%s", diskDirectory, defaultPartitionName);
    CreateDirectoryA(partitionPath, NULL);

    for(int sector = 0; sector < sectorsNumber; sector++) {
        char *fullFilePath = buildFilePath(diskDirectory, defaultPartitionName, sector);

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

int readDiskSectors(DiskInfo *diskInfo, char* partition, unsigned int numOfSectorsToRead, unsigned int sector, char* buffer, int &numOfSectorsRead)
{
    memset(buffer, '\0', strlen(buffer));

    for(int sectorNum = 0; sectorNum < numOfSectorsToRead; sectorNum++)
    {
        if(sector + sectorNum > diskInfo->sectorsNumber)
        {
            numOfSectorsRead = sectorNum;
            diskInfo->status = EC_SECTOR_NOT_FOUND;
            return EC_SECTOR_NOT_FOUND;
        }

        char* sectorReadBuffer = new char[diskInfo->sectorSizeBytes + 1];

        int readSectorResult = readSector(diskInfo, partition, sector + sectorNum, sectorReadBuffer);
        if(readSectorResult == SECTOR_READ_FAILED)
        {
            numOfSectorsRead = sectorNum;
            diskInfo->status = EC_READ_ERROR;
            return EC_READ_ERROR;
        }

        strcat(buffer, sectorReadBuffer);
    }

    numOfSectorsRead = numOfSectorsToRead;
    diskInfo->status = NO_ERROR;
    return NO_ERROR;
}

int writeDiskSectors(DiskInfo *diskInfo, char* partition, unsigned int numOfSectorsToWrite, unsigned int sector, char* buffer, int &numOfSectorsWritten)
{
    for(int sectorNum = 0; sectorNum < numOfSectorsToWrite; sectorNum++)
    {
        if(sector + sectorNum > diskInfo->sectorsNumber)
        {
            numOfSectorsWritten = sectorNum;
            diskInfo->status = EC_SECTOR_NOT_FOUND;
            return EC_SECTOR_NOT_FOUND;
        }

        if(strlen(buffer) == 0)  //we successfully wrote all buffer, but in less sectors than specified
        {
            numOfSectorsWritten = sectorNum;
            diskInfo->status = NO_ERROR;
            return NO_ERROR;
        }

        char* sectorWriteBuffer = new char[diskInfo->sectorSizeBytes + 1];
        strncpy(sectorWriteBuffer, buffer, diskInfo->sectorSizeBytes);
        sectorWriteBuffer[diskInfo->sectorSizeBytes] = '\0';
        size_t remainingBufferSize = std::max(static_cast<int>(strlen(buffer) - diskInfo->sectorSizeBytes), 0);
        memmove(buffer, buffer + diskInfo->sectorSizeBytes, remainingBufferSize);
        buffer[remainingBufferSize] = '\0';

        int writeSectorResult = writeSector(diskInfo, partition, sector + sectorNum, sectorWriteBuffer);
        if(writeSectorResult == SECTOR_WRITE_FAILED)
        {
            numOfSectorsWritten = sectorNum;
            diskInfo->status = EC_ERROR_IN_DISK_CONTROLLER;
            return EC_ERROR_IN_DISK_CONTROLLER;
        }
    }

    numOfSectorsWritten = numOfSectorsToWrite;
    diskInfo->status = EC_NO_ERROR;
    return EC_NO_ERROR;
}

int verifyDiskSectors(DiskInfo *diskInfo, char* partition, unsigned int numOfSectorsToVerify, unsigned int sector, char* buffer, int &numOfSectorsVerified)
{
    for(int sectorNum = 0; sectorNum < numOfSectorsToVerify; sectorNum++)
    {
        if(sector + sectorNum > diskInfo->sectorsNumber)
        {
            numOfSectorsVerified = sectorNum;
            diskInfo->status = EC_SECTOR_NOT_FOUND;
            return EC_SECTOR_NOT_FOUND;
        }

        if(strlen(buffer) == 0)  //we successfully verified all buffer, but in less sectors than specified
        {
            numOfSectorsVerified = sectorNum;
            diskInfo->status = EC_NO_ERROR;
            return EC_NO_ERROR;
        }

        char* sectorVerifyBuffer = new char[diskInfo->sectorSizeBytes + 1];
        strncpy(sectorVerifyBuffer, buffer, diskInfo->sectorSizeBytes);
        sectorVerifyBuffer[diskInfo->sectorSizeBytes] = '\0';
        size_t remainingBufferSize = std::max(static_cast<int>(strlen(buffer) - diskInfo->sectorSizeBytes), 0);
        memmove(buffer, buffer + diskInfo->sectorSizeBytes, remainingBufferSize);
        buffer[remainingBufferSize] = '\0';

        int verifySectorResult = verifySector(diskInfo, partition, sector + sectorNum, sectorVerifyBuffer);
        if(verifySectorResult == SECTOR_VERIFY_FAILED)  //this error can appear only if reading sector operation fails
        {
            numOfSectorsVerified = sectorNum;
            diskInfo->status = EC_READ_ERROR;
            return EC_READ_ERROR;
        }
        else if (verifySectorResult == SECTOR_VERIFY_UNEQUAL_DATA)
        {
            numOfSectorsVerified = sectorNum;
            diskInfo->status = EC_NO_ERROR;
            return MULTIPLE_SECTORS_VERIFY_UNEQUAL_DATA; //here we have a special case, where disk status != return value, so we can make difference between verify error & content mismatch
        }
    }

    numOfSectorsVerified = numOfSectorsToVerify;
    diskInfo->status = EC_NO_ERROR;
    return EC_NO_ERROR;
}

int formatDiskSectors(DiskInfo *diskInfo, char* partition, unsigned int sector)
{
    if(sector >= diskInfo->sectorsNumber)
    {
        diskInfo->status = EC_SECTOR_NOT_FOUND;
        return EC_SECTOR_NOT_FOUND;
    }

    for(int sectorNum = sector; sectorNum < diskInfo->sectorsNumber; sectorNum++)
    {
        char *fullFilePath = buildFilePath(diskInfo->diskDirectory, partition, sectorNum);
        HANDLE fileHandle = CreateFile(fullFilePath,GENERIC_WRITE,FILE_SHARE_READ,NULL,TRUNCATE_EXISTING,
                   FILE_ATTRIBUTE_NORMAL,NULL);

        if(fileHandle == INVALID_HANDLE_VALUE) //format sector failed
        {
            diskInfo->status = EC_ERROR_IN_DISK_CONTROLLER;
            return EC_ERROR_IN_DISK_CONTROLLER;
        }
    }

    diskInfo->status = EC_NO_ERROR;
    return EC_NO_ERROR;
}

//////////////
//
//
//
//
//
//
////////////// HELPER FUNCTIONS //////////////

static char* buildFilePath(const char* diskDirectory, char* partition, int sector)
{
    size_t fullFilePathLen = strlen(diskDirectory) + 128;
    char* fullFilePath = new char[fullFilePathLen];
    snprintf(fullFilePath, fullFilePathLen, "%s\\%s\\sector_%d", diskDirectory, partition, sector);

    return fullFilePath;
}

static int readSector(DiskInfo *diskInfo, char* partition, unsigned int sector, char *buffer)
{
    char *fullFilePath = buildFilePath(diskInfo->diskDirectory, partition, sector);

    HANDLE fileHandle = CreateFile(fullFilePath,OFN_READONLY,0,NULL,OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL,NULL);

    if(fileHandle == INVALID_HANDLE_VALUE)
    {
        return SECTOR_READ_FAILED;
    }

    DWORD dwBytesRead = 0;
    bool readFileResult = ReadFile(fileHandle, buffer, diskInfo->sectorSizeBytes, &dwBytesRead, NULL);
    if(!readFileResult)
    {
        return SECTOR_READ_FAILED;
    }

    buffer[diskInfo->sectorSizeBytes] = '\0';
    CloseHandle(fileHandle);

    return SECTOR_READ_SUCCESS;
}

static int writeSector(DiskInfo *diskInfo, char* partition, unsigned int sector, char *buffer)
{
    char* fullFilePath = buildFilePath(diskInfo->diskDirectory, partition, sector);

    HANDLE fileHandle = CreateFile(fullFilePath,OF_READWRITE,0,NULL,OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL,NULL);

    if(fileHandle == INVALID_HANDLE_VALUE)
    {
        return SECTOR_WRITE_FAILED;
    }

    DWORD bytesWritten;
    bool writeFileResult = WriteFile(fileHandle,buffer,strlen(buffer),&bytesWritten,nullptr);
    if(!writeFileResult)
    {
        return SECTOR_WRITE_FAILED;
    }

    CloseHandle(fileHandle);

    return  SECTOR_WRITE_SUCCESS;
}

static int verifySector(DiskInfo *diskInfo, char* partition, unsigned int sector, char* buffer)
{
    char* sectorDataBuffer = new char[diskInfo->sectorSizeBytes + 1];
    int readSectorResult = readSector(diskInfo, partition, sector, sectorDataBuffer);

    if(readSectorResult == SECTOR_READ_FAILED)
    {
        return SECTOR_VERIFY_FAILED;
    }

    if(strlen(sectorDataBuffer) != strlen(buffer))
    {
        return SECTOR_VERIFY_UNEQUAL_DATA;
    }

    for(int i = 0; i < strlen(buffer); i++)
        if(sectorDataBuffer[i] != buffer[i])
        {
            return SECTOR_VERIFY_UNEQUAL_DATA;
        }

    return SECTOR_VERIFY_EQUAL_DATA;
}