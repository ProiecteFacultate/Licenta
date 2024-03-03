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
#include "../include/fat32Codes.h"
#include "../include/fat32Attributes.h"
#include "../include/fat32.h"
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
        int searchDirectoryEntryResult = findDirectoryEntryByDirectoryName(diskInfo, bootSector, actualDirectoryEntry,
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

uint32_t getSubDirectories(DiskInfo* diskInfo, BootSector* bootSector, char* directoryPath)
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
            return DIR_CREATION_FAILED;
        }

        actualDirectoryName = strtok(nullptr, "/");
    }
}