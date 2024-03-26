#include <iostream>

#include "../include/disk.h"
#include "../include/diskUtils.h"
#include "../include/diskCodes.h"
#include "../include/ext2Init.h"

int main() {

    char* diskDirectory = "D:\\Facultate\\Licenta\\HardDisks\\HardDisk_16Mib\0";
    DiskInfo* diskInfo = nullptr;
    ext2Startup(diskDirectory, &diskInfo, 32760, 512); //16Mib

    uint32_t numOfSectorsRead;
    char* readBuffer = new char[1024];
    readDiskSectors(diskInfo, 2, 2, readBuffer, numOfSectorsRead);
    ext2_super_block superBlock = *(ext2_super_block*)&readBuffer[0];

    char* readBuffer_2 = new char[512];
    readDiskSectors(diskInfo, 1, 4, readBuffer_2, numOfSectorsRead);
    ext2_group_desc groupDesc1 = *(ext2_group_desc *)&readBuffer_2[0];
    ext2_group_desc groupDesc2 = *(ext2_group_desc *)&readBuffer_2[32];

    return 0;
}
