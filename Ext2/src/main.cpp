#include <iostream>
#include "string.h"
#include "chrono"

#include "../include/disk.h"
#include "../include/diskUtils.h"
#include "../include/diskCodes.h"
#include "../include/codes/ext2Attributes.h"
#include "../include/ext2Init.h"
#include "../include/ext2Api.h"
#include "../include/utils.h"
#include "../include/interface.h"

int main() {

    char* diskDirectory = "D:\\Facultate\\Licenta\\HardDisks\\HardDisk_16Mib\0";
    DiskInfo* diskInfo = nullptr;
    ext2Startup(diskDirectory, &diskInfo, 32760, 512); //16Mib
    ext2_super_block* superBlock = readFirstSuperBlock(diskInfo);

    std::string command;
    std::cout << "Waiting commands\n";

    while(true)
    {
        std::cout << '\n';
        std::cout.flush();
        std::getline(std::cin, command);

        if(command == "exit")
        {
            std::cout << "Process terminated with exit";
            break;
        }

        std::vector<std::string> tokens = splitString(command, ' ');

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        if(tokens[0] == "mkdir")
            commandCreateDirectory(diskInfo, superBlock, tokens);
        else if(tokens[0] == "ls")
            commandListSubdirectories(diskInfo, superBlock, tokens);
        else if(tokens[0] == "write")
            commandWriteFile(diskInfo, superBlock, tokens);
        else if(tokens[0] == "read")
            commandReadFile(diskInfo, superBlock, tokens);
        else if(tokens[0] == "truncate")
            commandTruncateFile(diskInfo, superBlock, tokens);
        else
            std::cout << "Unknown command \n";

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << "Operation time = " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << " : "
                   << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() % 1000  << '\n';
    }

    uint32_t numOfSectorsRead;

    char* readBuffer_1 = new char[1024];
    readDiskSectors(diskInfo, 1, 6, readBuffer_1, numOfSectorsRead);

    char* readBuffer_2 = new char[1024];
    readDiskSectors(diskInfo, 1, 8, readBuffer_2, numOfSectorsRead);

    char* readBuffer_3 = new char[1024];
    readDiskSectors(diskInfo, 1, 10, readBuffer_3, numOfSectorsRead);

    char* readBuffer_4 = new char[1024];
    readDiskSectors(diskInfo, 1, 266, readBuffer_4, numOfSectorsRead);

    char* readBuffer_5 = new char[1024];
    readDiskSectors(diskInfo, 1, 282, readBuffer_5, numOfSectorsRead);
    std::cout.write(readBuffer_5, 10);

    return 0;
}
