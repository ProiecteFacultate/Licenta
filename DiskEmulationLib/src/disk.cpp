#include "windows.h"
#include "string"
#include "string.h"
#include "iostream"

#include "../include/disk.h"
#include "../include/diskCallsResponse.h"


DiskParameters::DiskParameters(uint32_t sectorsNumber, uint16_t sectorSizeBytes) {
    this->sectorsNumber = sectorsNumber;
    this->sectorSizeBytes = sectorSizeBytes;
}

DiskInfo::DiskInfo(const char* diskDirectory, uint32_t sectorsNumber, uint16_t sectorSizeBytes, uint16_t status)   //DiskInfo constructor
{
    this->diskDirectory = diskDirectory;
    this->diskParameters = DiskParameters(sectorsNumber, sectorSizeBytes);
    this->status = status;

//    char *fullFilePath = buildFilePath(diskDirectory, sector);
//    HANDLE fileHandle = CreateFile(fullFilePath,GENERIC_READ,FILE_SHARE_READ,nullptr,CREATE_NEW,
//                                   FILE_ATTRIBUTE_NORMAL,nullptr);
}

DiskInfo initializeDisk(const char* diskDirectory, uint32_t sectorsNumber, uint16_t sectorSize)
{
    for(uint32_t sector = 0; sector < sectorsNumber; sector++) {
        char *fullFilePath = buildFilePath(diskDirectory, sector);

        HANDLE fileHandle = CreateFile(fullFilePath,GENERIC_READ,FILE_SHARE_READ,nullptr,CREATE_NEW,
                   FILE_ATTRIBUTE_NORMAL,nullptr);

        if(fileHandle == INVALID_HANDLE_VALUE && GetLastError() != ERROR_FILE_EXISTS)
        {
            CloseHandle(fileHandle);
            return  DiskInfo(nullptr, 0, 0, EC_NO_ERROR);
        }

        CloseHandle(fileHandle);
    }

    return  DiskInfo(diskDirectory, sectorsNumber, sectorSize, EC_NO_ERROR);
}

DiskInfo getDisk(const char* diskDirectory)
{

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

int readDiskSectors(DiskInfo *diskInfo, uint32_t numOfSectorsToRead, uint32_t sector, char* buffer, uint32_t &numOfSectorsRead)
{
    memset(buffer, '\0', strlen(buffer));

    for(uint32_t sectorNum = 0; sectorNum < numOfSectorsToRead; sectorNum++)
    {
        if(sector + sectorNum > diskInfo->diskParameters.sectorsNumber)
        {
            numOfSectorsRead = sectorNum;
            diskInfo->status = EC_SECTOR_NOT_FOUND;
            return EC_SECTOR_NOT_FOUND;
        }

        char* sectorReadBuffer = new char[diskInfo->diskParameters.sectorSizeBytes + 1];

        int readSectorResult = readSector(diskInfo, sector + sectorNum, sectorReadBuffer);
        if(readSectorResult == SECTOR_READ_FAILED)
        {
            numOfSectorsRead = sectorNum;
            diskInfo->status = EC_READ_ERROR;
            return EC_READ_ERROR;
        }

        strcat(buffer, sectorReadBuffer);
    }

    numOfSectorsRead = numOfSectorsToRead;
    diskInfo->status = EC_NO_ERROR;
    return EC_NO_ERROR;
}

int writeDiskSectors(DiskInfo *diskInfo, uint32_t numOfSectorsToWrite, uint32_t sector, char* buffer, uint32_t &numOfSectorsWritten)
{
    for(uint32_t sectorNum = 0; sectorNum < numOfSectorsToWrite; sectorNum++)
    {
        if(sector + sectorNum > diskInfo->diskParameters.sectorsNumber)
        {
            numOfSectorsWritten = sectorNum;
            diskInfo->status = EC_SECTOR_NOT_FOUND;
            return EC_SECTOR_NOT_FOUND;
        }

        if(strlen(buffer) == 0)  //we successfully wrote all buffer, but in less sectors than specified
        {
            numOfSectorsWritten = sectorNum;
            diskInfo->status = EC_NO_ERROR;
            return EC_NO_ERROR;
        }

        char* sectorWriteBuffer = new char[diskInfo->diskParameters.sectorSizeBytes + 1];
        strncpy(sectorWriteBuffer, buffer, diskInfo->diskParameters.sectorSizeBytes);
        sectorWriteBuffer[diskInfo->diskParameters.sectorSizeBytes] = '\0';
        size_t remainingBufferSize = std::max(static_cast<int>(strlen(buffer) - diskInfo->diskParameters.sectorSizeBytes), 0);
        memmove(buffer, buffer + diskInfo->diskParameters.sectorSizeBytes, remainingBufferSize);
        buffer[remainingBufferSize] = '\0';

        int writeSectorResult = writeSector(diskInfo, sector + sectorNum, sectorWriteBuffer);
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

int verifyDiskSectors(DiskInfo *diskInfo, uint32_t numOfSectorsToVerify, uint32_t sector, char* buffer, uint32_t &numOfSectorsVerified)
{
    for(uint32_t sectorNum = 0; sectorNum < numOfSectorsToVerify; sectorNum++)
    {
        if(sector + sectorNum > diskInfo->diskParameters.sectorsNumber)
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

        char* sectorVerifyBuffer = new char[diskInfo->diskParameters.sectorSizeBytes + 1];
        strncpy(sectorVerifyBuffer, buffer, diskInfo->diskParameters.sectorSizeBytes);
        sectorVerifyBuffer[diskInfo->diskParameters.sectorSizeBytes] = '\0';
        size_t remainingBufferSize = std::max(static_cast<int>(strlen(buffer) - diskInfo->diskParameters.sectorSizeBytes), 0);
        memmove(buffer, buffer + diskInfo->diskParameters.sectorSizeBytes, remainingBufferSize);
        buffer[remainingBufferSize] = '\0';

        int verifySectorResult = verifySector(diskInfo, sector + sectorNum, sectorVerifyBuffer);
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

int formatDiskSectors(DiskInfo *diskInfo, uint32_t sector)
{
    if(sector >= diskInfo->diskParameters.sectorsNumber)
    {
        diskInfo->status = EC_SECTOR_NOT_FOUND;
        return EC_SECTOR_NOT_FOUND;
    }

    for(uint32_t sectorNum = sector; sectorNum < diskInfo->diskParameters.sectorsNumber; sectorNum++)
    {
        char *fullFilePath = buildFilePath(diskInfo->diskDirectory, sectorNum);
        HANDLE fileHandle = CreateFile(fullFilePath,GENERIC_WRITE,FILE_SHARE_READ,nullptr,TRUNCATE_EXISTING,
                   FILE_ATTRIBUTE_NORMAL,nullptr);

        if(fileHandle == INVALID_HANDLE_VALUE) //format sector failed
        {
            CloseHandle(fileHandle);
            diskInfo->status = EC_ERROR_IN_DISK_CONTROLLER;
            return EC_ERROR_IN_DISK_CONTROLLER;
        }

        CloseHandle(fileHandle);
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

static char* buildFilePath(const char* diskDirectory, uint32_t sector)
{
    size_t fullFilePathLen = strlen(diskDirectory) + 32;
    char* fullFilePath = new char[fullFilePathLen];
    snprintf(fullFilePath, fullFilePathLen, "%s\\sector_%d", diskDirectory, sector);

    return fullFilePath;
}

static int readSector(DiskInfo *diskInfo, uint32_t sector, char *buffer)
{
    char *fullFilePath = buildFilePath(diskInfo->diskDirectory, sector);

    HANDLE fileHandle = CreateFile(fullFilePath,OFN_READONLY,0,nullptr,OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL,nullptr);

    if(fileHandle == INVALID_HANDLE_VALUE)
    {
        CloseHandle(fileHandle);
        return SECTOR_READ_FAILED;
    }

    DWORD dwBytesRead = 0;
    bool readFileResult = ReadFile(fileHandle, buffer, diskInfo->diskParameters.sectorSizeBytes, &dwBytesRead, nullptr);
    if(!readFileResult)
    {
        CloseHandle(fileHandle);
        return SECTOR_READ_FAILED;
    }

    buffer[diskInfo->diskParameters.sectorSizeBytes] = '\0';
    CloseHandle(fileHandle);

    return SECTOR_READ_SUCCESS;
}

static int writeSector(DiskInfo *diskInfo, uint32_t sector, char *buffer)
{
    char* fullFilePath = buildFilePath(diskInfo->diskDirectory, sector);

    HANDLE fileHandle = CreateFile(fullFilePath,OF_READWRITE,0,nullptr,OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL,nullptr);

    if(fileHandle == INVALID_HANDLE_VALUE)
    {
        CloseHandle(fileHandle);
        return SECTOR_WRITE_FAILED;
    }

    DWORD bytesWritten;
    bool writeFileResult = WriteFile(fileHandle,buffer,strlen(buffer),&bytesWritten,nullptr);
    if(!writeFileResult)
    {
        CloseHandle(fileHandle);
        return SECTOR_WRITE_FAILED;
    }

    CloseHandle(fileHandle);

    return  SECTOR_WRITE_SUCCESS;
}

static int verifySector(DiskInfo *diskInfo, uint32_t sector, char* buffer)
{
    char* sectorDataBuffer = new char[diskInfo->diskParameters.sectorSizeBytes + 1];
    int readSectorResult = readSector(diskInfo, sector, sectorDataBuffer);

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
