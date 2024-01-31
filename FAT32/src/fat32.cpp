#include "windows.h"
#include "string"
#include "string.h"
#include "iostream"

#include "../include/disk.h"
#include "../include/diskUtils.h"
#include "../include/diskCodes.h"
#include "../include/fat32Init.h"
#include "../include/fat32MethodUtils.h"
#include "../include/fat32.h"


uint32_t getNextCluster(DiskInfo* diskInfo, BootSector* bootSector, uint32_t actualCluster)
{
    uint32_t firstFatSector = bootSector->ReservedSectors;   //first fat table comes immediately after the reserved sectors
    char* fatTable = new char[bootSector->BytesPerSector];
    uint32_t nextClusterOffset = actualCluster * 4;
    uint32_t sector = firstFatSector + (nextClusterOffset / bootSector->BytesPerSector);
    uint32_t offsetInsideSector = nextClusterOffset % bootSector->BytesPerSector;

    uint32_t numOfBytesRead = 0;
    int readResult = readDiskSectors(diskInfo, 1, sector, fatTable, numOfBytesRead);

    uint32_t nextCluster = *(uint32_t*)&fatTable[offsetInsideSector];

    //TODO treat special cases like EOF

    nextCluster &= 0x0FFFFFFF;   //we should ignore the high 4 bits; only the low 28 matter

    delete[] fatTable;
    return nextCluster;
}

int createFolder(DiskInfo* diskInfo, BootSector* bootSector, char* directoryParentPath, char* directoryName)
{
    char* actualDir = strtok(directoryParentPath, "/");
    DirectoryEntry* actualDirEntry = nullptr;

    while(actualDir != nullptr)
    {
        actualDirEntry = findDirectoryEntry(diskInfo, bootSector, actualDirEntry, actualDir);
        actualDir = strtok(nullptr, "/");
    }
}


//////////////
//
//
//
//
//
//
////////////// HELPER FUNCTIONS //////////////


static DirectoryEntry* findDirectoryEntry(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* parentDirEntry, char* searchedDir)
{
    if(parentDirEntry == nullptr)    //it means that searchedDir is Root
    {
        DirectoryEntry* rootDirEntry = new DirectoryEntry;
        memset(rootDirEntry->FileName, '\0', 11);
        rootDirEntry->FirstClusterLow = bootSector->RootDirCluster & 0x0000FFFF;
        rootDirEntry->FirstClusterHigh = bootSector->RootDirCluster & 0xFFFF0000;

        return rootDirEntry;
    }

    char* clusterData = new char[bootSector->SectorsPerCluster * bootSector->BytesPerSector];
    uint32_t numOfSectorsRead = 0;
    uint32_t actualCluster = (parentDirEntry->FirstClusterHigh & 0xFFFF0000) | (parentDirEntry->FirstClusterLow & 0x0000FFFF);
    uint32_t firstSectorInCluster = bootSector->ReservedSectors + (actualCluster - 2) * bootSector->SectorsPerCluster;
    int readResult = readDiskSectors(diskInfo, bootSector->SectorsPerCluster, firstSectorInCluster, clusterData, numOfSectorsRead);

    while(readResult == EC_NO_ERROR)
    {
        DirectoryEntry* directoryEntry = findDirectoryEntryInCluster(bootSector, clusterData, searchedDir);

        if(directoryEntry == nullptr)   //file with given name don't exist
        {
            return nullptr;
        }
        else if(directoryEntry->FileName[0] == '\0')   //we reached cluster ending, but haven't found the fileName yet; so we read the next cluster
        {
            actualCluster = getNextCluster(diskInfo, bootSector, actualCluster);
            numOfSectorsRead = 0;
            firstSectorInCluster = bootSector->ReservedSectors + (actualCluster - 2) * bootSector->SectorsPerCluster;
            readResult = readDiskSectors(diskInfo, bootSector->SectorsPerCluster, firstSectorInCluster, clusterData, numOfSectorsRead);
        }
        else  //success
        {
            return directoryEntry;
        }
    }
}

static DirectoryEntry* findDirectoryEntryInCluster(BootSector* bootSector, char* clusterData, char* dirName)
{
    uint32_t index = 64;  //first 2 entries are for dot and dotdot
    DirectoryEntry* dirEntry = (DirectoryEntry*)&clusterData[index];
    if(dirEntry->FileName[0] == '\0')     //it means that there are no more directory entries, and the rest of the cluster is empty
    {
        return nullptr;
    }

    while(true)
    {
        if(compareFileNames(dirName, dirEntry->FileName) == true)
        {
            return dirEntry;
        }

        index += 32;
        if(index >= bootSector->SectorsPerCluster * bootSector->BytesPerSector)     //we reached cluster ending, but haven't found the fileName yet
        {
            memset(dirEntry, '\0', 32);
            return dirEntry;
        }

        dirEntry = (DirectoryEntry*)&clusterData[index];
    }
}