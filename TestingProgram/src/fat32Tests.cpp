#include "string.h"
#include "iostream"

#include "fat32TestApi.h"

#include "../include/utils.h"
#include "../include/attributes.h"
#include "../include/fat32Tests.h"

void fat32_test_1()
{
    uint64_t bufferSize = 5000000;

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
    int round = 1;

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

        if(round == 1)
        {
            std:: cout << "\n------------------------------- " << sectorsPerCluster << " Sectors Per Cluster -------------------------------\n";
            std::cout << "Round 1\n";
        }
        else
            std::cout << "\nRound 2\n";

        std::cout << "Number of bytes written: " <<  numberOfBytesWritten << "/" << bufferSize << '\n';
        printDurationSolo(timeElapsedMilliseconds);

        if(round == 2)
        {
            sectorsPerCluster *= 2;
            round = 1;
            deleteFiles(diskPath);
        }
        else
            round++;
    }
}

void fat32_test_2()
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

    while(sectorsPerCluster <= 16)
    {
        DiskInfo* diskInfo;
        BootSector* bootSector;
        FsInfo* fsInfo;
        initializeFAT32(diskPath, &diskInfo, &bootSector, &fsInfo, sectorsNumber, sectorSize, sectorsPerCluster);
        uint64_t totalWriteTime = 0, totalBytesWritten = 0;

        std:: cout << "\n------------------------------- " << sectorsPerCluster << " Sectors Per Cluster -------------------------------\n";

        for(int i = 100; i <= 99 + numOfFiles; i++)
        {
            memcpy(parentPathCopy, parentPath, 50);
            memcpy(fileNameCopy, fileName, 50);

            memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(i).c_str(), 3);

            result = fat32_create_directory(diskInfo, bootSector, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
        }

        for(int i = 100; i <= 99 + numOfFiles; i++)
        {
            memcpy(fullFilePathCopy, fullFilePath, 50);

            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(i).c_str(), 3);

            result = fat32_write_file(diskInfo, bootSector, fullFilePathCopy, buffer, bufferSize, TRUNCATE,
                                      numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

            totalWriteTime += timeElapsedMilliseconds;
            totalBytesWritten += numberOfBytesWritten;
        }

        std::cout << "Number of bytes written: " <<  totalBytesWritten << "/" << bufferSize * numOfFiles << '\n';
        printDurationSolo(totalWriteTime);
        sectorsPerCluster *= 2;
        deleteFiles(diskPath);
    }
}

void fat32_test_3()
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

    while(sectorsPerCluster <= 16)
    {
        DiskInfo* diskInfo;
        BootSector* bootSector;
        FsInfo* fsInfo;
        initializeFAT32(diskPath, &diskInfo, &bootSector, &fsInfo, sectorsNumber, sectorSize, sectorsPerCluster);
        uint64_t totalWriteTime = 0, totalBytesWritten = 0;

        std:: cout << "\n------------------------------- " << sectorsPerCluster << " Sectors Per Cluster -------------------------------\n";

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

            totalWriteTime += timeElapsedMilliseconds;
            totalBytesWritten += numberOfBytesWritten;

            //medium buffer
            memcpy(parentPathCopy, parentPath, 50);
            memcpy(fileNameCopy, fileName, 50);
            memcpy(fullFilePathCopy, fullFilePath, 50);

            memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(i * 3 + 1).c_str(), 2);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(i * 3 + 1).c_str(), 2);

            result = fat32_create_directory(diskInfo, bootSector, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
            result = fat32_write_file(diskInfo, bootSector, fullFilePathCopy, mediumBuffer, mediumBufferSize, TRUNCATE,
                                      numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

            totalWriteTime += timeElapsedMilliseconds;
            totalBytesWritten += numberOfBytesWritten;

            //big buffer
            memcpy(parentPathCopy, parentPath, 50);
            memcpy(fileNameCopy, fileName, 50);
            memcpy(fullFilePathCopy, fullFilePath, 50);

            memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(i * 3 + 2).c_str(), 2);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(i * 3 + 2).c_str(), 2);

            result = fat32_create_directory(diskInfo, bootSector, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
            result = fat32_write_file(diskInfo, bootSector, fullFilePathCopy, bigBuffer, bigBufferSize, TRUNCATE,
                                      numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

            totalWriteTime += timeElapsedMilliseconds;
            totalBytesWritten += numberOfBytesWritten;
        }

        std::cout << "Number of bytes written: " <<  totalBytesWritten << "/" << (smallBufferSize + mediumBufferSize + bigBufferSize) * numOfRounds << '\n';
        printDurationSolo(totalWriteTime);
        sectorsPerCluster *= 2;
        deleteFiles(diskPath);
    }
}

void fat32_test_4()
{
    uint64_t bufferSize = 10000000;

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
    uint32_t numberOfBytesWritten, reasonForIncompleteWrite, result, writeMode;

    uint32_t sectorsPerCluster = 4;
    int round = 1;

    while(sectorsPerCluster <= 16)
    {
        DiskInfo* diskInfo;
        BootSector* bootSector;
        FsInfo* fsInfo;
        initializeFAT32(diskPath, &diskInfo, &bootSector, &fsInfo, sectorsNumber, sectorSize, sectorsPerCluster);

        memcpy(parentPathCopy, parentPath, 50);
        memcpy(fileNameCopy, fileName, 50);
        memcpy(fullFilePathCopy, fullFilePath, 50);

        if(round == 1)
        {
            std:: cout << "\n------------------------------- " << sectorsPerCluster << " Sectors Per Cluster -------------------------------\n";
            std::cout << "Round 1\n";
            writeMode = TRUNCATE;
        }
        else
        {
            std::cout << "\nRound 2\n";
            writeMode = APPEND;
        }

        result = fat32_create_directory(diskInfo, bootSector, parentPathCopy, fileNameCopy,
                                        FILE, timeElapsedMilliseconds);
        result = fat32_write_file(diskInfo, bootSector, fullFilePathCopy, buffer, bufferSize, writeMode,
                                  numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

        std::cout << "Number of bytes written: " <<  numberOfBytesWritten << "/" << bufferSize << '\n';
        printDurationSolo(timeElapsedMilliseconds);

        if(round == 2)
        {
            sectorsPerCluster *= 2;
            round = 1;
            deleteFiles(diskPath);
        }
        else
            round++;
    }
}

void fat32_test_5()
{
    uint64_t bigBufferSize = 1000000;
    uint64_t smallBufferSize = 10000;
    uint32_t numOfRounds = 50;

    char* diskPath = new char[100];
    memcpy(diskPath, "D:\\Facultate\\Licenta\\HardDisks\\Automatic_Test_Solo\0", 100);

    char* bigBuffer = generateBuffer(bigBufferSize);
    char* smallBuffer = generateBuffer(smallBufferSize);
    uint32_t sectorsNumber = 204800;
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
    uint32_t numberOfBytesWritten, reasonForIncompleteWrite, result;

    uint32_t sectorsPerCluster = 4;

    while(sectorsPerCluster <= 16)
    {
        DiskInfo* diskInfo;
        BootSector* bootSector;
        FsInfo* fsInfo;
        initializeFAT32(diskPath, &diskInfo, &bootSector, &fsInfo, sectorsNumber, sectorSize, sectorsPerCluster);
        uint64_t totalWriteTimeOfBigFile = 0, totalBytesWritten = 0;

        std:: cout << "\n------------------------------- " << sectorsPerCluster << " Sectors Per Cluster -------------------------------\n";

        for(int i = 0; i < numOfRounds; i++)
        {
            //big file buffer
            memcpy(parentPathCopy, parentPath, 50);
            memcpy(bigFileNameCopy, bigFileName, 50);
            memcpy(bigFileFullPathCopy, bigFileFullPath, 50);

            uint32_t writeMode = (i == 0) ? TRUNCATE : APPEND;
            result = fat32_create_directory(diskInfo, bootSector, parentPathCopy, bigFileNameCopy, FILE, timeElapsedMilliseconds);
            result = fat32_write_file(diskInfo, bootSector, bigFileFullPathCopy, bigBuffer, bigBufferSize, writeMode,
                                      numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

            totalWriteTimeOfBigFile += timeElapsedMilliseconds;
            totalBytesWritten += numberOfBytesWritten;

            //small files
            for(uint32_t j = 0; j < 2; j++)
            {
                //first small file
                memcpy(parentPathCopy, parentPath, 50);
                memcpy(fileNameCopy, fileName, 50);
                memcpy(fullFilePathCopy, fullFilePath, 50);

                memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(j * 3).c_str(), 2);
                memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(j * 3).c_str(), 2);

                result = fat32_create_directory(diskInfo, bootSector, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
                result = fat32_write_file(diskInfo, bootSector, fullFilePathCopy, smallBuffer, smallBufferSize, TRUNCATE,
                                          numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

                //second small file
                memcpy(parentPathCopy, parentPath, 50);
                memcpy(fileNameCopy, fileName, 50);
                memcpy(fullFilePathCopy, fullFilePath, 50);

                memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(j * 3 + 1).c_str(), 2);
                memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(j * 3 + 1).c_str(), 2);

                result = fat32_create_directory(diskInfo, bootSector, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
                result = fat32_write_file(diskInfo, bootSector, fullFilePathCopy, smallBuffer, smallBufferSize, TRUNCATE,
                                          numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);
            }
        }

        std::cout << "Number of bytes written in big file: " <<  totalBytesWritten << "/" << bigBufferSize * numOfRounds << '\n';
        printDurationSolo(totalWriteTimeOfBigFile);
        sectorsPerCluster *= 2;
        deleteFiles(diskPath);
    }
}

void fat32_test_6()
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
    uint32_t numberOfBytesInBuffer, reasonForIncompleteOperation, result;

    uint32_t sectorsPerCluster = 4;

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
                                  numberOfBytesInBuffer, reasonForIncompleteOperation, timeElapsedMilliseconds);

        memcpy(parentPathCopy, parentPath, 50);
        memcpy(fullFilePathCopy, fullFilePath, 50);

        result = fat32_read_file(diskInfo, bootSector, fullFilePathCopy, buffer, bufferSize, 0,
                                  numberOfBytesInBuffer, reasonForIncompleteOperation, timeElapsedMilliseconds);

        std:: cout << "\n------------------------------- " << sectorsPerCluster << " Sectors Per Cluster -------------------------------\n";

        std::cout << "Number of bytes read: " <<  numberOfBytesInBuffer << "/" << bufferSize << '\n';
        printDurationSolo(timeElapsedMilliseconds);

        sectorsPerCluster *= 2;
        deleteFiles(diskPath);
    }
}

void fat32_test_7()
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
    uint32_t numberOfBytesInBuffer, numberOfBytesRead, reasonForIncompleteOperation, result;

    uint32_t sectorsPerCluster = 4;

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
                                  numberOfBytesInBuffer, reasonForIncompleteOperation, timeElapsedMilliseconds);

        memcpy(parentPathCopy, parentPath, 50);
        memcpy(fullFilePathCopy, fullFilePath, 50);

        result = fat32_read_file(diskInfo, bootSector, fullFilePathCopy, buffer, bufferSize / 2, bufferSize / 4,
                                 numberOfBytesRead, reasonForIncompleteOperation, timeElapsedMilliseconds);

        std:: cout << "\n------------------------------- " << sectorsPerCluster << " Sectors Per Cluster -------------------------------\n";

        std::cout << "Number of bytes read: " <<  numberOfBytesInBuffer << "/" << bufferSize / 2 << " --- Start: " << bufferSize / 4 << " End: " << (bufferSize / 4) * 3 << '\n';
        printDurationSolo(timeElapsedMilliseconds);

        sectorsPerCluster *= 2;
        deleteFiles(diskPath);
    }
}

void fat32_test_8()
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
    uint32_t numberOfBytesInBuffer, reasonForIncompleteOperation, result;

    uint32_t sectorsPerCluster = 4;

    while(sectorsPerCluster <= 16)
    {
        DiskInfo* diskInfo;
        BootSector* bootSector;
        FsInfo* fsInfo;
        initializeFAT32(diskPath, &diskInfo, &bootSector, &fsInfo, sectorsNumber, sectorSize, sectorsPerCluster);
        uint64_t totalReadTime = 0, totalBytesRead = 0;

        std:: cout << "\n------------------------------- " << sectorsPerCluster << " Sectors Per Cluster -------------------------------\n";

        for(int i = 100; i <= 99 + numOfFiles; i++)
        {
            memcpy(parentPathCopy, parentPath, 50);
            memcpy(fileNameCopy, fileName, 50);
            memcpy(fullFilePathCopy, fullFilePath, 50);

            memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(i).c_str(), 3);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(i).c_str(), 3);

            result = fat32_create_directory(diskInfo, bootSector, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
            result = fat32_write_file(diskInfo, bootSector, fullFilePathCopy, buffer, bufferSize, TRUNCATE,
                                      numberOfBytesInBuffer, reasonForIncompleteOperation, timeElapsedMilliseconds);
        }

        for(int i = 100; i <= 99 + numOfFiles; i++)
        {
            memcpy(fullFilePathCopy, fullFilePath, 50);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(i).c_str(), 3);

            result = fat32_read_file(diskInfo, bootSector, fullFilePathCopy, buffer, bufferSize, 0,
                                     numberOfBytesInBuffer, reasonForIncompleteOperation, timeElapsedMilliseconds);

            totalReadTime += timeElapsedMilliseconds;
            totalBytesRead += numberOfBytesInBuffer;
        }


        std::cout << "Number of bytes read: " <<  totalBytesRead << "/" << bufferSize * numOfFiles << '\n';
        printDurationSolo(totalReadTime);
        sectorsPerCluster *= 2;
        deleteFiles(diskPath);
    }
}

void fat32_test_9()
{
    uint64_t bigBufferSize = 1000000;
    uint64_t smallBufferSize = 10000;
    uint32_t numOfRounds = 50;

    char* diskPath = new char[100];
    memcpy(diskPath, "D:\\Facultate\\Licenta\\HardDisks\\Automatic_Test_Solo\0", 100);

    char* bigBuffer = generateBuffer(bigBufferSize);
    char* smallBuffer = generateBuffer(smallBufferSize);
    uint32_t sectorsNumber = 204800;
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
    uint32_t numberOfBytesInBuffer, reasonForIncompleteOperation, result;

    uint32_t sectorsPerCluster = 4;

    while(sectorsPerCluster <= 16)
    {
        DiskInfo* diskInfo;
        BootSector* bootSector;
        FsInfo* fsInfo;
        initializeFAT32(diskPath, &diskInfo, &bootSector, &fsInfo, sectorsNumber, sectorSize, sectorsPerCluster);
        uint64_t totalReadTimeOfBigFile = 0, totalBytesRead = 0;

        std:: cout << "\n------------------------------- " << sectorsPerCluster << " Sectors Per Cluster -------------------------------\n";

        for(int i = 0; i < numOfRounds; i++)
        {
            //big file buffer
            memcpy(parentPathCopy, parentPath, 50);
            memcpy(bigFileNameCopy, bigFileName, 50);
            memcpy(bigFileFullPathCopy, bigFileFullPath, 50);

            uint32_t writeMode = (i == 0) ? TRUNCATE : APPEND;
            result = fat32_create_directory(diskInfo, bootSector, parentPathCopy, bigFileNameCopy, FILE, timeElapsedMilliseconds);
            result = fat32_write_file(diskInfo, bootSector, bigFileFullPathCopy, bigBuffer, bigBufferSize, writeMode,
                                      numberOfBytesInBuffer, reasonForIncompleteOperation, timeElapsedMilliseconds);

            //small files
            for(uint32_t j = 0; j < 2; j++)
            {
                //first small file
                memcpy(parentPathCopy, parentPath, 50);
                memcpy(fileNameCopy, fileName, 50);
                memcpy(fullFilePathCopy, fullFilePath, 50);

                memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(j * 3).c_str(), 2);
                memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(j * 3).c_str(), 2);

                result = fat32_create_directory(diskInfo, bootSector, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
                result = fat32_write_file(diskInfo, bootSector, fullFilePathCopy, smallBuffer, smallBufferSize, TRUNCATE,
                                          numberOfBytesInBuffer, reasonForIncompleteOperation, timeElapsedMilliseconds);

                //second small file
                memcpy(parentPathCopy, parentPath, 50);
                memcpy(fileNameCopy, fileName, 50);
                memcpy(fullFilePathCopy, fullFilePath, 50);

                memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(j * 3 + 1).c_str(), 2);
                memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(j * 3 + 1).c_str(), 2);

                result = fat32_create_directory(diskInfo, bootSector, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
                result = fat32_write_file(diskInfo, bootSector, fullFilePathCopy, smallBuffer, smallBufferSize, TRUNCATE,
                                          numberOfBytesInBuffer, reasonForIncompleteOperation, timeElapsedMilliseconds);
            }
        }

        for(int i = 0; i < numOfRounds; i++)
        {
            memcpy(bigFileFullPathCopy, bigFileFullPath, 50);

            result = fat32_read_file(diskInfo, bootSector, bigFileFullPathCopy, bigBuffer, bigBufferSize, i * bigBufferSize,
                                     numberOfBytesInBuffer, reasonForIncompleteOperation, timeElapsedMilliseconds);

            totalReadTimeOfBigFile += timeElapsedMilliseconds;
            totalBytesRead += numberOfBytesInBuffer;
        }

        std::cout << "Number of bytes read from big file: " <<  totalBytesRead << "/" << bigBufferSize * numOfRounds << '\n';
        printDurationSolo(totalReadTimeOfBigFile);
        sectorsPerCluster *= 2;
        deleteFiles(diskPath);
    }
}