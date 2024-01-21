#ifndef DISKEMULATIONLIB_DISKUTILS_H
#define DISKEMULATIONLIB_DISKUTILS_H

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

#endif
