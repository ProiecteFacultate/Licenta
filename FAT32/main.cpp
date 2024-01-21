#include <iostream>
#include "disk.h"
#include "diskUtils.h"
#include "diskCodes.h"

int main() {
    char* diskDirectory = "D:\\Facultate\\Licenta\\Implementare\\HardDisk";
    DiskInfo* diskInfo = getDisk(diskDirectory);
    if(diskInfo == nullptr)
    {
        initializeDisk(diskDirectory, 16, 512);
        std::cout << "Disk initialized\n";
    }
    else
    {
        std::cout << "Sectors number: " << diskInfo->diskParameters.sectorsNumber << "\n";
        std::cout << "Sector size: " << diskInfo->diskParameters.sectorSizeBytes << "\n";
        std:: cout << "Disk size: " << diskInfo->diskParameters.sectorsNumber * diskInfo->diskParameters.sectorSizeBytes << "\n";
    }


    return 0;
}
