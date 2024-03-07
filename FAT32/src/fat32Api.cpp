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
    if(!checkDirectoryNameValidity(newDirectoryName))
        return DIR_CREATION_INVALID_DIRNAME;

    char* actualDirectoryName = strtok(directoryParentPath, "/");
    DirectoryEntry* actualDirectoryEntry = nullptr;
    DirectoryEntry* searchedDirectoryEntry = new DirectoryEntry();

    while(actualDirectoryName != nullptr)
    {
        uint32_t searchDirectoryEntryResult = findDirectoryEntryByDirectoryName(diskInfo, bootSector, actualDirectoryEntry,
                                                                           actualDirectoryName, searchedDirectoryEntry);
        if(searchDirectoryEntryResult == DIR_ENTRY_FOUND)
            actualDirectoryEntry = searchedDirectoryEntry;
        else
        {
            delete searchedDirectoryEntry, delete actualDirectoryName; //don't delete actualDirectoryEntry because is either null, or points to the same address as searchedDirectoryEntry
            return DIR_CREATION_FAILED;
        }

        actualDirectoryName = strtok(nullptr, "/");
    }

    uint32_t parentAlreadyContainsDirectoryWithGivenName = findDirectoryEntryByDirectoryName(diskInfo, bootSector, actualDirectoryEntry,
                                                                                             newDirectoryName, new DirectoryEntry());

    if(parentAlreadyContainsDirectoryWithGivenName == DIR_ENTRY_FOUND)
    {
        delete actualDirectoryEntry;
        return DIR_CREATION_NEW_NAME_ALREADY_EXISTS;
    }

    uint32_t emptyClusterNumber = -1;
    int searchEmptyClusterResult = searchEmptyCluster(diskInfo, bootSector, emptyClusterNumber);

    if(searchEmptyClusterResult == CLUSTER_SEARCH_FAILED)
    {
        delete actualDirectoryEntry;
        return DIR_CREATION_FAILED;
    }
    else if(searchEmptyClusterResult == CLUSTER_SEARCH_NO_FREE_CLUSTERS)
    {
        delete actualDirectoryEntry;
        return DIR_CREATION_NO_CLUSTER_AVAILABLE;
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

uint32_t getSubDirectories(DiskInfo* diskInfo, BootSector* bootSector, char* directoryPath, std::vector<DirectoryEntry*>& subDirectories)
{
    char* actualDirectoryName = strtok(directoryPath, "/");
    DirectoryEntry* actualDirectoryEntry = nullptr;
    DirectoryEntry* searchedDirectoryEntry = new DirectoryEntry();

    while(actualDirectoryName != nullptr)
    {
        int searchDirectoryEntryResult = findDirectoryEntryByDirectoryName(diskInfo, bootSector, actualDirectoryEntry,
                                                                           actualDirectoryName, searchedDirectoryEntry);
        if(searchDirectoryEntryResult == DIR_ENTRY_FOUND)
            actualDirectoryEntry = searchedDirectoryEntry;
        else
        {
            delete searchedDirectoryEntry, delete actualDirectoryName; //don't delete actualDirectoryEntry because is either null, or points to the same address as searchedDirectoryEntry
            return GET_SUB_DIRECTORIES_FAILED;
        }

        actualDirectoryName = strtok(nullptr, "/");
    }

    char* clusterData = new char[getClusterSize(bootSector)]; //declare here for time efficiency

    uint32_t actualCluster = getFirstClusterForDirectory(bootSector, actualDirectoryEntry);  //directory first cluster
    uint32_t offsetInCluster = 64; //we start from 64 because in the first sector of the first cluster we got dot & dotdot
    uint32_t numberOfClusterInParentDirectory = 0; //the cluster number in chain
    uint32_t occupiedBytesInCluster = 0; //if the cluster is full, then it is cluster size, otherwise smaller

    while (true)
    {
        occupiedBytesInCluster = actualDirectoryEntry->FileSize >= getClusterSize(bootSector) * (numberOfClusterInParentDirectory + 1) ? getClusterSize(bootSector)
                                                                                                                    : actualDirectoryEntry->FileSize % getClusterSize(bootSector);

        uint32_t numOfSectorsRead = 0;
        int readResult = readDiskSectors(diskInfo, bootSector->SectorsPerCluster, getFirstSectorForCluster(bootSector, actualCluster), 
                                         clusterData, numOfSectorsRead);

        if(readResult == EC_NO_ERROR)
        {
            for(; offsetInCluster < occupiedBytesInCluster; offsetInCluster += 32)
            {
                DirectoryEntry* directoryEntry = new DirectoryEntry(); //same for time efficiency
                memcpy(directoryEntry, clusterData + offsetInCluster, 32);
                subDirectories.push_back(directoryEntry);
            }
        }
        else
        {
            delete[] clusterData, delete searchedDirectoryEntry, delete actualDirectoryName;
            return GET_SUB_DIRECTORIES_SUCCESS;
        }

        uint32_t nextCluster = 0;
        uint32_t getNextClusterResult = getNextCluster(diskInfo, bootSector, actualCluster, nextCluster);

        if(getNextClusterResult == FAT_VALUE_RETRIEVE_FAILED)
        {
            delete[] clusterData, delete searchedDirectoryEntry, delete actualDirectoryName;
            return GET_SUB_DIRECTORIES_FAILED;
        }

        if(getNextClusterResult == FAT_VALUE_EOC)
        {
            delete[] clusterData, delete searchedDirectoryEntry, delete actualDirectoryName;
            return GET_SUB_DIRECTORIES_SUCCESS;
        }

        actualCluster = nextCluster;
        numberOfClusterInParentDirectory++;
        offsetInCluster = 0; //64 is only for the first cluster, for the rest of them is 0
    }
}