#ifndef DISKEMULATIONLIB_DISKINITIALIZATION_H
#define DISKEMULATIONLIB_DISKINITIALIZATION_H
#include "iostream"

struct DiskInfo {
    const char* diskDirectory;
    unsigned int sectorsNumber;
    unsigned int sectorSizeBytes;
    unsigned int totalSizeBytes;

    DiskInfo(const char* diskDirectory, unsigned int sectorsNumber, unsigned int sectorSizeBytes, unsigned long long totalSizeBytes);
};

DiskInfo initializeDisk(const char* diskDirectory, unsigned int sectorsNumber, unsigned int sectorSize);
void writeBytes(DiskInfo *diskInfo, int sector, const char* data);

#endif 
