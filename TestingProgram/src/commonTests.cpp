#include "iostream"

#include "fat32TestApi.h"
#include "ext2TestApi.h"
#include "hfsTestApi.h"

#include "../include/utils.h"
#include "../include/attributes.h"
#include "../include/commonTests.h"

void shouldCreateABigFile()
{
    char* disksDirectory = new char[20];
    memcpy(disksDirectory, "Test_1\\\0", 20);
    char* fat32Path = new char[500];
    char* ext2Path = new char[500];
    char* hfsPath = new char[500];
    buildAllHardDiskPaths(disksDirectory, fat32Path, ext2Path, hfsPath);

    uint64_t bufferSize = 80000000;
    char* buffer = generateBuffer(bufferSize);
    uint32_t sectorsNumber = 204800;
    uint32_t sectorSize = 512;
    char* parentPath = new char[50];
    memcpy(parentPath, "Root\0", 50);
    char* fileName = new char[50];
    memcpy(fileName, "File_1\0", 50);
    char* fullFilePath = new char[50];
    memcpy(fullFilePath, "Root/File_1\0", 50);
    char* parentPathCopy = new char[50];
    char* fileNameCopy = new char[50];
    char* fullFilePathCopy = new char[50];
    int64_t timeElapsedMilliseconds;
    uint32_t numberOfBytesWritten, reasonForIncompleteWrite, result;

    //FAT32
    DiskInfo* fat32_diskInfo;
    BootSector* fat32_bootSector;
    FsInfo* fat32_fsInfo;
    uint32_t fat32_sectorsPerCluster = 16;
    initializeFAT32(fat32Path, &fat32_diskInfo, &fat32_bootSector, &fat32_fsInfo, sectorsNumber, sectorSize,
                    fat32_sectorsPerCluster);

    memcpy(parentPathCopy, parentPath, 50);
    memcpy(fileNameCopy, fileName, 50);
    memcpy(fullFilePathCopy, fullFilePath, 50);

    result = fat32_create_directory(fat32_diskInfo, fat32_bootSector, parentPathCopy, fileNameCopy,
                                             FILE, timeElapsedMilliseconds);
    result = fat32_write_file(fat32_diskInfo, fat32_bootSector, fullFilePathCopy, buffer, bufferSize, TRUNCATE,
                              numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);
    std::cout << result << "\n";
    printDuration(timeElapsedMilliseconds, FAT32);

    //Ext2
    DiskInfo* ext2_diskInfo;
    ext2_super_block* ext2_superBlock;
    uint32_t ext2_blockSize = 8192;
    initializeExt2(ext2Path, &ext2_diskInfo, &ext2_superBlock, sectorsNumber, sectorSize, ext2_blockSize);

    memcpy(parentPathCopy, parentPath, 50);
    memcpy(fileNameCopy, fileName, 50);
    memcpy(fullFilePathCopy, fullFilePath, 50);

    result = ext2_create_directory(ext2_diskInfo, ext2_superBlock, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
    result = ext2_write_file(ext2_diskInfo, ext2_superBlock, fullFilePathCopy, buffer, bufferSize, TRUNCATE,
                             numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);
    std::cout << result << "\n";
    printDuration(timeElapsedMilliseconds, EXT2);

    //HFS
    DiskInfo* hfs_diskInfo;
    HFSPlusVolumeHeader* hfs_volumeHeader;
    ExtentsFileHeaderNode* hfs_extentsFileHeaderNode;
    CatalogFileHeaderNode* hfs_catalogFileHeaderNode;
    uint32_t hfs_blockSize = 8192;
    initializeHFS(hfsPath, &hfs_diskInfo, &hfs_volumeHeader, &hfs_extentsFileHeaderNode,
                  &hfs_catalogFileHeaderNode, sectorsNumber, sectorSize, hfs_blockSize);

    memcpy(parentPathCopy, parentPath, 50);
    memcpy(fileNameCopy, fileName, 50);
    memcpy(fullFilePathCopy, fullFilePath, 50);

    result = hfs_create_directory(hfs_diskInfo, hfs_volumeHeader, hfs_catalogFileHeaderNode, parentPathCopy,
                                  fileNameCopy, FILE, timeElapsedMilliseconds);
    result = hfs_write_file(hfs_diskInfo, hfs_volumeHeader, hfs_catalogFileHeaderNode, hfs_extentsFileHeaderNode,
                   fullFilePathCopy, buffer, bufferSize, TRUNCATE, numberOfBytesWritten, reasonForIncompleteWrite,
                   timeElapsedMilliseconds);
    std::cout << result << "\n";
    printDuration(timeElapsedMilliseconds, HFS);
}