#include "windows.h"
#include "string"

#include "../include/diskInitialization.h"

void initializeDisk(const char* diskDirectory, int totalSectorsNumber)
{
    for(int sector = 0; sector < totalSectorsNumber; sector++)
    {
        size_t fullFilePathLen = strlen(diskDirectory) + 32;
        char* fullFilePath = new char[fullFilePathLen];
        snprintf(fullFilePath, fullFilePathLen, "%s//sector_%d", diskDirectory, sector);

        HANDLE fileHandle = CreateFile(
                fullFilePath,
                GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL,
                CREATE_NEW,
                FILE_ATTRIBUTE_NORMAL,
                NULL);

        char* strText = "Hello World!";
        DWORD bytesWritten;
        WriteFile(
                fileHandle,
                strText,
                strlen(strText),
                &bytesWritten,
                nullptr);

        CloseHandle(fileHandle);
    }
}