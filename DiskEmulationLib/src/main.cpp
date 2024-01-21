#include "iostream"
#include "string"
#include "string.h"
#include "windows.h"

#include "../include/disk.h"
#include "../include/diskUtils.h"

int main() {
    initializeDisk("D:\\Facultate\\Licenta\\Implementare\\HardDisk", 16, 512);

    DiskInfo* disk = getDisk("D:\\Facultate\\Licenta\\Implementare\\HardDisk");
    std::cout << "Sectors number: " << disk->diskParameters.sectorsNumber << "\n";
    std::cout << "Sector size: " << disk->diskParameters.sectorSizeBytes << "\n";
    std::cout << "Disk size: " << disk->diskParameters.sectorsNumber *  disk->diskParameters.sectorSizeBytes << "\n";

//    char* writeBuffer = new char[diskInfo.diskParameters.sectorSizeBytes * 3];
//    strcpy(writeBuffer, "Hello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello World Hello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello World\0"
//    );
//    uint32_t numOfSectorsWritten = 0;
//    writeDiskSectors(&diskInfo, 4, 0, writeBuffer, numOfSectorsWritten);
//    std::cout << "Number of sectors written: " << numOfSectorsWritten << "\n";
//    char* readBuffer = new char[diskInfo.diskParameters.sectorSizeBytes * 3];
//    uint32_t numOfSectorsRead = 0;
//    readDiskSectors(&diskInfo, 3, 0, readBuffer, numOfSectorsRead);
//    std::cout << "Number of sectors read: " << numOfSectorsRead << "\n";
//    std::cout << readBuffer << "\n";
//
//    char* verifyBuffer = new char[diskInfo.diskParameters.sectorSizeBytes * 3];
//    strcpy(verifyBuffer, "Hello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello World Hello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello World\0"
//    );
//    uint32_t numOfSectorsVerified = 0;
//    verifyDiskSectors(&diskInfo, 3, 0, verifyBuffer, numOfSectorsVerified);
//    std::cout << "Num of sectors verified: " << numOfSectorsVerified;

  //  formatDiskSectors(&diskInfo, 0);

    return 0;
}
