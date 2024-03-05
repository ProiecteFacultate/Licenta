#include "windows.h"
#include "string"
#include "string.h"
#include "iostream"
#include "vector"

#include "../include/disk.h"
#include "../include/diskUtils.h"
#include "../include/diskCodes.h"
#include "../include/fat32Init.h"
#include "../include/Utils.h"
#include "../include/codes/fat32Codes.h"
#include "../include/codes/fat32ApiResponseCodes.h"
#include "../include/fat32Attributes.h"
#include "../include/fat32.h"
#include "../include/fat32FunctionUtils.h"
#include "../include/fat32Api.h"

uint32_t createDirectory(DiskInfo* diskInfo, BootSector* bootSector, char* directoryParentPath, char* newDirectoryName)
{
    if(checkDirectoryNameValidity(newDirectoryName))
        return DIR_CREATION_INVALID_DIRNAME;

    char* actualDirectoryName = strtok(directoryParentPath, "/");
    DirectoryEntry* actualDirectoryEntry = nullptr;
    DirectoryEntry* searchedDirectoryEntry = new DirectoryEntry();

    while(actualDirectoryName != nullptr)
    {
        uint32_t searchDirectoryEntryResult = findDirectoryEntryByDirectoryName(diskInfo, bootSector, actualDirectoryEntry,
                                                                           actualDirectoryName, &searchedDirectoryEntry);
        if(searchDirectoryEntryResult == DIR_ENTRY_FOUND)
            actualDirectoryEntry = searchedDirectoryEntry;
        else
        {
            delete searchedDirectoryEntry, delete actualDirectoryName; //don't delete actualDirectoryEntry because is either null, or points to the same address as searchedDirectoryEntry
            return DIR_CREATION_FAILED;
        }

        actualDirectoryName = strtok(nullptr, "/");
    }

    uint32_t emptyClusterNumber = -1;
    int searchEmptyClusterResult = searchEmptyCluster(diskInfo, bootSector, emptyClusterNumber);

    if(searchEmptyClusterResult == CLUSTER_SEARCH_FAILED)
    {
        delete actualDirectoryEntry;
        return DIR_CREATION_FAILED;
    }

    uint32_t updateFatResult = updateFat(diskInfo, bootSector, emptyClusterNumber, "\x0F\xFF\xFF\xFF"); //updates fat value for the first cluster in the new directory

    if(updateFatResult == FAT_UPDATE_FAILED)
    {
        delete actualDirectoryEntry;
        return DIR_CREATION_FAILED;
    }

    DirectoryEntry* newDirectoryEntry = new DirectoryEntry(); //the directory entry for the newly created directory
    addDirectoryEntryToParent(diskInfo, bootSector, actualDirectoryEntry, newDirectoryName, emptyClusterNumber, newDirectoryEntry);
    setupFirstClusterInDirectory(diskInfo, bootSector, actualDirectoryEntry ,emptyClusterNumber, newDirectoryEntry);

    delete actualDirectoryEntry;
    return DIR_CREATION_SUCCESS;
}

uint32_t getSubDirectories(DiskInfo* diskInfo, BootSector* bootSector, char* directoryPath, std::vector<DirectoryEntry*> subDirectories)
{
    char* actualDirectoryName = strtok(directoryPath, "/");
    DirectoryEntry* actualDirectoryEntry = nullptr;
    DirectoryEntry* searchedDirectoryEntry = new DirectoryEntry();

    while(actualDirectoryName != nullptr)
    {
        int searchDirectoryEntryResult = findDirectoryEntryByDirectoryName(diskInfo, bootSector, actualDirectoryEntry,
                                                                           actualDirectoryName, &searchedDirectoryEntry);
        if(searchDirectoryEntryResult == DIR_ENTRY_FOUND)
            actualDirectoryEntry = searchedDirectoryEntry;
        else
        {
            delete searchedDirectoryEntry, delete actualDirectoryName; //don't delete actualDirectoryEntry because is either null, or points to the same address as searchedDirectoryEntry
            return GET_SUB_DIRECTORIES_FAILED;
        }

        actualDirectoryName = strtok(nullptr, "/");
    }

    uint32_t actualCluster = ((uint32_t) actualDirectoryEntry->FirstClusterHigh << 16) | (uint32_t) actualDirectoryEntry->FirstClusterLow;  //directory first cluster

    while (true)
    {   uint32_t nextCluster = 0;
        uint32_t getNextClusterResult = getNextCluster(diskInfo, bootSector, actualCluster, nextCluster);
        
        if(getNextClusterResult == FAT_VALUE_RETRIEVE_FAILED)
        {
            delete searchedDirectoryEntry, delete actualDirectoryName;
            return GET_SUB_DIRECTORIES_FAILED;
        }

        uint32_t clusterOccupiedSpace = getClusterSize(bootSector);
        if(getNextClusterResult == FAT_VALUE_EOC) //it means this is the last cluster in directory so it might have less directory entries (unless it is completely full)
            clusterOccupiedSpace = actualDirectoryEntry->FileSize % getClusterSize(bootSector);

        char* clusterData = new char[getClusterSize(bootSector)];
        uint32_t numOfSectorsRead = 0;
        int readResult = readDiskSectors(diskInfo, bootSector->SectorsPerCluster, getFirstSectorForCluster(bootSector, actualCluster), 
                                         clusterData, numOfSectorsRead);
        
        if(readResult == EC_NO_ERROR)
        {
            for(uint32_t offset = 64; offset < clusterOccupiedSpace; offset += 32) //first 2 entries are dot & dotdot (in case of root they don't exist but the first is data about root
            {
                DirectoryEntry* directoryEntry = new DirectoryEntry();
                memcpy(directoryEntry, clusterData + offset, 32);
                subDirectories.push_back(directoryEntry);
            }
            delete[] clusterData;
        }
        else
        {
            delete searchedDirectoryEntry, delete[] clusterData, delete actualDirectoryName;
            return GET_SUB_DIRECTORIES_SUCCESS;
        }

        if(getNextClusterResult == FAT_VALUE_EOC)
            return GET_SUB_DIRECTORIES_SUCCESS;

        actualCluster = nextCluster;
    }
}