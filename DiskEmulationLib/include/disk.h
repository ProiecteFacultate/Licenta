//V 1.05

#ifndef DISKEMULATIONLIB_DISK_H
#define DISKEMULATIONLIB_DISK_H

#include "../include/diskUtils.h"

//the disk methods are based on x86 INT 13 - Diskette BIOS Services

DiskInfo* initializeDisk(const char* diskDirectory, uint32_t sectorsNumber, uint16_t sectorSize);
int fillDiskInitialMemory(DiskInfo *diskInfo);
DiskInfo* getDisk(const char* diskDirectory);

//Disk services

//INT 13, 01
int getDiskStatus(DiskInfo *diskInfo);
//INT 13, 02
int readDiskSectors(DiskInfo *diskInfo, uint32_t numOfSectorsToRead, uint32_t sector, char* buffer, uint32_t &numOfSectorsRead);
//INT 13, 03
int writeDiskSectors(DiskInfo *diskInfo, uint32_t numOfSectorsToWrite, uint32_t sector, char buffer[], uint32_t &numOfSectorsWritten);
//INT 13, 04
int verifyDiskSectors(DiskInfo *diskInfo, uint32_t numOfSectorsToVerify, uint32_t sector, char* buffer, uint32_t &numOfSectorsVerified);
//INT 13, 07
//clears sector data
int formatDiskSectors(DiskInfo *diskInfo, uint32_t sector);


//Helper methods
static int createMetadataFile(const char* diskDirectory, uint32_t sectorsNumber, uint16_t sectorSize);
static char* buildFilePath(const char* diskDirectory, uint32_t sector);
static int readSector(DiskInfo *diskInfo, uint32_t sector, char *buffer);
static int writeSector(DiskInfo *diskInfo, uint32_t sector, char *buffer);
static int verifySector(DiskInfo *diskInfo, uint32_t sector, char* buffer);

#endif 
