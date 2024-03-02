#include "windows.h"
#include "string"
#include "string.h"
#include "algorithm"
#include "iostream"

#include "../include/disk.h"
#include "../include/diskUtils.h"
#include "../include/diskCodes.h"
#include "../include/fat32Init.h"
#include "../include/fat32.h"

bool checkDirectoryNameValidity(const char* directoryName)
{
    char invalidChars[] = {0x22, 0x2A, 0x2B, 0x2C, 0x2F, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x5B, 0x5C, 0x5D};

    int index;
    for(index = 0; index < strlen(directoryName); index++) //we can use strlen since NULL is an invalid char in the name anyway
        if((directoryName[index] < 0x20 && !(index == 0 && directoryName[index] == 0x05)) || std::find(invalidChars, invalidChars + 14, directoryName[index]))
            return false;

    if(directoryName[0] == '.')
        return false;

    int nameSize = 0;
    index = 0;
    while(directoryName[index] < strlen(directoryName) && directoryName[index] != '.')
    {
        nameSize++;
        index++;
    }

    if(nameSize > 8)
        return false;

    int extensionSize = 0;
    index = strlen(directoryName) - 1;
    while(directoryName[index] >= 0 && directoryName[index] != '.')
    {
        extensionSize++;
        index--;
    }

    if(extensionSize > 3)
        return false;

    return true;
}

bool compareDirectoryNames(char* expected, const char* actual)
{
    for(int i = 0; i < 11; i++)
        expected[i] = toupper(expected[i]);

    int beforeDotIndex = 0;
    while(beforeDotIndex < 8 && expected[beforeDotIndex] != '.')
    {
        if(expected[beforeDotIndex] != actual[beforeDotIndex])
            return false;

        beforeDotIndex++;
    }

    int afterDotIndex = 8;
    while(expected[afterDotIndex] != '\0') //expected will have null value at the end (after the extension)
    {
        if(expected[afterDotIndex] != actual[afterDotIndex])
            return false;

        afterDotIndex++;
    }

    for(int i = beforeDotIndex; i < 8; i++)
        if(actual[i] != 0x20)
            return false;

    for(int i = afterDotIndex; i < 11; i++)
        if(actual[i] != 0x20)
            return false;

    return true;
}

void formatDirectoryName(const char* directoryName, char* formattedName)
{
    memset(formattedName, 0x20, 11);

    int index;
    for(index = 0; directoryName[index] != '\0' && directoryName[index] != '.'; index++)
        formattedName[index] = toupper(directoryName[index]);

    if(directoryName[index] == '.')
    {
        index++;
        int extensionIndex = 0;
        while(directoryName[index] != '\0')
        {
            formattedName[8 + extensionIndex] = toupper(directoryName[index]);
            extensionIndex++;
            index++;
        }
    }
}

uint16_t getFirstFatSector(BootSector* bootSector)
{
    return bootSector->ReservedSectors;   //first fat table comes immediately after the reserved sectors
}

uint32_t getFirstSectorForCluster(BootSector* bootSector, uint32_t cluster)
{
    return bootSector->ReservedSectors + bootSector->FatCount * bootSector->SectorsPerFat + (cluster - bootSector->RootDirCluster) * bootSector->SectorsPerCluster;
}

uint32_t getTotalNumberOfDataClusters(DiskInfo* diskInfo, BootSector* bootSector)
{
    uint32_t numberOfDataSectors = diskInfo->diskParameters.sectorsNumber - (bootSector->ReservedSectors + (bootSector->FatCount * bootSector->SectorsPerFat));
    return numberOfDataSectors / bootSector->SectorsPerCluster;
}

void copyNewDirectoryTimeToDotDirectoryEntries(DirectoryEntry* newDirectoryEntry, DirectoryEntry* dotDirectoryEntry, DirectoryEntry* dotDotDirectoryEntry)
{
    dotDirectoryEntry->CreationDate = newDirectoryEntry->CreationDate;
    dotDirectoryEntry->CreationTime = newDirectoryEntry->CreationTime;
    dotDirectoryEntry->CreationTimeTenths = newDirectoryEntry->CreationTimeTenths;
    dotDirectoryEntry->LastAccessedDate = newDirectoryEntry->LastAccessedDate;
    dotDirectoryEntry->LastWriteDate = newDirectoryEntry->LastWriteDate;
    dotDirectoryEntry->LastWriteTime = newDirectoryEntry->LastWriteTime;

    dotDotDirectoryEntry->CreationDate = newDirectoryEntry->CreationDate;
    dotDotDirectoryEntry->CreationTime = newDirectoryEntry->CreationTime;
    dotDotDirectoryEntry->CreationTimeTenths = newDirectoryEntry->CreationTimeTenths;
    dotDotDirectoryEntry->LastAccessedDate = newDirectoryEntry->LastAccessedDate;
    dotDotDirectoryEntry->LastWriteDate = newDirectoryEntry->LastWriteDate;
    dotDotDirectoryEntry->LastWriteTime = newDirectoryEntry->LastWriteTime;
}