#include "iostream"
#include "string"
#include "string.h"
#include "windows.h"

#include "disk.h"
#include "diskUtils.h"
#include "diskCodes.h"
#include "../include/fat32.h"

int main() {
    char* diskDirectory = "D:\\Facultate\\Licenta\\Implementare\\HardDisk";
    DiskInfo* diskInfo = getDisk(diskDirectory);
    if(diskInfo == nullptr)
    {
        initializeDisk(diskDirectory, 16, 512);
        std::cout << "Disk initialized\n";
    }
    else
    {
        std::cout << "Sectors number: " << diskInfo->diskParameters.sectorsNumber << "\n";
        std::cout << "Sector size: " << diskInfo->diskParameters.sectorSizeBytes << "\n";
        std:: cout << "Disk size: " << diskInfo->diskParameters.sectorsNumber * diskInfo->diskParameters.sectorSizeBytes << "\n";
    }

//    char* writeBuffer = new char[diskInfo->diskParameters.sectorSizeBytes * 3];
//    strcpy(writeBuffer, "\xEB\x0C\x90MSWIN4.1\x00\x02\x01\x00\x02\xE0\x00\xA4\x10\xF0\x09\x00\x12\x00\x02\x00\x00\x00\x00\x00\x00\x00\x29\x12\x34\x56\x78NANOBYTE OSFAT12\xEB\x0C\x90\"\n");
//    uint32_t numOfSectorsWritten = 0;
//    writeDiskSectors(diskInfo, 4, 0, writeBuffer, numOfSectorsWritten);

    char* writeBuffer = new char[diskInfo->diskParameters.sectorSizeBytes * 3];
    char* daraBuffer = "\x00\x02";
    copy_buffer(writeBuffer, daraBuffer, 2);
    uint32_t numOfSectorsWritten = 0;
    writeDiskSectors(diskInfo, 1, 1, writeBuffer, numOfSectorsWritten);

    read(diskInfo);

    return 0;
}
