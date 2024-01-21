#ifndef DISKEMULATIONLIB_DISK_H
#define DISKEMULATIONLIB_DISK_H

//V 1.02

//the disk methods are based on x86 INT 13 - Diskette BIOS Services

struct DiskParameters {
    uint32_t sectorsNumber;
    uint16_t sectorSizeBytes;

    DiskParameters() {};
    DiskParameters(uint32_t sectorsNumber, uint16_t sectorSizeBytes);
};

struct DiskInfo {
    const char* diskDirectory;
    DiskParameters diskParameters;
    uint16_t status;

    DiskInfo() {};
    DiskInfo(const char* diskDirectory, uint32_t sectorsNumber, uint16_t sectorSizeBytes,  uint16_t status);
};

DiskInfo initializeDisk(const char* diskDirectory, uint32_t sectorsNumber, uint16_t sectorSize);
DiskInfo getDisk(const char* diskDirectory);


//Disk services

//INT 13, 01
int getDiskStatus(DiskInfo *diskInfo);
//INT 13, 02
int readDiskSectors(DiskInfo *diskInfo, uint32_t numOfSectorsToRead, uint32_t sector, char* buffer, uint32_t &numOfSectorsRead);
//INT 13, 03
int writeDiskSectors(DiskInfo *diskInfo, uint32_t numOfSectorsToWrite, uint32_t sector, char* buffer, uint32_t &numOfSectorsWritten);
//INT 13, 04
int verifyDiskSectors(DiskInfo *diskInfo, uint32_t numOfSectorsToVerify, uint32_t sector, char* buffer, uint32_t &numOfSectorsVerified);
//INT 13, 07
//clears sector data
int formatDiskSectors(DiskInfo *diskInfo, uint32_t sector);


//Helper methods
static char* buildFilePath(const char* diskDirectory, uint32_t sector);
static int readSector(DiskInfo *diskInfo, uint32_t sector, char *buffer);
static int writeSector(DiskInfo *diskInfo, uint32_t sector, char *buffer);
static int verifySector(DiskInfo *diskInfo, uint32_t sector, char* buffer);

#endif 
