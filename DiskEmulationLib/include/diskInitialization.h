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
int readDiskSectors(DiskInfo *diskInfo, unsigned int numOfSectorsToRead, unsigned int sector, char* buffer, int &numOfSectorsRead);
//INT 13, 03
int writeDiskSectors(DiskInfo *diskInfo, unsigned int numOfSectorsToWrite, unsigned int sector, char* buffer, int &numOfSectorsWritten);
//INT 13, 04
//verifies if the sectors can be found
int verifyDiskSectors(DiskInfo *diskInfo, unsigned int numOfSectorsToVerify, unsigned int sector, int &numOfSectorsVerified);


//Helper methods
static char* buildFilePath(const char* diskDirectory, int sector);
static int readSector(DiskInfo *diskInfo, unsigned int sector, char *buffer);
static int writeSector(DiskInfo *diskInfo, unsigned int sector, char *buffer);
static int verifySector(DiskInfo *diskInfo, unsigned int sector);

#endif 
