#include "windows.h"
#include "string"
#include "string.h"
#include "iostream"

#include "../include/disk.h"
#include "../include/diskUtils.h"
#include "../include/diskCodes.h"
#include "../include/fat32Init.h"
#include "../include/Utils.h"
#include "../include/fat32FunctionUtils.h"
#include "../include/codes/fat32ApiResponseCodes.h"
#include "../include/codes/fat32Codes.h"
#include "../include/fat32Attributes.h"
#include "../include/fat32.h"


uint32_t findDirectoryEntryByDirectoryName(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* parentDirectoryEntry, char* searchedDirectoryName,
                                                  DirectoryEntry** searchedDirectoryEntry)
{
    uint32_t numOfSectorsRead = 0;
    uint32_t givenDirectoryEntryOffsetInParentCluster = 0;

    if(parentDirectoryEntry == nullptr)    //it means that searchedDir is Root
    {
        char* rootFirstSectorBuffer = new char[bootSector->BytesPerSector];
        int readResult = readDiskSectors(diskInfo, 1, getFirstSectorForCluster(bootSector, bootSector->RootDirCluster),
                                         rootFirstSectorBuffer, numOfSectorsRead);

        if(readResult != EC_NO_ERROR)
        {
            delete[] rootFirstSectorBuffer;
            return DIR_ENTRY_SEARCH_ERROR;
        }

        memcpy(*searchedDirectoryEntry, rootFirstSectorBuffer, 32);

        delete[] rootFirstSectorBuffer;
        return DIR_ENTRY_FOUND;
    }

    char* clusterData = new char[bootSector->SectorsPerCluster * bootSector->BytesPerSector];
    uint32_t actualCluster = ((uint32_t) parentDirectoryEntry->FirstClusterHigh << 16) | (uint32_t) parentDirectoryEntry->FirstClusterLow;
    uint32_t readResult = readDiskSectors(diskInfo, bootSector->SectorsPerCluster, getFirstSectorForCluster(bootSector, actualCluster),
                                     clusterData, numOfSectorsRead);
    uint32_t numberOfClusterInParentDirectory = 0; //the cluster number in chain
    uint32_t occupiedBytesInCluster = 0; //if the cluster is full, then it is cluster size, otherwise smaller

    while(readResult == EC_NO_ERROR)
    {
        if(parentDirectoryEntry->FileSize / getClusterSize(bootSector) <= numberOfClusterInParentDirectory)
            occupiedBytesInCluster = getClusterSize(bootSector);
        else
            occupiedBytesInCluster = parentDirectoryEntry->FileSize % getClusterSize(bootSector);

        uint32_t searchDirectoryEntryInClusterResult = findDirectoryEntryInGivenClusterData(bootSector, clusterData,searchedDirectoryName,
                                                                                            searchedDirectoryEntry, occupiedBytesInCluster, givenDirectoryEntryOffsetInParentCluster);

        switch (searchDirectoryEntryInClusterResult) {
            case DIR_ENTRY_NO_MORE_ENTRIES:
                delete[] clusterData;
                return DIR_ENTRY_DO_NOT_EXIST;
            case DIR_ENTRY_NOT_FOUND_IN_CLUSTER: {
                uint32_t nextCluster = 0;
                uint32_t getNextClusterResult = getNextCluster(diskInfo, bootSector, actualCluster, nextCluster);

                if(getNextClusterResult == FAT_VALUE_RETRIEVE_FAILED || getNextClusterResult == FAT_VALUE_EOC)
                {
                    delete[] clusterData;
                    return DIR_ENTRY_SEARCH_ERROR;
                }

                actualCluster = nextCluster;
                numOfSectorsRead = 0;
                readResult = readDiskSectors(diskInfo, bootSector->SectorsPerCluster, getFirstSectorForCluster(bootSector, actualCluster),
                                             clusterData,numOfSectorsRead);
                break;
            }
            default:
                delete[] clusterData;
                return DIR_ENTRY_FOUND;
        }
    }

    delete[] clusterData;
    return DIR_ENTRY_SEARCH_ERROR;
}

uint32_t findDirectoryEntryInGivenClusterData(BootSector* bootSector, char* clusterData, char* directoryName, DirectoryEntry** directoryEntry, uint32_t occupiedBytesInCluster,
                                              uint32_t& offset)
{
    offset = 0; //in case of first cluster of a directory, indexing from 0 will also check dot & dotdot, but this won't affect the result

    while(offset < occupiedBytesInCluster)
    {
        *directoryEntry = (DirectoryEntry*)&clusterData[offset];

        if(compareDirectoryNames(directoryName, (char*) (*directoryEntry)->FileName))
        {
            memcpy(*directoryEntry, &clusterData[offset], 32); //otherwise, if we just cast, when cluster data gets changed, directoryEntry data also gets
            return DIR_ENTRY_FOUND;
        }

        offset += 32;
    }

    if(offset >= getClusterSize(bootSector))
        return DIR_ENTRY_NOT_FOUND_IN_CLUSTER;
    else
        return DIR_ENTRY_NO_MORE_ENTRIES;
}

uint32_t searchEmptyCluster(DiskInfo* diskInfo, BootSector* bootSector, uint32_t& emptyClusterNumber)
{
    char* fatTable = new char[bootSector->BytesPerSector];
    uint32_t numOfSectorsRead = 0;
    uint32_t numberOfClusterEntriesPerFatSector = bootSector->BytesPerSector / 4;

    for(uint32_t sectorInFat = 0; sectorInFat < bootSector->SectorsPerFat; sectorInFat++)
    {
        int readResult = readDiskSectors(diskInfo, 1, getFirstFatSector(bootSector) + sectorInFat, fatTable, numOfSectorsRead);
        if(readResult != EC_NO_ERROR)
        {
            delete[] fatTable;
            return CLUSTER_SEARCH_FAILED;
        }

        for(uint32_t offset = 0; offset < bootSector->BytesPerSector; offset += 4)
        {
            uint32_t actualCluster = sectorInFat * numberOfClusterEntriesPerFatSector + offset / 4;
            if(actualCluster != 0 && actualCluster - 1 > getTotalNumberOfDataClusters(diskInfo, bootSector)) //check != 0 to avoid underflow (because we use uint and not int)
            {
                delete[] fatTable;
                return CLUSTER_SEARCH_NO_FREE_CLUSTERS;
            }

            uint32_t fatValue = *(uint32_t*)&fatTable[offset];
            if(fatValue == FAT_VALUE_FREE)
            {
                emptyClusterNumber = actualCluster;
                delete[] fatTable;
                return CLUSTER_SEARCH_SUCCESS;
            }
        }
    }

    delete[] fatTable;
    return CLUSTER_SEARCH_FAILED;
}

uint32_t updateFat(DiskInfo* diskInfo, BootSector* bootSector, uint32_t clusterNumber, char* value)
{
    char* fatTable = new char[bootSector->BytesPerSector];
    uint32_t clusterEntryPosition = clusterNumber * 4;
    uint32_t sector = getFirstFatSector(bootSector) + (clusterEntryPosition / bootSector->BytesPerSector);
    uint32_t offsetInsideSector = clusterEntryPosition % bootSector->BytesPerSector;

    uint32_t numOfSectorsRead = 0;
    int readResult = readDiskSectors(diskInfo, 1, sector, fatTable, numOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        delete[] fatTable;
        return FAT_UPDATE_FAILED;
    }

    memcpy(fatTable + offsetInsideSector, value, 4);
    uint32_t numOfSectorsWritten = 0;
    uint32_t writeResult = writeDiskSectors(diskInfo, 1, sector, fatTable, numOfSectorsWritten);

    if(writeResult != EC_NO_ERROR)
    {
        delete[] fatTable;
        return FAT_UPDATE_FAILED;
    }

    delete[] fatTable;
    return FAT_UPDATE_SUCCESS;
}

uint32_t addDirectoryEntryToParent(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* parentDirectoryEntry, char* newDirectoryName, //TODO increase parent file size when add child
                                                   uint32_t firstEmptyCluster, DirectoryEntry* newDirectoryEntry)
{
    uint32_t directoryFirstCluster = ((uint32_t) parentDirectoryEntry->FirstClusterHigh << 16) | (uint32_t) parentDirectoryEntry->FirstClusterLow;
    uint32_t firstClusterInChainWithFreeSpace = parentDirectoryEntry->FileSize / getClusterSize(bootSector); //number of cluster IN CHAIN: so 0,1,2,3
    uint32_t cluster = 0; //the cluster number of the first cluster with free space in the directory
    uint32_t findClusterResult = findNthClusterInChain(diskInfo, bootSector, directoryFirstCluster, firstClusterInChainWithFreeSpace, cluster);

    if(findClusterResult == CLUSTER_SEARCH_IN_CHAN_FAILED)
        return DIR_ENTRY_ADD_FAILED;
    else if(findClusterResult == CLUSTER_SEARCH_IN_CHAN_EOC)
    {
        uint32_t addNewClusterToDirectoryResult = addNewClusterToDirectory(diskInfo, bootSector, cluster, cluster); //we pass its value, but also want to update its value
        if(addNewClusterToDirectoryResult == DIR_ADD_NEW_CLUSTER_FAILED)
            return DIR_ENTRY_ADD_FAILED;
    }

    uint32_t firstSectorWithFreeSpaceInCluster = (parentDirectoryEntry->FileSize % getClusterSize(bootSector)) / bootSector->BytesPerSector;
    uint32_t sector = getFirstSectorForCluster(bootSector, cluster) + firstSectorWithFreeSpaceInCluster;
    uint32_t offsetInSector = parentDirectoryEntry->FileSize % bootSector->BytesPerSector; //first free byte

    char* sectorData = new char[bootSector->BytesPerSector];
    uint32_t numOfSectorsRead = 0;
    int readResult = readDiskSectors(diskInfo, 1, sector, sectorData, numOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        delete[] sectorData;
        return DIR_ENTRY_ADD_FAILED;
    }

    createDirectoryEntry(newDirectoryName, ATTR_DIRECTORY, firstEmptyCluster, newDirectoryEntry);
    memcpy(sectorData + offsetInSector, newDirectoryEntry, 32);
    uint32_t numOfSectorsWritten = 0;
    uint32_t writeResult =  writeDiskSectors(diskInfo, 1, sector, sectorData, numOfSectorsWritten);

    delete[] sectorData;
    return (writeResult == EC_NO_ERROR) ? DIR_ENTRY_ADD_SUCCESS : DIR_ENTRY_ADD_FAILED;
}

uint32_t addNewClusterToDirectory(DiskInfo* diskInfo, BootSector* bootSector, uint32_t lastClusterInDirectory, uint32_t& newCluster)
{
    int searchEmptyClusterResult = searchEmptyCluster(diskInfo, bootSector, newCluster);

    if(searchEmptyClusterResult == CLUSTER_SEARCH_FAILED)
        return DIR_ADD_NEW_CLUSTER_FAILED;

    uint32_t updateFatResult = updateFat(diskInfo, bootSector, newCluster, "\x0F\xFF\xFF\xFF"); //set EOC for the new cluster
    if(updateFatResult == FAT_UPDATE_FAILED)
        return DIR_ADD_NEW_CLUSTER_FAILED;

    updateFatResult = updateFat(diskInfo, bootSector, lastClusterInDirectory, (char*) &newCluster); //set EOC for the new cluster

    return (updateFatResult == FAT_UPDATE_SUCCESS) ? DIR_ADD_NEW_CLUSTER_SUCCESS : DIR_ADD_NEW_CLUSTER_FAILED;
}

uint32_t setupFirstClusterInDirectory(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* parentDirectoryEntry, uint32_t clusterNumber, DirectoryEntry* newDirectoryEntry)
{
    DirectoryEntry* dotDirectoryEntry = new DirectoryEntry();
    DirectoryEntry* dotDotDirectoryEntry = new DirectoryEntry();
    copyNewDirectoryTimeToDotDirectoryEntries(newDirectoryEntry, dotDirectoryEntry, dotDotDirectoryEntry);

    dotDirectoryEntry->FirstClusterHigh = clusterNumber >> 16;
    dotDirectoryEntry->FirstClusterLow = clusterNumber;

//    if(memcmp(parentDirectoryEntry->FileName, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 11) != 0)  //if it is 0 it means the parent is the root directory
//    {
//        memcpy(dotDotDirectoryEntry, parentDirectoryEntry, 32);
//    }
//    else //case for root directory, for which the first cluster is 2
//    {
//        dotDotDirectoryEntry->FirstClusterHigh = bootSector->RootDirCluster >> 16;
//        dotDotDirectoryEntry->FirstClusterLow = bootSector->RootDirCluster;
//    }

    memcpy(dotDotDirectoryEntry, parentDirectoryEntry, 32);

    char* writeBuffer = new char[bootSector->BytesPerSector];
    memset(writeBuffer, '\0', bootSector->BytesPerSector);
    memcpy(writeBuffer, dotDirectoryEntry, 32);
    memcpy(writeBuffer + 32, dotDotDirectoryEntry, 32);
    uint32_t numOfSectorsWritten = 0;
    int writeResult = writeDiskSectors(diskInfo, 1, getFirstSectorForCluster(bootSector, clusterNumber), writeBuffer, numOfSectorsWritten);

    delete[] writeBuffer;
    return (writeResult == EC_NO_ERROR) ? DIR_SETUP_FIRST_CLUSTER_SUCCESS : DIR_SETUP_FIRST_CLUSTER_FAILED;
}

uint32_t updateDirectoryEntry(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* givenDirectoryEntry, DirectoryEntry* newDirectoryEntry)
{
    uint32_t numOfSectorsRead = 0;
    uint32_t numOfSectorsWritten = 0;
    uint32_t givenDirectoryEntryOffsetInParentCluster = 0; //the offset in its cluster, not its offset relative to all its parent's clusters

    uint32_t givenDirectoryFirstCluster = getFirstClusterForDirectory(bootSector, givenDirectoryEntry);
    char* sectorData = new char[bootSector->BytesPerSector];
    uint32_t readResult = readDiskSectors(diskInfo, 1, getFirstSectorForCluster(bootSector, givenDirectoryFirstCluster),
                                          sectorData, numOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        delete[] sectorData;
        return DIRECTORY_ENTRY_UPDATE_FAILED;
    }

    if(givenDirectoryFirstCluster == bootSector->RootDirCluster) //it means that the given directory entry is root
    {
        memcpy(sectorData + 32, newDirectoryEntry, 32);
        uint32_t writeResult =  writeDiskSectors(diskInfo, 1, getFirstSectorForCluster(bootSector, givenDirectoryFirstCluster),
                                                 sectorData, numOfSectorsWritten);

        delete[] sectorData;

        if(writeResult != EC_NO_ERROR)
            return DIRECTORY_ENTRY_UPDATE_FAILED;

        uint32_t updateSubDirectoriesDotDotResult = updateSubDirectoriesDotDotEntries(diskInfo, bootSector, givenDirectoryEntry, newDirectoryEntry);
        return (updateSubDirectoriesDotDotResult == DIRECTORY_UPDATE_SUBDIRECTORIES_DOT_DOT_SUCCESS) ? DIRECTORY_ENTRY_UPDATE_SUCCESS : DIRECTORY_ENTRY_UPDATE_FAILED;
    }

    DirectoryEntry* givenDirectoryDotDotEntry = (DirectoryEntry*)&sectorData[32]; //aka parent directory entry (but only with file size & first cluster)
    char* clusterData = new char[bootSector->SectorsPerCluster * bootSector->BytesPerSector];
    uint32_t actualCluster = getFirstClusterForDirectory(bootSector, givenDirectoryDotDotEntry); //the first cluster of the given directory's parent
    readResult = readDiskSectors(diskInfo, bootSector->SectorsPerCluster, getFirstSectorForCluster(bootSector, actualCluster),
                                          clusterData, numOfSectorsRead);
    uint32_t numberOfClusterInParentDirectory = 0; //the cluster number in chain
    uint32_t occupiedBytesInCluster = 0; //if the cluster is full, then it is cluster size, otherwise smaller
    DirectoryEntry* mock = new DirectoryEntry();

    while(readResult == EC_NO_ERROR)
    {
        if(givenDirectoryEntry->FileSize / getClusterSize(bootSector) <= numberOfClusterInParentDirectory)
            occupiedBytesInCluster = getClusterSize(bootSector);
        else
            occupiedBytesInCluster = givenDirectoryEntry->FileSize % getClusterSize(bootSector);

        uint32_t searchDirectoryEntryInClusterResult = findDirectoryEntryInGivenClusterData(bootSector, clusterData, (char*) givenDirectoryEntry->FileName,
                                                                                            &mock, occupiedBytesInCluster, givenDirectoryEntryOffsetInParentCluster);

        if(searchDirectoryEntryInClusterResult == DIR_ENTRY_FOUND)
        {
            memcpy(clusterData + givenDirectoryEntryOffsetInParentCluster, newDirectoryEntry, 32);
            uint32_t writeResult =  writeDiskSectors(diskInfo, 1, getFirstSectorForCluster(bootSector, actualCluster),
                                                clusterData, numOfSectorsWritten);

            delete[] sectorData, delete[] clusterData;

            if(writeResult != EC_NO_ERROR)
                return DIRECTORY_ENTRY_UPDATE_FAILED;

            uint32_t updateSubDirectoriesDotDotResult = updateSubDirectoriesDotDotEntries(diskInfo, bootSector, givenDirectoryEntry, newDirectoryEntry);
            return (updateSubDirectoriesDotDotResult == DIRECTORY_UPDATE_SUBDIRECTORIES_DOT_DOT_SUCCESS) ? DIRECTORY_ENTRY_UPDATE_SUCCESS : DIRECTORY_ENTRY_UPDATE_FAILED;
        }

        //ELSE: we know for sure that the given directory entry exists in its parent, so the only other possible result is DIR_ENTRY_NOT_FOUND_IN_CLUSTER

        uint32_t nextCluster = 0;
        uint32_t getNextClusterResult = getNextCluster(diskInfo, bootSector, actualCluster, nextCluster);

        //also we can't have EOC, since this would mean we already reached end of parent without finding the given directory entry which is impossible
        if(getNextClusterResult == FAT_VALUE_RETRIEVE_FAILED)
        {
            delete[] sectorData, delete[] clusterData;
            return DIRECTORY_ENTRY_UPDATE_FAILED;
        }

        actualCluster = nextCluster;
        numOfSectorsRead = 0;
        readResult = readDiskSectors(diskInfo, bootSector->SectorsPerCluster, getFirstSectorForCluster(bootSector, actualCluster),
                                     clusterData,numOfSectorsRead);
    }
}
