#include "iostream"
#include "string"
#include "string.h"
#include "windows.h"

#include "../include/disk.h"

int main() {
    DiskInfo diskInfo = initializeDisk("D:\\Facultate\\Licenta\\Implementare\\HardDisk", 16, 512);
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
    std::cout << readBuffer << "\n";

    char* verifyBuffer = new char[diskInfo.sectorSizeBytes * 3];
    strcpy(verifyBuffer, "Hello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello World Hello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello World\0"
    );
    int numOfSectorsVerified = 0;
    verifyDiskSectors(&diskInfo, 3, 0, verifyBuffer, numOfSectorsVerified);
    std::cout << "Num of sectors verified: " << numOfSectorsVerified;

  //  formatDiskSectors(&diskInfo, 0);

    return 0;
}
