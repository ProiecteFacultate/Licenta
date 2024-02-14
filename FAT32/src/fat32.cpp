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
#include "../include/fat32Attributes.h"
#include "../include/fat32.h"


uint32_t getNextCluster(DiskInfo* diskInfo, BootSector* bootSector, uint32_t actualCluster, uint32_t& nextCluster)
{
    uint32_t firstFatSector = bootSector->ReservedSectors;   //first fat table comes immediately after the reserved sectors
    char* fatTable = new char[bootSector->BytesPerSector];
    uint32_t nextClusterOffset = actualCluster * 4;
    uint32_t sector = firstFatSector + (nextClusterOffset / bootSector->BytesPerSector);
    uint32_t offsetInsideSector = nextClusterOffset % bootSector->BytesPerSector;

    uint32_t numOfSectorsRead = 0;
    int readResult = readDiskSectors(diskInfo, 1, sector, fatTable, numOfSectorsRead);

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
    DirectoryEntry* searchedDirectoryEntry = new DirectoryEntry();

    while(actualDirectoryName != nullptr)
    {
        int searchDirectoryEntryResult = findDirectoryEntry(diskInfo, bootSector, actualDirectoryEntry, actualDirectoryName, &searchedDirectoryEntry);
        if(searchDirectoryEntryResult == DIR_ENTRY_FOUND)
        {
            actualDirectoryEntry = searchedDirectoryEntry;
        }
        else
        {
            delete actualDirectoryEntry, delete searchedDirectoryEntry, delete actualDirectoryName;
            return DIR_CREATION_FAILED;
        }

        actualDirectoryName = strtok(nullptr, "/");
    }

    uint32_t emptyClusterNumber = -1;
    int searchEmptyClusterResult = searchEmptyCluster(diskInfo, bootSector, emptyClusterNumber);

    if(searchEmptyClusterResult == CLUSTER_SEARCH_FAILED)
    {
        delete actualDirectoryEntry, delete searchedDirectoryEntry, delete actualDirectoryName;
        return DIR_CREATION_FAILED;
    }

    int changeFatClusterValueForFirstCluster = changeFatClusterValue(diskInfo, bootSector, emptyClusterNumber, "\xFF\xFF\xFF\xFF");

    if(changeFatClusterValueForFirstCluster == FAT_UPDATE_FAILED)
    {
        delete actualDirectoryEntry, delete searchedDirectoryEntry, delete actualDirectoryName;
        return DIR_CREATION_FAILED;
    }

    addDirectoryEntryToParentDirectory(diskInfo, bootSector, actualDirectoryEntry, directoryName, emptyClusterNumber);
    setupFirstClusterInDirectory(diskInfo, bootSector, nullptr ,emptyClusterNumber);

    delete actualDirectoryEntry, delete searchedDirectoryEntry, delete actualDirectoryName;
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
                delete[] clusterData;
                return DIR_ENTRY_DO_NOT_EXIST;
            case DIR_ENTRY_NOT_FOUND_IN_CLUSTER: {
                uint32_t nextCluster = 0;
                uint32_t getNextClusterResult = getNextCluster(diskInfo, bootSector, actualCluster, nextCluster);
                actualCluster = nextCluster;

                if(getNextClusterResult == FAT_VALUE_EOC)
                {
                    delete[] clusterData;
                    return DIR_ENTRY_SEARCH_ERROR;
                }

                numOfSectorsRead = 0;
                firstSectorInCluster = bootSector->ReservedSectors + (actualCluster - 2) * bootSector->SectorsPerCluster;
                readResult = readDiskSectors(diskInfo, bootSector->SectorsPerCluster, firstSectorInCluster, clusterData,numOfSectorsRead);

                if(readResult != EC_NO_ERROR)
                {
                    delete[] clusterData;
                    return DIR_ENTRY_SEARCH_ERROR;
                }

                break;
            }
            default:
                delete[] clusterData;
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
                delete[] fatTable;
                return CLUSTER_SEARCH_NO_FREE_CLUSTERS;
            }

            uint32_t fatValue = *(uint32_t*)&fatTable[offset];
            if(fatValue == FAT_VALUE_FREE)
            {
                emptyClusterNumber = sectorInFat * numberOfClusterEntriesPerFatSector + offset / 4;
                delete[] fatTable;
                return CLUSTER_SEARCH_SUCCESS;
            }
        }
    }

    delete[] fatTable;
    return CLUSTER_SEARCH_FAILED;
}

static int changeFatClusterValue(DiskInfo* diskInfo, BootSector* bootSector, uint32_t clusterNumber, char* value)
{
    uint32_t firstFatSector = bootSector->ReservedSectors;   //first fat table comes immediately after the reserved sectors
    char* fatTable = new char[bootSector->BytesPerSector];
    uint32_t clusterEntryPosition = clusterNumber * 4;
    uint32_t sector = firstFatSector + (clusterEntryPosition / bootSector->BytesPerSector);
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
    int writeResult = writeDiskSectors(diskInfo, 1, sector, fatTable, numOfSectorsWritten);

    if(writeResult != EC_NO_ERROR)
    {
        delete[] fatTable;
        return FAT_UPDATE_FAILED;
    }

    delete[] fatTable;
    return FAT_UPDATE_SUCCESS;
}

static int findFirstFreeOffsetInDirectory(BootSector* bootSector, char* clusterData, uint32_t& firstFreeOffset)
{
    uint32_t clusterSize = bootSector->SectorsPerCluster * bootSector->BytesPerSector;
    uint32_t offset = 64;  //first 2 entries are for dot and dotdot

    while(offset < clusterSize)
    {
        uint32_t directoryEntryAsValue = *(uint32_t*)&clusterData[offset];
        if(directoryEntryAsValue == 0)
        {
            firstFreeOffset = offset;
            return FIND_FIRST_FREE_OFFSET_IN_DIR_SUCCESS;
        }

        offset += 32;
        if(offset >= bootSector->SectorsPerCluster * bootSector->BytesPerSector)     //we reached cluster ending, but haven't found a free space yet
        {
            return FIND_FIRST_FREE_OFFSET_IN_DIR_FAILED;
        }
    }
}

static void createDirectoryEntry(BootSector* bootSector, char* directoryName, uint8_t directoryAttribute, uint32_t firstCluster, DirectoryEntry* directoryEntry)
{
    memcpy(&directoryEntry[0], directoryName, 11);
    directoryEntry->Attributes = directoryAttribute;
    directoryEntry->Reserved = 0;
    directoryEntry->CreationTimeTenths = 99;    //TODO change values
    directoryEntry->CreationTime = 10000;
    directoryEntry->CreationDate = 1000;
    directoryEntry->LastAccessedDate = 500;
    directoryEntry->FirstClusterHigh = firstCluster & 0xFFFF0000;
    directoryEntry->LastModificationTime = 200;
    directoryEntry->LastAccessedDate = 10;
    directoryEntry->FirstClusterLow = firstCluster & 0x0000FFFF;
    directoryEntry->FileSize = 0;
}

static int addDirectoryEntryToParentDirectory(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* parentDirectoryEntry, char* newDirectoryName, uint32_t firstEmptyCluster)
{
    uint32_t clusterSize = bootSector->SectorsPerCluster * bootSector->BytesPerSector;
    char* clusterData = new char[clusterSize];
    uint32_t numOfSectorsRead = 0;
    uint32_t actualCluster = (parentDirectoryEntry->FirstClusterHigh & 0xFFFF0000) | (parentDirectoryEntry->FirstClusterLow & 0x0000FFFF);
    uint32_t firstSectorInCluster = bootSector->ReservedSectors + (actualCluster - 2) * bootSector->SectorsPerCluster;
    int readResult = readDiskSectors(diskInfo, bootSector->SectorsPerCluster, firstSectorInCluster, clusterData, numOfSectorsRead);

    uint32_t firstFreeOffset = -1;
    while(readResult == EC_NO_ERROR)
    {
        int searchFirstFreeOffsetResult = findFirstFreeOffsetInDirectory(bootSector, clusterData, firstFreeOffset);

        if(searchFirstFreeOffsetResult == FIND_FIRST_FREE_OFFSET_IN_DIR_SUCCESS)
        {
            uint32_t sectorInCluster = firstFreeOffset / bootSector->BytesPerSector;
            uint32_t sector = bootSector->ReservedSectors + bootSector->FatCount * bootSector->SectorsPerFat + (actualCluster - 2) + sectorInCluster;
            char* writeBuffer = new char[bootSector->BytesPerSector];
            memcpy(writeBuffer, clusterData + sectorInCluster * bootSector->BytesPerSector, bootSector->BytesPerSector);
            DirectoryEntry* newDirectoryEntry = new DirectoryEntry();
            createDirectoryEntry(bootSector, newDirectoryName, ATTR_DIRECTORY, firstEmptyCluster, newDirectoryEntry);
            memcpy(writeBuffer + firstFreeOffset, newDirectoryEntry, 32);
            uint32_t numOfSectorsWritten = 0;
            int writeResult =  writeDiskSectors(diskInfo, 1, sector, writeBuffer, numOfSectorsWritten);

            delete[] clusterData, delete[] writeBuffer;
            return (writeResult == EC_NO_ERROR) ? DIR_ENTRY_ADD_SUCCESS : DIR_ENTRY_ADD_FAILED;
        }
        else
        {
            uint32_t nextCluster = 0;
            uint32_t getNextClusterResult = getNextCluster(diskInfo, bootSector, actualCluster, nextCluster);
            actualCluster = nextCluster;

            if(getNextClusterResult == FAT_VALUE_EOC)
            {
                delete[] clusterData;
                return DIR_ENTRY_ADD_FAILED;
            }

            numOfSectorsRead = 0;
            firstSectorInCluster = bootSector->ReservedSectors + (actualCluster - 2) * bootSector->SectorsPerCluster;
            readResult = readDiskSectors(diskInfo, bootSector->SectorsPerCluster, firstSectorInCluster, clusterData,numOfSectorsRead);
        }
    }

    delete[] clusterData;
    return DIR_ENTRY_ADD_FAILED;
}

static int setupFirstClusterInDirectory(DiskInfo* diskInfo, BootSector* bootSector, DirectoryEntry* parentDirectoryEntry, uint32_t clusterNumber)
{
    DirectoryEntry* dotDirectoryEntry = new DirectoryEntry();
    DirectoryEntry* dotDotDirectoryEntry = new DirectoryEntry();

    dotDirectoryEntry->FirstClusterHigh = clusterNumber & 0xFFFF0000;
    dotDirectoryEntry->FirstClusterLow = clusterNumber & 0x0000FFFF;

    if(parentDirectoryEntry != nullptr)
    {
        dotDotDirectoryEntry->FirstClusterHigh = parentDirectoryEntry->FirstClusterHigh;
        dotDotDirectoryEntry->FirstClusterLow = parentDirectoryEntry->FirstClusterLow;
    }

    char* writeBuffer = new char[bootSector->BytesPerSector];
    memcpy(writeBuffer, dotDotDirectoryEntry, 32);
    memcpy(writeBuffer + 32, dotDotDirectoryEntry, 32);
    uint32_t numOfSectorsWritten = 0;
    uint32_t firstSectorInCluster = bootSector->ReservedSectors + (clusterNumber - 2) * bootSector->SectorsPerCluster;
    int writeResult = writeDiskSectors(diskInfo, 1, firstSectorInCluster, writeBuffer, numOfSectorsWritten);

    return (writeResult == EC_NO_ERROR) ? DIR_SETUP_FIRST_CLUSTER_SUCCESS : DIR_SETUP_FIRST_CLUSTER_FAILED;
}