#include "iostream"
#include "string"
#include "string.h"
#include "stdio.h"
#include "windows.h"

#include "disk.h"
#include "diskUtils.h"
#include "diskCodes.h"
#include "../include/fat32Init.h"
#include "../include/fat32.h"

int initialLoad(DiskInfo** diskInfo, BootSector** bootSector, FsInfo** fsInfo)
{
    char* diskDirectory = "D:\\Facultate\\Licenta\\HardDisks\\HardDisk_144Mib";
    *diskInfo = getDisk(diskDirectory);
    if(diskInfo == nullptr)
    {
        *diskInfo = initializeDisk(diskDirectory, 288000, 512);
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

   // initializeFat(*diskInfo, *bootSector);   //TODO initialize only when file system created
}

int main() {
    DiskInfo* diskInfo = nullptr;
    BootSector* bootSector = nullptr;
    FsInfo* fsInfo = nullptr;

    initialLoad(&diskInfo, &bootSector, &fsInfo);

    char* parentPath = new char[100];
    memcpy(parentPath, "Root\0", 13);
    char* newDir = new char[7];
    memcpy(newDir, "NewDir\0", 7);
    int createDirectoryResult = createDirectory(diskInfo, bootSector, parentPath, newDir);
    std::cout << "Directory creation: " << createDirectoryResult << "\n";

    return 0;
}
