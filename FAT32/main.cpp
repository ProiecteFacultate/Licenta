#include <iostream>
#include "disk.h"
#include "diskCallsResponse.h"

int main() {
    DiskInfo diskInfo = initializeDisk("D:\\Facultate\\Licenta\\Implementare\\HardDisk", 5, 512);
    std::cout << "Sectors number: " << diskInfo.sectorsNumber << "\n";
    std::cout << "Sector size: " << diskInfo.sectorSizeBytes << "\n";
    std:: cout << "Disk size: " << diskInfo.totalSizeBytes << "\n";

    return 0;
}
