#include "iostream"
#include "string"
#include "string.h"
#include "stdio.h"
#include "windows.h"

#include "disk.h"
#include "diskUtils.h"
#include "diskCodes.h"
#include "../include/fat32Utils.h"

int main() {
    char* diskDirectory = "D:\\Facultate\\Licenta\\HardDisks\\HardDisk_144Mb";
    DiskInfo* diskInfo = getDisk(diskDirectory);
    if(diskInfo == nullptr)
    {
        diskInfo = initializeDisk(diskDirectory, 288000, 512);
        std::cout << "Disk initialized\n";
    }
    else
    {
        std::cout << "Sectors number: " << diskInfo->diskParameters.sectorsNumber << "\n";
        std::cout << "Sector size: " << diskInfo->diskParameters.sectorSizeBytes << "\n";
        std:: cout << "Disk size: " << diskInfo->diskParameters.sectorsNumber * diskInfo->diskParameters.sectorSizeBytes << "\n";
    }

    if(!checkBootSectorsInitialized(diskInfo))
    {
        std::cout << "Initializing boot sectors...\n";
        initializeBootSectors(diskInfo);
    }
    else
    {
        std::cout << "Boot sectors already initialized\n";
    }

    return 0;
}
