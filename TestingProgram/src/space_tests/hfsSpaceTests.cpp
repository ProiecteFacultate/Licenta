#include "iomanip"
#include "vector"
#include "iostream"

#include "hfsStructures.h"
#include "hfs.h"
#include "hfsFunctionUtils.h"
#include "hfsTestApi.h"

#include "../../include/utils.h"
#include "../../include/attributes.h"
#include "../../include/space_tests/hfsSpaceTests.h"

void hfs_space_test_1()
{
    char* diskPath = new char[100];
    memcpy(diskPath, "D:\\Facultate\\Licenta\\HardDisks\\Automatic_Test_Solo\0", 100);

    //CAUTION we don't have 1Mib because due to the fixed number of nodes in trees the minimum number of sectors required by structures > 2048
    uint32_t sectorsNumbers[] = {2048 * 100, 2048 * 1024, 2048 * 1024 * 3}; //100Mib, 1Gib, 3Gib
    std::string sectorsNumbersInString[] = {"100Mib", "1Gib", "3Gib"};

    uint32_t sectorSize = 512;
    uint32_t blockSize = 8192;

    std::cout << std::setprecision(3) << std::fixed;

    for(uint32_t i = 0; i < 3; i++)
    {
        DiskInfo *diskInfo;
        HFSPlusVolumeHeader *volumeHeader;
        ExtentsFileHeaderNode *extentsFileHeaderNode;
        CatalogFileHeaderNode *catalogFileHeaderNode;
        initializeHFS(diskPath, &diskInfo, &volumeHeader, &extentsFileHeaderNode, &catalogFileHeaderNode,
                      sectorsNumbers[i], sectorSize, blockSize);

        uint32_t numberOfFileSystemStructuresBlocks = volumeHeader->catalogFile.extents[0].startBlock + volumeHeader->catalogFile.extents[0].blockCount;
        uint32_t numberOfFileSystemStructuresSectors = numberOfFileSystemStructuresBlocks * getNumberOfSectorsPerBlock(diskInfo, volumeHeader);
        uint64_t numberOfFileSystemStructuresBytes = numberOfFileSystemStructuresSectors * sectorSize;
        uint32_t totalBytesOnDisk = sectorsNumbers[i] * sectorSize;
        double structuresPercentage = ((double) numberOfFileSystemStructuresBytes / (double) totalBytesOnDisk) * 100;

        std::cout << sectorsNumbersInString[i] << " disk ---- " << numberOfFileSystemStructuresBytes << "/" << totalBytesOnDisk << " (" << structuresPercentage
                  << "%) bytes occupied by file system structures\n";

        deleteFiles(diskPath);
    }
}

void hfs_space_test_2()
{
    uint64_t bufferSize = 50000000;

    char* diskPath = new char[100];
    memcpy(diskPath, "D:\\Facultate\\Licenta\\HardDisks\\Automatic_Test_Solo\0", 100);

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

    uint32_t blockSize = 2048;
    std::cout << std::setprecision(2) << std::fixed;

    while(blockSize <= 8192)
    {
        DiskInfo *diskInfo;
        HFSPlusVolumeHeader *volumeHeader;
        ExtentsFileHeaderNode *extentsFileHeaderNode;
        CatalogFileHeaderNode *catalogFileHeaderNode;
        initializeHFS(diskPath, &diskInfo, &volumeHeader, &extentsFileHeaderNode, &catalogFileHeaderNode, sectorsNumber, sectorSize, blockSize);

        memcpy(parentPathCopy, parentPath, 50);
        memcpy(fileNameCopy, fileName, 50);
        memcpy(fullFilePathCopy, fullFilePath, 50);

        result = hfs_create_directory(diskInfo, volumeHeader, catalogFileHeaderNode, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
        result = hfs_write_file(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, fullFilePathCopy, buffer, bufferSize,
                                TRUNCATE, numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

        memcpy(parentPathCopy, parentPath, 50);
        std::vector<CatalogDirectoryRecord*> subDirectories;
        result = hfs_get_subdirectories(diskInfo, volumeHeader, catalogFileHeaderNode, parentPathCopy, subDirectories, timeElapsedMilliseconds);
        uint32_t totalSizeOnDisk, totalFilesSizes, totalInternalFragmentation;
        calculateInternalFragmentation(volumeHeader, subDirectories, totalSizeOnDisk, totalFilesSizes, totalInternalFragmentation);

        std:: cout << "\n------------------------------- " << blockSize << " Block Size -------------------------------\n";

        double internalFragmentationPercentage = ((double) totalInternalFragmentation / (double) totalSizeOnDisk) * 100;
        std::cout << totalInternalFragmentation << "/" << totalFilesSizes << "/" << totalSizeOnDisk << " (internal fragmentation/file size/size on disk) --- ("
                  << internalFragmentationPercentage << "%) (internal fragmentation/size on disk)";

        blockSize *= 2;
        deleteFiles(diskPath);
    }
}

void hfs_space_test_3()
{
    uint64_t bufferSize = 500000;
    uint32_t numOfFiles = 100;

    char* diskPath = new char[100];
    memcpy(diskPath, "D:\\Facultate\\Licenta\\HardDisks\\Automatic_Test_Solo\0", 100);

    char* buffer = generateBuffer(bufferSize);
    uint32_t sectorsNumber = 204800;
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
    uint32_t numberOfBytesWritten, reasonForIncompleteWrite, result;

    uint32_t blockSize = 2048;
    std::cout << std::setprecision(2) << std::fixed;

    while(blockSize <= 8192)
    {
        DiskInfo *diskInfo;
        HFSPlusVolumeHeader *volumeHeader;
        ExtentsFileHeaderNode *extentsFileHeaderNode;
        CatalogFileHeaderNode *catalogFileHeaderNode;
        initializeHFS(diskPath, &diskInfo, &volumeHeader, &extentsFileHeaderNode, &catalogFileHeaderNode, sectorsNumber, sectorSize, blockSize);
        uint64_t totalWriteTime = 0, totalBytesWritten = 0;

        for(int i = 100; i <= 99 + numOfFiles; i++)
        {
            memcpy(parentPathCopy, parentPath, 50);
            memcpy(fileNameCopy, fileName, 50);
            memcpy(fullFilePathCopy, fullFilePath, 50);

            memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(i).c_str(), 3);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(i).c_str(), 3);

            result = hfs_create_directory(diskInfo, volumeHeader, catalogFileHeaderNode, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
            result = hfs_write_file(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, fullFilePathCopy, buffer, bufferSize,
                                    TRUNCATE, numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);
        }

        memcpy(parentPathCopy, parentPath, 50);
        std::vector<CatalogDirectoryRecord*> subDirectories;
        result = hfs_get_subdirectories(diskInfo, volumeHeader, catalogFileHeaderNode, parentPathCopy, subDirectories, timeElapsedMilliseconds);
        uint32_t totalSizeOnDisk, totalFilesSizes, totalInternalFragmentation;
        calculateInternalFragmentation(volumeHeader, subDirectories, totalSizeOnDisk, totalFilesSizes, totalInternalFragmentation);

        std:: cout << "\n------------------------------- " << blockSize << " Block Size -------------------------------\n";

        double internalFragmentationPercentage = ((double) totalInternalFragmentation / (double) totalSizeOnDisk) * 100;
        std::cout << totalInternalFragmentation << "/" << totalFilesSizes << "/" << totalSizeOnDisk << " (internal fragmentation/file size/size on disk) --- ("
                  << internalFragmentationPercentage << "%) (internal fragmentation/size on disk)";

        blockSize *= 2;
        deleteFiles(diskPath);
    }
}

void hfs_space_test_ignore_1()
{
    uint64_t bufferSize = 100000; //CAUTION this isn't the actual file size, it differs, see below in code
    uint32_t numOfFiles = 100;

    char* diskPath = new char[100];
    memcpy(diskPath, "D:\\Facultate\\Licenta\\HardDisks\\Automatic_Test_Solo\0", 100);

    char* buffer = generateBuffer(bufferSize);
    uint32_t sectorsNumber = 204800;
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
    uint32_t numberOfBytesWritten, reasonForIncompleteWrite, result;

    uint32_t blockSize = 2048;
    std::cout << std::setprecision(3) << std::fixed;

    while(blockSize <= 8192)
    {
        DiskInfo *diskInfo;
        HFSPlusVolumeHeader *volumeHeader;
        ExtentsFileHeaderNode *extentsFileHeaderNode;
        CatalogFileHeaderNode *catalogFileHeaderNode;
        initializeHFS(diskPath, &diskInfo, &volumeHeader, &extentsFileHeaderNode, &catalogFileHeaderNode, sectorsNumber, sectorSize, blockSize);
        uint64_t totalWriteTime = 0, totalBytesWritten = 0;

        //1 byte over the block size
        bufferSize = blockSize + 1;
        for(int i = 100; i <= 99 + numOfFiles; i++)
        {
            memcpy(parentPathCopy, parentPath, 50);
            memcpy(fileNameCopy, fileName, 50);
            memcpy(fullFilePathCopy, fullFilePath, 50);

            memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(i).c_str(), 3);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(i).c_str(), 3);

            result = hfs_create_directory(diskInfo, volumeHeader, catalogFileHeaderNode, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
            result = hfs_write_file(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, fullFilePathCopy, buffer, bufferSize,
                                    TRUNCATE, numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);
        }

        memcpy(parentPathCopy, parentPath, 50);
        std::vector<CatalogDirectoryRecord*> subDirectories;
        result = hfs_get_subdirectories(diskInfo, volumeHeader, catalogFileHeaderNode, parentPathCopy, subDirectories, timeElapsedMilliseconds);
        uint32_t totalSizeOnDisk, totalFilesSizes, totalInternalFragmentation;
        calculateInternalFragmentation(volumeHeader, subDirectories, totalSizeOnDisk, totalFilesSizes, totalInternalFragmentation);

        std:: cout << "\n------------------------------- " << blockSize << " Block Size -------------------------------\n";

        double internalFragmentationPercentage;
        if(totalInternalFragmentation == 0)
            internalFragmentationPercentage = 0;
        else
            internalFragmentationPercentage = ((double) totalInternalFragmentation / (double) totalSizeOnDisk) * 100;

        std::cout << "\nWith " << bufferSize << " bytes written per sector:\n";
        std::cout << totalInternalFragmentation << "/" << totalSizeOnDisk << " (" << internalFragmentationPercentage << "%) --- (internal fragmentation/size on disk)"
                  << " for total of " << totalFilesSizes << " bytes written\n";

        //block size
        bufferSize = 8 * blockSize;
        for(int i = 100; i <= 99 + numOfFiles; i++)
        {
            memcpy(fullFilePathCopy, fullFilePath, 50);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(i).c_str(), 3);
            result = hfs_write_file(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, fullFilePathCopy, buffer, bufferSize,
                                    TRUNCATE, numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);
        }

        memcpy(parentPathCopy, parentPath, 50);
        subDirectories.clear();
        result = hfs_get_subdirectories(diskInfo, volumeHeader, catalogFileHeaderNode, parentPathCopy, subDirectories, timeElapsedMilliseconds);
        calculateInternalFragmentation(volumeHeader, subDirectories, totalSizeOnDisk, totalFilesSizes, totalInternalFragmentation);

        internalFragmentationPercentage = ((double) totalInternalFragmentation / (double) totalSizeOnDisk) * 100;
        std::cout << "\nWith " << bufferSize << "bytes written per sector:\n";
        std::cout << totalInternalFragmentation << "/" << totalSizeOnDisk << " (" << internalFragmentationPercentage << "%) --- (internal fragmentation/size on disk)"
                  << " for total of " << totalFilesSizes << " bytes written\n";

        blockSize *= 2;
        deleteFiles(diskPath);
    }
}

void hfs_space_test_4()
{
    uint64_t bigBufferSize = 9000000;
    uint64_t mediumBufferSize = 900000;
    uint64_t smallBufferSize = 90000;
    uint32_t numOfRounds = 5;

    char* diskPath = new char[100];
    memcpy(diskPath, "D:\\Facultate\\Licenta\\HardDisks\\Automatic_Test_Solo\0", 100);

    char* bigBuffer = generateBuffer(bigBufferSize);
    char* mediumBuffer = generateBuffer(mediumBufferSize);
    char* smallBuffer = generateBuffer(smallBufferSize);
    uint32_t sectorsNumber = 204800;
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
    uint32_t numberOfBytesWritten, reasonForIncompleteWrite, result;

    uint32_t blockSize = 2048;
    std::cout << std::setprecision(3) << std::fixed;

    while(blockSize <= 8192)
    {
        DiskInfo *diskInfo;
        HFSPlusVolumeHeader *volumeHeader;
        ExtentsFileHeaderNode *extentsFileHeaderNode;
        CatalogFileHeaderNode *catalogFileHeaderNode;
        initializeHFS(diskPath, &diskInfo, &volumeHeader, &extentsFileHeaderNode, &catalogFileHeaderNode, sectorsNumber, sectorSize, blockSize);

        for(int i = 0; i < numOfRounds; i++)
        {
            //small buffer
            memcpy(parentPathCopy, parentPath, 50);
            memcpy(fileNameCopy, fileName, 50);
            memcpy(fullFilePathCopy, fullFilePath, 50);

            memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(i * 3).c_str(), 2);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(i * 3).c_str(), 2);

            result = hfs_create_directory(diskInfo, volumeHeader, catalogFileHeaderNode, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
            result = hfs_write_file(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, fullFilePathCopy, smallBuffer,
                                    smallBufferSize, TRUNCATE, numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

            //medium buffer
            memcpy(parentPathCopy, parentPath, 50);
            memcpy(fileNameCopy, fileName, 50);
            memcpy(fullFilePathCopy, fullFilePath, 50);

            memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(i * 3 + 1).c_str(), 2);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(i * 3 + 1).c_str(), 2);

            result = hfs_create_directory(diskInfo, volumeHeader, catalogFileHeaderNode, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
            result = hfs_write_file(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, fullFilePathCopy, mediumBuffer,
                                    mediumBufferSize, TRUNCATE, numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

            //big buffer
            memcpy(parentPathCopy, parentPath, 50);
            memcpy(fileNameCopy, fileName, 50);
            memcpy(fullFilePathCopy, fullFilePath, 50);

            memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(i * 3 + 2).c_str(), 2);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(i * 3 + 2).c_str(), 2);

            result = hfs_create_directory(diskInfo, volumeHeader, catalogFileHeaderNode, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
            result = hfs_write_file(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, fullFilePathCopy, bigBuffer, bigBufferSize,
                                    TRUNCATE, numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);
        }

        memcpy(parentPathCopy, parentPath, 50);
        std::vector<CatalogDirectoryRecord*> subDirectories;
        result = hfs_get_subdirectories(diskInfo, volumeHeader, catalogFileHeaderNode, parentPathCopy, subDirectories, timeElapsedMilliseconds);
        uint32_t totalSizeOnDisk, totalFilesSizes, totalInternalFragmentation;
        calculateInternalFragmentation(volumeHeader, subDirectories, totalSizeOnDisk, totalFilesSizes, totalInternalFragmentation);

        std:: cout << "\n------------------------------- " << blockSize << " Block Size -------------------------------\n";

        double internalFragmentationPercentage = ((double) totalInternalFragmentation / (double) totalSizeOnDisk) * 100;
        std::cout << totalInternalFragmentation << "/" << totalFilesSizes << "/" << totalSizeOnDisk << " (internal fragmentation/file size/size on disk) --- ("
                  << internalFragmentationPercentage << "%) (internal fragmentation/size on disk)";

        blockSize *= 2;
        deleteFiles(diskPath);
    }
}

//////////////////////

void calculateInternalFragmentation(HFSPlusVolumeHeader* volumeHeader, std::vector<CatalogDirectoryRecord*> directories, uint32_t& totalSizeOnDisk, uint32_t& totalFilesSizes,
                                    uint32_t& totalInternalFragmentation)
{
    totalSizeOnDisk = 0;
    totalFilesSizes = 0;
    totalInternalFragmentation = 0;

    for(auto directory : directories)
    {
        totalSizeOnDisk += directory->catalogData.hfsPlusForkData.totalBlocks * volumeHeader->blockSize;
        totalFilesSizes += directory->catalogData.fileSize;
    }

    totalInternalFragmentation = totalSizeOnDisk - totalFilesSizes;
}