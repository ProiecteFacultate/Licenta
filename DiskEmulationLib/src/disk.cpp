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

    char* fullFilePath = new char[200];
    memset(fullFilePath, 0, 100);
    memcpy(fullFilePath, diskDirectory, strlen(diskDirectory));
    strcat(fullFilePath, "\\Data");

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

    return new DiskInfo(diskDirectory, sectorsNumber, sectorSize, EC_NO_ERROR);
}

int fillDiskInitialMemory(DiskInfo *diskInfo, uint32_t batchSize)
{
    uint32_t numOfSectorsWritten = 0;
    char* buffer = new char[diskInfo->diskParameters.sectorSizeBytes * batchSize];
    uint32_t startSector;
    int retryWriteCount = 2;
    memset(buffer, '\0', diskInfo->diskParameters.sectorSizeBytes * batchSize);

    for(startSector = 0; startSector + batchSize < diskInfo->diskParameters.sectorsNumber; startSector += batchSize)
    {
        int writeResult = writeSectorsEfficientlyForInitialization(diskInfo, batchSize, startSector, buffer, numOfSectorsWritten);
        while(writeResult != EC_NO_ERROR && retryWriteCount > 0)
        {
            writeResult = writeSectorsEfficientlyForInitialization(diskInfo, batchSize, startSector, buffer, numOfSectorsWritten);
            retryWriteCount--;
        }

        if(writeResult != EC_NO_ERROR && retryWriteCount == 0)
        {
            throw std::runtime_error("Failed to fill disk initial memory");
        }
    }

    if(startSector <= diskInfo->diskParameters.sectorsNumber)
    {
        retryWriteCount = 2;
        uint32_t numOfRemainedSectors = diskInfo->diskParameters.sectorsNumber - startSector;

        int writeResult = writeSectorsEfficientlyForInitialization(diskInfo, numOfRemainedSectors, startSector, buffer, numOfSectorsWritten);
        while(writeResult != EC_NO_ERROR && retryWriteCount > 0)
        {
            writeResult = writeSectorsEfficientlyForInitialization(diskInfo, numOfRemainedSectors, startSector, buffer, numOfSectorsWritten);
            retryWriteCount--;
        }

        if(writeResult != EC_NO_ERROR && retryWriteCount == 0)
        {
            throw std::runtime_error("Failed to fill disk initial memory");
        }
    }

    return EC_NO_ERROR;
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
    CloseHandle(fileHandle);

    if(!readFileResult)
    {
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

static char* buildFilePath(const char* diskDirectory)
{
    char* fullFilePath = new char[200];
    memset(fullFilePath, 0, 100);
    memcpy(fullFilePath, diskDirectory, strlen(diskDirectory));
    strcat(fullFilePath, "\\Data");

    return fullFilePath;
}

static int readSector(DiskInfo *diskInfo, uint32_t sector, char *buffer)
{
    char *fullFilePath = buildFilePath(diskInfo->diskDirectory);

    HANDLE fileHandle = CreateFile(fullFilePath,OFN_READONLY,0,nullptr,OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL,nullptr);

    if(fileHandle == INVALID_HANDLE_VALUE)
    {
        CloseHandle(fileHandle);
        delete[] fullFilePath;
        return SECTOR_READ_FAILED;
    }

    OVERLAPPED overlapped;
    memset(&overlapped, 0, sizeof(OVERLAPPED));
    overlapped.Offset = diskInfo->diskParameters.sectorSizeBytes * sector;
    overlapped.hEvent = nullptr;

    DWORD dwBytesRead = 0;
    bool readFileResult = ReadFile(fileHandle, buffer, diskInfo->diskParameters.sectorSizeBytes, &dwBytesRead, &overlapped);

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
    char* fullFilePath = buildFilePath(diskInfo->diskDirectory);

    HANDLE fileHandle = CreateFile(fullFilePath,OF_READWRITE,0,nullptr,OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL,nullptr);

    if(fileHandle == INVALID_HANDLE_VALUE)
    {
        CloseHandle(fileHandle);
        delete[] fullFilePath;
        return SECTOR_WRITE_FAILED;
    }

    char* writeBuffer = new char[diskInfo->diskParameters.sectorSizeBytes];
    memcpy(writeBuffer, buffer, diskInfo->diskParameters.sectorSizeBytes);

    OVERLAPPED overlapped;
    memset(&overlapped, 0, sizeof(OVERLAPPED));
    overlapped.Offset = diskInfo->diskParameters.sectorSizeBytes * sector;
    overlapped.hEvent = nullptr;

    DWORD bytesWritten = 0;
    bool writeFileResult = WriteFile(fileHandle,writeBuffer,diskInfo->diskParameters.sectorSizeBytes, &bytesWritten, &overlapped);
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

static int writeSectorsEfficientlyForInitialization(DiskInfo *diskInfo, uint32_t numOfSectorsToWrite, uint32_t sector, char* buffer, uint32_t &numOfSectorsWritten)
{
    char* fullFilePath = buildFilePath(diskInfo->diskDirectory);

    HANDLE fileHandle = CreateFile(fullFilePath,OF_READWRITE,0,nullptr,OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL,nullptr);

    if(fileHandle == INVALID_HANDLE_VALUE)
    {
        numOfSectorsWritten = 0;
        CloseHandle(fileHandle);
        delete[] fullFilePath;
        return EC_ERROR_IN_DISK_CONTROLLER;
    }

    uint32_t numOfBytesToWrite = diskInfo->diskParameters.sectorSizeBytes * numOfSectorsToWrite;
    char* writeBuffer = new char[numOfBytesToWrite];
    memcpy(writeBuffer, buffer, numOfBytesToWrite);

    OVERLAPPED overlapped;
    memset(&overlapped, 0, sizeof(OVERLAPPED));
    overlapped.Offset = diskInfo->diskParameters.sectorSizeBytes * sector;
    overlapped.hEvent = nullptr;

    DWORD bytesWritten = 0;
    bool writeFileResult = WriteFile(fileHandle,writeBuffer,numOfBytesToWrite, &bytesWritten, &overlapped);
    if(!writeFileResult || bytesWritten < numOfBytesToWrite)
    {
        numOfSectorsWritten = bytesWritten / diskInfo->diskParameters.sectorSizeBytes;
        CloseHandle(fileHandle);
        delete[] fullFilePath;
        delete[] writeBuffer;
        return EC_ERROR_IN_DISK_CONTROLLER;
    }

    CloseHandle(fileHandle);
    delete[] fullFilePath;
    delete[] writeBuffer;

    return EC_NO_ERROR;
}
