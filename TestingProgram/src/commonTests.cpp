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

    uint32_t fileSizes[] = {1024 * 1024, 1024 * 1024 * 5, 1024 * 1024 * 10, 1024 * 1024 * 25, 1024 * 1024 * 50, 1024 * 1024 * 75, 1024 * 1024 * 100};
    std::string fileSizesInString[] = {"1Mib", "5Mib", "10Mib", "25Mib", "50Mib", "75Mib", "100Mib"};

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

    uint32_t fileSizes[] = {1024 * 1024, 1024 * 1024 * 5, 1024 * 1024 * 10, 1024 * 1024 * 25, 1024 * 1024 * 50, 1024 * 1024 * 75, 1024 * 1024 * 100};
    std::string fileSizesInString[] = {"1Mib", "5Mib", "10Mib", "25Mib", "50Mib", "75Mib", "100Mib"};

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

    uint32_t fileSizes[] = {1024 * 10,  1024 * 50, 1024 * 100, 1024 * 250, 1024 * 500, 1024 * 750, 1024 * 1024};
    std::string fileSizesInString[] = { "10Kib", "50Kib", "100Kib", "250Kib", "500Kib", "750Kib", "1Mib"};
    std::string totalFileSizesInString[] = {"~1000Kib", "~4,9Mib", "~9,8Mib", "~24,4Mib", "~48,8Mib", "~73,2Mib", "100Mib"}; //we assume 100 files
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

void common_time_test_4()
{
    char* diskPath = new char[100];
    memcpy(diskPath, "D:\\Facultate\\Licenta\\HardDisks\\Automatic_Test_Solo\0", 100);

    uint32_t fileSizes[] = {1024 * 1024, 1024 * 1024 * 5, 1024 * 1024 * 10, 1024 * 1024 * 25, 1024 * 1024 * 50, 1024 * 1024 * 75, 1024 * 1024 * 100};
    std::string fileSizesInString[] = {"1Mib", "5Mib", "10Mib", "25Mib", "50Mib", "75Mib", "100Mib"};

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
    uint32_t numberOfBytesWrittenWithTruncate, numberOfBytesWrittenWithAppend, reasonForIncompleteWrite, result;

    std::cout << std::setprecision(0) << std::fixed;

    for (int i = 0; i < 7; i++)
    {
        std:: cout << "\n------------------------------- " << fileSizesInString[i] << " -------------------------------\n";

        uint32_t diskSize = fileSizes[i] * 2 + 1024 * 1024 * 100;
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
                                  numberOfBytesWrittenWithTruncate, reasonForIncompleteWrite, timeElapsedMilliseconds);

        memcpy(fullFilePathCopy, fullFilePath, 50);
        result = fat32_write_file(fat32_diskInfo, fat32_bootSector, fullFilePathCopy, buffer, bufferSize, APPEND,
                                  numberOfBytesWrittenWithAppend, reasonForIncompleteWrite, timeElapsedMilliseconds);

        double bytesPerSecond = ((double) numberOfBytesWrittenWithAppend / (double) timeElapsedMilliseconds) * 1000;
        std::cout << "FAT32 --- Number of bytes written: " <<  numberOfBytesWrittenWithTruncate << "-" << numberOfBytesWrittenWithAppend << " (truncate-append)/"
                  << bufferSize << " (" << fileSizesInString[i] << ") --- " << bytesPerSecond << " bytes/second --- ";
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
                                 numberOfBytesWrittenWithTruncate, reasonForIncompleteWrite, timeElapsedMilliseconds);

        memcpy(fullFilePathCopy, fullFilePath, 50);
        result = ext2_write_file(ext2_diskInfo, ext2_superBlock, fullFilePathCopy, buffer, bufferSize, APPEND,
                                 numberOfBytesWrittenWithAppend, reasonForIncompleteWrite, timeElapsedMilliseconds);

        bytesPerSecond = ((double) numberOfBytesWrittenWithAppend / (double) timeElapsedMilliseconds) * 1000;
        std::cout << "Ext2 --- Number of bytes written: " <<  numberOfBytesWrittenWithTruncate << "-" << numberOfBytesWrittenWithAppend << " (truncate-append)/"
                  << bufferSize << " (" << fileSizesInString[i] << ") --- " << bytesPerSecond << " bytes/second --- ";
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
                                fullFilePathCopy, buffer, bufferSize, TRUNCATE, numberOfBytesWrittenWithTruncate, reasonForIncompleteWrite,
                                timeElapsedMilliseconds);

        memcpy(fullFilePathCopy, fullFilePath, 50);
        result = hfs_write_file(hfs_diskInfo, hfs_volumeHeader, hfs_catalogFileHeaderNode, hfs_extentsFileHeaderNode,
                                fullFilePathCopy, buffer, bufferSize, APPEND, numberOfBytesWrittenWithAppend, reasonForIncompleteWrite,
                                timeElapsedMilliseconds);
        bytesPerSecond = ((double) numberOfBytesWrittenWithAppend / (double) timeElapsedMilliseconds) * 1000;
        std::cout << "HFS+ --- Number of bytes written: " <<  numberOfBytesWrittenWithTruncate << "-" << numberOfBytesWrittenWithAppend << " (truncate-append)/"
                  << bufferSize << " (" << fileSizesInString[i] << ") --- " << bytesPerSecond << " bytes/second --- ";
        printDurationSolo(timeElapsedMilliseconds);
    }
}

void common_time_test_5()
{
    char* diskPath = new char[100];
    memcpy(diskPath, "D:\\Facultate\\Licenta\\HardDisks\\Automatic_Test_Solo\0", 100);

    uint32_t fileSizes[] = { 1024 * 1024, 1024 * 1024 * 5, 1024 * 1024 * 10, 1024 * 1024 * 25, 1024 * 1024 * 50, 1024 * 1024 * 75, 1024 * 1024 * 100};
    std::string fileSizesInString[] = { "1Mib", "5Mib", "10Mib", "25Mib", "50Mib", "75Mib", "100Mib" };
    std::string fileSizesInStringPerWrite[] = {"64Kib", "320Kib", "640Kib", "~1,6Mib", "~3,2Mib", "~4,8Mib",  "~6,2Mib"}; //this is for 16 writes

    uint32_t fat32_sectorsPerCluster = 16;
    uint32_t ext2_blockSize = 8192;
    uint32_t hfs_blockSize = 8192;
    uint32_t numberOfWrites = 16;
    uint64_t smallBufferSize = 5000;
    uint32_t sectorSize = 512;

    char* parentPath = new char[50];
    memcpy(parentPath, "Root\0", 50);
    char* fileName = new char[50];
    memcpy(fileName, "File_\0\0\0\0", 50);
    char* fullFilePath = new char[50];
    memcpy(fullFilePath, "Root/File_\0\0\0\0", 50);
    char* bigFileName = new char[50];
    memcpy(bigFileName, "BigFile\0\0\0\0", 50);
    char* bigFileFullPath = new char[50];
    memcpy(bigFileFullPath, "Root/BigFile\0\0\0\0", 50);
    char* parentPathCopy = new char[50];
    char* fileNameCopy = new char[50];
    char* fullFilePathCopy = new char[50];
    char* bigFileNameCopy = new char[50];
    char* bigFileFullPathCopy = new char[50];

    int64_t timeElapsedMilliseconds;
    uint32_t numberOfBytesWritten, reasonForIncompleteWrite, result, writeMode;

    std::cout << std::setprecision(0) << std::fixed;

    for (int i = 0; i < 7; i++)
    {
        std:: cout << "\n------------------------------- " << fileSizesInStringPerWrite[i] << "/write -------------------------------\n";

        uint32_t diskSize = fileSizes[i] + 1024 * 1024 * 100;
        uint32_t sectorsNumber = diskSize / sectorSize;

        uint32_t bufferSize = fileSizes[i] / numberOfWrites;
        char *buffer = generateBuffer(bufferSize);
        char *smallBuffer = generateBuffer(smallBufferSize);

        //FAT32
        uint32_t totalBytesWritten = 0, totalMilliseconds = 0;
        deleteFiles(diskPath);

        DiskInfo *fat32_diskInfo;
        BootSector *fat32_bootSector;
        FsInfo *fat32_fsInfo;

        initializeFAT32(diskPath, &fat32_diskInfo, &fat32_bootSector, &fat32_fsInfo, sectorsNumber, sectorSize, fat32_sectorsPerCluster);

        memcpy(parentPathCopy, parentPath, 50);
        memcpy(bigFileNameCopy, bigFileName, 50);
        result = fat32_create_directory(fat32_diskInfo, fat32_bootSector, parentPathCopy, bigFileNameCopy, FILE, timeElapsedMilliseconds);

        for(int j = 0; j < numberOfWrites; j++)
        {
            //big file
            memcpy(bigFileFullPathCopy, bigFileFullPath, 50);
            writeMode = (j == 0) ? TRUNCATE : APPEND;

            result = fat32_write_file(fat32_diskInfo, fat32_bootSector, bigFileFullPathCopy, buffer, bufferSize, writeMode,
                                      numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

            totalBytesWritten += numberOfBytesWritten;
            totalMilliseconds += timeElapsedMilliseconds;

            //small file
            memcpy(parentPathCopy, parentPath, 50);
            memcpy(fileNameCopy, fileName, 50);
            memcpy(fullFilePathCopy, fullFilePath, 50);

            memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(j).c_str(), 2);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(j).c_str(), 2);

            result = fat32_create_directory(fat32_diskInfo, fat32_bootSector, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
            result = fat32_write_file(fat32_diskInfo, fat32_bootSector, fullFilePathCopy, buffer, bufferSize, TRUNCATE,
                                      numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);
        }

        double bytesPerSecond = ((double) totalBytesWritten / (double) totalMilliseconds) * 1000;
        std::cout << "FAT32 --- Number of bytes written: " <<  totalBytesWritten << "/" << fileSizes[i]
                  << " (" << fileSizesInString[i] << ") --- " << bytesPerSecond << " bytes/second --- ";
        printDurationSolo(totalMilliseconds);

        /////////////////////

        //Ext2
        totalBytesWritten = 0, totalMilliseconds = 0;
        deleteFiles(diskPath);

        DiskInfo *ext2_diskInfo;
        ext2_super_block *ext2_superBlock;
        initializeExt2(diskPath, &ext2_diskInfo, &ext2_superBlock, sectorsNumber, sectorSize, ext2_blockSize);

        memcpy(parentPathCopy, parentPath, 50);
        memcpy(bigFileNameCopy, bigFileName, 50);
        result = ext2_create_directory(ext2_diskInfo, ext2_superBlock, parentPathCopy, bigFileNameCopy, FILE,timeElapsedMilliseconds);

        for(int j = 0; j < numberOfWrites; j++)
        {
            //big file
            memcpy(bigFileFullPathCopy, bigFileFullPath, 50);
            writeMode = (j == 0) ? TRUNCATE : APPEND;

            result = ext2_write_file(ext2_diskInfo, ext2_superBlock, bigFileFullPathCopy, buffer, bufferSize, writeMode,
                                     numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

            totalBytesWritten += numberOfBytesWritten;
            totalMilliseconds += timeElapsedMilliseconds;

            //small file
            memcpy(parentPathCopy, parentPath, 50);
            memcpy(fileNameCopy, fileName, 50);
            memcpy(fullFilePathCopy, fullFilePath, 50);

            memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(j).c_str(), 2);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(j).c_str(), 2);

            result = ext2_create_directory(ext2_diskInfo, ext2_superBlock, parentPathCopy, fileNameCopy, FILE,timeElapsedMilliseconds);
            result = ext2_write_file(ext2_diskInfo, ext2_superBlock, fullFilePathCopy, smallBuffer, smallBufferSize, TRUNCATE,
                                     numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);
        }

        bytesPerSecond = ((double) totalBytesWritten / (double) totalMilliseconds) * 1000;
        std::cout << "Ext2 --- Number of bytes written: " <<  totalBytesWritten << "/" << fileSizes[i]
                  << " (" << fileSizesInString[i] << ") --- " << bytesPerSecond << " bytes/second --- ";
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

        memcpy(parentPathCopy, parentPath, 50);
        memcpy(bigFileNameCopy, bigFileName, 50);
        result = hfs_create_directory(hfs_diskInfo, hfs_volumeHeader, hfs_catalogFileHeaderNode, parentPathCopy,
                                      bigFileNameCopy, FILE, timeElapsedMilliseconds);

        for(int j = 0; j < numberOfWrites; j++)
        {
            //big file
            memcpy(bigFileFullPathCopy, bigFileFullPath, 50);
            writeMode = (j == 0) ? TRUNCATE : APPEND;

            result = hfs_write_file(hfs_diskInfo, hfs_volumeHeader, hfs_catalogFileHeaderNode, hfs_extentsFileHeaderNode,
                                    bigFileFullPathCopy, buffer, bufferSize, writeMode, numberOfBytesWritten,
                                    reasonForIncompleteWrite,timeElapsedMilliseconds);

            totalBytesWritten += numberOfBytesWritten;
            totalMilliseconds += timeElapsedMilliseconds;

            //small file
            memcpy(parentPathCopy, parentPath, 50);
            memcpy(fileNameCopy, fileName, 50);
            memcpy(fullFilePathCopy, fullFilePath, 50);

            memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(j).c_str(), 2);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(j).c_str(), 2);

            result = hfs_create_directory(hfs_diskInfo, hfs_volumeHeader, hfs_catalogFileHeaderNode, parentPathCopy,
                                          fileNameCopy, FILE, timeElapsedMilliseconds);
            result = hfs_write_file(hfs_diskInfo, hfs_volumeHeader, hfs_catalogFileHeaderNode,
                                    hfs_extentsFileHeaderNode, fullFilePathCopy, smallBuffer, smallBufferSize, TRUNCATE,
                                    numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);
        }

        bytesPerSecond = ((double) totalBytesWritten / (double) totalMilliseconds) * 1000;
        std::cout << "HFS+ --- Number of bytes written: " <<  totalBytesWritten << "/" << fileSizes[i]
                  << " (" << fileSizesInString[i] << ") --- " << bytesPerSecond << " bytes/second --- ";
        printDurationSolo(totalMilliseconds);
    }
}

void common_time_test_6() {
    char* diskPath = new char[100];
    memcpy(diskPath, "D:\\Facultate\\Licenta\\HardDisks\\Automatic_Test_Solo\0", 100);

    uint32_t fileSizes[] = {1024 * 1024, 1024 * 1024 * 5, 1024 * 1024 * 10, 1024 * 1024 * 25, 1024 * 1024 * 50, 1024 * 1024 * 75, 1024 * 1024 * 100};
    std::string fileSizesInString[] = {"1Mib", "5Mib", "10Mib", "25Mib", "50Mib", "75Mib", "100Mib"};

    uint32_t fat32_sectorsPerCluster = 16;
    uint32_t ext2_blockSize = 8192;
    uint32_t hfs_blockSize = 8192;

    uint32_t sectorSize = 512;
    char* parentPath = new char[50];
    memcpy(parentPath, "Root\0", 50);
    char* fileName = new char[50];
    memcpy(fileName, "File_\0\0\0\0", 50);
    char* fullFilePath = new char[50];
    memcpy(fullFilePath, "Root/File_\0\0\0\0", 50);
    char* bigFileName = new char[50];
    memcpy(bigFileName, "BigFile\0\0\0\0", 50);
    char* parentPathCopy = new char[50];
    char* fileNameCopy = new char[50];
    char* fullFilePathCopy = new char[50];
    int64_t timeElapsedMilliseconds;
    uint32_t numberOfBytesWritten, numberOfBytesRead, reasonForIncompleteOperation, result;

    std::cout << std::setprecision(0) << std::fixed;

    for (int i = 0; i < 7; i++)
    {
        std:: cout << "\n------------------------------- " << fileSizesInString[i] << " -------------------------------\n";

        uint32_t diskSize = fileSizes[i] + 1024 * 1024 * 100;
        uint32_t sectorsNumber = diskSize / sectorSize;

        uint32_t bufferSize = fileSizes[i];
        char *buffer = generateBuffer(bufferSize);
        char *readBuffer = new char[bufferSize];

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
                                  numberOfBytesWritten, reasonForIncompleteOperation, timeElapsedMilliseconds);

        memcpy(fullFilePathCopy, fullFilePath, 50);
        result = fat32_read_file(fat32_diskInfo, fat32_bootSector, fullFilePathCopy, readBuffer, bufferSize, 0,
                                 numberOfBytesRead, reasonForIncompleteOperation, timeElapsedMilliseconds);

        double bytesPerSecond = ((double) numberOfBytesRead / (double) timeElapsedMilliseconds) * 1000;
        std::cout << "FAT32 --- Number of bytes read " <<  numberOfBytesRead << "/" << bufferSize << " (" << fileSizesInString[i] << ") --- "
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
                                 numberOfBytesWritten, reasonForIncompleteOperation, timeElapsedMilliseconds);

        memcpy(fullFilePathCopy, fullFilePath, 50);
        result = ext2_read_file(ext2_diskInfo, ext2_superBlock, fullFilePathCopy, readBuffer, bufferSize, 0,
                                numberOfBytesRead, reasonForIncompleteOperation, timeElapsedMilliseconds);

        bytesPerSecond = ((double) numberOfBytesRead / (double) timeElapsedMilliseconds) * 1000;
        std::cout << "Ext2 --- Number of bytes read: " <<  numberOfBytesRead << "/" << bufferSize << " (" << fileSizesInString[i] << ") --- "
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
                                fullFilePathCopy, buffer, bufferSize, TRUNCATE, numberOfBytesWritten, reasonForIncompleteOperation,
                                timeElapsedMilliseconds);

        memcpy(fullFilePathCopy, fullFilePath, 50);
        result = hfs_read_file(hfs_diskInfo, hfs_volumeHeader, hfs_catalogFileHeaderNode, hfs_extentsFileHeaderNode,
                               fullFilePathCopy, readBuffer, bufferSize, 0, numberOfBytesRead, reasonForIncompleteOperation,
                               timeElapsedMilliseconds);

        bytesPerSecond = ((double) numberOfBytesRead / (double) timeElapsedMilliseconds) * 1000;
        std::cout << "HFS+ --- Number of bytes read: " <<  numberOfBytesRead << "/" << bufferSize << " (" << fileSizesInString[i] << ") --- "
                  << bytesPerSecond << " bytes/second --- ";
        printDurationSolo(timeElapsedMilliseconds);
    }
}

void common_time_test_7()
{
    char* diskPath = new char[100];
    memcpy(diskPath, "D:\\Facultate\\Licenta\\HardDisks\\Automatic_Test_Solo\0", 100);

    uint32_t fileSizes[] = {1024 * 10,  1024 * 50, 1024 * 100, 1024 * 250, 1024 * 500, 1024 * 750, 1024 * 1024};
    std::string fileSizesInString[] = { "10Kib", "50Kib", "100Kib", "250Kib", "500Kib", "750Kib", "1Mib"};
    std::string totalFileSizesInString[] = {"~1000Kib", "~4,9Mib", "~9,8Mib", "~24,4Mib", "~48,8Mib", "~73,2Mib", "100Mib"}; //we assume 100 files
    uint32_t numberOfFiles = 100;

    uint32_t fat32_sectorsPerCluster = 16;
    uint32_t ext2_blockSize = 8192;
    uint32_t hfs_blockSize = 8192;

    uint32_t sectorSize = 512;
    char* parentPath = new char[50];
    memcpy(parentPath, "Root\0", 50);
    char* fileName = new char[50];
    memcpy(fileName, "File_\0\0\0\0", 50);
    char* fullFilePath = new char[50];
    memcpy(fullFilePath, "Root/File_\0\0\0\0", 50);
    char* parentPathCopy = new char[50];
    char* fileNameCopy = new char[50];
    char* fullFilePathCopy = new char[50];
    int64_t timeElapsedMilliseconds;
    uint32_t numberOfBytesWritten, numberOfBytesRead, reasonForIncompleteOperation, result;

    std::cout << std::setprecision(0) << std::fixed;

    for (int i = 0; i < 7; i++)
    {
        std:: cout << "\n------------------------------- " << fileSizesInString[i] << "/file -------------------------------\n";

        uint32_t diskSize = fileSizes[i] * numberOfFiles + 1024 * 1024 * 100;
        uint32_t sectorsNumber = diskSize / sectorSize;

        uint32_t bufferSize = fileSizes[i];
        char *buffer = generateBuffer(bufferSize);
        char *readBuffer = new char[bufferSize];

        //FAT32
        uint32_t totalBytesRead = 0, totalMilliseconds = 0;
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

            memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(j).c_str(), 2);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(j).c_str(), 2);

            result = fat32_create_directory(fat32_diskInfo, fat32_bootSector, parentPathCopy, fileNameCopy,
                                            FILE, timeElapsedMilliseconds);
            result = fat32_write_file(fat32_diskInfo, fat32_bootSector, fullFilePathCopy, buffer, bufferSize, TRUNCATE,
                                      numberOfBytesWritten, reasonForIncompleteOperation, timeElapsedMilliseconds);
        }

        for(uint32_t j = 0; j < numberOfFiles; j++)
        {
            memcpy(fullFilePathCopy, fullFilePath, 50);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(j).c_str(), 2);

            result = fat32_read_file(fat32_diskInfo, fat32_bootSector, fullFilePathCopy, readBuffer, bufferSize, 0,
                                     numberOfBytesRead, reasonForIncompleteOperation, timeElapsedMilliseconds);

            totalBytesRead += numberOfBytesRead;
            totalMilliseconds += timeElapsedMilliseconds;
        }

        double bytesPerSecond = ((double) totalBytesRead / (double) totalMilliseconds) * 1000;
        std::cout << "FAT32 --- Number of bytes read: " << totalBytesRead << "/" << bufferSize * numberOfFiles << " ("
                  << totalFileSizesInString[i] << " total) --- " << bytesPerSecond << " bytes/second --- ";
        printDurationSolo(totalMilliseconds);

        /////////////////////

        //Ext2
        totalBytesRead = 0, totalMilliseconds = 0;
        deleteFiles(diskPath);

        DiskInfo *ext2_diskInfo;
        ext2_super_block *ext2_superBlock;
        initializeExt2(diskPath, &ext2_diskInfo, &ext2_superBlock, sectorsNumber, sectorSize, ext2_blockSize);

        for(uint32_t j = 0; j < numberOfFiles; j++)
        {
            memcpy(parentPathCopy, parentPath, 50);
            memcpy(fileNameCopy, fileName, 50);
            memcpy(fullFilePathCopy, fullFilePath, 50);

            memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(j).c_str(), 2);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(j).c_str(), 2);

            result = ext2_create_directory(ext2_diskInfo, ext2_superBlock, parentPathCopy, fileNameCopy, FILE,
                                           timeElapsedMilliseconds);
            result = ext2_write_file(ext2_diskInfo, ext2_superBlock, fullFilePathCopy, buffer, bufferSize, TRUNCATE,
                                     numberOfBytesWritten, reasonForIncompleteOperation, timeElapsedMilliseconds);
        }

        for(uint32_t j = 0; j < numberOfFiles; j++)
        {
            memcpy(fullFilePathCopy, fullFilePath, 50);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(j).c_str(), 2);

            result = ext2_read_file(ext2_diskInfo, ext2_superBlock, fullFilePathCopy, readBuffer, bufferSize, 0,
                                    numberOfBytesRead, reasonForIncompleteOperation, timeElapsedMilliseconds);

            totalBytesRead += numberOfBytesRead;
            totalMilliseconds += timeElapsedMilliseconds;
        }

        bytesPerSecond = ((double) totalBytesRead / (double) totalMilliseconds) * 1000;
        std::cout << "Ext2 --- Number of bytes read: " << totalBytesRead << "/" << bufferSize * numberOfFiles << " ("
                  << totalFileSizesInString[i] << " total) --- " << bytesPerSecond << " bytes/second --- ";
        printDurationSolo(totalMilliseconds);

        /////////////////////

        //HFS
        totalBytesRead = 0, totalMilliseconds = 0;
        deleteFiles(diskPath);

        DiskInfo *hfs_diskInfo;
        HFSPlusVolumeHeader *hfs_volumeHeader;
        ExtentsFileHeaderNode *hfs_extentsFileHeaderNode;
        CatalogFileHeaderNode *hfs_catalogFileHeaderNode;
        initializeHFS(diskPath, &hfs_diskInfo, &hfs_volumeHeader, &hfs_extentsFileHeaderNode,
                      &hfs_catalogFileHeaderNode, sectorsNumber, sectorSize, hfs_blockSize);

        for(uint32_t j = 0; j < numberOfFiles; j++)
        {
            memcpy(parentPathCopy, parentPath, 50);
            memcpy(fileNameCopy, fileName, 50);
            memcpy(fullFilePathCopy, fullFilePath, 50);

            memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(j).c_str(), 2);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(j).c_str(), 2);

            result = hfs_create_directory(hfs_diskInfo, hfs_volumeHeader, hfs_catalogFileHeaderNode, parentPathCopy,
                                          fileNameCopy, FILE, timeElapsedMilliseconds);
            result = hfs_write_file(hfs_diskInfo, hfs_volumeHeader, hfs_catalogFileHeaderNode,
                                    hfs_extentsFileHeaderNode, fullFilePathCopy, buffer, bufferSize, TRUNCATE, numberOfBytesWritten,
                                    reasonForIncompleteOperation, timeElapsedMilliseconds);
        }

        for(uint32_t j = 0; j < numberOfFiles; j++)
        {
            memcpy(fullFilePathCopy, fullFilePath, 50);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(j).c_str(), 2);

            result = hfs_read_file(hfs_diskInfo, hfs_volumeHeader, hfs_catalogFileHeaderNode, hfs_extentsFileHeaderNode,
                                   fullFilePathCopy, readBuffer, bufferSize, 0, numberOfBytesRead, reasonForIncompleteOperation,
                                   timeElapsedMilliseconds);

            totalBytesRead += numberOfBytesRead;
            totalMilliseconds += timeElapsedMilliseconds;
        }

        bytesPerSecond = ((double) totalBytesRead / (double) totalMilliseconds) * 1000;
        std::cout << "HFS+ --- Number of bytes read: " << totalBytesRead << "/" << bufferSize * numberOfFiles << " ("
                  << totalFileSizesInString[i] << " total) --- " << bytesPerSecond << " bytes/second --- ";
        printDurationSolo(totalMilliseconds);
    }
}
