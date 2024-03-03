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
#include "../include/Utils.h"
#include "../include/fat32Codes.h"
#include "../include/fat32Attributes.h"
#include "../include/fat32FunctionUtils.h"

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

uint32_t getNextCluster(DiskInfo* diskInfo, BootSector* bootSector, uint32_t actualCluster, uint32_t& nextCluster)
{
    char* fatTable = new char[bootSector->BytesPerSector];
    uint32_t nextClusterOffset = actualCluster * 4;
    uint32_t sector = getFirstFatSector(bootSector) + (nextClusterOffset / bootSector->BytesPerSector);
    uint32_t offsetInsideSector = nextClusterOffset % bootSector->BytesPerSector;

    uint32_t numOfSectorsRead = 0;
    int readResult = readDiskSectors(diskInfo, 1, sector, fatTable, numOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        return FAT_VALUE_RETRIEVE_FAILED;
    }

    nextCluster = *(uint32_t*)&fatTable[offsetInsideSector] & 0x0FFFFFFF; //the high 4 bits are ignored
    delete[] fatTable;

    switch (nextCluster) {
        case 0x00000000:
            return FAT_VALUE_FREE;
        case 0x00000001:
            return FAT_VALUE_RESERVED_1;
        case 0x00000002 ... 0x0FFFFFEF:
            return FAT_VALUE_USED;
        case 0x0FFFFFF0 ... 0x0FFFFFF5:
            return FAT_VALUE_RESERVED_2;
        case 0x0FFFFFF6:
            return FAT_VALUE_RESERVED_3;
        case 0x0FFFFFF7:
            return FAT_VALUE_BAD_SECTOR;
        default:
            return FAT_VALUE_EOC;
    }
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

uint32_t findNthClusterInChain(DiskInfo* diskInfo, BootSector* bootSector, uint32_t firstCluster, uint32_t n, uint32_t& foundClusterNumber)
{
    if(n == 0)
    {
        foundClusterNumber = firstCluster;
        return CLUSTER_SEARCH_SUCCESS;
    }

    int index = 1;
    uint32_t actualCluster = firstCluster;

    while(index <= n)
    {
        uint32_t getNextClusterResult = getNextCluster(diskInfo, bootSector, actualCluster, foundClusterNumber);

        if(getNextClusterResult == FAT_VALUE_RETRIEVE_FAILED)
            return CLUSTER_SEARCH_IN_CHAN_FAILED;
        else if(index == n)
            return CLUSTER_SEARCH_IN_CHAN_SUCCESS;
        else if(getNextClusterResult == FAT_VALUE_EOC)
            return CLUSTER_SEARCH_IN_CHAN_EOC;

        actualCluster = foundClusterNumber;
        index++;
    }
}

void createDirectoryEntry(char* directoryName, uint8_t directoryAttribute, uint32_t firstCluster, DirectoryEntry* directoryEntry)
{
    SYSTEMTIME time;
    GetSystemTime(&time);
    char* formattedName = new char[11];
    formatDirectoryName(directoryName, formattedName);

    memcpy(&directoryEntry[0], formattedName, 11);
    directoryEntry->Attributes = directoryAttribute;
    directoryEntry->Reserved = 0;
    directoryEntry->CreationTimeTenths = time.wMilliseconds / 10;
    directoryEntry->CreationTime = (time.wHour * 3600 + time.wMinute * 60 + time.wSecond) / 2; //we have granularity of 2 secs, otherwise 16 bits is not enough. Multiply with 2 for real value in sec
    directoryEntry->CreationDate = ((time.wYear - 1900) << 9) | (time.wMonth << 5) | time.wDay; //high 7 bits represent how many years since 1900, next 5 for month, last 4 for day
    directoryEntry->LastAccessedDate = directoryEntry->CreationDate;
    directoryEntry->FirstClusterHigh = firstCluster >> 16;
    directoryEntry->LastWriteTime = directoryEntry->CreationTime;
    directoryEntry->LastAccessedDate = directoryEntry->CreationDate;
    directoryEntry->FirstClusterLow = firstCluster;
    directoryEntry->FileSize = 64; //dot and dotdot directoryEntries
}
