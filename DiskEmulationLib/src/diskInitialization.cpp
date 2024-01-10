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
}

DiskInfo initializeDisk(const char* diskDirectory, unsigned int sectorsNumber, unsigned int sectorSize)
{
    for(int sector = 0; sector < sectorsNumber; sector++)
    {
        size_t fullFilePathLen = strlen(diskDirectory) + 32;
        char* fullFilePath = new char[fullFilePathLen];
        snprintf(fullFilePath, fullFilePathLen, "%s\\sector_%d", diskDirectory, sector);

        HANDLE fileHandle = CreateFile(
                fullFilePath,
                GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL,
                CREATE_NEW,
                FILE_ATTRIBUTE_NORMAL,
                NULL);

//        char* strText = "Hello World!";
//        DWORD bytesWritten;
//        WriteFile(
//                fileHandle,
//                strText,
//                strlen(strText),
//                &bytesWritten,
//                nullptr);
//
//        CloseHandle(fileHandle);
    }

    long long totalDiskSizeBytes = sectorsNumber * sectorSize;
    return DiskInfo(diskDirectory, sectorsNumber, sectorSize, totalDiskSizeBytes);
}

void writeBytes(DiskInfo* diskInfo, int sector, const char* data)
{
    size_t fullFilePathLen = strlen(diskInfo->diskDirectory) + 32;
    char* fullFilePath = new char[fullFilePathLen];
    snprintf(fullFilePath, fullFilePathLen, "%s\\sector_%d", diskInfo->diskDirectory, sector);

    HANDLE fileHandle = CreateFile(
            fullFilePath,
            OF_READWRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
    );

    DWORD bytesWritten;
    WriteFile(fileHandle,data,strlen(data),&bytesWritten,nullptr);
    CloseHandle(fileHandle);
}