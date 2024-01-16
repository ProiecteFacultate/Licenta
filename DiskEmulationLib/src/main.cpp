#include "iostream"
#include "string"
#include "string.h"

#include "../include/diskInitialization.h"

int main() {
    DiskInfo diskInfo = initializeDisk("D:\\Facultate\\Licenta\\Implementare\\HardDisk", 5, 512);
    std::cout << "Sectors number: " << diskInfo.sectorsNumber << "\n";
    std::cout << "Sector size: " << diskInfo.sectorSizeBytes << "\n";
    std:: cout << "Disk size: " << diskInfo.totalSizeBytes << "\n";

    char* defaultPartitionName = new char[128];
    strcpy(defaultPartitionName, "Partition_1\0");

    char* writeBuffer = new char[diskInfo.sectorSizeBytes * 3];
    strcpy(writeBuffer, "Hello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello World Hello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello World\0"
    );
    int numOfSectorsWritten = 0;
    writeDiskSectors(&diskInfo, defaultPartitionName, 4, 0, writeBuffer, numOfSectorsWritten);
    std::cout << "Number of sectors written: " << numOfSectorsWritten << "\n";
    char* readBuffer = new char[diskInfo.sectorSizeBytes * 3];
    int numOfSectorsRead = 0;
    readDiskSectors(&diskInfo, defaultPartitionName, 3, 0, readBuffer, numOfSectorsRead);
    std::cout << "Number of sectors read: " << numOfSectorsRead << "\n";
    std::cout << readBuffer << "\n";

    char* verifyBuffer = new char[diskInfo.sectorSizeBytes * 3];
    strcpy(verifyBuffer, "Hello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello World Hello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello World\0"
    );
    int numOfSectorsVerified = 0;
    verifyDiskSectors(&diskInfo, defaultPartitionName, 3, 0, verifyBuffer, numOfSectorsVerified);
    std::cout << "Num of sectors verified: " << numOfSectorsVerified;

  //  formatDiskSectors(&diskInfo, 0);

    return 0;
}
