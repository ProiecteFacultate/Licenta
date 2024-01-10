#include "iostream"
#include "string"
#include "string.h"

#include "../include/diskInitialization.h"

int main() {
    DiskInfo diskInfo = initializeDisk("D:\\Facultate\\Licenta\\Implementare\\HardDisk", 5, 512);
    std::cout << "Sectors number: " << diskInfo.sectorsNumber << "\n";
    std::cout << "Sector size: " << diskInfo.sectorSizeBytes << "\n";
    std:: cout << "Disk size: " << diskInfo.totalSizeBytes << "\n";

    const char* text = "Hello World";
    writeBytes(&diskInfo, 0, text);
    std::cout << readBytes(&diskInfo, 0, 5);

    return 0;
}
