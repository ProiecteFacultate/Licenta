#include "windows.h"
#include "string"
#include "string.h"
#include "iostream"

#include "../include/disk.h"
#include "../include/diskUtils.h"
#include "../include/diskCodes.h"

DiskInfo* initializeDisk(const char* diskDirectory, uint32_t sectorsNumber, uint16_t sectorSize)
{
    if(createMetadataFile(diskDirectory, sectorsNumber, sectorSize) == METADATA_SECTOR_WRITE_FAILED)
    {
        return nullptr;
    }

    for(uint32_t sector = 0; sector < sectorsNumber; sector++) {
        char *fullFilePath = buildFilePath(diskDirectory, sector);

        HANDLE fileHandle = CreateFile(fullFilePath,GENERIC_READ,FILE_SHARE_READ,nullptr,CREATE_ALWAYS,
                   FILE_ATTRIBUTE_NORMAL,nullptr);

        if(fileHandle == INVALID_HANDLE_VALUE && GetLastError() != ERROR_ALREADY_EXISTS)
        {
            CloseHandle(fileHandle);
            delete[] fullFilePath;
            return nullptr;
        }

        delete[] fullFilePath;
        CloseHandle(fileHandle);
    }

    return new DiskInfo(diskDirectory, sectorsNumber, sectorSize, EC_NO_ERROR);
}

DiskInfo* getDisk(const char* diskDirectory)
{
    size_t metadataFilePathLen = strlen(diskDirectory) + 32;
    char* metadataFilePath = new char[metadataFilePathLen];
    snprintf(metadataFilePath, metadataFilePathLen, "%s\\Metadata", diskDirectory);

    HANDLE fileHandle = CreateFile(metadataFilePath,OFN_READONLY,0,nullptr,OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL,nullptr);

    if(fileHandle == INVALID_HANDLE_VALUE)
    {
        CloseHandle(fileHandle);
        delete[] metadataFilePath;
        return nullptr;
    }

    DWORD dwBytesRead = 0;
    DiskMetadata diskMetadata;
    bool readFileResult = ReadFile(fileHandle, &diskMetadata, sizeof(diskMetadata), &dwBytesRead, nullptr);
    if(!readFileResult)
    {
        CloseHandle(fileHandle);
        delete[] metadataFilePath;
        return nullptr;
    }

    delete[] metadataFilePath;
    return new DiskInfo(diskDirectory, diskMetadata.sectorsNumber, diskMetadata.sectorSizeBytes, EC_NO_ERROR);
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
    for(uint32_t sectorNum = 0; sectorNum < numOfSectorsToRead; sectorNum++)
    {
        if(sector + sectorNum > diskInfo->diskParameters.sectorsNumber)
        {
            numOfSectorsRead = sectorNum;
            diskInfo->status = EC_SECTOR_NOT_FOUND;
            return EC_SECTOR_NOT_FOUND;
        }

        char* sectorReadBuffer = new char[diskInfo->diskParameters.sectorSizeBytes];
        int readSectorResult = readSector(diskInfo, sector + sectorNum, sectorReadBuffer);

        if(readSectorResult == SECTOR_READ_FAILED)
        {
            numOfSectorsRead = sectorNum;
            diskInfo->status = EC_READ_ERROR;
            delete[] sectorReadBuffer;
            return EC_READ_ERROR;
        }

        concat_buffer(buffer, sectorReadBuffer, diskInfo->diskParameters.sectorSizeBytes * sectorNum,diskInfo->diskParameters.sectorSizeBytes);
        delete[] sectorReadBuffer;
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

        int writeSectorResult = writeSector(diskInfo, sector + sectorNum, buffer);

        if(writeSectorResult == SECTOR_WRITE_FAILED)
        {
            numOfSectorsWritten = sectorNum;
            diskInfo->status = EC_ERROR_IN_DISK_CONTROLLER;
            return EC_ERROR_IN_DISK_CONTROLLER;
        }

        buffer += diskInfo->diskParameters.sectorSizeBytes;
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

        int verifySectorResult = verifySector(diskInfo, sector + sectorNum, buffer);

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

        buffer += diskInfo->diskParameters.sectorSizeBytes;
    }

    numOfSectorsVerified = numOfSectorsToVerify;
    diskInfo->status = EC_NO_ERROR;
    return EC_NO_ERROR;
}

int formatDiskSectors(DiskInfo *diskInfo, uint32_t sector)
{
    char* formatBuffer = new char [diskInfo->diskParameters.sectorSizeBytes];
    memset(formatBuffer, '\0', diskInfo->diskParameters.sectorSizeBytes);

    for(uint32_t sectorNum = sector; sectorNum < diskInfo->diskParameters.sectorsNumber; sectorNum++)
    {
        if(sector >= diskInfo->diskParameters.sectorsNumber)
        {
            diskInfo->status = EC_SECTOR_NOT_FOUND;
            delete[] formatBuffer;
            return EC_SECTOR_NOT_FOUND;
        }

        int writeSectorResult = writeSector(diskInfo, sector + sectorNum, formatBuffer);

        if(writeSectorResult == SECTOR_WRITE_FAILED)
        {
            diskInfo->status = EC_ERROR_IN_DISK_CONTROLLER;
            delete[] formatBuffer;
            return EC_ERROR_IN_DISK_CONTROLLER;
        }
    }

    diskInfo->status = EC_NO_ERROR;

    delete[] formatBuffer;

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

static int createMetadataFile(const char* diskDirectory, uint32_t sectorsNumber, uint16_t sectorSize)
{
    size_t metadataFilePathLen = strlen(diskDirectory) + 32;
    char* metadataFilePath = new char[metadataFilePathLen];
    snprintf(metadataFilePath, metadataFilePathLen, "%s\\Metadata", diskDirectory);

    HANDLE metadataFileHandle = CreateFile(metadataFilePath,OF_READWRITE,FILE_SHARE_WRITE,nullptr,CREATE_NEW,
                                           FILE_ATTRIBUTE_NORMAL,nullptr);

    if(metadataFileHandle == INVALID_HANDLE_VALUE)
    {
        CloseHandle(metadataFileHandle);
        return METADATA_SECTOR_WRITE_FAILED;
    }

    DiskMetadata diskMetadata = DiskMetadata(sectorsNumber, sectorSize);
    DWORD bytesWritten;
    bool writeFileResult = WriteFile(metadataFileHandle, &diskMetadata,sizeof(diskMetadata),&bytesWritten,nullptr);
    if(!writeFileResult || bytesWritten != sizeof(diskMetadata))
    {
        CloseHandle(metadataFileHandle);
        return METADATA_SECTOR_WRITE_FAILED;
    }

    CloseHandle(metadataFileHandle);

    return METADATA_SECTOR_WRITE_SUCCESS;
}

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
        delete[] fullFilePath;
        return SECTOR_READ_FAILED;
    }

    DWORD dwBytesRead = 0;
    bool readFileResult = ReadFile(fileHandle, buffer, diskInfo->diskParameters.sectorSizeBytes, &dwBytesRead, nullptr);

    if(!readFileResult || dwBytesRead < diskInfo->diskParameters.sectorSizeBytes)
    {
        CloseHandle(fileHandle);
        delete[] fullFilePath;
        return SECTOR_READ_FAILED;
    }

    CloseHandle(fileHandle);
    delete[] fullFilePath;

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
        delete[] fullFilePath;
        return SECTOR_WRITE_FAILED;
    }

    char* writeBuffer = new char[diskInfo->diskParameters.sectorSizeBytes];
    copy_buffer(writeBuffer, buffer, diskInfo->diskParameters.sectorSizeBytes);

    DWORD bytesWritten = 0;
    bool writeFileResult = WriteFile(fileHandle,writeBuffer,diskInfo->diskParameters.sectorSizeBytes, &bytesWritten,nullptr);
    if(!writeFileResult || bytesWritten != diskInfo->diskParameters.sectorSizeBytes)
    {
        CloseHandle(fileHandle);
        delete[] fullFilePath;
        delete[] writeBuffer;
        return SECTOR_WRITE_FAILED;
    }

    CloseHandle(fileHandle);
    delete[] fullFilePath;
    delete[] writeBuffer;

    return  SECTOR_WRITE_SUCCESS;
}

static int verifySector(DiskInfo *diskInfo, uint32_t sector, char* buffer)
{
    char* sectorDataBuffer = new char[diskInfo->diskParameters.sectorSizeBytes];
    int readSectorResult = readSector(diskInfo, sector, sectorDataBuffer);

    if(readSectorResult == SECTOR_READ_FAILED)
    {
        delete[] sectorDataBuffer;
        return SECTOR_VERIFY_FAILED;
    }

    for(int i = 0; i < diskInfo->diskParameters.sectorSizeBytes; i++)
        if(sectorDataBuffer[i] != buffer[i])
        {
            delete[] sectorDataBuffer;
            return SECTOR_VERIFY_UNEQUAL_DATA;
        }

    delete[] sectorDataBuffer;
    return SECTOR_VERIFY_EQUAL_DATA;
}

