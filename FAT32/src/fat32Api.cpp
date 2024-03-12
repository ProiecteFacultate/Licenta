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

uint32_t createDirectory(DiskInfo* diskInfo, BootSector* bootSector, char* directoryParentPath, char* newDirectoryName, uint32_t newDirectoryAttribute)
{
    if(!checkDirectoryNameValidity(newDirectoryName))
        return DIR_CREATION_INVALID_DIRNAME;

    DirectoryEntry* actualDirectoryEntry = nullptr;
    uint32_t findDirectoryEntryResult = findDirectoryEntryByFullPath(diskInfo, bootSector, directoryParentPath,
                                                                     &actualDirectoryEntry);
    if(findDirectoryEntryResult != FIND_DIRECTORY_ENTRY_BY_PATH_SUCCESS)
        return DIR_CREATION_PARENT_DO_NOT_EXIST;

    uint32_t parentAlreadyContainsDirectoryWithGivenName = findDirectoryEntryByDirectoryName(diskInfo,
                                                                                             bootSector,
                                                                                             actualDirectoryEntry,
                                                                                             newDirectoryName,
                                                                                             new DirectoryEntry());

    if(actualDirectoryEntry->Attributes != ATTR_DIRECTORY)
    {
        delete actualDirectoryEntry;
        return DIR_CREATION_PARENT_NOT_A_DIRECTORY;
    }

    if(parentAlreadyContainsDirectoryWithGivenName == DIR_ENTRY_FOUND)
    {
        if(actualDirectoryEntry != nullptr)
            delete actualDirectoryEntry;
        return DIR_CREATION_NEW_NAME_ALREADY_EXISTS;
    }

    uint32_t emptyClusterNumber = -1;
    uint32_t searchEmptyClusterResult = searchEmptyCluster(diskInfo, bootSector, emptyClusterNumber);

    if(searchEmptyClusterResult == CLUSTER_SEARCH_FAILED)
    {
        if(actualDirectoryEntry != nullptr)
            delete actualDirectoryEntry;
        return DIR_CREATION_FAILED;
    }
    else if(searchEmptyClusterResult == CLUSTER_SEARCH_NO_FREE_CLUSTERS)
    {
        if(actualDirectoryEntry != nullptr)
            delete actualDirectoryEntry;
        return DIR_CREATION_NO_CLUSTER_AVAILABLE;
    }

    uint32_t updateFatResult = updateFat(diskInfo, bootSector, emptyClusterNumber, "\xFF\xFF\xFF\x0F"); //updates fat value for the first cluster in the new directory

    if(updateFatResult == FAT_UPDATE_FAILED)
    {
        if(actualDirectoryEntry != nullptr)
            delete actualDirectoryEntry;
        return DIR_CREATION_FAILED;
    }

    DirectoryEntry* newDirectoryEntry = new DirectoryEntry(); //the directory entry for the newly created directory
    addDirectoryEntryToParent(diskInfo, bootSector, actualDirectoryEntry, newDirectoryName, emptyClusterNumber, newDirectoryEntry, newDirectoryAttribute);
    setupFirstClusterInDirectory(diskInfo, bootSector, actualDirectoryEntry ,emptyClusterNumber, newDirectoryEntry);

    if(actualDirectoryEntry != nullptr)
        delete actualDirectoryEntry;
    return DIR_CREATION_SUCCESS;
}

uint32_t getSubDirectories(DiskInfo* diskInfo, BootSector* bootSector, char* directoryPath, std::vector<DirectoryEntry*>& subDirectories)
{
    DirectoryEntry* actualDirectoryEntry = nullptr;
    uint32_t findDirectoryEntryResult = findDirectoryEntryByFullPath(diskInfo, bootSector, directoryPath,
                                                                     &actualDirectoryEntry);
    if(findDirectoryEntryResult != FIND_DIRECTORY_ENTRY_BY_PATH_SUCCESS)
        return GET_SUB_DIRECTORIES_FAILED;

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
            if(actualDirectoryEntry != nullptr)
                delete actualDirectoryEntry;
            delete[] clusterData;
            return GET_SUB_DIRECTORIES_SUCCESS;
        }

        uint32_t nextCluster = 0;
        uint32_t getNextClusterResult = getNextCluster(diskInfo, bootSector, actualCluster, nextCluster);

        if(getNextClusterResult == FAT_VALUE_RETRIEVE_FAILED)
        {
            if(actualDirectoryEntry != nullptr)
                delete actualDirectoryEntry;
            delete[] clusterData;
            return GET_SUB_DIRECTORIES_FAILED;
        }

        if(getNextClusterResult == FAT_VALUE_EOC)
        {
            if(actualDirectoryEntry != nullptr)
                delete actualDirectoryEntry;
            delete[] clusterData;
            return GET_SUB_DIRECTORIES_SUCCESS;
        }

        actualCluster = nextCluster;
        numberOfClusterInParentDirectory++;
        offsetInCluster = 0; //64 is only for the first cluster, for the rest of them is 0
    }
}

uint32_t write(DiskInfo* diskInfo, BootSector* bootSector, char* directoryPath, char* dataBuffer, uint32_t maxBytesToWrite, uint32_t& numberOfBytesWritten, uint32_t writeAttribute,
               uint32_t& reasonForIncompleteWrite)
{
    if(strcmp(directoryPath, "Root\0") == 0) //you can't write directly to root
        return WRITE_BYTES_TO_FILE_CAN_NOT_WRITE_GIVEN_FILE;

    DirectoryEntry* actualDirectoryEntry = nullptr;
    uint32_t findDirectoryEntryResult = findDirectoryEntryByFullPath(diskInfo, bootSector, directoryPath,
                                                                     &actualDirectoryEntry);
    if(findDirectoryEntryResult != FIND_DIRECTORY_ENTRY_BY_PATH_SUCCESS)
        return WRITE_BYTES_TO_FILE_FAILED;

    if(actualDirectoryEntry->Attributes == ATTR_READ_ONLY || actualDirectoryEntry->Attributes == ATTR_DIRECTORY)
    {
        delete actualDirectoryEntry;
        return WRITE_BYTES_TO_FILE_CAN_NOT_WRITE_GIVEN_FILE;
    }

    uint32_t writeResult;

    if(writeAttribute == WRITE_WITH_TRUNCATE)
        writeResult = writeBytesToFileWithTruncate(diskInfo, bootSector, actualDirectoryEntry, dataBuffer, maxBytesToWrite, numberOfBytesWritten,
                                                   reasonForIncompleteWrite);
    else
        writeResult = writeBytesToFileWithAppend(diskInfo, bootSector, actualDirectoryEntry, dataBuffer, maxBytesToWrite, numberOfBytesWritten,
                                                   reasonForIncompleteWrite);

    if(writeResult == WRITE_BYTES_TO_FILE_SUCCESS)
    {
        if(writeAttribute == WRITE_WITH_TRUNCATE)
            actualDirectoryEntry->FileSize = 64 + numberOfBytesWritten; //64 for dot & dotdot entries
        else
            actualDirectoryEntry->FileSize = actualDirectoryEntry->FileSize + numberOfBytesWritten;

        DirectoryEntry* newDirectoryEntry = new DirectoryEntry();
        memcpy(newDirectoryEntry, actualDirectoryEntry, 32);
        updateDirectoryEntry(diskInfo, bootSector, actualDirectoryEntry, newDirectoryEntry); //CAUTION we don't query this, so if it fails, we have corrupted data

        delete actualDirectoryEntry, delete newDirectoryEntry;
        return WRITE_BYTES_TO_FILE_SUCCESS;
    }

    delete actualDirectoryEntry;
    return WRITE_BYTES_TO_FILE_FAILED;
}