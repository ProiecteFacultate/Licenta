#include "iostream"

#include "hfsTestApi.h"

#include "../../include/utils.h"
#include "../../include/attributes.h"
#include "../../include/time_tests/hfsTimeTests.h"

void hfs_time_test_1()
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

        uint64_t totalSeconds = timeElapsedMilliseconds / 1000;
        uint32_t bytesPerSecond = numberOfBytesWritten / totalSeconds;
        std::cout << "Number of bytes written: " <<  numberOfBytesWritten << "/" << bufferSize << " --- " << bytesPerSecond << " bytes/second --- ";
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

void hfs_time_test_2()
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

            memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(i).c_str(), 3);

            result = hfs_create_directory(diskInfo, volumeHeader, catalogFileHeaderNode, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
        }

        for(int i = 100; i <= 99 + numOfFiles; i++)
        {
            memcpy(fullFilePathCopy, fullFilePath, 50);

            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(i).c_str(), 3);

            result = hfs_write_file(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, fullFilePathCopy, buffer, bufferSize,
                                    TRUNCATE, numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

            totalWriteTime += timeElapsedMilliseconds;
            totalBytesWritten += numberOfBytesWritten;
        }

        uint64_t totalSeconds = totalWriteTime / 1000;
        uint32_t bytesPerSecond = totalBytesWritten / totalSeconds;
        std::cout << "Number of bytes written: " <<  totalBytesWritten << "/" << bufferSize * numOfFiles << " --- " << bytesPerSecond << " bytes/second --- ";
        printDurationSolo(totalWriteTime);
        blockSize *= 2;
        deleteFiles(diskPath);
    }
}

void hfs_time_test_3()
{
    uint64_t bigBufferSize = 9000000;
    uint64_t mediumBufferSize = 900000;
    uint64_t smallBufferSize = 9000;
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

        uint64_t totalSeconds = totalWriteTime / 1000;
        uint32_t bytesPerSecond = totalBytesWritten / totalSeconds;
        std::cout << "Number of bytes written: " <<  totalBytesWritten << "/" << (smallBufferSize + mediumBufferSize + bigBufferSize) * numOfRounds << " --- "
                  << bytesPerSecond << " bytes/second --- ";
        printDurationSolo(totalWriteTime);
        blockSize *= 2;
        deleteFiles(diskPath);
    }
}

void hfs_time_test_4()
{
    uint64_t bufferSize = 25000000;

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

        if(round == 1)
        {
            std::cout << "\n------------------------------- " << blockSize << " Block Size -------------------------------\n";
            std::cout << "Write with TRUNCATE\n";
            writeMode = TRUNCATE;
        }
        else
        {
            std::cout << "Write with APPEND\n";
            writeMode = APPEND;
        }

        result = hfs_create_directory(diskInfo, volumeHeader, catalogFileHeaderNode, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
        result = hfs_write_file(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, fullFilePathCopy, buffer, bufferSize,
                                writeMode, numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

        uint64_t totalSeconds = timeElapsedMilliseconds / 1000;
        uint32_t bytesPerSecond = numberOfBytesWritten / totalSeconds;
        std::cout << "Number of bytes written: " <<  numberOfBytesWritten << "/" << bufferSize << " --- " << bytesPerSecond << " bytes/second --- ";
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

void hfs_time_test_5()
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

    uint32_t blockSize = 2048;

    while(blockSize <= 8192)
    {
        DiskInfo* diskInfo;
        HFSPlusVolumeHeader* volumeHeader;
        ExtentsFileHeaderNode* extentsFileHeaderNode;
        CatalogFileHeaderNode* catalogFileHeaderNode;
        initializeHFS(diskPath, &diskInfo, &volumeHeader, &extentsFileHeaderNode, &catalogFileHeaderNode, sectorsNumber, sectorSize, blockSize);
        uint64_t totalWriteTimeOfBigFile = 0, totalBytesWritten = 0;

        std::cout << "\n------------------------------- " << blockSize << " Block Size -------------------------------\n";

        for(int i = 0; i < numOfRounds; i++)
        {
            //big file buffer
            memcpy(parentPathCopy, parentPath, 50);
            memcpy(bigFileNameCopy, bigFileName, 50);
            memcpy(bigFileFullPathCopy, bigFileFullPath, 50);

            uint32_t writeMode = (i == 0) ? TRUNCATE : APPEND;
            result = hfs_create_directory(diskInfo, volumeHeader, catalogFileHeaderNode, parentPathCopy, bigFileNameCopy, FILE, timeElapsedMilliseconds);
            result = hfs_write_file(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, bigFileFullPathCopy, bigBuffer, bigBufferSize,
                                    writeMode, numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

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

                result = hfs_create_directory(diskInfo, volumeHeader, catalogFileHeaderNode, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
                result = hfs_write_file(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, fullFilePathCopy, smallBuffer, smallBufferSize,
                                        TRUNCATE, numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

                //second small file
                memcpy(parentPathCopy, parentPath, 50);
                memcpy(fileNameCopy, fileName, 50);
                memcpy(fullFilePathCopy, fullFilePath, 50);

                memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(j * 3 + 1).c_str(), 2);
                memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(j * 3 + 1).c_str(), 2);

                result = hfs_create_directory(diskInfo, volumeHeader, catalogFileHeaderNode, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
                result = hfs_write_file(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, fullFilePathCopy, smallBuffer, smallBufferSize,
                                        TRUNCATE, numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);
            }
        }

        uint64_t totalSeconds = totalWriteTimeOfBigFile / 1000;
        uint32_t bytesPerSecond = totalBytesWritten / totalSeconds;
        std::cout << "Number of bytes written in big file: " <<  totalBytesWritten << "/" << bigBufferSize * numOfRounds << " --- " << bytesPerSecond << " bytes/second --- ";
        printDurationSolo(totalWriteTimeOfBigFile);
        blockSize *= 2;
        deleteFiles(diskPath);
    }
}

void hfs_time_test_6()
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

        uint64_t totalSeconds = timeElapsedMilliseconds / 1000;
        uint32_t bytesPerSecond = numberOfBytesInBuffer / totalSeconds;
        std::cout << "Number of bytes read: " <<  numberOfBytesInBuffer << "/" << bufferSize << " --- " << bytesPerSecond << " bytes/second --- ";
        printDurationSolo(timeElapsedMilliseconds);

        blockSize *= 2;
        deleteFiles(diskPath);
    }
}

void hfs_time_test_7()
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
    uint32_t numberOfBytesInBuffer, numberOfBytesRead, reasonForIncompleteWrite, result;

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

        result = hfs_read_file(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, fullFilePathCopy, buffer, bufferSize/ 2,
                               bufferSize / 4, numberOfBytesRead, reasonForIncompleteWrite, timeElapsedMilliseconds);

        std::cout << "\n------------------------------- " << blockSize << " Block Size -------------------------------\n";

        uint64_t totalSeconds = timeElapsedMilliseconds / 1000;
        uint32_t bytesPerSecond = numberOfBytesRead / totalSeconds;
        std::cout << "Number of bytes read: " <<  numberOfBytesRead << "/" << bufferSize / 2 << " --- Start: " << bufferSize / 4 << " End: " << (bufferSize / 4) * 3
                  << " --- " << bytesPerSecond << " bytes/second --- ";
        printDurationSolo(timeElapsedMilliseconds);

        blockSize *= 2;
        deleteFiles(diskPath);
    }
}

void hfs_time_test_8()
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

    uint32_t blockSize = 2048;

    while(blockSize <= 8192)
    {
        DiskInfo* diskInfo;
        HFSPlusVolumeHeader* volumeHeader;
        ExtentsFileHeaderNode* extentsFileHeaderNode;
        CatalogFileHeaderNode* catalogFileHeaderNode;
        initializeHFS(diskPath, &diskInfo, &volumeHeader, &extentsFileHeaderNode, &catalogFileHeaderNode, sectorsNumber, sectorSize, blockSize);
        uint64_t totalReadTime = 0, totalBytesRead = 0;

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
        }

        for(int i = 100; i <= 99 + numOfFiles; i++)
        {
            memcpy(fullFilePathCopy, fullFilePath, 50);
            memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(i).c_str(), 3);

            result = hfs_read_file(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, fullFilePathCopy, buffer, bufferSize, 0,
                                   numberOfBytesInBuffer, reasonForIncompleteOperation, timeElapsedMilliseconds);

            totalReadTime += timeElapsedMilliseconds;
            totalBytesRead += numberOfBytesInBuffer;
        }

        uint64_t totalSeconds = totalReadTime / 1000;
        uint32_t bytesPerSecond = totalBytesRead / totalSeconds;
        std::cout << "Number of bytes read: " <<  totalBytesRead << "/" << bufferSize * numOfFiles  << " --- " << bytesPerSecond << " bytes/second --- ";
        printDurationSolo(totalReadTime);
        blockSize *= 2;
        deleteFiles(diskPath);
    }
}

void hfs_time_test_ignore_1()
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

    uint32_t blockSize = 2048;

    while(blockSize <= 8192)
    {
        DiskInfo* diskInfo;
        HFSPlusVolumeHeader* volumeHeader;
        ExtentsFileHeaderNode* extentsFileHeaderNode;
        CatalogFileHeaderNode* catalogFileHeaderNode;
        initializeHFS(diskPath, &diskInfo, &volumeHeader, &extentsFileHeaderNode, &catalogFileHeaderNode, sectorsNumber, sectorSize, blockSize);
        uint64_t totalReadTimeOfBigFile = 0, totalBytesRead = 0;

        std::cout << "\n------------------------------- " << blockSize << " Block Size -------------------------------\n";

        for(int i = 0; i < numOfRounds; i++)
        {
            //big file buffer
            memcpy(parentPathCopy, parentPath, 50);
            memcpy(bigFileNameCopy, bigFileName, 50);
            memcpy(bigFileFullPathCopy, bigFileFullPath, 50);

            uint32_t writeMode = (i == 0) ? TRUNCATE : APPEND;
            result = hfs_create_directory(diskInfo, volumeHeader, catalogFileHeaderNode, parentPathCopy, bigFileNameCopy, FILE, timeElapsedMilliseconds);
            result = hfs_write_file(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, bigFileFullPathCopy, bigBuffer, bigBufferSize,
                                    writeMode, numberOfBytesInBuffer, reasonForIncompleteOperation, timeElapsedMilliseconds);

            //small files
            for(uint32_t j = 0; j < 2; j++)
            {
                //first small file
                memcpy(parentPathCopy, parentPath, 50);
                memcpy(fileNameCopy, fileName, 50);
                memcpy(fullFilePathCopy, fullFilePath, 50);

                memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(j * 3).c_str(), 2);
                memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(j * 3).c_str(), 2);

                result = hfs_create_directory(diskInfo, volumeHeader, catalogFileHeaderNode, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
                result = hfs_write_file(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, fullFilePathCopy, smallBuffer, smallBufferSize,
                                        TRUNCATE, numberOfBytesInBuffer, reasonForIncompleteOperation, timeElapsedMilliseconds);

                //second small file
                memcpy(parentPathCopy, parentPath, 50);
                memcpy(fileNameCopy, fileName, 50);
                memcpy(fullFilePathCopy, fullFilePath, 50);

                memcpy(fileNameCopy + strlen(fileNameCopy), std::to_string(j * 3 + 1).c_str(), 2);
                memcpy(fullFilePathCopy + strlen(fullFilePathCopy), std::to_string(j * 3 + 1).c_str(), 2);

                result = hfs_create_directory(diskInfo, volumeHeader, catalogFileHeaderNode, parentPathCopy, fileNameCopy, FILE, timeElapsedMilliseconds);
                result = hfs_write_file(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, fullFilePathCopy, smallBuffer, smallBufferSize,
                                        TRUNCATE, numberOfBytesInBuffer, reasonForIncompleteOperation, timeElapsedMilliseconds);
            }
        }

        for(int i = 0; i < numOfRounds ; i++)
        {
            memcpy(bigFileFullPathCopy, bigFileFullPath, 50);

            result = hfs_read_file(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, bigFileFullPathCopy, bigBuffer, bigBufferSize,
                                   0, numberOfBytesInBuffer, reasonForIncompleteOperation, timeElapsedMilliseconds);

            totalReadTimeOfBigFile += timeElapsedMilliseconds;
            totalBytesRead += numberOfBytesInBuffer;
        }

        uint64_t totalSeconds = totalReadTimeOfBigFile / 1000;
        uint32_t bytesPerSecond = totalBytesRead / totalSeconds;
        std::cout << "Number of bytes read from big file: " <<  totalBytesRead << "/" << bigBufferSize * numOfRounds << " --- " << bytesPerSecond << " bytes/second --- ";
        printDurationSolo(totalReadTimeOfBigFile);
        blockSize *= 2;
        deleteFiles(diskPath);
    }
}