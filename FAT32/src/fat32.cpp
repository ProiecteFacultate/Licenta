#include "windows.h"
#include "string"
#include "string.h"
#include "iostream"

#include "../include/disk.h"
#include "../include/diskUtils.h"
#include "../include/diskCodes.h"
#include "../include/fat32Init.h"
#include "../include/fat32MethodUtils.h"
#include "../include/fat32Codes.h"
#include "../include/fat32.h"


uint32_t getNextCluster(DiskInfo* diskInfo, BootSector* bootSector, uint32_t actualCluster, uint32_t& nextCluster)
{
    uint32_t firstFatSector = bootSector->ReservedSectors;   //first fat table comes immediately after the reserved sectors
    char* fatTable = new char[bootSector->BytesPerSector];
    uint32_t nextClusterOffset = actualCluster * 4;
    uint32_t sector = firstFatSector + (nextClusterOffset / bootSector->BytesPerSector);
    uint32_t offsetInsideSector = nextClusterOffset % bootSector->BytesPerSector;

    uint32_t numOfBytesRead = 0;
    int readResult = readDiskSectors(diskInfo, 1, sector, fatTable, numOfBytesRead);

    nextCluster = *(uint32_t*)&fatTable[offsetInsideSector];
    delete[] fatTable;

    switch (nextCluster) {
        case 0x0000000:
            return FAT_VALUE_FREE;
        case 0x0000001:
            return FAT_VALUE_RESERVED_1;
        case 0x0000002 ... 0xFFFFFEF:
            nextCluster &= 0x0FFFFFFF;     //we should ignore the high 4 bits; only the low 28 matter
            return FAT_VALUE_USED;
        case 0xFFFFFF0 ... 0xFFFFFF5:
            return FAT_VALUE_RESERVED_2;
        case 0xFFFFFF6:
            return FAT_VALUE_RESERVED_3;
        case 0xFFFFFF7:
            return FAT_VALUE_BAD_SECTOR;
        default:
            return FAT_VALUE_EOC;
    }
}

int createDirectory(DiskInfo* diskInfo, BootSector* bootSector, char* directoryParentPath, char* directoryName)
{
    char* actualDirectoryName = strtok(directoryParentPath, "/");
    DirectoryEntry* actualDirectoryEntry = nullptr;

    while(actualDirectoryName != nullptr)
    {
        DirectoryEntry* searchedDirectoryEntry = new DirectoryEntry();
        int searchDirectoryEntryResult = findDirectoryEntry(diskInfo, bootSector, actualDirectoryEntry, actualDirectoryName, &searchedDirectoryEntry);
        if(searchDirectoryEntryResult == DIR_ENTRY_FOUND)
        {
            actualDirectoryEntry = searchedDirectoryEntry;
        }
        else
        {
            return DIR_CREATION_FAILED;
        }

        actualDirectoryName = strtok(nullptr, "/");
    }

    uint32_t emptyClusterNumber = -1;
    searchEmptyCluster(diskInfo, bootSector, emptyClusterNumber);

    return DIR_CREATION_SUCCESS;
}


//////////////
//
//
//
//
//
//
////////////// HELPER FUNCTIONS //////////////


static int findDirectoryEntry(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* parentDirectoryEntry, char* searchedDirectoryName, DirectoryEntry** searchedDirectoryEntry)
{
    if(parentDirectoryEntry == nullptr)    //it means that searchedDir is Root
    {
        memset(&(*searchedDirectoryEntry)->FileName, '\0', 11);
        (*searchedDirectoryEntry)->FirstClusterLow = bootSector->RootDirCluster & 0x0000FFFF;
        (*searchedDirectoryEntry)->FirstClusterHigh = bootSector->RootDirCluster & 0xFFFF0000;

        return DIR_ENTRY_FOUND;
    }

    char* clusterData = new char[bootSector->SectorsPerCluster * bootSector->BytesPerSector];
    uint32_t numOfSectorsRead = 0;
    uint32_t actualCluster = (parentDirectoryEntry->FirstClusterHigh & 0xFFFF0000) | (parentDirectoryEntry->FirstClusterLow & 0x0000FFFF);
    uint32_t firstSectorInCluster = bootSector->ReservedSectors + (actualCluster - 2) * bootSector->SectorsPerCluster;
    int readResult = readDiskSectors(diskInfo, bootSector->SectorsPerCluster, firstSectorInCluster, clusterData, numOfSectorsRead);

    while(readResult == EC_NO_ERROR)
    {
        int searchDirectoryEntryInClusterResult = findDirectoryEntryInCluster(bootSector, clusterData, searchedDirectoryName, searchedDirectoryEntry);

        switch (searchDirectoryEntryInClusterResult) {
            case DIR_ENTRY_NO_MORE_ENTRIES:
                return DIR_ENTRY_DO_NOT_EXIST;
            case DIR_ENTRY_NOT_FOUND_IN_CLUSTER: {
                uint32_t nextCluster = 0;
                actualCluster = getNextCluster(diskInfo, bootSector, actualCluster, nextCluster);
                numOfSectorsRead = 0;
                firstSectorInCluster = bootSector->ReservedSectors + (actualCluster - 2) * bootSector->SectorsPerCluster;
                readResult = readDiskSectors(diskInfo, bootSector->SectorsPerCluster, firstSectorInCluster, clusterData,numOfSectorsRead);
                break;
            }
            default:
                return DIR_ENTRY_FOUND;
        }
    }
}

static int findDirectoryEntryInCluster(BootSector* bootSector, char* clusterData, char* dirName, DirectoryEntry** directoryEntry)
{
    uint32_t index = 64;  //first 2 entries are for dot and dotdot
    *directoryEntry = (DirectoryEntry*)&clusterData[index];
    if((*directoryEntry)->FileName[0] == '\0')     //it means that there are no more directory entries, and the rest of the cluster is empty
    {
        return DIR_ENTRY_NO_MORE_ENTRIES;
    }

    while(true)
    {
        if(compareFileNames(dirName, (*directoryEntry)->FileName))
        {
            return DIR_ENTRY_FOUND;
        }

        index += 32;
        if(index >= bootSector->SectorsPerCluster * bootSector->BytesPerSector)     //we reached cluster ending, but haven't found the fileName yet
        {
            return DIR_ENTRY_NOT_FOUND_IN_CLUSTER;
        }

        *directoryEntry = (DirectoryEntry*)&clusterData[index];
    }
}

static int searchEmptyCluster(DiskInfo* diskInfo, BootSector* bootSector, uint32_t& emptyClusterNumber)
{
    uint32_t firstFatSector = bootSector->ReservedSectors;   //first fat table comes immediately after the reserved sectors
    char* fatTable = new char[bootSector->BytesPerSector];
    uint32_t numOfSectorsRead = 0;
    uint32_t numberOfDataSectors = diskInfo->diskParameters.sectorsNumber - (bootSector->ReservedSectors + (bootSector->FatCount * bootSector->SectorsPerFat));
    uint32_t numberOfClusters = numberOfDataSectors / bootSector->SectorsPerCluster;
    uint32_t numberOfClusterEntriesPerFatSector = bootSector->BytesPerSector / 4;

    for(uint32_t sectorInFat = 0; sectorInFat < bootSector->SectorsPerFat; sectorInFat++)
    {
        int readResult = readDiskSectors(diskInfo, 1, firstFatSector + sectorInFat, fatTable, numOfSectorsRead);
        for(uint32_t offset = 0; offset < bootSector->BytesPerSector; offset += 4)
        {
            if(sectorInFat * numberOfClusterEntriesPerFatSector + offset / 4 + 1 > numberOfClusters)
            {
                return CLUSTER_SEARCH_NO_FREE_CLUSTERS;
            }

            uint32_t fatValue = *(uint32_t*)&fatTable[offset];
            if(fatValue == FAT_VALUE_FREE)
            {
                emptyClusterNumber = sectorInFat * numberOfClusterEntriesPerFatSector + offset / 4;
                return CLUSTER_SEARCH_SUCCESS;
            }
        }
    }

    return CLUSTER_SEARCH_FAILED;
}