#include "windows.h"
#include "iostream"
#include "string.h"

#include <cstring>
#include <cstdlib>
#include <ctime>

#include "disk.h"
#include "diskUtils.h"

#include "fat32Init.h"
#include "fat32Structures.h"

#include "ext2Init.h"
#include "ext2Structures.h"

#include "hfsInit.h"
#include "hfsStructures.h"

#include "../include/attributes.h"
#include "../include/utils.h"

void initializeFAT32(char* hardDiskPath, DiskInfo** diskInfo, BootSector** bootSector, FsInfo** fsInfo, uint32_t sectorsNumber, uint32_t sectorSize, uint32_t sectorsPerCluster)
{
    *diskInfo = nullptr;
    *bootSector = nullptr;
    *fsInfo = nullptr;

    fat32Startup(hardDiskPath, diskInfo, bootSector, fsInfo, sectorsNumber, sectorSize, sectorsPerCluster, false);
}

void initializeExt2(char* hardDiskPath, DiskInfo** diskInfo, ext2_super_block** superBlock, uint32_t sectorsNumber, uint32_t sectorSize, uint32_t blockSize)
{
    *diskInfo = nullptr;
    ext2Startup(hardDiskPath, diskInfo, sectorsNumber, sectorSize, blockSize, false);
    *superBlock = readFirstSuperBlock(*diskInfo);
}

void initializeHFS(char* hardDiskPath, DiskInfo** diskInfo, HFSPlusVolumeHeader** volumeHeader, ExtentsFileHeaderNode** extentsFileHeaderNode,
                   CatalogFileHeaderNode** catalogFileHeaderNode, uint32_t sectorsNumber, uint32_t sectorSize, uint32_t blockSize)
{
    *diskInfo = nullptr;
    hfsStartup(hardDiskPath, diskInfo, sectorsNumber, sectorSize, blockSize, false);
    *volumeHeader = readVolumeHeader(*diskInfo);
    *extentsFileHeaderNode = readExtentsOverflowFileHeaderNode(*diskInfo, *volumeHeader);
    *catalogFileHeaderNode = readCatalogFileHeaderNode(*diskInfo, *volumeHeader);
}

///////////////////////////////

void buildAllHardDiskPaths(char* testHardDisksDirectory, char* fat32Path, char* ext2Path, char* hfsPath)
{
    char* mainPath = new char [100];
    memcpy(mainPath, "D:\\Facultate\\Licenta\\HardDisks\\\0", 100);

    memcpy(mainPath + strlen(mainPath), testHardDisksDirectory, strlen(testHardDisksDirectory) + 1);

    char* fat32Text = new char [100];
    memcpy(fat32Text, "HardDisk_FAT32\0", 100);
    memcpy(fat32Path, mainPath, strlen(mainPath) + 1);
    memcpy(fat32Path + strlen(fat32Path), fat32Text, strlen(fat32Text) + 1);

    char* ext2Text = new char [100];
    memcpy(ext2Text, "HardDisk_Ext2\0", 100);
    memcpy(ext2Path, mainPath, strlen(mainPath) + 1);
    memcpy(ext2Path + strlen(ext2Path), ext2Text, strlen(ext2Text) + 1);

    char* hfsText = new char [100];
    memcpy(hfsText, "HardDisk_HFS\0", 100);
    memcpy(hfsPath, mainPath, strlen(mainPath) + 1);
    memcpy(hfsPath + strlen(hfsPath), hfsText, strlen(hfsText) + 1);
}

void buildFat32HardDiskPath(char* testHardDisksDirectory, char* fat32Path)
{
    char* mainPath = new char [100];
    memcpy(mainPath, "D:\\Facultate\\Licenta\\HardDisks\\\0", 100);

    memcpy(mainPath + strlen(mainPath), testHardDisksDirectory, strlen(testHardDisksDirectory) + 1);

    char* fat32Text = new char [100];
    memcpy(fat32Text, "HardDisk_FAT32\0", 100);
    memcpy(fat32Path, mainPath, strlen(mainPath) + 1);
    memcpy(fat32Path + strlen(fat32Path), fat32Text, strlen(fat32Text) + 1);
}

void buildExt2HardDiskPaths(char* testHardDisksDirectory, char* ext2Path)
{
    char* mainPath = new char [100];
    memcpy(mainPath, "D:\\Facultate\\Licenta\\HardDisks\\\0", 100);

    memcpy(mainPath + strlen(mainPath), testHardDisksDirectory, strlen(testHardDisksDirectory) + 1);

    char* ext2Text = new char [100];
    memcpy(ext2Text, "HardDisk_Ext2\0", 100);
    memcpy(ext2Path, mainPath, strlen(mainPath) + 1);
    memcpy(ext2Path + strlen(ext2Path), ext2Text, strlen(ext2Text) + 1);
}

void buildHfsHardDiskPaths(char* testHardDisksDirectory, char* hfsPath)
{
    char* mainPath = new char [100];
    memcpy(mainPath, "D:\\Facultate\\Licenta\\HardDisks\\\0", 100);

    memcpy(mainPath + strlen(mainPath), testHardDisksDirectory, strlen(testHardDisksDirectory) + 1);

    char* hfsText = new char [100];
    memcpy(hfsText, "HardDisk_HFS\0", 100);
    memcpy(hfsPath, mainPath, strlen(mainPath) + 1);
    memcpy(hfsPath + strlen(hfsPath), hfsText, strlen(hfsText) + 1);
}

///////////////////////////////

void printDuration(uint64_t milliseconds, uint32_t fileSystem)
{
    uint64_t totalSeconds = milliseconds / 1000;
    uint64_t millisecondsDisplayed = milliseconds % 1000;
    uint64_t secondsDisplayed = totalSeconds % 60;
    uint64_t minutesDisplayed = totalSeconds / 60;

    switch (fileSystem) {
        case FAT32:
            std::cout << "FAT32 ";
            break;
        case EXT2:
            std::cout << "EXT2 ";
            break;
        default:
            std::cout << "HFS ";
    }

    std::cout << "--- Time --- Minutes: " << minutesDisplayed << " --- Seconds: " << secondsDisplayed << " --- Milliseconds: " << millisecondsDisplayed << '\n';
}

void printDurationSolo(uint64_t milliseconds)
{
    uint64_t totalSeconds = milliseconds / 1000;
    uint64_t millisecondsDisplayed = milliseconds % 1000;
    uint64_t secondsDisplayed = totalSeconds % 60;
    uint64_t minutesDisplayed = totalSeconds / 60;

    std::cout << "Minutes: " << minutesDisplayed << " --- Seconds: " << secondsDisplayed << " --- Milliseconds: " << millisecondsDisplayed << '\n';
}

char* generateBuffer(uint64_t size)
{
    srand (time(nullptr));
    char* buffer = new char[size];

    for(uint32_t i = 0; i < size; i++)
        buffer[i] = rand() % 256;

    return buffer;
}

void deleteFiles(char* diskPath)
{
    char* diskPathCopy = new char[100];
    memcpy(diskPathCopy, diskPath, 100);

    memcpy(diskPathCopy + strlen(diskPathCopy), "\\Metadata\0", 20);
    DeleteFile(diskPathCopy);

    memcpy(diskPathCopy, diskPath, 100);
    memcpy(diskPathCopy + strlen(diskPathCopy), "\\Data\0", 20);
    DeleteFile(diskPathCopy);
}