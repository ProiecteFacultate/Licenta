#include <iostream>

#include "../include/disk.h"
#include "../include/diskUtils.h"
#include "../include/diskCodes.h"
#include "../include/structures.h"
#include "../include/hfsInit.h"

int main() {
    char* diskDirectory = "D:\\Facultate\\Licenta\\HardDisks\\HardDisk_100Mib\0";
    DiskInfo* diskInfo = nullptr;
    hfsStartup(diskDirectory, &diskInfo, 204800, 512); //100Mib
    HFSPlusVolumeHeader* volumeHeader = readVolumeHeader(diskInfo);

    char* readBuffer = new char[4096];
    uint32_t numberOfSectorsRead = 0;
    uint32_t readResult = readDiskSectors(diskInfo, 2, 30, readBuffer, numberOfSectorsRead);
    BTNodeDescriptor* extentsHeaderNode = (BTNodeDescriptor*)&readBuffer[0];

    readResult = readDiskSectors(diskInfo, 2, 2078, readBuffer, numberOfSectorsRead);
    BTNodeDescriptor* catalogHeaderNode = (BTNodeDescriptor*)&readBuffer[0];

    return 0;
}