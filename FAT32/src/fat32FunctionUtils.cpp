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
#include "../include/codes/fat32Codes.h"
#include "../include/fat32Attributes.h"
#include "../include/fat32FunctionUtils.h"

uint32_t getClusterSize(BootSector* bootSector)
{
    return bootSector->SectorsPerCluster * bootSector->BytesPerSector;
}

uint16_t getFirstFatSector(BootSector* bootSector)
{
    return bootSector->ReservedSectors;   //first fat table comes immediately after the reserved sectors
}

uint32_t getFirstClusterForDirectory(BootSector* bootSector, DirectoryEntry* directoryEntry)
{
    return ((uint32_t) directoryEntry->FirstClusterHigh << 16) | (uint32_t) directoryEntry->FirstClusterLow;
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

    delete[] formattedName;
}

uint32_t updateSubDirectoriesDotDotEntries(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* givenDirectoryEntry, DirectoryEntry* newDotDotEntry)
{
    uint32_t numOfSectorsRead = 0;
    uint32_t numberOfSectorsWritten = 0;
    uint32_t nextCluster = 0;
    char* firstSectorInSubDirectoryData = new char[bootSector->BytesPerSector];

    char* clusterData = new char[getClusterSize(bootSector)];
    uint32_t actualCluster = getFirstClusterForDirectory(bootSector, givenDirectoryEntry);
    uint32_t readResult = readDiskSectors(diskInfo, bootSector->SectorsPerCluster, getFirstSectorForCluster(bootSector, actualCluster),
                                          clusterData, numOfSectorsRead);
    uint32_t numberOfClusterInParentDirectory = 0; //the cluster number in chain
    uint32_t occupiedBytesInCluster = 0; //if the cluster is full, then it is cluster size, otherwise smaller
    uint32_t offsetInCluster = 64; //we start from 64 because in the first sector of the first cluster we got dot & dotdot

    while(readResult == EC_NO_ERROR)
    {
        occupiedBytesInCluster = givenDirectoryEntry->FileSize >= getClusterSize(bootSector) * (numberOfClusterInParentDirectory + 1) ? getClusterSize(bootSector)
                                                                                                                     : givenDirectoryEntry->FileSize % getClusterSize(bootSector);

        //iterate over subDirectories entries in this cluster and update their dotdot entry
        for(; offsetInCluster < occupiedBytesInCluster; offsetInCluster += 32)
        {
            DirectoryEntry* subDirectoryEntry = (DirectoryEntry*)&clusterData[offsetInCluster];
            uint32_t subDirectoryFirstSector = getFirstSectorForCluster(bootSector, getFirstClusterForDirectory(bootSector, subDirectoryEntry));
            readResult = readDiskSectors(diskInfo, 1, subDirectoryFirstSector,firstSectorInSubDirectoryData, numOfSectorsRead);

            if(readResult != EC_NO_ERROR)
            {
                delete[] clusterData, delete[] firstSectorInSubDirectoryData;
                return DIRECTORY_UPDATE_SUBDIRECTORIES_DOT_DOT_FAILED;
            }

            memcpy(firstSectorInSubDirectoryData + 32, newDotDotEntry, 32);
            uint32_t writeResult = writeDiskSectors(diskInfo, 1, subDirectoryFirstSector, firstSectorInSubDirectoryData, numberOfSectorsWritten);

            if(writeResult != EC_NO_ERROR)
            {
                delete[] clusterData, delete[] firstSectorInSubDirectoryData;
                return DIRECTORY_UPDATE_SUBDIRECTORIES_DOT_DOT_FAILED;
            }
        }

        //get the next cluster of the given directory
        uint32_t getNextClusterResult = getNextCluster(diskInfo, bootSector, actualCluster, nextCluster);

        if(getNextClusterResult == FAT_VALUE_RETRIEVE_FAILED)
        {
            delete[] clusterData, delete[] firstSectorInSubDirectoryData;
            return DIRECTORY_UPDATE_SUBDIRECTORIES_DOT_DOT_FAILED;
        }
        else if(getNextClusterResult == FAT_VALUE_EOC)
        {
            delete[] clusterData, delete[] firstSectorInSubDirectoryData;
            return DIRECTORY_UPDATE_SUBDIRECTORIES_DOT_DOT_SUCCESS;
        }

        actualCluster = nextCluster;
        numOfSectorsRead = 0;
        readResult = readDiskSectors(diskInfo, bootSector->SectorsPerCluster, getFirstSectorForCluster(bootSector, actualCluster),
                                     clusterData,numOfSectorsRead);

        offsetInCluster = 0; //64 is only for the first cluster, for the rest of them is 0
    }
}