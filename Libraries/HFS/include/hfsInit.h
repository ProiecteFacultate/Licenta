#include "../include/disk.h"
#include "../include/hfsStructures.h"

#ifndef HFS__HFSINIT_H
#define HFS__HFSINIT_H

void hfsStartup(char* diskDirectory, DiskInfo** diskInfo, uint32_t sectorsNumber, uint32_t sectorSize, uint32_t blockSize, bool printSteps);

HFSPlusVolumeHeader* readVolumeHeader(DiskInfo* diskInfo);

ExtentsFileHeaderNode* readExtentsOverflowFileHeaderNode(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader);

CatalogFileHeaderNode* readCatalogFileHeaderNode(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader);

//////////

static bool checkDiskInitialization(char* diskDirectory);
static void initializeDisk(char* diskDirectory, DiskInfo** diskInfo, uint32_t sectorsNumber, uint32_t sectorSize, bool printSteps);

static bool checkHfsFileSystemInitialization(DiskInfo* diskInfo);

/////////////
//initialize forks data

static void initializeVolumeHeader(DiskInfo* diskInfo, uint32_t blockSize);
static void initializeAllocationFileForkData(DiskInfo* diskInfo, HFSPlusForkData* forkData, uint32_t blockSize);
static void initializeExtentsOverflowFileForkData(HFSPlusForkData* forkData, uint32_t blockSize, uint32_t totalNodes, HFSPlusForkData * allocationFileForkData);
static void initializeCatalogFileForkData(HFSPlusForkData* forkData, uint32_t blockSize, uint32_t totalNodes, HFSPlusForkData * extentsOverflowFileForkData);

///////////////
//init the structure files

static void initializeAllocationFile(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader);
static void initializeExtentsOverflowFile(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader);
static void initializeCatalogFile(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader);

/////////
static uint32_t getFirstBlockForExtentsOverflowFile(HFSPlusForkData * allocationFileForkData);
static uint32_t getFirstBlockForCatalogFile(HFSPlusForkData* extentsFileForkData);

#endif