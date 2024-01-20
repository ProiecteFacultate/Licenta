#ifndef DISKEMULATIONLIB_DISK_H
#define DISKEMULATIONLIB_DISK_H

//V 1.01

//the disk methods are based on x86 INT 13 - Diskette BIOS Services

struct DiskInfo {
    const char* diskDirectory;
    unsigned int sectorsNumber;
    unsigned int sectorSizeBytes;
    unsigned int totalSizeBytes;
    unsigned int status;

    DiskInfo() {};
    DiskInfo(const char* diskDirectory, unsigned int sectorsNumber, unsigned int sectorSizeBytes, unsigned long long totalSizeBytes, unsigned int status);
};

DiskInfo initializeDisk(const char* diskDirectory, unsigned int sectorsNumber, unsigned int sectorSize);
DiskInfo getDisk(const char* diskDirectory);


//Disk services

//INT 13, 01
int getDiskStatus(DiskInfo *diskInfo);
//INT 13, 02
int readDiskSectors(DiskInfo *diskInfo, unsigned int numOfSectorsToRead, unsigned int sector, char* buffer, int &numOfSectorsRead);
//INT 13, 03
int writeDiskSectors(DiskInfo *diskInfo, unsigned int numOfSectorsToWrite, unsigned int sector, char* buffer, int &numOfSectorsWritten);
//INT 13, 04
int verifyDiskSectors(DiskInfo *diskInfo, unsigned int numOfSectorsToVerify, unsigned int sector, char* buffer, int &numOfSectorsVerified);
//INT 13, 07
//clears sector data
int formatDiskSectors(DiskInfo *diskInfo, unsigned int sector);


//Helper methods
static char* buildFilePath(const char* diskDirectory, int sector);
static int readSector(DiskInfo *diskInfo, unsigned int sector, char *buffer);
static int writeSector(DiskInfo *diskInfo, unsigned int sector, char *buffer);
static int verifySector(DiskInfo *diskInfo, unsigned int sector, char* buffer);

#endif 
