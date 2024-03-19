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
#include "../include/structures.h"
#include "../include/fat32Attributes.h"
#include "../include/utils.h"
#include "../include/interface.h"

int main() {
    char* diskDirectory = "D:\\Facultate\\Licenta\\HardDisks\\HardDisk_512Kib";
    DiskInfo* diskInfo = nullptr;
    BootSector* bootSector = nullptr;
    FsInfo* fsInfo = nullptr;

    fat32Startup(diskDirectory, &diskInfo, &bootSector, &fsInfo, 1024, 512);

    std::string command;
    std::cout << "Waiting commands\n";

//    char* filePath = new char[100];
//    memset(filePath, 0, 100);
//    memcpy(filePath, "Root/File_1", 11);
//
//    uint32_t numberOfBytesRead;
//    uint32_t reasonForIncompleteRead;
//    char* readBuffer = new char[100000];
//    memset(readBuffer, 0, 100000);
//    read(diskInfo, bootSector, filePath, readBuffer, 0, 100000,numberOfBytesRead, reasonForIncompleteRead);
//    std::cout << "Bytes read: " << numberOfBytesRead << '\n';
//    std::cout.write(readBuffer, numberOfBytesRead);

    while(true)
    {
        std::cout << '\n';
        std::getline(std::cin, command);

        if(command == "exit")
        {
            std::cout << "Process terminated with exit";
            break;
        }

        std::vector<std::string> tokens = splitString(command, ' ');

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

    return 0;
}
