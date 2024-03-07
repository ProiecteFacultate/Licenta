#include "iostream"
#include "string"
#include "string.h"
#include "stdio.h"
#include "windows.h"
#include "vector"

#include "disk.h"
#include "diskUtils.h"
#include "diskCodes.h"
#include "../include/fat32Init.h"
#include "../include/fat32.h"
#include "../include/fat32Api.h"

bool creatingDiskAndFileSystem = false;
bool debugMode = false;

int initialLoad(DiskInfo** diskInfo, BootSector** bootSector, FsInfo** fsInfo)
{
    char* diskDirectory = "D:\\Facultate\\Licenta\\HardDisks\\HardDisk_512Kib";
    *diskInfo = getDisk(diskDirectory);
    if(*diskInfo == nullptr)
    {
        *diskInfo = initializeDisk(diskDirectory, 1024, 512);
        fillDiskInitialMemory(*diskInfo);
        std::cout << "Disk initialized\n";
    }
    else
    {
        std::cout << "Sectors number: " << (*diskInfo)->diskParameters.sectorsNumber << "\n";
        std::cout << "Sector size: " << (*diskInfo)->diskParameters.sectorSizeBytes << "\n";
        std:: cout << "Disk size: " << (*diskInfo)->diskParameters.sectorsNumber * (*diskInfo)->diskParameters.sectorSizeBytes << "\n";
    }

    if(!checkBootSectorsInitialized(*diskInfo))
    {
        std::cout << "Initializing boot sectors...\n";
        initializeBootSectors(*diskInfo);
    }
    else
    {
        std::cout << "Boot sectors already initialized\n";
    }

    *bootSector = readBootSector(*diskInfo);
    *fsInfo = readFsInfo(*diskInfo, *bootSector);

    if(creatingDiskAndFileSystem == true)
    {
        initializeFat(*diskInfo, *bootSector);   //TODO initialize only when file system created
    }
}

int main() {
    DiskInfo* diskInfo = nullptr;
    BootSector* bootSector = nullptr;
    FsInfo* fsInfo = nullptr;

    initialLoad(&diskInfo, &bootSector, &fsInfo);

    if(debugMode == true)
    {
        uint32_t numOfSectorsRead = 0;
        char* rootSectorData = new char[bootSector->BytesPerSector];
        readDiskSectors(diskInfo, 1, 104, rootSectorData, numOfSectorsRead);
        char* sectorData1 = new char[bootSector->BytesPerSector];
        readDiskSectors(diskInfo, 1, 120, sectorData1, numOfSectorsRead);
        char* sectorData2 = new char[bootSector->BytesPerSector];
        readDiskSectors(diskInfo, 1, 136, sectorData2, numOfSectorsRead);
        char* sectorData3 = new char[bootSector->BytesPerSector];
        readDiskSectors(diskInfo, 1, 152, sectorData3, numOfSectorsRead);

//        char* sectorData1 = new char[bootSector->BytesPerSector];
//        readDiskSectors(diskInfo, 1, 120, sectorData1, numOfSectorsRead);
//        DirectoryEntry* dir = (DirectoryEntry*)&sectorData1[0];
        return 0;
    }

    if(creatingDiskAndFileSystem == false)
    {
        char* parentPath = new char[100];
        char* newDir = new char[10];
        memcpy(parentPath, "Root\0", 5);
        char* originalParentPath = new char[100];
        memcpy(originalParentPath, parentPath, 100);
        std::cout << '\n';
        memset(newDir, '\0', 10);
//        memcpy(newDir, "Level_3", 7);
//        int createDirectoryResult = createDirectory(diskInfo, bootSector, parentPath, newDir);
//        std::cout << "Directory creation for " << newDir << " : " << createDirectoryResult << "\n";


        for(int x = 0; x <= 99; x++)
        {
            memcpy(newDir, "Dir_", 4);
            std::string xAsString = (x < 10 ? "0" : "") + std::to_string(x);
            memcpy(newDir + 4, &xAsString[0], 2);
            int createDirectoryResult = createDirectory(diskInfo, bootSector, parentPath, newDir);
            std::cout << "Directory creation for " << newDir << " : " << createDirectoryResult << "\n";
        }

        uint32_t nextCluster = 0;
        //getNextCluster(diskInfo, bootSector, 2, nextCluster)

        std::vector<DirectoryEntry*> subDirectories;
        int getSubdirectoriesResult = getSubDirectories(diskInfo, bootSector, originalParentPath, subDirectories);
        std::string prompt1 = "\nGet subdirectories result: ";
        std::cout << prompt1 << getSubdirectoriesResult << "\n";
        for(auto dir : subDirectories)
        {
            char* fileName = new char[12];
            memcpy(fileName, dir->FileName, 11);
            fileName[11] = 0;
            std::cout << fileName << "\n";
        }
    }


    return 0;
}
