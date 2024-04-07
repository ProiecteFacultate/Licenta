#include <iostream>
#include "string.h"

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
        std::getline(std::cin, command);

        if(command == "exit")
        {
            std::cout << "Process terminated with exit";
            break;
        }

        std::vector<std::string> tokens = splitString(command, ' ');

        if(tokens[0] == "mkdir")
            commandCreateDirectory(diskInfo, superBlock, tokens);
        else if(tokens[0] == "ls")
            commandListSubdirectories(diskInfo, superBlock, tokens);
        else if(tokens[0] == "write")
            commandWriteFile(diskInfo, superBlock, tokens);
        else if(tokens[0] == "read")
            commandReadFile(diskInfo, superBlock, tokens);
        else
            std::cout << "Unknown command \n";
    }

//    uint32_t numOfSectorsRead;
//    char* readBuffer = new char[1024];
//    readDiskSectors(diskInfo, 2, 2, readBuffer, numOfSectorsRead);
//
//    char* readBuffer_2 = new char[512];
//    readDiskSectors(diskInfo, 1, 4, readBuffer_2, numOfSectorsRead);
//    ext2_group_desc groupDesc1 = *(ext2_group_desc *)&readBuffer_2[0];
//    ext2_group_desc groupDesc2 = *(ext2_group_desc *)&readBuffer_2[32];
//
//    char* readBuffer_3 = new char[512];
//    readDiskSectors(diskInfo, 1, 6, readBuffer_3, numOfSectorsRead);
//
//    char* readBuffer_4 = new char[512];
//    readDiskSectors(diskInfo, 1, 8, readBuffer_4, numOfSectorsRead);
//
//    char* readBuffer_5 = new char[512];
//    readDiskSectors(diskInfo, 1, 10, readBuffer_5, numOfSectorsRead);
//    ext2_inode rootInode = *(ext2_inode*)&readBuffer_5[0];
//
//    char* parentName = new char[50];
//    memcpy(parentName, "Root\0", 5);
//    char* newDirectoryName = new char[50];
//    uint32_t createDirectoryResult;
//
//    for(uint32_t i = 1; i <= 9; i++)
//    {
//        memset(newDirectoryName, 0, 50);
//        memcpy(newDirectoryName, "Dir_", 4);
//        int charsWritten = sprintf(newDirectoryName + 4, "%d", i);
//        createDirectoryResult = createDirectory(diskInfo, superBlock, parentName, newDirectoryName, FILE_TYPE_FOLDER);
//    }
//
//    memcpy(parentName, "Root/Dir_1/File_1\0", 30);
//    memcpy(newDirectoryName, "File_1\0", 8);
//    createDirectoryResult = createDirectory(diskInfo, superBlock, parentName, newDirectoryName, FILE_TYPE_REGULAR_FILE);
//
//    char* readBuffer_6 = new char[2048];
//    readDiskSectors(diskInfo, 4, 266, readBuffer_6, numOfSectorsRead);
//    ext2_dir_entry dir1Entry = *(ext2_dir_entry*)&readBuffer_6[0];
//    ext2_dir_entry dir2Entry = *(ext2_dir_entry*)&readBuffer_6[128];

    return 0;
}
