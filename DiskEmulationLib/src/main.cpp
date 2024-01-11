#include "iostream"
#include "string"
#include "string.h"

#include "../include/diskInitialization.h"

int main() {
    DiskInfo diskInfo = initializeDisk("D:\\Facultate\\Licenta\\Implementare\\HardDisk", 5, 512);
    std::cout << "Sectors number: " << diskInfo.sectorsNumber << "\n";
    std::cout << "Sector size: " << diskInfo.sectorSizeBytes << "\n";
    std:: cout << "Disk size: " << diskInfo.totalSizeBytes << "\n";

    char* writeBuffer = new char[diskInfo.sectorSizeBytes * 3];
    strcpy(writeBuffer, "Hello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello World Hello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello World\0"
    );
    int numOfSectorsWritten = 0;
    writeDiskSectors(&diskInfo, 4, 0, writeBuffer, numOfSectorsWritten);
    std::cout << "Number of sectors written: " << numOfSectorsWritten << "\n";
    char* readBuffer = new char[diskInfo.sectorSizeBytes * 3];
    int numOfSectorsRead = 0;
    readDiskSectors(&diskInfo, 3, 0, readBuffer, numOfSectorsRead);
    std::cout << "Number of sectors read: " << numOfSectorsRead << "\n";
    std::cout << readBuffer;

    return 0;
}
