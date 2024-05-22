#include "iostream"

#include "ext2TestApi.h"

#include "../include/utils.h"
#include "../include/attributes.h"
#include "../include/ext2Tests.h"

void ext2_test_1()
{
    uint64_t bufferSize = 50000000;

    char* diskPath = new char[100];
    memcpy(diskPath, "D:\\Facultate\\Licenta\\HardDisks\\Ext2_Tests\\Test_1\0", 100);

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
    int round = 1;

    while(blockSize <= 8192)
    {
        DiskInfo* diskInfo;
        ext2_super_block* superBlock;
        initializeExt2(diskPath, &diskInfo, &superBlock, sectorsNumber, sectorSize, blockSize);

        memcpy(parentPathCopy, parentPath, 50);
        memcpy(fileNameCopy, fileName, 50);
        memcpy(fullFilePathCopy, fullFilePath, 50);

        result = ext2_create_directory(diskInfo, superBlock, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
        result = ext2_write_file(diskInfo, superBlock, fullFilePathCopy, buffer, bufferSize, TRUNCATE,
                                 numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

        if(round == 1)
        {
            std::cout << "\n------------------------------- " << blockSize << " Block Size -------------------------------\n";
            std::cout << "Round 1\n";
        }
        else
            std::cout << "\nRound 2\n";

        std::cout << "Number of bytes written: " <<  numberOfBytesWritten << "/" << bufferSize << '\n';
        printDurationSolo(timeElapsedMilliseconds);

        if(round == 2)
        {
            blockSize *= 2;
            round = 1;
            deleteFiles(diskPath);
        }
        else
            round++;
    }
}

void ext2_test_2()
{
    uint64_t bufferSize = 500000;
    uint32_t numOfFiles = 100;

    char* diskPath = new char[100];
    memcpy(diskPath, "D:\\Facultate\\Licenta\\HardDisks\\Ext2_Tests\\Test_2\0", 100);

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

    while(blockSize <= 8192)
    {
        DiskInfo* diskInfo;
        ext2_super_block* superBlock;
        initializeExt2(diskPath, &diskInfo, &superBlock, sectorsNumber, sectorSize, blockSize);
        uint64_t totalWriteTime = 0, totalBytesWritten = 0;

        std:: cout << "\n------------------------------- " << blockSize << " Block Size -------------------------------\n";

        for(int i = 100; i <= 99 + numOfFiles; i++)
        {
            memcpy(parentPathCopy, parentPath, 50);
            memcpy(fileNameCopy, fileName, 50);
            memcpy(fullFilePathCopy, fullFilePath, 50);

            memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(i).c_str(), 3);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(i).c_str(), 3);

            result = ext2_create_directory(diskInfo, superBlock, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
            result = ext2_write_file(diskInfo, superBlock, fullFilePathCopy, buffer, bufferSize, TRUNCATE,
                                     numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

            totalWriteTime += timeElapsedMilliseconds;
            totalBytesWritten += numberOfBytesWritten;
        }

        std::cout << "Number of bytes written: " <<  totalBytesWritten << "/" << bufferSize * numOfFiles << '\n';
        printDurationSolo(totalWriteTime);
        blockSize *= 2;
        deleteFiles(diskPath);
    }
}