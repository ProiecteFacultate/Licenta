#include "windows.h"
#include "string"
#include "string.h"
#include "iostream"
#include "vector"

#include "../include/disk.h"
#include "../include/diskUtils.h"
#include "../include/diskCodes.h"
#include "../include/fat32Init.h"
#include "../include/utils.h"
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
        return DIR_CREATION_PARENT_NOT_A_FOLDER;
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

uint32_t getSubDirectories(DiskInfo* diskInfo, BootSector* bootSector, char* directoryPath, std::vector<DirectoryEntry*>& subDirectories) //TODO REFACTOR
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

    if(actualDirectoryEntry->Attributes != ATTR_FILE)
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

uint32_t read(DiskInfo* diskInfo, BootSector* bootSector, char* directoryPath, char* readBuffer, uint32_t startingPosition, uint32_t maxBytesToRead, uint32_t& numberOfBytesRead,
              uint32_t& reasonForIncompleteRead)
{
    uint32_t readResult;
    uint32_t numOfBytesReadFromThisCluster;
    uint32_t numOfSectorsRead;
    uint32_t actualCluster;
    uint32_t nextCluster;

    if(strcmp(directoryPath, "Root\0") == 0) //you can't read directly to root
        return READ_BYTES_FROM_FILE_CAN_NOT_READ_GIVEN_FILE;

    DirectoryEntry* actualDirectoryEntry = nullptr;
    uint32_t findDirectoryEntryResult = findDirectoryEntryByFullPath(diskInfo, bootSector, directoryPath,
                                                                     &actualDirectoryEntry);
    if(findDirectoryEntryResult != FIND_DIRECTORY_ENTRY_BY_PATH_SUCCESS)
        return READ_BYTES_FROM_FILE_GIVEN_FILE_DO_NOT_EXIST;

    if(actualDirectoryEntry->Attributes != ATTR_FILE)
    {
        delete actualDirectoryEntry;
        return READ_BYTES_FROM_FILE_CAN_NOT_READ_GIVEN_FILE;
    }

    startingPosition += 64; //if the starting position is for example 10, we will start to read from 74, because first 64 bytes are dot & dotdot
    if(startingPosition > actualDirectoryEntry->FileSize)
    {
        delete actualDirectoryEntry;
        return READ_BYTES_FROM_FILE_GIVEN_START_EXCEEDS_FILE_SIZE;
    }

    uint32_t startingPositionClusterInChain = startingPosition / getClusterSize(bootSector); //0, 1, 2
    uint32_t givenDirectoryFirstCluster = getFirstClusterForDirectory(bootSector, actualDirectoryEntry);
    uint32_t startingPositionOffsetInCluster = startingPosition % getClusterSize(bootSector);
    char* clusterData = new char[getClusterSize(bootSector)]; //CAUTION we use a second buffer instead of reading directly in readBuffer because if we would do this, it could...
    //CAUTION overflow the read buffer (since we are reading whole sectors) and this could overwrite other data in heap

    uint32_t findClusterResult = findNthClusterInChain(diskInfo, bootSector, givenDirectoryFirstCluster, startingPositionClusterInChain, actualCluster);
    if(findClusterResult != CLUSTER_SEARCH_IN_CHAN_SUCCESS)
    {
        delete[] clusterData, delete actualDirectoryEntry;
        return READ_BYTES_FROM_FILE_FAILED;
    }

    readResult = readDiskSectors(diskInfo, bootSector->SectorsPerCluster, getFirstSectorForCluster(bootSector, actualCluster),
                                 clusterData, numOfSectorsRead);
    if(readResult != EC_NO_ERROR)
    {
        delete[] clusterData, delete actualDirectoryEntry;
        return READ_BYTES_FROM_FILE_FAILED;
    }

    numberOfBytesRead = std::min(actualDirectoryEntry->FileSize - startingPosition, std::min(getClusterSize(bootSector) - startingPositionOffsetInCluster, maxBytesToRead));
    memcpy(readBuffer, clusterData + startingPositionOffsetInCluster, numberOfBytesRead);

    while(numberOfBytesRead < maxBytesToRead)
    {
        uint32_t getNextClusterResult = getNextCluster(diskInfo, bootSector, actualCluster, nextCluster);

        if(getNextClusterResult == FAT_VALUE_RETRIEVE_FAILED || getNextClusterResult == FAT_VALUE_EOC)
        {
            delete[] clusterData, delete actualDirectoryEntry;
            reasonForIncompleteRead = (getNextClusterResult == FAT_VALUE_EOC) ? INCOMPLETE_BYTES_READ_DUE_TO_NO_FILE_NOT_LONG_ENOUGH : INCOMPLETE_BYTES_READ_DUE_TO_OTHER;
            return READ_BYTES_FROM_FILE_SUCCESS; //we managed to read bytes for first cluster, so it's still considered a read success
        }

        actualCluster = nextCluster;
        readResult = readDiskSectors(diskInfo, bootSector->SectorsPerCluster, getFirstSectorForCluster(bootSector, actualCluster),
                                              clusterData, numOfSectorsRead);

        if(readResult != EC_NO_ERROR)
        {
            delete[] clusterData, delete actualDirectoryEntry;
            return READ_BYTES_FROM_FILE_FAILED;
        }

        numOfBytesReadFromThisCluster = std::min(actualDirectoryEntry->FileSize - startingPosition - numberOfBytesRead, std::min(getClusterSize(bootSector), maxBytesToRead - numberOfBytesRead));
        memcpy(readBuffer + numberOfBytesRead, clusterData, numOfBytesReadFromThisCluster);
        numberOfBytesRead += numOfBytesReadFromThisCluster;
    }

    delete[] clusterData, delete actualDirectoryEntry;
    return READ_BYTES_FROM_FILE_SUCCESS;
}

uint32_t truncateFile(DiskInfo* diskInfo, BootSector* bootSector, char* directoryPath, uint32_t newSize)
{
    if(strcmp(directoryPath, "Root\0") == 0) //you can't truncate the root
        return TRUNCATE_FILE_CAN_NOT_TRUNCATE_GIVEN_FILE;

    DirectoryEntry* actualDirectoryEntry = nullptr;
    uint32_t findDirectoryEntryResult = findDirectoryEntryByFullPath(diskInfo, bootSector, directoryPath,
                                                                     &actualDirectoryEntry);
    if(findDirectoryEntryResult != FIND_DIRECTORY_ENTRY_BY_PATH_SUCCESS)
        return TRUNCATE_FILE_FAILED;

    if(actualDirectoryEntry->Attributes != ATTR_FILE)
    {
        delete actualDirectoryEntry;
        return TRUNCATE_FILE_CAN_NOT_TRUNCATE_GIVEN_FILE;
    }

    newSize += 64; //the given value refers only to the size of the file content, so it is not taking in account 64 for dot & dotdot
    if(newSize > actualDirectoryEntry->FileSize)
        return TRUNCATE_FILE_NEW_SIZE_GREATER_THAN_ACTUAL_SIZE;

    DirectoryEntry* newDirectoryEntry = new DirectoryEntry();
    memcpy(newDirectoryEntry, actualDirectoryEntry, 32);
    newDirectoryEntry->FileSize = newSize;
    uint32_t directoryEntryUpdateResult = updateDirectoryEntry(diskInfo, bootSector, actualDirectoryEntry, newDirectoryEntry);

    return (directoryEntryUpdateResult == DIRECTORY_ENTRY_UPDATE_SUCCESS) ? TRUNCATE_FILE_SUCCESS : TRUNCATE_FILE_FAILED;
}

uint32_t deleteDirectoryByPath(DiskInfo* diskInfo, BootSector* bootSector, char* directoryPath)
{
    if(strcmp(directoryPath, "Root\0") == 0) //you can't delete the root
        return DELETE_DIRECTORY_CAN_NOT_DELETE_GIVEN_DIRECTORY;

    char* parentPath = new char[strlen(directoryPath)];
    extractParentPathFromPath(directoryPath, parentPath);

    DirectoryEntry* actualDirectoryEntry = nullptr;
    uint32_t findDirectoryEntryResult = findDirectoryEntryByFullPath(diskInfo, bootSector, directoryPath,
                                                                     &actualDirectoryEntry);
    if(findDirectoryEntryResult != FIND_DIRECTORY_ENTRY_BY_PATH_SUCCESS)
        return DELETE_DIRECTORY_FAILED;

    DirectoryEntry* parentDirectoryEntry = nullptr;
    findDirectoryEntryResult = findDirectoryEntryByFullPath(diskInfo, bootSector, directoryPath,
                                                                     &parentDirectoryEntry);
    if(findDirectoryEntryResult != FIND_DIRECTORY_ENTRY_BY_PATH_SUCCESS)
        return DELETE_DIRECTORY_FAILED;

    uint32_t deleteDirectoryEntryResult = deleteDirectoryEntry(diskInfo, bootSector, actualDirectoryEntry);
    if(deleteDirectoryEntryResult != DELETE_DIRECTORY_ENTRY_SUCCESS)
        return DELETE_DIRECTORY_FAILED;

    uint32_t deleteDirectoryEntryForParentResult = deleteDirectoryEntryFromParent(diskInfo, bootSector, actualDirectoryEntry, parentDirectoryEntry);
    return (deleteDirectoryEntryForParentResult == DELETE_DIRECTORY_ENTRY_SUCCESS) ? DELETE_DIRECTORY_SUCCESS : DELETE_DIRECTORY_ENTRY_FAILED;
}