#ifndef DISKEMULATIONLIB_DISKUTILS_H
#define DISKEMULATIONLIB_DISKUTILS_H

#include <cstdint>

struct DiskMetadata {
    uint32_t sectorsNumber;
    uint16_t sectorSizeBytes;

    DiskMetadata() {};
    DiskMetadata(uint32_t sectorsNumber, uint16_t sectorSizeBytes);
} __attribute__((packed));

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

//The following 2 methods don't stop when reaching NULL character (\0 or \x00). They treat it like any other character
void copy_buffer(char* destination, const char* source, size_t num);
void concat_buffer(char* destination, const char* source, size_t destinationInitialSize, size_t num);

#endif
