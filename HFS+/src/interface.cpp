#include "vector"
#include "string"
#include "string.h"
#include "iostream"
#include "math.h"

#include "../include/hfsApi.h"
#include "../include/structures.h"
#include "../include/codes/hfsAttributes.h"
#include "../include/codes/hfsApiResponseCodes.h"
#include "../include/interface.h"

void commandCreateDirectory(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, std::vector<std::string> commandTokens)
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

    int16_t directoryAttribute;
    if(commandTokens[3] == "DIRECTORY_TYPE_FOLDER")
        directoryAttribute = DIRECTORY_TYPE_FOLDER;
    else if(commandTokens[3] == "DIRECTORY_TYPE_FILE")
        directoryAttribute = DIRECTORY_TYPE_FILE;
    else
    {
        std::cout << "'" << commandTokens[3] << "' is not a valid directory attribute!\n";
        return;
    }

    uint32_t createDirectoryResult = createDirectory(diskInfo, volumeHeader, catalogFileHeaderNode, parentPath, newDirectoryName, directoryAttribute);

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
        case DIRECTORY_CREATION_PARENT_FAILED_TO_INSERT_RECORD_IN_CATALOG_TREE:
            std::cout << "Failed to insert record in catalog tree!\n";
            break;
        case DIRECTORY_CREATION_FAILED_FOR_OTHER_REASON:
            std::cout << "Failed to create directory for unknown reason!\n";
            break;
        case DIRECTORY_CREATION_SUCCESS:
            std::cout << "Directory created successfully!\n";
            break;
    }
}

void commandListSubdirectories(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, std::vector<std::string> commandTokens)
{
    if(commandTokens.size() >= 2)
    {
//        if(commandTokens[1] == "-l")
//            commandListSubdirectoriesWithSize(diskInfo, superBlock, commandTokens);
//        else
            commandListSubdirectoriesWithoutSize(diskInfo, volumeHeader, catalogFileHeaderNode, commandTokens);
    }
    else
        std::cout << "Insufficient arguments for any type of 'ls' command!\n";
}

/////////////////////////////////////////////////

static void commandListSubdirectoriesWithoutSize(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode,
                                                 std::vector<std::string> commandTokens)
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

    std::vector<CatalogDirectoryRecord*> subDirectories;
    uint32_t getSubdirectoriesResult = getSubDirectoriesByParentPath(diskInfo, volumeHeader, catalogFileHeaderNode, parentPath, subDirectories);

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

            for(CatalogDirectoryRecord* child: subDirectories)
            {
                uint32_t len = child->catalogKey.nodeName.length;
                char* fileName = new char[len + 1];
                memcpy(fileName, child->catalogKey.nodeName.chars, len);
                fileName[len] = 0;
                std::cout << fileName << "\n";
            }
    }
}