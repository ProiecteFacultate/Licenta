#include "windows.h"
#include "string"
#include "string.h"
#include "iostream"

#include "../include/disk.h"
#include "../include/diskUtils.h"
#include "../include/diskCodes.h"

DiskMetadata::DiskMetadata(uint32_t sectorsNumber, uint16_t sectorSizeBytes)
{
    this->sectorsNumber = sectorsNumber;
    this->sectorSizeBytes = sectorSizeBytes;
}


DiskParameters::DiskParameters(uint32_t sectorsNumber, uint16_t sectorSizeBytes)
{
    this->sectorsNumber = sectorsNumber;
    this->sectorSizeBytes = sectorSizeBytes;
}

DiskInfo::DiskInfo(const char* diskDirectory, uint32_t sectorsNumber, uint16_t sectorSizeBytes, uint16_t status)
{
    this->diskDirectory = diskDirectory;
    this->diskParameters = DiskParameters(sectorsNumber, sectorSizeBytes);
    this->status = status;
}


void copy_buffer(char* destination, const char* source, size_t num)
{
    for(int i = 0; i < num; i++)
    {
        destination[i] = source[i];
    }
}

void concat_buffer(char* destination, const char* source, size_t destinationInitialSize, size_t num)
{
    for(int i = 0; i < num; i++)
    {
        destination[destinationInitialSize + i] = source[i];
    }
}