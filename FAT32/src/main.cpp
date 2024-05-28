#include "iostream"
#include "string"
#include "string.h"
#include "stdio.h"
#include "windows.h"
#include "vector"
#include "fstream"

#include "disk.h"
#include "diskUtils.h"
#include "diskCodes.h"
#include "../include/fat32Init.h"
#include "../include/fat32.h"
#include "../include/fat32Api.h"
#include "../include/fat32Structures.h"
#include "../include/codes/fat32Attributes.h"
#include "../include/utils.h"
#include "../include/interface.h"
#include "../include/fat32TestApi.h"

int main() {
    char* diskDirectory = "D:\\Facultate\\Licenta\\HardDisks\\HardDisk_100Mib";
    DiskInfo* diskInfo = nullptr;
    BootSector* bootSector = nullptr;
    FsInfo* fsInfo = nullptr;

    fat32Startup(diskDirectory, &diskInfo, &bootSector, &fsInfo, 204800, 512, 8,true);

    std::string command;
    std::cout << "Waiting commands\n";

    while(true)
    {
        std::cout << '\n';
        std::getline(std::cin, command);

        if(command == "exit")
        {
            std::cout << "Process terminated with exit";
            break;
        }

        std::vector<std::string> tokens = fat32_splitString(command, ' ');

        if(tokens[0] == "mkdir")
            commandCreateDirectory(diskInfo, bootSector, tokens);
        else if(tokens[0] == "ls")
            commandListSubdirectories(diskInfo, bootSector, tokens);
        else if(tokens[0] == "write")
            commandWriteFile(diskInfo, bootSector, tokens);
        else if(tokens[0] == "read")
            commandReadFile(diskInfo, bootSector, tokens);
        else if(tokens[0] == "truncate")
            commandTruncateFile(diskInfo, bootSector, tokens);
        else if(tokens[0] == "rmdir")
            commandDeleteDirectory(diskInfo, bootSector, tokens);
        else if(tokens[0] == "la")
            commandShowDirectoryAttributes(diskInfo, bootSector, tokens);
        else
            std::cout << "Unknown command \n";
    }

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

    uint32_t result = fat32_create_directory(diskInfo, bootSector, parentPath, fileName,
                                    DIRECTORY_TYPE_FILE, timeElapsedMilliseconds);
    result = fat32_write_file(diskInfo, bootSector, fullFilePath, buffer, bufferSize, WRITE_WITH_TRUNCATE,
                              numberOfBytesWritten, reasonForIncompleteWrite, timeElapsedMilliseconds);

    uint64_t totalSeconds = timeElapsedMilliseconds / 1000;
    uint64_t millisecondsDisplayed = timeElapsedMilliseconds % 1000;
    uint64_t secondsDisplayed = totalSeconds % 60;
    uint64_t minutesDisplayed = totalSeconds / 60;
    std::cout << result << " --- Time --- Minutes: " << minutesDisplayed << " --- Seconds: " << secondsDisplayed << " --- Milliseconds: " << millisecondsDisplayed <<  '\n';

    return 0;
}
