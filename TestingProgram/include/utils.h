#include "cstdint"

#include "disk.h"
#include "diskUtils.h"

#include "fat32Init.h"
#include "fat32Structures.h"

#include "ext2Init.h"
#include "ext2Structures.h"

#include "hfsInit.h"
#include "hfsStructures.h"

#ifndef TESTINGPROGRAM_UTILS_H
#define TESTINGPROGRAM_UTILS_H

void initializeFAT32(char* hardDiskPath, DiskInfo** diskInfo, BootSector** bootSector, FsInfo** fsInfo, uint32_t sectorsNumber, uint32_t sectorSize, uint32_t sectorsPerCluster);

void initializeExt2(char* hardDiskPath, DiskInfo** diskInfo, ext2_super_block** superBlock, uint32_t sectorsNumber, uint32_t sectorSize, uint32_t blockSize);

void initializeHFS(char* hardDiskPath, DiskInfo** diskInfo, HFSPlusVolumeHeader** volumeHeader, ExtentsFileHeaderNode** extentsFileHeaderNode,
                   CatalogFileHeaderNode** catalogFileHeaderNode, uint32_t sectorsNumber, uint32_t sectorSize, uint32_t blockSize);

/////////////////////////////////

void buildAllHardDiskPaths(char* testHardDisksDirectory, char* fat32Path, char* ext2Path, char* hfsPath);
void buildFat32HardDiskPath(char* testHardDisksDirectory, char* fat32Path);
void buildExt2HardDiskPaths(char* testHardDisksDirectory, char* ext2Path);
void buildHfsHardDiskPaths(char* testHardDisksDirectory, char* hfsPath);

////////////////////////////////

void printDuration(uint64_t milliseconds, uint32_t fileSystem);
void printDurationSolo(uint64_t milliseconds);
char* generateBuffer(uint64_t size);

void deleteFiles(char* diskPath);

#endif
