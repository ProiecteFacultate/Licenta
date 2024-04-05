#include "vector"
#include "string"
#include "string.h"
#include "iostream"

#include "../include/ext2Api.h"
#include "../include/structures.h"
#include "../include/codes/ext2Attributes.h"
#include "../include/codes/ext2ApiResponseCodes.h"
#include "../include/interface.h"

void commandCreateDirectory(DiskInfo* diskInfo, ext2_super_block* superBlock, std::vector<std::string> commandTokens)
{
    if(commandTokens.size() < 4)
    {
        std::cout << "Insufficient arguments for 'mkdir' command!\n";
        return;
    }
    else if(commandTokens.size() > 4)
    {
        std::cout << "Too many arguments for 'mkdir' command!\n";
        return;
    }

    char* parentPath = new char[100];
    memset(parentPath, 0, 100);
    memcpy(parentPath, commandTokens[1].c_str(), commandTokens[1].length());

    char* newDirectoryName = new char[100];
    memset(newDirectoryName, 0, 100);
    memcpy(newDirectoryName, commandTokens[2].c_str(), commandTokens[2].length());

    uint32_t directoryAttribute;
    if(commandTokens[3] == "FILE_TYPE_FOLDER")
        directoryAttribute = FILE_TYPE_FOLDER;
    else if(commandTokens[3] == "FILE_TYPE_REGULAR_FILE")
        directoryAttribute = FILE_TYPE_REGULAR_FILE;
    else
    {
        std::cout << "'" << commandTokens[3] << "' is not a valid directory attribute!\n";
        return;
    }

    uint32_t createDirectoryResult = createDirectory(diskInfo, superBlock, parentPath, newDirectoryName, directoryAttribute);

    switch (createDirectoryResult) {
        case DIRECTORY_CREATION_PARENT_DO_NOT_EXIST:
            std::cout << "The provided parent path do not exist!\n";
            break;
        case DIRECTORY_CREATION_PARENT_NOT_A_FOLDER:
            std::cout << "The provided parent is not a folder!\n";
            break;
        case DIRECTORY_CREATION_NEW_NAME_ALREADY_EXISTS:
            std::cout << "Parent already contains a directory with given name!\n";
            break;
        case DIRECTORY_CREATION_NO_FREE_INODES:
            std::cout << "No free inodes!\n";
            break;
        case DIRECTORY_CREATION_NO_FREE_DATA_BLOCKS:
            std::cout << "No free data blocks!\n";
            break;
        case DIRECTORY_CREATION_FAILED_FOR_OTHER_REASON:
            std::cout << "Failed to create directory!\n";
            break;
        case DIRECTORY_CREATION_SUCCESS:
            std::cout << "Directory created successfully!\n";
            break;
    }
}

void commandListSubdirectories(DiskInfo* diskInfo, ext2_super_block* superBlock, std::vector<std::string> commandTokens)
{
    if(commandTokens.size() >= 2)
    {
        if(commandTokens[1] != "-l")
            commandListSubdirectoriesWithoutSize(diskInfo, superBlock, commandTokens);
//        else TODO WHEN SIZE IS READY
//            commandListSubdirectoriesWithoutSize(diskInfo, bootSector, commandTokens);
    }
    else
        std::cout << "Insufficient arguments for any type of 'ls' command!\n";
}

/////////////////////////////////////////////////

static void commandListSubdirectoriesWithoutSize(DiskInfo* diskInfo, ext2_super_block* superBlock, std::vector<std::string> commandTokens)
{
    if(commandTokens.size() < 2)
    {
        std::cout << "Insufficient arguments for 'ls' command!\n";
        return;
    }
    else if(commandTokens.size() > 2)
    {
        std::cout << "Too many arguments for 'ls' command!\n";
        return;
    }

    char* parentPath = new char[100];
    memset(parentPath, 0, 100);
    memcpy(parentPath, commandTokens[1].c_str(), commandTokens[1].length());
    char* originalParentPath = new char[100];
    memset(originalParentPath, 0, 100);
    memcpy(originalParentPath, parentPath, commandTokens[1].length());

    std::vector<std::pair<ext2_inode*, ext2_dir_entry*>> subDirectories;
    uint32_t getSubdirectoriesResult = getSubDirectories(diskInfo, superBlock, parentPath, subDirectories);

    switch (getSubdirectoriesResult) {
        case GET_SUBDIRECTORIES_GIVEN_DIRECTORY_DO_NOT_EXIST:
            std::cout<< "Given directory do not exist!\n";
            break;
        case GET_SUBDIRECTORIES_GIVEN_DIRECTORY_NOT_A_FOLDER:
            std::cout<< "Files can do not have subdirectories!\n";
            break;
        case GET_SUBDIRECTORIES_FAILED_FOR_OTHER_REASON:
            std::cout << "Failed to retrieve subdirectories for " << originalParentPath << "\n";
            break;
        case GET_SUBDIRECTORIES_SUCCESS:
            std::cout << "Subdirectories for " << originalParentPath<< ":\n";

            for(std::pair<ext2_inode*, ext2_dir_entry*> child: subDirectories)
            {
                char* fileName = new char[child.second->name_len + 1];
                memcpy(fileName, child.second->name, child.second->name_len);
                fileName[child.second->name_len] = 0;
                std::cout << fileName << "\n";
            }
    }
}