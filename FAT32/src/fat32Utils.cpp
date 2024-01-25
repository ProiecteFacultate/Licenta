#include "windows.h"
#include "string"
#include "string.h"
#include "iostream"

#include "../include/disk.h"
#include "../include/diskUtils.h"
#include "../include/fat32Utils.h"


bool checkBootSectorsInitialized(DiskInfo* diskInfo)
{
    char* firstBootSectorBuffer = new char[diskInfo->diskParameters.sectorSizeBytes];

    uint32_t numberOfSectorsRead = 0;
    readDiskSectors(diskInfo, 1, 0, firstBootSectorBuffer, numberOfSectorsRead);

    uint16_t bootSignature = *(uint16_t*)&firstBootSectorBuffer[510];

    delete[] firstBootSectorBuffer;

    return bootSignature == 43605;   //0xAA55
}

void initializeBootSectors(DiskInfo* diskInfo)
{
    char* firstBootSectorBuffer = new char[diskInfo->diskParameters.sectorSizeBytes];
    char* firstBootSectorData = "\xEB\x0C\x90MSWIN4.1\x00\x02\x40\x20\x00\x02\x00\x00\x00\x00\xFA\x00\x00\x22\x00\x22\x00\x00\x00\x00\x00\x00\x00\x00\x00" // BPB
            "\x22\x00\x00\x00\x00\x4F\x00\x00\x02\x00\x00\x00\x01\x00\x06\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x29\x00\x00\x00\x00LABEL      FAT32   "; // EBP
    copy_buffer(firstBootSectorBuffer, firstBootSectorData, 90);
    copy_buffer(firstBootSectorBuffer + 510, "\x55\xAA", 2);

    uint32_t numberOfSectorsWritten = 0;
    writeDiskSectors(diskInfo, 1, 0, firstBootSectorBuffer, numberOfSectorsWritten);


    char* SecondBootSectorBuffer = new char[diskInfo->diskParameters.sectorSizeBytes];
    char* SecondBootSectorData = "\x52\x52\x61\x41\x72\x72\x41\x61\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x55\xAA"; //FsInfo
    copy_buffer(SecondBootSectorBuffer, "\x52\x52\x61\x41", 4);
    copy_buffer(SecondBootSectorBuffer + 484, SecondBootSectorData, 28);

    numberOfSectorsWritten = 0;
    writeDiskSectors(diskInfo, 1, 1, SecondBootSectorBuffer, numberOfSectorsWritten);

    delete[] firstBootSectorBuffer;
    delete[] firstBootSectorData;
    delete[] SecondBootSectorBuffer;
    delete[] SecondBootSectorData;
}