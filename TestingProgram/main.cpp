#include <iostream>
#include <utils.h>
#include <interface.h>

#include "fat32Api.h"
#include ""

int main() {
    char* diskDirectory = "D:\\Facultate\\Licenta\\HardDisks\\HardDisk_512Kib\0";
    DiskInfo* diskInfo = nullptr;
    BootSector* bootSector = nullptr;
    FsInfo* fsInfo = nullptr;

    initialLoad(diskDirectory, &diskInfo, &bootSector, &fsInfo);

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
