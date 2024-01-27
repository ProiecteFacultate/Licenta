#include "iostream"
#include "string"
#include "string.h"
#include "windows.h"

#include "../include/disk.h"
#include "../include/diskUtils.h"

int main() {
    char* diskDirectory = "D:\\Facultate\\Licenta\\HardDisks\\HardDisk_144Mb";
    DiskInfo* diskInfo = getDisk(diskDirectory);
    if(diskInfo == nullptr)
    {
        diskInfo = initializeDisk(diskDirectory, 16, 512);
        fillDiskInitialMemory(diskInfo);
        std::cout << "Disk initialized\n";
    }
    else
    {
        std::cout << "Sectors number: " << diskInfo->diskParameters.sectorsNumber << "\n";
        std::cout << "Sector size: " << diskInfo->diskParameters.sectorSizeBytes << "\n";
        std:: cout << "Disk size: " << diskInfo->diskParameters.sectorsNumber * diskInfo->diskParameters.sectorSizeBytes << "\n";
    }

    char* writeBuffer = new char[diskInfo->diskParameters.sectorSizeBytes * 4];
    memcpy(writeBuffer,
                "Hello WorldHello WorldAAAAAAAAA\0ZZ\0o WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello World Hello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello TTTTTTWorldHello WorldHello WorldKKK",
                diskInfo->diskParameters.sectorSizeBytes * 4);
    uint32_t numOfSectorsWritten = 0;
    writeDiskSectors(diskInfo, 4, 0, writeBuffer, numOfSectorsWritten);
    std::cout << "Number of sectors written: " << numOfSectorsWritten << "\n";

    char* readBuffer = new char[diskInfo->diskParameters.sectorSizeBytes * 3];
    uint32_t numOfSectorsRead = 0;
    readDiskSectors(diskInfo, 3, 0, readBuffer, numOfSectorsRead);
    for(int i = 0; i < diskInfo->diskParameters.sectorSizeBytes * 3; i++)
        std::cout << readBuffer[i];
    std::cout << "\n\nNumber of sectors read: " << numOfSectorsRead << "\n";

//    char* verifyBuffer = new char[diskInfo.diskParameters.sectorSizeBytes * 3];
//    strcpy(verifyBuffer, "Hello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello World Hello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello World\0"
//    );
//    uint32_t numOfSectorsVerified = 0;
//    verifyDiskSectors(&diskInfo, 3, 0, verifyBuffer, numOfSectorsVerified);
//    std::cout << "Num of sectors verified: " << numOfSectorsVerified;

  //  formatDiskSectors(&diskInfo, 0);

    return 0;
}
