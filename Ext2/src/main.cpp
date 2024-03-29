#include <iostream>

#include "../include/disk.h"
#include "../include/diskUtils.h"
#include "../include/diskCodes.h"
#include "../include/codes/ext2Attributes.h"
#include "../include/ext2Init.h"
#include "../include/ext2Api.h"

int main() {

    char* diskDirectory = "D:\\Facultate\\Licenta\\HardDisks\\HardDisk_16Mib\0";
    DiskInfo* diskInfo = nullptr;
    ext2Startup(diskDirectory, &diskInfo, 32760, 512); //16Mib
    ext2_super_block* superBlock = readFirstSuperBlock(diskInfo);

//    uint32_t numOfSectorsRead;
//    char* readBuffer = new char[1024];
//    readDiskSectors(diskInfo, 2, 2, readBuffer, numOfSectorsRead);
//    ext2_super_block superBlock = *(ext2_super_block*)&readBuffer[0];
//
//    char* readBuffer_2 = new char[512];
//    readDiskSectors(diskInfo, 1, 4, readBuffer_2, numOfSectorsRead);
//    ext2_group_desc groupDesc1 = *(ext2_group_desc *)&readBuffer_2[0];
//    ext2_group_desc groupDesc2 = *(ext2_group_desc *)&readBuffer_2[32];
//
//    char* readBuffer_3 = new char[512];
//    readDiskSectors(diskInfo, 1, 6, readBuffer_3, numOfSectorsRead);
//
//    char* readBuffer_4 = new char[512];
//    readDiskSectors(diskInfo, 1, 8, readBuffer_4, numOfSectorsRead);
//
//    char* readBuffer_5 = new char[512];
//    readDiskSectors(diskInfo, 1, 10, readBuffer_5, numOfSectorsRead);
//    ext2_inode rootInode = *(ext2_inode*)&readBuffer_5[0];

    char* parentName = new char[50];
    memcpy(parentName, "Root\0", 8);
    char* newDirectoryName = new char[50];
    memcpy(newDirectoryName, "Dir_1\0", 6);
    createDirectory(diskInfo, superBlock, parentName, newDirectoryName, FILE_TYPE_DIRECTORY);

    return 0;
}
