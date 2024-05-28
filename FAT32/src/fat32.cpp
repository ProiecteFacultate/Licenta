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
#include "../include/fat32Structures.h"
#include "../include/fat32FunctionUtils.h"
#include "../include/codes/fat32ApiResponseCodes.h"
#include "../include/codes/fat32Codes.h"
#include "../include/codes/fat32Attributes.h"
#include "../include/fat32.h"


uint32_t findDirectoryEntryByDirectoryName(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* parentDirectoryEntry, char* searchedDirectoryName,
                                           DirectoryEntry* searchedDirectoryEntry)
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

        memcpy(searchedDirectoryEntry, rootFirstSectorBuffer, 32);

        delete[] rootFirstSectorBuffer;
        return DIR_ENTRY_FOUND;
    }

    char* clusterData = new char[getClusterSize(bootSector)];
    uint32_t actualCluster = getFirstClusterForDirectory(bootSector, parentDirectoryEntry);
    uint32_t readResult = readDiskSectors(diskInfo, bootSector->SectorsPerCluster, getFirstSectorForCluster(bootSector, actualCluster),
                                     clusterData, numOfSectorsRead);
    uint32_t numberOfClusterInParentDirectory = 0; //the cluster number in chain
    uint32_t occupiedBytesInCluster = 0; //if the cluster is full, then it is cluster size, otherwise smaller

    while(readResult == EC_NO_ERROR)
    {
        occupiedBytesInCluster = parentDirectoryEntry->FileSize >= getClusterSize(bootSector) * (numberOfClusterInParentDirectory + 1) ? getClusterSize(bootSector)
                                                                                                                  : parentDirectoryEntry->FileSize % getClusterSize(bootSector);

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
                numberOfClusterInParentDirectory++;
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

uint32_t findDirectoryEntryInGivenClusterData(BootSector* bootSector, char* clusterData, char* directoryName, DirectoryEntry* directoryEntry, uint32_t occupiedBytesInCluster,
                                              uint32_t& offset)
{
    offset = 0; //in case of first cluster of a directory, indexing from 0 will also check dot & dotdot, but this won't affect the result
    char* desiredFileName = new char[12];
    char* actualFileName = new char[12];

    while(offset < occupiedBytesInCluster)
    {
        memset(desiredFileName, '\0', 12);
        memcpy(desiredFileName, directoryName, 11);
        memset(actualFileName, '\0', 12);
        memcpy(actualFileName, ((DirectoryEntry*)&clusterData[offset])->FileName, 11);

        if(compareDirectoryNames(desiredFileName, actualFileName))
        {
            memcpy(directoryEntry, &clusterData[offset], 32); //otherwise, if we just cast, when cluster data gets changed, directoryEntry data also gets
            delete[] desiredFileName;
            return DIR_ENTRY_FOUND;
        }

        offset += 32;
    }

    delete[] desiredFileName;

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

    delete[] fatTable;
    return (writeResult == EC_NO_ERROR) ? FAT_UPDATE_SUCCESS : FAT_UPDATE_FAILED;
}

uint32_t updateFatForTwoNeighbours(DiskInfo* diskInfo, BootSector* bootSector, uint32_t clusterNumber_1, char* value_1, uint32_t clusterNumber_2, char* value_2)
{
    uint32_t clusterEntryPosition_1 = clusterNumber_1 * 4;
    uint32_t sector_1 = getFirstFatSector(bootSector) + (clusterEntryPosition_1 / bootSector->BytesPerSector);
    uint32_t clusterEntryPosition_2 = clusterNumber_2 * 4;
    uint32_t sector_2 = getFirstFatSector(bootSector) + (clusterEntryPosition_2 / bootSector->BytesPerSector);

    if(sector_1 == sector_2)
    {
        uint32_t numOfSectorsRead, numOfSectorsWritten;
        char* fatTable = new char[bootSector->BytesPerSector];
        int readResult = readDiskSectors(diskInfo, 1, sector_1, fatTable, numOfSectorsRead);

        if(readResult != EC_NO_ERROR)
        {
            delete[] fatTable;
            return FAT_UPDATE_FAILED;
        }

        uint32_t offsetInsideSector_1 = clusterEntryPosition_1 % bootSector->BytesPerSector;
        uint32_t offsetInsideSector_2 = clusterEntryPosition_2 % bootSector->BytesPerSector;

        memcpy(fatTable + offsetInsideSector_1, value_1, 4);
        memcpy(fatTable + offsetInsideSector_2, value_2, 4);

        uint32_t writeResult = writeDiskSectors(diskInfo, 1, sector_1, fatTable, numOfSectorsWritten);

        delete[] fatTable;
        return (writeResult == EC_NO_ERROR) ? FAT_UPDATE_SUCCESS : FAT_UPDATE_FAILED;
    }
    else
    {
        uint32_t updateResult_1 = updateFat(diskInfo, bootSector, clusterNumber_1, value_1);
        uint32_t updateResult_2 = updateFat(diskInfo, bootSector, clusterNumber_2, value_2);

        if(updateResult_1 == FAT_UPDATE_FAILED || updateResult_2 == FAT_UPDATE_FAILED)
            return FAT_UPDATE_FAILED;

        return FAT_UPDATE_SUCCESS;
    }
}

uint32_t addDirectoryEntryToParent(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* parentDirectoryEntry, char* newDirectoryName,
                                                   uint32_t firstEmptyCluster, DirectoryEntry* newDirectoryEntry, uint32_t newDirectoryAttribute)
{
    uint32_t firstClusterInChainWithFreeSpace = parentDirectoryEntry->FileSize / getClusterSize(bootSector); //number of cluster IN CHAIN: so 0,1,2,3
    if(parentDirectoryEntry->FileSize % getClusterSize(bootSector) == 0)
        firstClusterInChainWithFreeSpace++;

    uint32_t cluster = 0; //the cluster number of the first cluster with free space in the directory
    uint32_t findClusterResult = findNthClusterInChain(diskInfo, bootSector, getFirstClusterForDirectory(bootSector, parentDirectoryEntry),
                                                       firstClusterInChainWithFreeSpace, cluster);

    if(findClusterResult == CLUSTER_SEARCH_IN_CHAN_FAILED)
        return DIR_ENTRY_ADD_FAILED;
    else if(findClusterResult == CLUSTER_SEARCH_IN_CHAN_EOC)
    {
        uint32_t addNewClusterToDirectoryResult = addNewClusterToDirectory(diskInfo, bootSector, cluster, cluster); //we pass its value, but also want to update its value
        if(addNewClusterToDirectoryResult == DIR_ADD_NEW_CLUSTER_FAILED)
            return DIR_ENTRY_ADD_FAILED;
        else if(addNewClusterToDirectoryResult == DIR_ADD_NEW_CLUSTER_NO_CLUSTER_AVAILABLE)
            return DIR_ENTRY_ADD_NO_CLUSTER_AVAILABLE;
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

    createDirectoryEntry(newDirectoryName, newDirectoryAttribute, firstEmptyCluster, newDirectoryEntry);
    memcpy(sectorData + offsetInSector, newDirectoryEntry, 32);
    uint32_t numOfSectorsWritten = 0;
    uint32_t writeResult =  writeDiskSectors(diskInfo, 1, sector, sectorData, numOfSectorsWritten);

    delete[] sectorData;

    if(writeResult != EC_NO_ERROR)
        return DIR_ENTRY_ADD_FAILED;

    parentDirectoryEntry->FileSize += 32; //we also want to update the parent directory entry file size
    parentDirectoryEntry->LastAccessedDate = fat32_getCurrentDateFormatted();
    //we can use parentDirectoryEntry for both with no problem
    uint32_t updateParentDirectoryEntryResult = updateDirectoryEntry(diskInfo, bootSector, parentDirectoryEntry, parentDirectoryEntry);

    return (updateParentDirectoryEntryResult == DIRECTORY_ENTRY_UPDATE_SUCCESS) ? DIR_ENTRY_ADD_SUCCESS : DIR_ENTRY_ADD_FAILED;
}

uint32_t addNewClusterToDirectory(DiskInfo* diskInfo, BootSector* bootSector, uint32_t lastClusterInDirectory, uint32_t& newCluster)
{
    uint32_t searchEmptyClusterResult = searchEmptyCluster(diskInfo, bootSector, newCluster);

    if(searchEmptyClusterResult == CLUSTER_SEARCH_FAILED)
        return DIR_ADD_NEW_CLUSTER_FAILED;
    else if(searchEmptyClusterResult == CLUSTER_SEARCH_NO_FREE_CLUSTERS)
        return DIR_ADD_NEW_CLUSTER_NO_CLUSTER_AVAILABLE;

    //set EOC for the new cluster, and the number of the new cluster for the previous last cluster
    uint32_t updateFatResult = updateFatForTwoNeighbours(diskInfo, bootSector, newCluster, "\xFF\xFF\xFF\x0F",
                                                         lastClusterInDirectory, (char*) &newCluster);

    return (updateFatResult == FAT_UPDATE_SUCCESS) ? DIR_ADD_NEW_CLUSTER_SUCCESS : DIR_ADD_NEW_CLUSTER_FAILED;
}

uint32_t setupFirstClusterInDirectory(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* parentDirectoryEntry, uint32_t clusterNumber, DirectoryEntry* newDirectoryEntry)
{
    DirectoryEntry* dotDirectoryEntry = new DirectoryEntry();
    DirectoryEntry* dotDotDirectoryEntry = new DirectoryEntry();
    copyNewDirectoryTimeToDotDirectoryEntries(newDirectoryEntry, dotDirectoryEntry, dotDotDirectoryEntry);

    dotDirectoryEntry->FirstClusterHigh = clusterNumber >> 16;
    dotDirectoryEntry->FirstClusterLow = clusterNumber;

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
        memcpy(sectorData, newDirectoryEntry, 32);
        uint32_t writeResult = writeDiskSectors(diskInfo, 1, getFirstSectorForCluster(bootSector, givenDirectoryFirstCluster),
                                                 sectorData, numOfSectorsWritten);

        delete[] sectorData;

        if(writeResult != EC_NO_ERROR)
            return DIRECTORY_ENTRY_UPDATE_FAILED;

        uint32_t updateSubDirectoriesDotDotResult = updateSubDirectoriesDotDotEntries(diskInfo, bootSector, givenDirectoryEntry, newDirectoryEntry);
        return (updateSubDirectoriesDotDotResult == DIRECTORY_UPDATE_SUBDIRECTORIES_DOT_DOT_SUCCESS) ? DIRECTORY_ENTRY_UPDATE_SUCCESS : DIRECTORY_ENTRY_UPDATE_FAILED;
    }

    DirectoryEntry* givenDirectoryDotDotEntry = (DirectoryEntry*)&sectorData[32]; //aka parent directory entry (but only with file size & first cluster)
    char* clusterData = new char[getClusterSize(bootSector)];
    uint32_t actualCluster = getFirstClusterForDirectory(bootSector, givenDirectoryDotDotEntry); //the first cluster of the given directory's parent
    readResult = readDiskSectors(diskInfo, bootSector->SectorsPerCluster, getFirstSectorForCluster(bootSector, actualCluster),
                                          clusterData, numOfSectorsRead);
    uint32_t numberOfClusterInParentDirectory = 0; //the cluster number in chain
    uint32_t occupiedBytesInCluster = 0; //if the cluster is full, then it is cluster size, otherwise smaller

    while(readResult == EC_NO_ERROR)
    {
        occupiedBytesInCluster = givenDirectoryDotDotEntry->FileSize >= getClusterSize(bootSector) * (numberOfClusterInParentDirectory + 1) ? getClusterSize(bootSector)
                                                                                                                      : givenDirectoryDotDotEntry->FileSize % getClusterSize(bootSector);

        uint32_t searchDirectoryEntryInClusterResult = findDirectoryEntryInGivenClusterData(bootSector, clusterData, (char*) givenDirectoryEntry->FileName,
                                                                                            new DirectoryEntry(), occupiedBytesInCluster, givenDirectoryEntryOffsetInParentCluster);

        if(searchDirectoryEntryInClusterResult == DIR_ENTRY_FOUND)
        {
            memcpy(clusterData + givenDirectoryEntryOffsetInParentCluster, newDirectoryEntry, 32);
            uint32_t writeResult =  writeDiskSectors(diskInfo, bootSector->SectorsPerCluster, getFirstSectorForCluster(bootSector, actualCluster),
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
        numberOfClusterInParentDirectory++;
        numOfSectorsRead = 0;
        readResult = readDiskSectors(diskInfo, bootSector->SectorsPerCluster, getFirstSectorForCluster(bootSector, actualCluster),
                                     clusterData,numOfSectorsRead);
    }
}

uint32_t writeBytesToFileWithTruncate(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* directoryEntry, char* dataBuffer, uint32_t maxBytesToWrite,
                                      uint32_t& numberOfBytesWritten, uint32_t& reasonForIncompleteWrite)
{
    numberOfBytesWritten = 0;
    char* clusterData = new char[getClusterSize(bootSector)];
    uint32_t numOfSectorsRead = 0;
    uint32_t numOfSectorsWritten = 0;
    uint32_t numOfBytesToWriteToActualCluster = 0;
    uint32_t nextCluster = 0;

    uint32_t givenDirectoryFirstCluster = getFirstClusterForDirectory(bootSector, directoryEntry);
    uint32_t readResult = readDiskSectors(diskInfo, bootSector->SectorsPerCluster, getFirstSectorForCluster(bootSector, givenDirectoryFirstCluster),
                                          clusterData, numOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        delete[] clusterData;
        return WRITE_BYTES_TO_FILE_FAILED;
    }

    numOfBytesToWriteToActualCluster = std::min(getClusterSize(bootSector) - 64, maxBytesToWrite);
    memcpy(clusterData + 64, dataBuffer, numOfBytesToWriteToActualCluster); //start at index 64 because it is the first cluster & contains dot & dotdot
    uint32_t writeResult =  writeDiskSectors(diskInfo, bootSector->SectorsPerCluster, getFirstSectorForCluster(bootSector, givenDirectoryFirstCluster),
                                             clusterData, numOfSectorsWritten);

    if(writeResult != EC_NO_ERROR)
    {
        delete[] clusterData;
        return WRITE_BYTES_TO_FILE_FAILED;
    }

    numberOfBytesWritten = numOfBytesToWriteToActualCluster;
    uint32_t actualCluster = givenDirectoryFirstCluster;

    while(numberOfBytesWritten < maxBytesToWrite)
    {
        uint32_t getNextClusterResult = getNextCluster(diskInfo, bootSector, actualCluster, nextCluster);

        if(getNextClusterResult == FAT_VALUE_RETRIEVE_FAILED)
        {
            updateFat(diskInfo, bootSector, actualCluster, "\xFF\xFF\xFF\x0F"); //sets actual cluster as EOC
            //we can't free the next clusters in chain since fat retrieve failed; they will remain occupied without being part of a cluster (trash clusters)
            reasonForIncompleteWrite = INCOMPLETE_BYTES_WRITE_DUE_TO_OTHER;
            delete[] clusterData;
            return WRITE_BYTES_TO_FILE_SUCCESS; //we managed to write bytes for first cluster, so it's still considered a write success
        }
        else if(getNextClusterResult == FAT_VALUE_EOC)
        {
            uint32_t addNewClusterToDirectoryResult = addNewClusterToDirectory(diskInfo, bootSector, actualCluster, nextCluster);
            if(addNewClusterToDirectoryResult == DIR_ADD_NEW_CLUSTER_FAILED || addNewClusterToDirectoryResult == DIR_ADD_NEW_CLUSTER_NO_CLUSTER_AVAILABLE)
            {
                reasonForIncompleteWrite = (addNewClusterToDirectoryResult == DIR_ADD_NEW_CLUSTER_NO_CLUSTER_AVAILABLE) ? INCOMPLETE_BYTES_WRITE_DUE_TO_NO_CLUSTERS_AVAILABLE
                        : INCOMPLETE_BYTES_WRITE_DUE_TO_OTHER;
                delete[] clusterData;
                return WRITE_BYTES_TO_FILE_SUCCESS; //we managed to write bytes for first cluster, so it's still considered a write success
            }
        }

        actualCluster = nextCluster;
        numOfBytesToWriteToActualCluster = std::min(getClusterSize(bootSector), maxBytesToWrite - numberOfBytesWritten);
        memcpy(clusterData, dataBuffer + numberOfBytesWritten, numOfBytesToWriteToActualCluster);
        writeResult = writeDiskSectors(diskInfo, bootSector->SectorsPerCluster, getFirstSectorForCluster(bootSector, actualCluster),
                                                 clusterData, numOfSectorsWritten);

        if(writeResult != EC_NO_ERROR)
        {
            getNextClusterResult = getNextCluster(diskInfo, bootSector, actualCluster, nextCluster);
            updateFat(diskInfo, bootSector, actualCluster, "\xFF\xFF\xFF\x0F"); //sets actual cluster as EOC
            if(!(getNextClusterResult == FAT_VALUE_RETRIEVE_FAILED || getNextClusterResult == FAT_VALUE_EOC)) //if it fails to retrieve, then we will have trash clusters
                freeClustersInChainStartingWithGivenCluster(diskInfo, bootSector, nextCluster);

            reasonForIncompleteWrite = INCOMPLETE_BYTES_WRITE_DUE_TO_OTHER;
            delete[] clusterData;
            return WRITE_BYTES_TO_FILE_SUCCESS; //we managed to write bytes for first cluster, so it's still considered a write success
        }

        numberOfBytesWritten += numOfBytesToWriteToActualCluster;
    }

    uint32_t getNextClusterResult = getNextCluster(diskInfo, bootSector, actualCluster, nextCluster);
    updateFat(diskInfo, bootSector, actualCluster, "\xFF\xFF\xFF\x0F"); //sets actual cluster as EOC
    if(!(getNextClusterResult == FAT_VALUE_RETRIEVE_FAILED || getNextClusterResult == FAT_VALUE_EOC)) //if it fails to retrieve, then we will have trash clusters
        freeClustersInChainStartingWithGivenCluster(diskInfo, bootSector, nextCluster);

    delete[] clusterData;
    return WRITE_BYTES_TO_FILE_SUCCESS;
}

uint32_t writeBytesToFileWithAppend(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* directoryEntry, char* dataBuffer, uint32_t maxBytesToWrite,
                                    uint32_t& numberOfBytesWritten, uint32_t& reasonForIncompleteWrite)
{
    numberOfBytesWritten = 0;
    char* clusterData = new char[getClusterSize(bootSector)];
    uint32_t numOfSectorsRead = 0;
    uint32_t numOfSectorsWritten = 0;
    uint32_t numOfBytesToWriteToActualCluster;
    uint32_t occupiedBytesInCluster;
    uint32_t numberOfClusterInDirectory = 0; //the cluster number in chain
    uint32_t directoryCurrentSize = directoryEntry->FileSize; //we use this to calculate correctly occupiedBytesInCluster
    uint32_t nextCluster = 0;

    uint32_t givenDirectoryFirstCluster = getFirstClusterForDirectory(bootSector, directoryEntry);

    numberOfBytesWritten = 0;
    uint32_t actualCluster = givenDirectoryFirstCluster;

    while(true)
    {
        occupiedBytesInCluster = directoryCurrentSize >= getClusterSize(bootSector) * (numberOfClusterInDirectory + 1) ? getClusterSize(bootSector)
                                                                                                                           : directoryCurrentSize % getClusterSize(bootSector);
        numOfBytesToWriteToActualCluster = std::min(getClusterSize(bootSector) - occupiedBytesInCluster, maxBytesToWrite - numberOfBytesWritten);
        if(numOfBytesToWriteToActualCluster > 0)
        {
            uint32_t readResult = readDiskSectors(diskInfo, bootSector->SectorsPerCluster, getFirstSectorForCluster(bootSector, actualCluster),
                                                  clusterData, numOfSectorsRead);
            if(readResult != EC_NO_ERROR)
            {
                updateFat(diskInfo, bootSector, actualCluster, "\xFF\xFF\xFF\x0F"); //sets actual cluster as EOC
                delete[] clusterData;
                return WRITE_BYTES_TO_FILE_FAILED;
            }

            memcpy(clusterData + occupiedBytesInCluster, dataBuffer + numberOfBytesWritten, numOfBytesToWriteToActualCluster);
            uint32_t writeResult = writeDiskSectors(diskInfo, bootSector->SectorsPerCluster, getFirstSectorForCluster(bootSector, actualCluster),
                                                     clusterData, numOfSectorsWritten);

            if(writeResult != EC_NO_ERROR)
            {
                updateFat(diskInfo, bootSector, actualCluster, "\xFF\xFF\xFF\x0F"); //sets actual cluster as EOC
                delete[] clusterData;
                return WRITE_BYTES_TO_FILE_FAILED;
            }

            numberOfBytesWritten += numOfBytesToWriteToActualCluster;
        }

        if(numberOfBytesWritten == maxBytesToWrite)
        {
            updateFat(diskInfo, bootSector, actualCluster, "\xFF\xFF\xFF\x0F"); //sets actual cluster as EOC
            delete[] clusterData;
            return WRITE_BYTES_TO_FILE_SUCCESS;
        }

        uint32_t getNextClusterResult = getNextCluster(diskInfo, bootSector, actualCluster, nextCluster);

        if(getNextClusterResult == FAT_VALUE_RETRIEVE_FAILED)
        {
            updateFat(diskInfo, bootSector, actualCluster, "\xFF\xFF\xFF\x0F"); //sets actual cluster as EOC
            reasonForIncompleteWrite = INCOMPLETE_BYTES_WRITE_DUE_TO_OTHER;
            delete[] clusterData;
            return WRITE_BYTES_TO_FILE_SUCCESS; //we managed to write bytes for first cluster, so it's still considered a write success
        }
        else if(getNextClusterResult == FAT_VALUE_EOC)
        {
            uint32_t addNewClusterToDirectoryResult = addNewClusterToDirectory(diskInfo, bootSector, actualCluster, nextCluster);
            if(addNewClusterToDirectoryResult == DIR_ADD_NEW_CLUSTER_FAILED || addNewClusterToDirectoryResult == DIR_ADD_NEW_CLUSTER_NO_CLUSTER_AVAILABLE)
            {
                reasonForIncompleteWrite = (addNewClusterToDirectoryResult == DIR_ADD_NEW_CLUSTER_NO_CLUSTER_AVAILABLE) ? INCOMPLETE_BYTES_WRITE_DUE_TO_NO_CLUSTERS_AVAILABLE
                                                                                                                        : INCOMPLETE_BYTES_WRITE_DUE_TO_OTHER;
                delete[] clusterData;
                return WRITE_BYTES_TO_FILE_SUCCESS; //we managed to write bytes for first cluster, so it's still considered a write success
            }
        }

        actualCluster = nextCluster;
        numberOfClusterInDirectory++;
        directoryCurrentSize = directoryEntry->FileSize + numberOfBytesWritten;
    }
}

uint32_t getSubDirectoriesByParentDirectoryEntry(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* parentDirectoryEntry, std::vector<DirectoryEntry*>& subDirectories)
{
    char* clusterData = new char[getClusterSize(bootSector)]; //declare here for time efficiency

    uint32_t actualCluster = getFirstClusterForDirectory(bootSector, parentDirectoryEntry);  //directory first cluster
    uint32_t offsetInCluster = 64; //we start from 64 because in the first sector of the first cluster we got dot & dotdot
    uint32_t numberOfClusterInParentDirectory = 0; //the cluster number in chain
    uint32_t occupiedBytesInCluster = 0; //if the cluster is full, then it is cluster size, otherwise smaller

    while (true)
    {
        occupiedBytesInCluster = parentDirectoryEntry->FileSize >= getClusterSize(bootSector) * (numberOfClusterInParentDirectory + 1) ? getClusterSize(bootSector)
                                                                                                              : parentDirectoryEntry->FileSize % getClusterSize(bootSector);

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
            delete[] clusterData;
            return GET_SUB_DIRECTORIES_SUCCESS;
        }

        uint32_t nextCluster = 0;
        uint32_t getNextClusterResult = getNextCluster(diskInfo, bootSector, actualCluster, nextCluster);

        if(getNextClusterResult == FAT_VALUE_RETRIEVE_FAILED)
        {
            delete[] clusterData;
            return GET_SUB_DIRECTORIES_FAILED;
        }

        if(getNextClusterResult == FAT_VALUE_EOC)
        {
            delete[] clusterData;
            return GET_SUB_DIRECTORIES_SUCCESS;
        }

        actualCluster = nextCluster;
        numberOfClusterInParentDirectory++;
        offsetInCluster = 0; //64 is only for the first cluster, for the rest of them is 0
    }
}

uint32_t freeClustersOfDirectoryAndChildren(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* directoryEntry)
{
    std::vector<DirectoryEntry*> subDirectories;

    if(directoryEntry->Attributes == DIRECTORY_TYPE_FOLDER)
    {
        uint32_t getSubdirectoriesResult = getSubDirectoriesByParentDirectoryEntry(diskInfo, bootSector, directoryEntry, subDirectories);
        if(getSubdirectoriesResult != GET_SUB_DIRECTORIES_SUCCESS)
            return FREE_CLUSTERS_OF_DIRECTORY_AND_CHILDREN_FAILED;

        for(DirectoryEntry* childDirectoryEntry : subDirectories)
        {
            uint32_t deleteChildDirectoryEntryResult = freeClustersOfDirectoryAndChildren(diskInfo, bootSector,
                                                                                          childDirectoryEntry);
            if(deleteChildDirectoryEntryResult == FREE_CLUSTERS_OF_DIRECTORY_AND_CHILDREN_FAILED)
            {
                for(DirectoryEntry* entry : subDirectories)
                    delete entry;
            }
        }

        for(DirectoryEntry* childDirectoryEntry : subDirectories)
            delete childDirectoryEntry;
    }

    uint32_t firstClusterInDirectory = getFirstClusterForDirectory(bootSector, directoryEntry);
    uint32_t freeClustersInChainResult = freeClustersInChainStartingWithGivenCluster(diskInfo, bootSector, firstClusterInDirectory);
    return (freeClustersInChainResult == FREE_CLUSTERS_IN_CHAIN_SUCCESS) ? FREE_CLUSTERS_OF_DIRECTORY_AND_CHILDREN_SUCCESS : FREE_CLUSTERS_OF_DIRECTORY_AND_CHILDREN_FAILED;
}

uint32_t deleteDirectoryEntryFromParent(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* givenDirectoryEntry, DirectoryEntry* parentDirectoryEntry)
{
    char* clusterData = new char[getClusterSize(bootSector)];
    uint32_t numOfSectorsRead = 0;
    uint32_t numOfSectorsWritten = 0;
    uint32_t givenDirectoryEntryOffsetInParentCluster = 0; //the offset in its cluster, not its offset relative to all its parent's clusters
    uint32_t readResult;
    uint32_t lastDirectoryEntryInParentOffset;

    uint32_t actualCluster = getFirstClusterForDirectory(bootSector, parentDirectoryEntry);
    uint32_t numberOfClusterInParentDirectory = 0; //the cluster number in chain
    uint32_t occupiedBytesInCluster = 0; //if the cluster is full, then it is cluster size, otherwise smaller

    while(true)
    {
        occupiedBytesInCluster = parentDirectoryEntry->FileSize >= getClusterSize(bootSector) * (numberOfClusterInParentDirectory + 1) ? getClusterSize(bootSector)
                                                                                                            : parentDirectoryEntry->FileSize % getClusterSize(bootSector);

        readResult = readDiskSectors(diskInfo, bootSector->SectorsPerCluster, getFirstSectorForCluster(bootSector, actualCluster),
                                     clusterData, numOfSectorsRead);

        if(readResult != EC_NO_ERROR)
        {
            delete[] clusterData;
            return DELETE_DIRECTORY_ENTRY_FAILED;
        }

        uint32_t searchDirectoryEntryInClusterResult = findDirectoryEntryInGivenClusterData(bootSector, clusterData, (char*) givenDirectoryEntry->FileName,
                                                                                            new DirectoryEntry(), occupiedBytesInCluster, givenDirectoryEntryOffsetInParentCluster);

        if(searchDirectoryEntryInClusterResult == DIR_ENTRY_FOUND)
        {
            uint32_t clusterNumberInChainContainingLastDirectoryEntry = parentDirectoryEntry->FileSize / getClusterSize(bootSector);
            if(parentDirectoryEntry->FileSize % getClusterSize(bootSector) == 0)
            {
                clusterNumberInChainContainingLastDirectoryEntry--;
                lastDirectoryEntryInParentOffset = getClusterSize(bootSector) - 32;
            }
            else
                lastDirectoryEntryInParentOffset = (parentDirectoryEntry->FileSize % getClusterSize(bootSector)) - 32;

            uint32_t clusterWithLastDirectoryEntryInParent = 0; //the clusterWithLastDirectoryEntryInParent number of the last clusterWithLastDirectoryEntryInParent in chain
            uint32_t findClusterResult = findNthClusterInChain(diskInfo, bootSector, getFirstClusterForDirectory(bootSector, parentDirectoryEntry),
                                                               clusterNumberInChainContainingLastDirectoryEntry, clusterWithLastDirectoryEntryInParent);

            if(findClusterResult != CLUSTER_SEARCH_IN_CHAN_SUCCESS)
                return DELETE_DIRECTORY_ENTRY_FAILED;

            readResult = readDiskSectors(diskInfo, bootSector->SectorsPerCluster, getFirstSectorForCluster(bootSector, clusterWithLastDirectoryEntryInParent),
                                         clusterData, numOfSectorsRead);

            if(readResult != EC_NO_ERROR)
            {
                delete[] clusterData;
                return DELETE_DIRECTORY_ENTRY_FAILED;
            }

            DirectoryEntry* lastDirectoryEntryInParentDirectory = (DirectoryEntry*)&clusterData[lastDirectoryEntryInParentOffset];
            DirectoryEntry* newParentDirectoryEntry = new DirectoryEntry();
            memcpy(newParentDirectoryEntry, parentDirectoryEntry, 32);
            newParentDirectoryEntry->FileSize -= 32;
            newParentDirectoryEntry->LastAccessedDate = fat32_getCurrentDateFormatted();
            uint32_t updateParentDirectoryEntryResult = updateDirectoryEntry(diskInfo, bootSector, parentDirectoryEntry, newParentDirectoryEntry);

            if(updateParentDirectoryEntryResult == DIRECTORY_ENTRY_UPDATE_FAILED)
                return DELETE_DIRECTORY_ENTRY_FAILED;

            //this happens for the case where the given directory entry to delete is the last directory entry in parent, so we just reduce parent size, without moving the entry
            if(compareDirectoryNames((char*) givenDirectoryEntry->FileName, (char*) lastDirectoryEntryInParentDirectory->FileName) == true)
                return DELETE_DIRECTORY_ENTRY_SUCCESS;

            //read cluster data again in order to refresh it after updating directory entry above
            readResult = readDiskSectors(diskInfo, bootSector->SectorsPerCluster, getFirstSectorForCluster(bootSector, actualCluster),
                                         clusterData, numOfSectorsRead);

            if(readResult != EC_NO_ERROR)
            {
                delete[] clusterData;
                return DELETE_DIRECTORY_ENTRY_FAILED;
            }

            memcpy(clusterData + givenDirectoryEntryOffsetInParentCluster, lastDirectoryEntryInParentDirectory, 32);
            uint32_t writeResult = writeDiskSectors(diskInfo, bootSector->SectorsPerCluster, getFirstSectorForCluster(bootSector, actualCluster),
                                                     clusterData, numOfSectorsWritten);

            return (writeResult == EC_NO_ERROR) ? DELETE_DIRECTORY_ENTRY_SUCCESS : DELETE_DIRECTORY_ENTRY_FAILED;
        }

        //ELSE: we know for sure that the given directory entry exists in its parent, so the only other possible result is DIR_ENTRY_NOT_FOUND_IN_CLUSTER
        uint32_t nextCluster = 0;
        uint32_t getNextClusterResult = getNextCluster(diskInfo, bootSector, actualCluster, nextCluster);

        //also we can't have EOC, since this would mean we already reached end of parent without finding the given directory entry which is impossible
        if(getNextClusterResult == FAT_VALUE_RETRIEVE_FAILED)
        {
            delete[] clusterData;
            return DELETE_DIRECTORY_ENTRY_FAILED;
        }

        actualCluster = nextCluster;
        numberOfClusterInParentDirectory++;
        numOfSectorsRead = 0;
        readResult = readDiskSectors(diskInfo, bootSector->SectorsPerCluster, getFirstSectorForCluster(bootSector, actualCluster),
                                     clusterData,numOfSectorsRead);

        if(readResult != EC_NO_ERROR)
            return DELETE_DIRECTORY_ENTRY_FAILED;
    }
}

uint32_t getDirectoryDetailsByDirectoryEntry(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* directoryEntry, uint32_t& size, uint32_t& sizeOnDisk)
{
    std::vector<DirectoryEntry*> subDirectories;

    if(directoryEntry->Attributes == DIRECTORY_TYPE_FOLDER)
    {
        uint32_t getSubdirectoriesResult = getSubDirectoriesByParentDirectoryEntry(diskInfo, bootSector, directoryEntry, subDirectories);
        if(getSubdirectoriesResult != GET_SUB_DIRECTORIES_SUCCESS)
            return DIR_GET_FULL_SIZE_FAILED;

        for(DirectoryEntry* childDirectoryEntry : subDirectories)
        {
            uint32_t getChildDirectorySizeResult = getDirectoryDetailsByDirectoryEntry(diskInfo, bootSector,
                                                                                       childDirectoryEntry, size,
                                                                                       sizeOnDisk);
            if(getChildDirectorySizeResult == DIR_GET_FULL_SIZE_FAILED)
            {
                for(DirectoryEntry* entry : subDirectories)
                    delete entry;
            }

            uint32_t numOfClustersOccupiedClusters = childDirectoryEntry->FileSize / getClusterSize(bootSector);
            if(childDirectoryEntry->FileSize % getClusterSize(bootSector) == 0) //in case the size occupies the last sector at maximum
                numOfClustersOccupiedClusters--;

            sizeOnDisk += (numOfClustersOccupiedClusters + 1) * getClusterSize(bootSector);
            size += childDirectoryEntry->FileSize - 64;
        }

        for(DirectoryEntry* entry : subDirectories)
            delete entry;
    }

    return DIR_GET_FULL_SIZE_SUCCESS;
}