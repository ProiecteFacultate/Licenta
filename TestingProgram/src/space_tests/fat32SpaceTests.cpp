#include "iomanip"
#include "iostream"
#include "vector"

#include "fat32Structures.h"
#include "fat32FunctionUtils.h"
#include "fat32TestApi.h"

#include "../../include/utils.h"
#include "../../include/attributes.h"
#include "../../include/space_tests/fat32SpaceTests.h"

void fat32_space_test_1()
{
    char* diskPath = new char[100];
    memcpy(diskPath, "D:\\Facultate\\Licenta\\HardDisks\\Automatic_Test_Solo\0", 100);

    uint32_t sectorsNumbers[] = {2048, 2048 * 100, 2048 * 1024, 2048 * 1024 * 3}; //1Mib, 100Mib, 1Gib, 3Gib
    std::string sectorsNumbersInString[] = { "1Mib", "100Mib", "1Gib", "3Gib"};

    uint32_t sectorSize = 512;
    uint32_t sectorsPerCluster = 16;

    std::cout << std::setprecision(3) << std::fixed;

    for(uint32_t i = 0; i < 4; i++)
    {
        DiskInfo* diskInfo;
        BootSector* bootSector;
        FsInfo* fsInfo;
        initializeFAT32(diskPath, &diskInfo, &bootSector, &fsInfo, sectorsNumbers[i], sectorSize, sectorsPerCluster);

        uint32_t numberOfFileSystemStructuresSectors = getFirstSectorForCluster(bootSector, 2); //in fat32 the first data cluster (root cluster is number 2)
        uint32_t numberOfFileSystemStructuresBytes = numberOfFileSystemStructuresSectors * sectorSize;
        uint32_t totalBytesOnDisk = sectorsNumbers[i] * sectorSize;
        double structuresPercentage = ((double) numberOfFileSystemStructuresBytes / (double) totalBytesOnDisk) * 100;

        std::cout << sectorsNumbersInString[i] << " disk ---- " <<  numberOfFileSystemStructuresBytes << "/" << totalBytesOnDisk << " (" << structuresPercentage << "%) bytes occupied by file system structures\n";

        deleteFiles(diskPath);
    }
}

void fat32_space_test_2()
{
    uint64_t bufferSize = 1000000;

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

    uint32_t sectorsPerCluster = 4;
    std::cout << std::setprecision(2) << std::fixed;

    while(sectorsPerCluster <= 16)
    {
        DiskInfo* diskInfo;
        BootSector* bootSector;
        FsInfo* fsInfo;
        initializeFAT32(diskPath, &diskInfo, &bootSector, &fsInfo, sectorsNumber, sectorSize, sectorsPerCluster);

        memcpy(parentPathCopy, parentPath, 50);
        memcpy(fileNameCopy, fileName, 50);
        memcpy(fullFilePathCopy, fullFilePath, 50);

        result = fat32_create_directory(diskInfo, bootSector, parentPathCopy, fileNameCopy,
                                        FILE, timeElapsedMilliseconds);
        result = fat32_write_file(diskInfo, bootSector, fullFilePathCopy, buffer, bufferSize, TRUNCATE,
                                  numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

        memcpy(parentPathCopy, parentPath, 50);
        std::vector<std::pair<DirectoryEntry*, DirectorySizeDetails*>> subDirectories;
        result = fat32_get_subdirectories(diskInfo, bootSector, parentPathCopy, subDirectories, timeElapsedMilliseconds);
        uint32_t totalSizeOnDisk, totalFilesSizes, totalInternalFragmentation;
        calculateInternalFragmentation(subDirectories, totalSizeOnDisk, totalFilesSizes, totalInternalFragmentation);

        std:: cout << "\n------------------------------- " << sectorsPerCluster << " Sectors Per Cluster -------------------------------\n";

        double internalFragmentationPercentage = ((double) totalInternalFragmentation / (double) totalSizeOnDisk) * 100;
        std::cout << totalInternalFragmentation << "/" << totalSizeOnDisk << " (" << internalFragmentationPercentage << "%) --- (internal fragmentation/size on disk)"
        << " for total of " << numberOfBytesWritten << " bytes written\n";

        sectorsPerCluster *= 2;
        deleteFiles(diskPath);
    }
}

void fat32_space_test_3()
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

    uint32_t sectorsPerCluster = 4;
    std::cout << std::setprecision(3) << std::fixed;

    while(sectorsPerCluster <= 16)
    {
        DiskInfo* diskInfo;
        BootSector* bootSector;
        FsInfo* fsInfo;
        initializeFAT32(diskPath, &diskInfo, &bootSector, &fsInfo, sectorsNumber, sectorSize, sectorsPerCluster);
        uint64_t totalWriteTime = 0, totalBytesWritten = 0;

        for(int i = 100; i <= 99 + numOfFiles; i++)
        {
            memcpy(parentPathCopy, parentPath, 50);
            memcpy(fileNameCopy, fileName, 50);
            memcpy(fullFilePathCopy, fullFilePath, 50);

            memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(i).c_str(), 3);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(i).c_str(), 3);

            result = fat32_create_directory(diskInfo, bootSector, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
            result = fat32_write_file(diskInfo, bootSector, fullFilePathCopy, buffer, bufferSize, TRUNCATE,
                                      numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);
        }

        memcpy(parentPathCopy, parentPath, 50);
        std::vector<std::pair<DirectoryEntry*, DirectorySizeDetails*>> subDirectories;
        result = fat32_get_subdirectories(diskInfo, bootSector, parentPathCopy, subDirectories, timeElapsedMilliseconds);
        uint32_t totalSizeOnDisk, totalFilesSizes, totalInternalFragmentation;
        calculateInternalFragmentation(subDirectories, totalSizeOnDisk, totalFilesSizes, totalInternalFragmentation);

        std:: cout << "\n------------------------------- " << sectorsPerCluster << " Sectors Per Cluster -------------------------------\n";

        double internalFragmentationPercentage = ((double) totalInternalFragmentation / (double) totalSizeOnDisk) * 100;
        std::cout << totalInternalFragmentation << "/" << totalSizeOnDisk << " (" << internalFragmentationPercentage << "%) --- (internal fragmentation/size on disk)"
                  << " for total of " << totalFilesSizes << " bytes written\n";

        sectorsPerCluster *= 2;
        deleteFiles(diskPath);
    }
}

void fat32_space_test_4()
{
    uint64_t bufferSize = 10000; //CAUTION this isn't the actual file size, it differs, see below in code
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

    uint32_t sectorsPerCluster = 4;
    std::cout << std::setprecision(3) << std::fixed;

    while(sectorsPerCluster <= 16)
    {
        DiskInfo* diskInfo;
        BootSector* bootSector;
        FsInfo* fsInfo;
        initializeFAT32(diskPath, &diskInfo, &bootSector, &fsInfo, sectorsNumber, sectorSize, sectorsPerCluster);
        uint64_t totalWriteTime = 0, totalBytesWritten = 0;

        //1 byte over cluster size
        bufferSize = sectorsPerCluster * sectorSize - 63;
        for(int i = 100; i <= 99 + numOfFiles; i++)
        {
            memcpy(parentPathCopy, parentPath, 50);
            memcpy(fileNameCopy, fileName, 50);
            memcpy(fullFilePathCopy, fullFilePath, 50);

            memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(i).c_str(), 3);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(i).c_str(), 3);

            result = fat32_create_directory(diskInfo, bootSector, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
            result = fat32_write_file(diskInfo, bootSector, fullFilePathCopy, buffer, bufferSize, TRUNCATE,
                                      numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);
        }

        memcpy(parentPathCopy, parentPath, 50);
        std::vector<std::pair<DirectoryEntry*, DirectorySizeDetails*>> subDirectories;
        result = fat32_get_subdirectories(diskInfo, bootSector, parentPathCopy, subDirectories, timeElapsedMilliseconds);
        uint32_t totalSizeOnDisk, totalFilesSizes, totalInternalFragmentation;
        calculateInternalFragmentation(subDirectories, totalSizeOnDisk, totalFilesSizes, totalInternalFragmentation);

        std:: cout << "\n------------------------------- " << sectorsPerCluster << " Sectors Per Cluster -------------------------------\n";

        double internalFragmentationPercentage;
        if(totalInternalFragmentation == 0)
            internalFragmentationPercentage = 0;
        else
            internalFragmentationPercentage = ((double) totalInternalFragmentation / (double) totalSizeOnDisk) * 100;

        std::cout << "\nWith " << bufferSize << " bytes written per sector:\n";
        std::cout << totalInternalFragmentation << "/" << totalSizeOnDisk << " (" << internalFragmentationPercentage << "%) --- (internal fragmentation/size on disk)"
                  << " for total of " << totalFilesSizes << " bytes written\n";

        //cluster size
        bufferSize = sectorsPerCluster * sectorSize - 64;
        for(int i = 100; i <= 99 + numOfFiles; i++)
        {
            memcpy(fullFilePathCopy, fullFilePath, 50);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(i).c_str(), 3);
            result = fat32_write_file(diskInfo, bootSector, fullFilePathCopy, buffer, bufferSize, TRUNCATE,
                                      numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);
        }

        memcpy(parentPathCopy, parentPath, 50);
        subDirectories.clear();
        result = fat32_get_subdirectories(diskInfo, bootSector, parentPathCopy, subDirectories, timeElapsedMilliseconds);
        calculateInternalFragmentation(subDirectories, totalSizeOnDisk, totalFilesSizes, totalInternalFragmentation);

        internalFragmentationPercentage = ((double) totalInternalFragmentation / (double) totalSizeOnDisk) * 100;
        std::cout << "\nWith " << bufferSize << "bytes written per sector:\n";
        std::cout << totalInternalFragmentation << "/" << totalSizeOnDisk << " (" << internalFragmentationPercentage << "%) --- (internal fragmentation/size on disk)"
                  << " for total of " << totalFilesSizes << " bytes written\n";

        sectorsPerCluster *= 2;
        deleteFiles(diskPath);
    }
}

void fat32_space_test_5()
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

    uint32_t sectorsPerCluster = 4;
    std::cout << std::setprecision(3) << std::fixed;

    while(sectorsPerCluster <= 16)
    {
        DiskInfo* diskInfo;
        BootSector* bootSector;
        FsInfo* fsInfo;
        initializeFAT32(diskPath, &diskInfo, &bootSector, &fsInfo, sectorsNumber, sectorSize, sectorsPerCluster);

        for(int i = 0; i < numOfRounds; i++)
        {
            //small buffer
            memcpy(parentPathCopy, parentPath, 50);
            memcpy(fileNameCopy, fileName, 50);
            memcpy(fullFilePathCopy, fullFilePath, 50);

            memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(i * 3).c_str(), 2);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(i * 3).c_str(), 2);

            result = fat32_create_directory(diskInfo, bootSector, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
            result = fat32_write_file(diskInfo, bootSector, fullFilePathCopy, smallBuffer, smallBufferSize, TRUNCATE,
                                      numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

            //medium buffer
            memcpy(parentPathCopy, parentPath, 50);
            memcpy(fileNameCopy, fileName, 50);
            memcpy(fullFilePathCopy, fullFilePath, 50);

            memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(i * 3 + 1).c_str(), 2);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(i * 3 + 1).c_str(), 2);

            result = fat32_create_directory(diskInfo, bootSector, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
            result = fat32_write_file(diskInfo, bootSector, fullFilePathCopy, mediumBuffer, mediumBufferSize, TRUNCATE,
                                      numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

            //big buffer
            memcpy(parentPathCopy, parentPath, 50);
            memcpy(fileNameCopy, fileName, 50);
            memcpy(fullFilePathCopy, fullFilePath, 50);

            memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(i * 3 + 2).c_str(), 2);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(i * 3 + 2).c_str(), 2);

            result = fat32_create_directory(diskInfo, bootSector, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
            result = fat32_write_file(diskInfo, bootSector, fullFilePathCopy, bigBuffer, bigBufferSize, TRUNCATE,
                                      numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);
        }

        memcpy(parentPathCopy, parentPath, 50);
        std::vector<std::pair<DirectoryEntry*, DirectorySizeDetails*>> subDirectories;
        result = fat32_get_subdirectories(diskInfo, bootSector, parentPathCopy, subDirectories, timeElapsedMilliseconds);
        uint32_t totalSizeOnDisk, totalFilesSizes, totalInternalFragmentation;
        calculateInternalFragmentation(subDirectories, totalSizeOnDisk, totalFilesSizes, totalInternalFragmentation);

        std:: cout << "\n------------------------------- " << sectorsPerCluster << " Sectors Per Cluster -------------------------------\n";

        double internalFragmentationPercentage = ((double) totalInternalFragmentation / (double) totalSizeOnDisk) * 100;
        std::cout << totalInternalFragmentation << "/" << totalSizeOnDisk << " (" << internalFragmentationPercentage << "%) --- (internal fragmentation/size on disk)"
                  << " for total of " << totalFilesSizes << " bytes written\n";

        sectorsPerCluster *= 2;
        deleteFiles(diskPath);
    }
}

//////////////////////

void calculateInternalFragmentation(std::vector<std::pair<DirectoryEntry*, DirectorySizeDetails*>> directories, uint32_t& totalSizeOnDisk, uint32_t& totalFilesSizes,
                                    uint32_t& totalInternalFragmentation)
{
    totalSizeOnDisk = 0;
    totalFilesSizes = 0;
    totalInternalFragmentation = 0;

    for(auto directory : directories)
    {
        totalSizeOnDisk += directory.second->SizeOnDisk;
        totalFilesSizes += directory.second->FileSize;
    }

    totalInternalFragmentation = totalSizeOnDisk - totalFilesSizes;
}