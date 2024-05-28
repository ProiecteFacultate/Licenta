#include "iostream"
#include "iomanip"

#include "fat32TestApi.h"
#include "ext2TestApi.h"
#include "hfsTestApi.h"

#include "../include/utils.h"
#include "../include/attributes.h"
#include "../include/commonTests.h"

void common_time_test_1() {
    char* diskPath = new char[100];
    memcpy(diskPath, "D:\\Facultate\\Licenta\\HardDisks\\Automatic_Test_Solo\0", 100);

    uint32_t fileSizes[] = {1024, 1024 * 100, 1024 * 1024, 1024 * 1024 * 10, 1024 * 1024 * 20, 1024 * 1024 * 50,
                            1024 * 1024 * 100, 1024 * 1024 * 200,};
    std::string fileSizesInString[] = {"1Kib", "100Kib", "1Mib", "10Mib", "20Mib", "50Mib", "100Mib", "200Mib"};

    uint32_t fat32_sectorsPerCluster = 16;
    uint32_t ext2_blockSize = 8192;
    uint32_t hfs_blockSize = 8192;

    uint32_t sectorSize = 512;
    char *parentPath = new char[50];
    memcpy(parentPath, "Root\0", 50);
    char *fileName = new char[50];
    memcpy(fileName, "File_1\0", 50);
    char *fullFilePath = new char[50];
    memcpy(fullFilePath, "Root/File_1\0", 50);
    char *parentPathCopy = new char[50];
    char *fileNameCopy = new char[50];
    char *fullFilePathCopy = new char[50];
    int64_t timeElapsedMilliseconds;
    uint32_t numberOfBytesWritten, reasonForIncompleteWrite, result;

    std::cout << std::setprecision(0) << std::fixed;

    for (int i = 0; i < 8; i++)
    {
        std:: cout << "\n------------------------------- " << fileSizesInString[i] << " -------------------------------\n";

        uint32_t diskSize = fileSizes[i] + 1024 * 1024 * 100;
        uint32_t sectorsNumber = diskSize / sectorSize;

        uint32_t bufferSize = fileSizes[i];
        char *buffer = generateBuffer(bufferSize);

        //FAT32
        deleteFiles(diskPath);

        DiskInfo *fat32_diskInfo;
        BootSector *fat32_bootSector;
        FsInfo *fat32_fsInfo;

        initializeFAT32(diskPath, &fat32_diskInfo, &fat32_bootSector, &fat32_fsInfo, sectorsNumber, sectorSize,
                        fat32_sectorsPerCluster);

        memcpy(parentPathCopy, parentPath, 50);
        memcpy(fileNameCopy, fileName, 50);
        memcpy(fullFilePathCopy, fullFilePath, 50);

        result = fat32_create_directory(fat32_diskInfo, fat32_bootSector, parentPathCopy, fileNameCopy,
                                        FILE, timeElapsedMilliseconds);
        result = fat32_write_file(fat32_diskInfo, fat32_bootSector, fullFilePathCopy, buffer, bufferSize, TRUNCATE,
                                  numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

        double bytesPerSecond = ((double) numberOfBytesWritten / (double) timeElapsedMilliseconds) * 1000;
        std::cout << "FAT32 --- Number of bytes written: " <<  numberOfBytesWritten << "/" << bufferSize << " (" << fileSizesInString[i] << ") --- "
                  << bytesPerSecond << " bytes/second --- ";
        printDurationSolo(timeElapsedMilliseconds);

        /////////////////////

        //Ext2
        deleteFiles(diskPath);

        DiskInfo *ext2_diskInfo;
        ext2_super_block *ext2_superBlock;
        initializeExt2(diskPath, &ext2_diskInfo, &ext2_superBlock, sectorsNumber, sectorSize, ext2_blockSize);

        memcpy(parentPathCopy, parentPath, 50);
        memcpy(fileNameCopy, fileName, 50);
        memcpy(fullFilePathCopy, fullFilePath, 50);
        result = ext2_create_directory(ext2_diskInfo, ext2_superBlock, parentPathCopy, fileNameCopy, FILE,
                                       timeElapsedMilliseconds);
        result = ext2_write_file(ext2_diskInfo, ext2_superBlock, fullFilePathCopy, buffer, bufferSize, TRUNCATE,
                                 numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

        bytesPerSecond = ((double) numberOfBytesWritten / (double) timeElapsedMilliseconds) * 1000;
        std::cout << "Ext2 --- Number of bytes written: " <<  numberOfBytesWritten << "/" << bufferSize << " (" << fileSizesInString[i] << ") --- "
                  << bytesPerSecond << " bytes/second --- ";
        printDurationSolo(timeElapsedMilliseconds);

        /////////////////////

        //HFS
        deleteFiles(diskPath);

        DiskInfo *hfs_diskInfo;
        HFSPlusVolumeHeader *hfs_volumeHeader;
        ExtentsFileHeaderNode *hfs_extentsFileHeaderNode;
        CatalogFileHeaderNode *hfs_catalogFileHeaderNode;
        initializeHFS(diskPath, &hfs_diskInfo, &hfs_volumeHeader, &hfs_extentsFileHeaderNode,
                      &hfs_catalogFileHeaderNode, sectorsNumber, sectorSize, hfs_blockSize);

        memcpy(parentPathCopy, parentPath, 50);
        memcpy(fileNameCopy, fileName, 50);
        memcpy(fullFilePathCopy, fullFilePath, 50);

        result = hfs_create_directory(hfs_diskInfo, hfs_volumeHeader, hfs_catalogFileHeaderNode, parentPathCopy,
                                      fileNameCopy, FILE, timeElapsedMilliseconds);
        result = hfs_write_file(hfs_diskInfo, hfs_volumeHeader, hfs_catalogFileHeaderNode, hfs_extentsFileHeaderNode,
                                fullFilePathCopy, buffer, bufferSize, TRUNCATE, numberOfBytesWritten,
                                reasonForIncompleteWrite,
                                timeElapsedMilliseconds);

        bytesPerSecond = ((double) numberOfBytesWritten / (double) timeElapsedMilliseconds) * 1000;
        std::cout << "HFS+ --- Number of bytes written: " <<  numberOfBytesWritten << "/" << bufferSize << " (" << fileSizesInString[i] << ") --- "
                  << bytesPerSecond << " bytes/second --- ";
        printDurationSolo(timeElapsedMilliseconds);
    }
}

void common_time_test_2() {
    char* diskPath = new char[100];
    memcpy(diskPath, "D:\\Facultate\\Licenta\\HardDisks\\Automatic_Test_Solo\0", 100);

    uint32_t fileSizes[] = {1024, 1024 * 100, 1024 * 1024, 1024 * 1024 * 10, 1024 * 1024 * 20, 1024 * 1024 * 50,
                            1024 * 1024 * 100, 1024 * 1024 * 200,};
    std::string fileSizesInString[] = {"1Kib", "100Kib", "1Mib", "10Mib", "20Mib", "50Mib", "100Mib", "200Mib"};

    uint32_t fat32_sectorsPerCluster = 16;
    uint32_t ext2_blockSize = 8192;
    uint32_t hfs_blockSize = 8192;

    uint32_t sectorSize = 512;
    char *parentPath = new char[50];
    memcpy(parentPath, "Root\0", 50);
    char *fileName = new char[50];
    memcpy(fileName, "File_1\0", 50);
    char *fullFilePath = new char[50];
    memcpy(fullFilePath, "Root/File_1\0", 50);
    char *parentPathCopy = new char[50];
    char *fileNameCopy = new char[50];
    char *fullFilePathCopy = new char[50];
    int64_t timeElapsedMilliseconds;
    uint32_t numberOfBytesWritten, reasonForIncompleteWrite, result;

    std::cout << std::setprecision(0) << std::fixed;

    for (int i = 0; i < 8; i++)
    {
        std:: cout << "\n------------------------------- " << fileSizesInString[i] << " -------------------------------\n";

        uint32_t diskSize = fileSizes[i] + 1024 * 1024 * 100;
        uint32_t sectorsNumber = diskSize / sectorSize;

        uint32_t bufferSize = fileSizes[i];
        char *buffer = generateBuffer(bufferSize);

        //FAT32
        deleteFiles(diskPath);

        DiskInfo *fat32_diskInfo;
        BootSector *fat32_bootSector;
        FsInfo *fat32_fsInfo;

        initializeFAT32(diskPath, &fat32_diskInfo, &fat32_bootSector, &fat32_fsInfo, sectorsNumber, sectorSize,
                        fat32_sectorsPerCluster);

        memcpy(parentPathCopy, parentPath, 50);
        memcpy(fileNameCopy, fileName, 50);
        memcpy(fullFilePathCopy, fullFilePath, 50);

        result = fat32_create_directory(fat32_diskInfo, fat32_bootSector, parentPathCopy, fileNameCopy,
                                        FILE, timeElapsedMilliseconds);
        result = fat32_write_file(fat32_diskInfo, fat32_bootSector, fullFilePathCopy, buffer, bufferSize, TRUNCATE,
                                  numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

        memcpy(fullFilePathCopy, fullFilePath, 50);
        result = fat32_write_file(fat32_diskInfo, fat32_bootSector, fullFilePathCopy, buffer, bufferSize, TRUNCATE,
                                  numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

        double bytesPerSecond = ((double) numberOfBytesWritten / (double) timeElapsedMilliseconds) * 1000;
        std::cout << "FAT32 --- Number of bytes written: " <<  numberOfBytesWritten << "/" << bufferSize << " (" << fileSizesInString[i] << ") --- "
                  << bytesPerSecond << " bytes/second --- ";
        printDurationSolo(timeElapsedMilliseconds);

        /////////////////////

        //Ext2
        deleteFiles(diskPath);

        DiskInfo *ext2_diskInfo;
        ext2_super_block *ext2_superBlock;
        initializeExt2(diskPath, &ext2_diskInfo, &ext2_superBlock, sectorsNumber, sectorSize, ext2_blockSize);

        memcpy(parentPathCopy, parentPath, 50);
        memcpy(fileNameCopy, fileName, 50);
        memcpy(fullFilePathCopy, fullFilePath, 50);
        result = ext2_create_directory(ext2_diskInfo, ext2_superBlock, parentPathCopy, fileNameCopy, FILE,
                                       timeElapsedMilliseconds);
        result = ext2_write_file(ext2_diskInfo, ext2_superBlock, fullFilePathCopy, buffer, bufferSize, TRUNCATE,
                                 numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

        memcpy(fullFilePathCopy, fullFilePath, 50);
        result = ext2_write_file(ext2_diskInfo, ext2_superBlock, fullFilePathCopy, buffer, bufferSize, TRUNCATE,
                                 numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

        bytesPerSecond = ((double) numberOfBytesWritten / (double) timeElapsedMilliseconds) * 1000;
        std::cout << "Ext2 --- Number of bytes written: " <<  numberOfBytesWritten << "/" << bufferSize << " (" << fileSizesInString[i] << ") --- "
                  << bytesPerSecond << " bytes/second --- ";
        printDurationSolo(timeElapsedMilliseconds);

        /////////////////////

        //HFS
        deleteFiles(diskPath);

        DiskInfo *hfs_diskInfo;
        HFSPlusVolumeHeader *hfs_volumeHeader;
        ExtentsFileHeaderNode *hfs_extentsFileHeaderNode;
        CatalogFileHeaderNode *hfs_catalogFileHeaderNode;
        initializeHFS(diskPath, &hfs_diskInfo, &hfs_volumeHeader, &hfs_extentsFileHeaderNode,
                      &hfs_catalogFileHeaderNode, sectorsNumber, sectorSize, hfs_blockSize);

        memcpy(parentPathCopy, parentPath, 50);
        memcpy(fileNameCopy, fileName, 50);
        memcpy(fullFilePathCopy, fullFilePath, 50);

        result = hfs_create_directory(hfs_diskInfo, hfs_volumeHeader, hfs_catalogFileHeaderNode, parentPathCopy,
                                      fileNameCopy, FILE, timeElapsedMilliseconds);
        result = hfs_write_file(hfs_diskInfo, hfs_volumeHeader, hfs_catalogFileHeaderNode, hfs_extentsFileHeaderNode,
                                fullFilePathCopy, buffer, bufferSize, TRUNCATE, numberOfBytesWritten, reasonForIncompleteWrite,
                                timeElapsedMilliseconds);

        memcpy(fullFilePathCopy, fullFilePath, 50);
        result = hfs_write_file(hfs_diskInfo, hfs_volumeHeader, hfs_catalogFileHeaderNode, hfs_extentsFileHeaderNode,
                                fullFilePathCopy, buffer, bufferSize, TRUNCATE, numberOfBytesWritten, reasonForIncompleteWrite,
                                timeElapsedMilliseconds);

        bytesPerSecond = ((double) numberOfBytesWritten / (double) timeElapsedMilliseconds) * 1000;
        std::cout << "HFS+ --- Number of bytes written: " <<  numberOfBytesWritten << "/" << bufferSize << " (" << fileSizesInString[i] << ") --- "
                  << bytesPerSecond << " bytes/second --- ";
        printDurationSolo(timeElapsedMilliseconds);
    }
}

void common_time_test_3()
{
    char* diskPath = new char[100];
    memcpy(diskPath, "D:\\Facultate\\Licenta\\HardDisks\\Automatic_Test_Solo\0", 100);

    uint32_t fileSizes[] = {1024, 1024 * 10, 1024 * 100, 1024 * 200, 1024 * 500, 1024 * 1024, 1024 * 1024 * 2};
    std::string fileSizesInString[] = {"1Kib", "10Kib", "100Kib", "200Kib", "500Kib", "1Mib", "2Mib"};
    std::string totalFileSizesInString[] = {"100Kib", "1Mib", "10Mib", "20Mib", "50Mib", "100Mib", "200Mib"}; //we assume 100 files
    uint32_t numberOfFiles = 100;

    uint32_t fat32_sectorsPerCluster = 16;
    uint32_t ext2_blockSize = 8192;
    uint32_t hfs_blockSize = 8192;

    uint32_t sectorSize = 512;
    char *parentPath = new char[50];
    memcpy(parentPath, "Root\0", 50);
    char *fileName = new char[50];
    memcpy(fileName, "File_1\0", 50);
    char *fullFilePath = new char[50];
    memcpy(fullFilePath, "Root/File_1\0", 50);
    char *parentPathCopy = new char[50];
    char *fileNameCopy = new char[50];
    char *fullFilePathCopy = new char[50];
    int64_t timeElapsedMilliseconds;
    uint32_t numberOfBytesWritten, reasonForIncompleteWrite, result;

    std::cout << std::setprecision(0) << std::fixed;

    for (int i = 0; i < 7; i++)
    {
        std:: cout << "\n------------------------------- " << fileSizesInString[i] << "/file -------------------------------\n";

        uint32_t diskSize = fileSizes[i] * numberOfFiles + 1024 * 1024 * 100;
        uint32_t sectorsNumber = diskSize / sectorSize;

        uint32_t bufferSize = fileSizes[i];
        char *buffer = generateBuffer(bufferSize);

        //FAT32
        uint32_t totalBytesWritten = 0, totalMilliseconds = 0;
        deleteFiles(diskPath);

        DiskInfo *fat32_diskInfo;
        BootSector *fat32_bootSector;
        FsInfo *fat32_fsInfo;
        initializeFAT32(diskPath, &fat32_diskInfo, &fat32_bootSector, &fat32_fsInfo, sectorsNumber, sectorSize,
                        fat32_sectorsPerCluster);

        for(uint32_t j = 0; j < numberOfFiles; j++)
        {
            memcpy(parentPathCopy, parentPath, 50);
            memcpy(fileNameCopy, fileName, 50);
            memcpy(fullFilePathCopy, fullFilePath, 50);

            result = fat32_create_directory(fat32_diskInfo, fat32_bootSector, parentPathCopy, fileNameCopy,
                                            FILE, timeElapsedMilliseconds);
            result = fat32_write_file(fat32_diskInfo, fat32_bootSector, fullFilePathCopy, buffer, bufferSize, TRUNCATE,
                                      numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

            totalBytesWritten += numberOfBytesWritten;
            totalMilliseconds += timeElapsedMilliseconds;
        }

        double bytesPerSecond = ((double) totalBytesWritten / (double) totalMilliseconds) * 1000;
        std::cout << "FAT32 --- Number of bytes written: " << totalBytesWritten << "/" << bufferSize * numberOfFiles << " ("
                  << totalFileSizesInString[i] << " total) --- " << bytesPerSecond << " bytes/second --- ";
        printDurationSolo(totalMilliseconds);

        /////////////////////

        //Ext2
        totalBytesWritten = 0, totalMilliseconds = 0;
        deleteFiles(diskPath);

        DiskInfo *ext2_diskInfo;
        ext2_super_block *ext2_superBlock;
        initializeExt2(diskPath, &ext2_diskInfo, &ext2_superBlock, sectorsNumber, sectorSize, ext2_blockSize);

        for(uint32_t j = 0; j < numberOfFiles; j++)
        {
            memcpy(parentPathCopy, parentPath, 50);
            memcpy(fileNameCopy, fileName, 50);
            memcpy(fullFilePathCopy, fullFilePath, 50);
            result = ext2_create_directory(ext2_diskInfo, ext2_superBlock, parentPathCopy, fileNameCopy, FILE,
                                           timeElapsedMilliseconds);
            result = ext2_write_file(ext2_diskInfo, ext2_superBlock, fullFilePathCopy, buffer, bufferSize, TRUNCATE,
                                     numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

            totalBytesWritten += numberOfBytesWritten;
            totalMilliseconds += timeElapsedMilliseconds;
        }

        bytesPerSecond = ((double) totalBytesWritten / (double) totalMilliseconds) * 1000;
        std::cout << "Ext2 --- Number of bytes written: " << totalBytesWritten << "/" << bufferSize * numberOfFiles << " ("
                  << totalFileSizesInString[i] << " total) --- " << bytesPerSecond << " bytes/second --- ";
        printDurationSolo(totalMilliseconds);

        /////////////////////

        //HFS
        totalBytesWritten = 0, totalMilliseconds = 0;
        deleteFiles(diskPath);

        DiskInfo *hfs_diskInfo;
        HFSPlusVolumeHeader *hfs_volumeHeader;
        ExtentsFileHeaderNode *hfs_extentsFileHeaderNode;
        CatalogFileHeaderNode *hfs_catalogFileHeaderNode;
        initializeHFS(diskPath, &hfs_diskInfo, &hfs_volumeHeader, &hfs_extentsFileHeaderNode,
                      &hfs_catalogFileHeaderNode, sectorsNumber, sectorSize, hfs_blockSize);

        for(uint32_t j = 0; j < numberOfFiles; j++) {
            memcpy(parentPathCopy, parentPath, 50);
            memcpy(fileNameCopy, fileName, 50);
            memcpy(fullFilePathCopy, fullFilePath, 50);

            result = hfs_create_directory(hfs_diskInfo, hfs_volumeHeader, hfs_catalogFileHeaderNode, parentPathCopy,
                                          fileNameCopy, FILE, timeElapsedMilliseconds);
            result = hfs_write_file(hfs_diskInfo, hfs_volumeHeader, hfs_catalogFileHeaderNode,
                                    hfs_extentsFileHeaderNode,
                                    fullFilePathCopy, buffer, bufferSize, TRUNCATE, numberOfBytesWritten,
                                    reasonForIncompleteWrite,
                                    timeElapsedMilliseconds);

            totalBytesWritten += numberOfBytesWritten;
            totalMilliseconds += timeElapsedMilliseconds;
        }

        bytesPerSecond = ((double) totalBytesWritten / (double) totalMilliseconds) * 1000;
        std::cout << "HFS+ --- Number of bytes written: " << totalBytesWritten << "/" << bufferSize * numberOfFiles << " ("
                  << totalFileSizesInString[i] << " total) --- " << bytesPerSecond << " bytes/second --- ";
        printDurationSolo(totalMilliseconds);
    }
}