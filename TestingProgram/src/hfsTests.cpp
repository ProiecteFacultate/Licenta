#include "iostream"

#include "hfsTestApi.h"

#include "../include/utils.h"
#include "../include/attributes.h"
#include "../include/hfsTests.h"

void hfs_test_1()
{
    uint64_t bufferSize = 50000000;

    char* diskPath = new char[100];
    memcpy(diskPath, "D:\\Facultate\\Licenta\\HardDisks\\Automatic_Tests\\HFS_Tests\\Test_1\0", 100);

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
        HFSPlusVolumeHeader* volumeHeader;
        ExtentsFileHeaderNode* extentsFileHeaderNode;
        CatalogFileHeaderNode* catalogFileHeaderNode;
        initializeHFS(diskPath, &diskInfo, &volumeHeader, &extentsFileHeaderNode, &catalogFileHeaderNode, sectorsNumber, sectorSize, blockSize);

        memcpy(parentPathCopy, parentPath, 50);
        memcpy(fileNameCopy, fileName, 50);
        memcpy(fullFilePathCopy, fullFilePath, 50);

        result = hfs_create_directory(diskInfo, volumeHeader, catalogFileHeaderNode, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
        result = hfs_write_file(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, fullFilePathCopy, buffer, bufferSize,
                                TRUNCATE, numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

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

void hfs_test_2()
{
    uint64_t bufferSize = 500000;
    uint32_t numOfFiles = 100;

    char* diskPath = new char[100];
    memcpy(diskPath, "D:\\Facultate\\Licenta\\HardDisks\\Automatic_Tests\\HFS_Tests\\Test_2\0", 100);

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
        HFSPlusVolumeHeader* volumeHeader;
        ExtentsFileHeaderNode* extentsFileHeaderNode;
        CatalogFileHeaderNode* catalogFileHeaderNode;
        initializeHFS(diskPath, &diskInfo, &volumeHeader, &extentsFileHeaderNode, &catalogFileHeaderNode, sectorsNumber, sectorSize, blockSize);
        uint64_t totalWriteTime = 0, totalBytesWritten = 0;

        std:: cout << "\n------------------------------- " << blockSize << " Block Size -------------------------------\n";

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

            totalWriteTime += timeElapsedMilliseconds;
            totalBytesWritten += numberOfBytesWritten;
        }

        std::cout << "Number of bytes written: " <<  totalBytesWritten << "/" << bufferSize * numOfFiles << '\n';
        printDurationSolo(totalWriteTime);
        blockSize *= 2;
        deleteFiles(diskPath);
    }
}

void hfs_test_3()
{
    uint64_t bigBufferSize = 9000000;
    uint64_t mediumBufferSize = 900000;
    uint64_t smallBufferSize = 9000;
    uint32_t numOfRounds = 5;

    char* diskPath = new char[100];
    memcpy(diskPath, "D:\\Facultate\\Licenta\\HardDisks\\Automatic_Tests\\HFS_Tests\\Test_3\0", 100);

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

    while(blockSize <= 8192)
    {
        DiskInfo* diskInfo;
        HFSPlusVolumeHeader* volumeHeader;
        ExtentsFileHeaderNode* extentsFileHeaderNode;
        CatalogFileHeaderNode* catalogFileHeaderNode;
        initializeHFS(diskPath, &diskInfo, &volumeHeader, &extentsFileHeaderNode, &catalogFileHeaderNode, sectorsNumber, sectorSize, blockSize);
        uint64_t totalWriteTime = 0, totalBytesWritten = 0;

        std:: cout << "\n------------------------------- " << blockSize << " Block Size -------------------------------\n";

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

            totalWriteTime += timeElapsedMilliseconds;
            totalBytesWritten += numberOfBytesWritten;

            //medium buffer
            memcpy(parentPathCopy, parentPath, 50);
            memcpy(fileNameCopy, fileName, 50);
            memcpy(fullFilePathCopy, fullFilePath, 50);

            memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(i * 3 + 1).c_str(), 2);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(i * 3 + 1).c_str(), 2);

            result = hfs_create_directory(diskInfo, volumeHeader, catalogFileHeaderNode, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
            result = hfs_write_file(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, fullFilePathCopy, mediumBuffer,
                                    mediumBufferSize, TRUNCATE, numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

            totalWriteTime += timeElapsedMilliseconds;
            totalBytesWritten += numberOfBytesWritten;

            //big buffer
            memcpy(parentPathCopy, parentPath, 50);
            memcpy(fileNameCopy, fileName, 50);
            memcpy(fullFilePathCopy, fullFilePath, 50);

            memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(i * 3 + 2).c_str(), 2);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(i * 3 + 2).c_str(), 2);

            result = hfs_create_directory(diskInfo, volumeHeader, catalogFileHeaderNode, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
            result = hfs_write_file(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, fullFilePathCopy, bigBuffer, bigBufferSize,
                                    TRUNCATE, numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

            totalWriteTime += timeElapsedMilliseconds;
            totalBytesWritten += numberOfBytesWritten;
        }

        std::cout << "Number of bytes written: " <<  totalBytesWritten << "/" << (smallBufferSize + mediumBufferSize + bigBufferSize) * numOfRounds << '\n';
        printDurationSolo(totalWriteTime);
        blockSize *= 2;
        deleteFiles(diskPath);
    }
}

void hfs_test_4()
{
    uint64_t bufferSize = 25000000;

    char* diskPath = new char[100];
    memcpy(diskPath, "D:\\Facultate\\Licenta\\HardDisks\\Automatic_Tests\\HFS_Tests\\Test_4\0", 100);

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
    uint32_t numberOfBytesWritten, reasonForIncompleteWrite, result, writeMode;

    uint32_t blockSize = 2048;
    int round = 1;

    while(blockSize <= 8192)
    {
        DiskInfo* diskInfo;
        HFSPlusVolumeHeader* volumeHeader;
        ExtentsFileHeaderNode* extentsFileHeaderNode;
        CatalogFileHeaderNode* catalogFileHeaderNode;
        initializeHFS(diskPath, &diskInfo, &volumeHeader, &extentsFileHeaderNode, &catalogFileHeaderNode, sectorsNumber, sectorSize, blockSize);

        memcpy(parentPathCopy, parentPath, 50);
        memcpy(fileNameCopy, fileName, 50);
        memcpy(fullFilePathCopy, fullFilePath, 50);

        result = hfs_create_directory(diskInfo, volumeHeader, catalogFileHeaderNode, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
        result = hfs_write_file(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, fullFilePathCopy, buffer, bufferSize,
                                TRUNCATE, numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

        if(round == 1)
        {
            std::cout << "\n------------------------------- " << blockSize << " Block Size -------------------------------\n";
            std::cout << "Round 1\n";
            writeMode = TRUNCATE;
        }
        else
        {
            std::cout << "\nRound 2\n";
            writeMode = APPEND;
        }

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

void hfs_test_5()
{
    uint64_t bufferSize = 50000000;

    char* diskPath = new char[100];
    memcpy(diskPath, "D:\\Facultate\\Licenta\\HardDisks\\Automatic_Tests\\HFS_Tests\\Test_5\0", 100);

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
    uint32_t numberOfBytesInBuffer, reasonForIncompleteWrite, result;

    uint32_t blockSize = 2048;

    while(blockSize <= 8192)
    {
        DiskInfo* diskInfo;
        HFSPlusVolumeHeader* volumeHeader;
        ExtentsFileHeaderNode* extentsFileHeaderNode;
        CatalogFileHeaderNode* catalogFileHeaderNode;
        initializeHFS(diskPath, &diskInfo, &volumeHeader, &extentsFileHeaderNode, &catalogFileHeaderNode, sectorsNumber, sectorSize, blockSize);

        memcpy(parentPathCopy, parentPath, 50);
        memcpy(fileNameCopy, fileName, 50);
        memcpy(fullFilePathCopy, fullFilePath, 50);

        result = hfs_create_directory(diskInfo, volumeHeader, catalogFileHeaderNode, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
        result = hfs_write_file(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, fullFilePathCopy, buffer, bufferSize,
                                TRUNCATE, numberOfBytesInBuffer, reasonForIncompleteWrite, timeElapsedMilliseconds);

        memcpy(parentPathCopy, parentPath, 50);
        memcpy(fullFilePathCopy, fullFilePath, 50);

        result = hfs_read_file(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, fullFilePathCopy, buffer, bufferSize, 0,
                                numberOfBytesInBuffer, reasonForIncompleteWrite, timeElapsedMilliseconds);

        std::cout << "\n------------------------------- " << blockSize << " Block Size -------------------------------\n";

        std::cout << "Number of bytes read: " <<  numberOfBytesInBuffer << "/" << bufferSize << '\n';
        printDurationSolo(timeElapsedMilliseconds);

        blockSize *= 2;
        deleteFiles(diskPath);
    }
}

void hfs_test_7()
{
    uint64_t bufferSize = 5000;
    uint32_t numOfFiles = 100;

    char* diskPath = new char[100];
    memcpy(diskPath, "D:\\Facultate\\Licenta\\HardDisks\\Automatic_Tests\\HFS_Tests\\Test_7\0", 100);

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
    uint32_t numberOfBytesInBuffer, reasonForIncompleteOperation, result;

    uint32_t blockSize = 2048;

    while(blockSize <= 8192)
    {
        DiskInfo* diskInfo;
        HFSPlusVolumeHeader* volumeHeader;
        ExtentsFileHeaderNode* extentsFileHeaderNode;
        CatalogFileHeaderNode* catalogFileHeaderNode;
        initializeHFS(diskPath, &diskInfo, &volumeHeader, &extentsFileHeaderNode, &catalogFileHeaderNode, sectorsNumber, sectorSize, blockSize);
        uint64_t totalWriteTime = 0, totalBytesRead = 0;

        std::cout << "\n------------------------------- " << blockSize << " Block Size -------------------------------\n";

        for(int i = 100; i <= 99 + numOfFiles; i++)
        {
            memcpy(parentPathCopy, parentPath, 50);
            memcpy(fileNameCopy, fileName, 50);
            memcpy(fullFilePathCopy, fullFilePath, 50);

            memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(i).c_str(), 3);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(i).c_str(), 3);

            result = hfs_create_directory(diskInfo, volumeHeader, catalogFileHeaderNode, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
            result = hfs_write_file(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, fullFilePathCopy, buffer, bufferSize,
                                    TRUNCATE, numberOfBytesInBuffer, reasonForIncompleteOperation, timeElapsedMilliseconds);

            memcpy(fullFilePathCopy, fullFilePath, 50);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(i).c_str(), 3);

            result = hfs_read_file(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, fullFilePathCopy, buffer, bufferSize, 0,
                                   numberOfBytesInBuffer, reasonForIncompleteOperation, timeElapsedMilliseconds);

            totalWriteTime += timeElapsedMilliseconds;
            totalBytesRead += numberOfBytesInBuffer;
        }

        std::cout << "Number of bytes read: " <<  totalBytesRead << "/" << bufferSize * numOfFiles << '\n';
        printDurationSolo(totalWriteTime);
        blockSize *= 2;
        deleteFiles(diskPath);
    }
}