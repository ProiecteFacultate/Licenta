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

int initialLoad(char* diskDirectory, DiskInfo** diskInfo, BootSector** bootSector, FsInfo** fsInfo)
{
    *diskInfo = getDisk(diskDirectory);
    if(*diskInfo == nullptr)
    {
        *diskInfo = initializeDisk(diskDirectory, 1024, 512);
        fillDiskInitialMemory(*diskInfo);
        std::cout << "Disk initialized\n";
    }
    else
    {
        std::cout << "Sectors number: " << (*diskInfo)->diskParameters.sectorsNumber << "\n";
        std::cout << "Sector size: " << (*diskInfo)->diskParameters.sectorSizeBytes << "\n";
        std:: cout << "Disk size: " << (*diskInfo)->diskParameters.sectorsNumber * (*diskInfo)->diskParameters.sectorSizeBytes << "\n";
    }

    bool fat32Initialized = false;
    if(!checkBootSectorsInitialized(*diskInfo))
    {
        std::cout << "Initializing boot sectors...\n";
        initializeBootSectors(*diskInfo);
    }
    else
    {
        fat32Initialized = true;
        std::cout << "Boot sectors already initialized\n";
    }

    *bootSector = readBootSector(*diskInfo);
    *fsInfo = readFsInfo(*diskInfo, *bootSector);

    if(fat32Initialized == false)
    {
        initializeFat(*diskInfo, *bootSector);   //initialize only when file system created
    }
}

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
