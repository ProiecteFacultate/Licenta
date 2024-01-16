#ifndef DISKEMULATIONLIB_DISKINITIALIZATION_H
#define DISKEMULATIONLIB_DISKINITIALIZATION_H

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
int readDiskSectors(DiskInfo *diskInfo, char* partition, unsigned int numOfSectorsToRead, unsigned int sector, char* buffer, int &numOfSectorsRead);
//INT 13, 03
int writeDiskSectors(DiskInfo *diskInfo, char* partition, unsigned int numOfSectorsToWrite, unsigned int sector, char* buffer, int &numOfSectorsWritten);
//INT 13, 04
//verifies sector data matches expected buffer
int verifyDiskSectors(DiskInfo *diskInfo, char* partition, unsigned int numOfSectorsToVerify, unsigned int sector, char* buffer, int &numOfSectorsVerified);
//INT 13, 07
int formatDiskSectors(DiskInfo *diskInfo, char* partition, int newSectorSize);


//Helper methods
static char* buildFilePath(const char* diskDirectory, char* partition, int sector);
static int readSector(DiskInfo *diskInfo, char* partition, unsigned int sector, char *buffer);
static int writeSector(DiskInfo *diskInfo, char* partition, unsigned int sector, char *buffer);
static int verifySector(DiskInfo *diskInfo, char* partition, unsigned int sector, char* buffer);

#endif 
