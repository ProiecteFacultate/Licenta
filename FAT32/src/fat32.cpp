#include "windows.h"
#include "string"
#include "string.h"
#include "iostream"

#include "../include/disk.h"
#include "../include/diskUtils.h"
#include "../include/fat32.h"

void read(DiskInfo* diskInfo)
{
    char* readBuffer = new char[diskInfo->diskParameters.sectorSizeBytes * 3];
    uint32_t numOfSectorsRead = 0;
    readDiskSectors(diskInfo, 1, 1, readBuffer, numOfSectorsRead);

    Test* test = reinterpret_cast<Test *>(readBuffer);

    std::cout << "Number of sectors read: " << numOfSectorsRead << "\n";
    std::cout << test->val << "\n";
}