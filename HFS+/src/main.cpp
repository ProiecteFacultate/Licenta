#include <iostream>
#include "string.h"
#include "vector"
#include "chrono"

#include "../include/disk.h"
#include "../include/diskUtils.h"
#include "../include/diskCodes.h"
#include "../include/structures.h"
#include "../include/hfsInit.h"
#include "../include/utils.h"
#include "../include/hfsFunctionUtils.h"
#include "../include/catalog_file//bTreeCatalog.h"
#include "../include/codes/hfsAttributes.h"
#include "../include/hfsApi.h"
#include "../include/interface.h"
#include "../include/hfsTestApi.h"

int main() {
    char* diskDirectory = "D:\\Facultate\\Licenta\\HardDisks\\HardDisk_100Mib\0";
    DiskInfo* diskInfo = nullptr;
    hfsStartup(diskDirectory, &diskInfo, 204800, 512); //100Mib
    HFSPlusVolumeHeader* volumeHeader = readVolumeHeader(diskInfo);
    ExtentsFileHeaderNode* extentsOverflowFileHeaderNode = readExtentsOverflowFileHeaderNode(diskInfo, volumeHeader);
    CatalogFileHeaderNode* catalogFileHeaderNode = readCatalogFileHeaderNode(diskInfo, volumeHeader);

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
            commandCreateDirectory(diskInfo, volumeHeader, catalogFileHeaderNode, tokens);
        else if(tokens[0] == "ls")
            commandListSubdirectories(diskInfo, volumeHeader, catalogFileHeaderNode, tokens);
        else if(tokens[0] == "write")
            commandWriteFile(diskInfo, volumeHeader, catalogFileHeaderNode, extentsOverflowFileHeaderNode, tokens);
        else if(tokens[0] == "read")
            commandReadFile(diskInfo, volumeHeader, catalogFileHeaderNode, extentsOverflowFileHeaderNode, tokens);
        else if(tokens[0] == "truncate")
            commandTruncateFile(diskInfo, volumeHeader, catalogFileHeaderNode, extentsOverflowFileHeaderNode, tokens);
        else if(tokens[0] == "la")
            commandShowDirectoryAttributes(diskInfo, volumeHeader, catalogFileHeaderNode, tokens);
        else if(tokens[0] == "rmdir")
            commandDeleteDirectory(diskInfo, volumeHeader, catalogFileHeaderNode, extentsOverflowFileHeaderNode, tokens);
        else
            std::cout << "Unknown command \n";

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << "Operation time = " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << " : "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() % 1000  << '\n';
    }

  //  return 0;


    char* parentPath = new char[100];
    memcpy(parentPath, "Root\0", 5);
    char* newDirName = new char[100];
    memcpy(newDirName, "File\0", 5);
    int64_t mil;
    hfs_create_directory(diskInfo, volumeHeader, catalogFileHeaderNode, parentPath, newDirName, 0, mil);
    std::vector<CatalogDirectoryRecord*> subDirectories;
    memcpy(parentPath, "Root\0", 5);
    hfs_get_subdirectories(diskInfo, volumeHeader, catalogFileHeaderNode, parentPath, subDirectories, mil);
//
//    char* parentPath = new char[100];
//    memcpy(parentPath, "Root\0", 5);
//    char* newDirName = new char[100];

//    for(int i = 10; i <= 99; i++)
//    {
//        memcpy(newDirName, "Dir_", 4);
//        memcpy(newDirName + 4, std::to_string(i).c_str(), 2);
//        newDirName[6] = 0;
//
//        uint32_t createDirectoryResult = createDirectory(diskInfo, volumeHeader, catalogFileHeaderNode, parentPath,
//                        newDirName, DIRECTORY_TYPE_FOLDER);
//
//        std::cout << "Create " << newDirName << ": " << createDirectoryResult << '\n';
//    }

//    memcpy(parentPath, "Root/Dir_10/Dir_L2\0", 19);
//    memcpy(newDirName, "F", 1);
//    newDirName[1] = 0;
//    uint32_t createDirectoryResult = createDirectory(diskInfo, volumeHeader, catalogFileHeaderNode, parentPath,
//                                                     newDirName, DIRECTORY_TYPE_FILE);
//
//    std::cout << "Create " << newDirName << ": " << createDirectoryResult << '\n';
//
//    char* readBuffer = new char[8000000];
//    uint32_t numberOfSectorsRead = 0;
//    uint32_t readResult = readDiskSectors(diskInfo, 1, 4, readBuffer, numberOfSectorsRead);
//    BTNodeDescriptor* extentsHeaderNode = (BTNodeDescriptor*)&readBuffer[0];
//    BTHeaderRec* extentsHeaderRecord = (BTHeaderRec*)&readBuffer[14];

//    readResult = readDiskSectors(diskInfo, 2, 2078, readBuffer, numberOfSectorsRead);
//    BTNodeDescriptor* catalogHeaderNode = (BTNodeDescriptor*)&readBuffer[0];

    return 0;
}
