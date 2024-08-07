#include <iostream>
#include "string.h"
#include "chrono"

#include "../include/disk.h"
#include "../include/diskUtils.h"
#include "../include/diskCodes.h"
#include "../include/codes/ext2Attributes.h"
#include "../include/ext2Init.h"
#include "../include/ext2Api.h"
#include "../include/ext2FunctionUtils.h"
#include "../include/utils.h"
#include "../include/interface.h"
#include "../include/ext2TestApi.h"

int main() {

    char* diskDirectory = "D:\\Facultate\\Licenta\\HardDisks\\HardDisk_100Mib\0";
    DiskInfo* diskInfo = nullptr;
    ext2Startup(diskDirectory, &diskInfo, 204800, 512, 8192, true); //16Mib
    ext2_super_block* superBlock = readFirstSuperBlock(diskInfo);

    std::string command;
//    std::cout << "Waiting commands\n";

//    while(true)
//    {
//        std::cout << '\n';
//        std::cout.flush();
//        std::getline(std::cin, command);
//
//        if(command == "exit")
//        {
//            std::cout << "Process terminated with exit";
//            break;
//        }
//
//        std::vector<std::string> tokens = ext2_splitString(command, ' ');
//
//        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
//
//        if(tokens[0] == "mkdir")
//            commandCreateDirectory(diskInfo, superBlock, tokens);
//        else if(tokens[0] == "ls")
//            commandListSubdirectories(diskInfo, superBlock, tokens);
//        else if(tokens[0] == "write")
//            commandWriteFile(diskInfo, superBlock, tokens);
//        else if(tokens[0] == "read")
//            commandReadFile(diskInfo, superBlock, tokens);
//        else if(tokens[0] == "truncate")
//            commandTruncateFile(diskInfo, superBlock, tokens);
//        else if(tokens[0] == "la")
//            commandShowDirectoryAttributes(diskInfo, superBlock, tokens);
//        else if(tokens[0] == "rmdir")
//            commandDeleteDirectory(diskInfo, superBlock, tokens);
//        else if(tokens[0] == "preallocate")
//            commandPreallocateBlocks(diskInfo, superBlock, tokens);
//        else
//            std::cout << "Unknown command \n";
//
//        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
//        std::cout << "Operation time = " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << " : "
//                   << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() % 1000  << '\n';
//    }

    char* parentPath = new char[50];
    memcpy(parentPath, "Root\0", 50);
    char* fileName = new char[50];
    memcpy(fileName, "File_1\0", 50);
    char* fullFilePath = new char[50];
    memcpy(fullFilePath, "Root/File_1\0", 50);
    int64_t timeElapsedMilliseconds;
    uint32_t numberOfBytesWritten, reasonForIncompleteWrite;
    uint64_t bufferSize = 80000000;
    char* buffer = new char[bufferSize];

    uint32_t result = ext2_create_directory(diskInfo, superBlock, parentPath, fileName, DIRECTORY_TYPE_FILE, timeElapsedMilliseconds);
    result = ext2_write_file(diskInfo, superBlock, fullFilePath, buffer, bufferSize, WRITE_WITH_TRUNCATE,
                              numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

    uint64_t totalSeconds = timeElapsedMilliseconds / 1000;
    uint64_t millisecondsDisplayed = timeElapsedMilliseconds % 1000;
    uint64_t secondsDisplayed = totalSeconds % 60;
    uint64_t minutesDisplayed = totalSeconds / 60;
    std::cout << result << " --- Time --- Minutes: " << minutesDisplayed << " --- Seconds: " << secondsDisplayed << " --- Milliseconds: " << millisecondsDisplayed <<  '\n';

    return 0;
}
