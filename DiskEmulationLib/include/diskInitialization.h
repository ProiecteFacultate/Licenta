#ifndef DISKEMULATIONLIB_DISKINITIALIZATION_H
#define DISKEMULATIONLIB_DISKINITIALIZATION_H
#include "iostream"

//the disk methods are based on x86 INT 13 - Diskette BIOS Services

struct DiskInfo {
    const char* diskDirectory;
    unsigned int sectorsNumber;
    unsigned int sectorSizeBytes;
    unsigned int totalSizeBytes;
    unsigned int status;

    DiskInfo(const char* diskDirectory, unsigned int sectorsNumber, unsigned int sectorSizeBytes, unsigned long long totalSizeBytes);
};

DiskInfo initializeDisk(const char* diskDirectory, unsigned int sectorsNumber, unsigned int sectorSize);


//Disk services

//INT 13, 01
int getDiskStatus(DiskInfo *diskInfo);
//INT 13, 02
int readDiskSectors(DiskInfo *diskInfo, int numOfSectorsToRead, int sector, char* buffer, int &numOfSectorsRead);
//INT 13, 03
int writeDiskSectors(DiskInfo *diskInfo, int numOfSectorsToWrite, int sector, char* buffer, int &numOfSectorsWritten);


//Helper methods
static char* buildFilePath(const char* diskDirectory, int sector);
static int readSector(DiskInfo *diskInfo, int sector, char *buffer);
static int writeSector(DiskInfo *diskInfo, int sector, char *buffer);

#endif 
