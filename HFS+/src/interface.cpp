#include "vector"
#include "string"
#include "string.h"
#include "iostream"
#include "math.h"

#include "../include/hfsApi.h"
#include "../include/structures.h"
#include "../include/codes/hfsAttributes.h"
#include "../include/codes/hfsApiResponseCodes.h"
#include "../include/codes/hfsCodes.h"
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

void commandWriteFile(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, ExtentsFileHeaderNode* extentsFileHeaderNode,
                      std::vector<std::string> commandTokens)
{
    if(commandTokens.size() < 5)
    {
        std::cout << "Insufficient arguments for 'write' command!\n";
        return;
    }
    else if(commandTokens.size() > 5)
    {
        std::cout << "Too many arguments for 'write' command!\n";
        return;
    }

    char* filePath = new char[100];
    memset(filePath, 0, 100);
    memcpy(filePath, commandTokens[1].c_str(), commandTokens[1].length());

    if(commandTokens[2][0] == '-')
    {
        std::cout << "Maximum number of bytes to write can't be a negative number!\n";
        return;
    }
    uint32_t maxBytesToWrite = atoi(commandTokens[2].c_str());

    uint32_t writeAttribute;
    if(commandTokens[3] == "TRUNCATE")
        writeAttribute = WRITE_WITH_TRUNCATE;
    else if(commandTokens[3] == "APPEND")
        writeAttribute = WRITE_WITH_APPEND;
    else
    {
        std::cout << "'" << commandTokens[3] << "' is not a valid write argument!\n";
        return;
    }

    uint32_t bufferSize = ((maxBytesToWrite / volumeHeader->blockSize) + 1) * volumeHeader->blockSize; //in order to avoid overflows
    char* text = new char[bufferSize];
    memset(text, 0, maxBytesToWrite);
    uint32_t numberOfBytesAlreadyRead = 0;
    std::string lineData;
    while(std::getline(std::cin, lineData))
    {
        if(lineData == commandTokens[4]) //end of text marker
            break;

        uint32_t numOfBytesFromThisLineToAddToText = std::min((uint32_t) lineData.length(), maxBytesToWrite - numberOfBytesAlreadyRead);
        memcpy(text + numberOfBytesAlreadyRead, lineData.c_str(), numOfBytesFromThisLineToAddToText);
        numberOfBytesAlreadyRead += numOfBytesFromThisLineToAddToText;
    }

    uint32_t numberOfBytesWritten;
    uint32_t reasonForIncompleteWrite;
    uint32_t writeFileResult = write(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, filePath, text, maxBytesToWrite, numberOfBytesWritten,
                                     writeAttribute, reasonForIncompleteWrite);

    switch (writeFileResult)
    {
        case WRITE_BYTES_TO_FILE_CAN_NOT_WRITE_GIVEN_FILE:
            std::cout << "Can not write to given file type!\n"; //it may be root, or a FILE_TYPE_FOLDER instead of a FILE_TYPE_REGULAR_FILE as needed
            break;
        case WRITE_BYTES_TO_FILE_GIVEN_FILE_DO_NOT_EXIST_OR_SEARCH_FAIL:
            std::cout << "Given file do not exist or search fail!\n";
            break;
        case WRITE_BYTES_TO_FILE_FAILED_FOR_OTHER_REASON:
            std::cout << "Could not write any bytes to file!\n";
            break;
        case WRITE_BYTES_TO_FILE_SUCCESS:
            std::cout << "Wrote " << numberOfBytesWritten << " to file!\n";
            if(numberOfBytesWritten < maxBytesToWrite)
            {
                if(reasonForIncompleteWrite == INCOMPLETE_BYTES_WRITE_DUE_TO_NO_FREE_BLOCKS)
                    std::cout << "The write was incomplete due to insufficient space on disk\n";
                else
                    std::cout << "The write was incomplete due to unspecified reasons\n";
            }
    }
}

void commandReadFile(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, ExtentsFileHeaderNode* extentsFileHeaderNode,
                     std::vector<std::string> commandTokens)
{
    if(commandTokens.size() < 4)
    {
        std::cout << "Insufficient arguments for 'read' command!\n";
        return;
    }
    else if(commandTokens.size() > 4)
    {
        std::cout << "Too many arguments for 'read' command!\n";
        return;
    }

    char* filePath = new char[100];
    memset(filePath, 0, 100);
    memcpy(filePath, commandTokens[1].c_str(), commandTokens[1].length());
    char* originalFilePath = new char[100];
    memset(originalFilePath, 0, 100);
    memcpy(originalFilePath, filePath, commandTokens[1].length());

    if(commandTokens[2][0] == '-')
    {
        std::cout << "Starting read position can't be a negative number!\n";
        return;
    }

    if(commandTokens[3][0] == '-')
    {
        std::cout << "Maximum number of bytes to read can't be a negative number!\n";
        return;
    }

    uint32_t startingPosition = atoi(commandTokens[2].c_str());
    uint32_t maxBytesToRead = atoi(commandTokens[3].c_str());

    uint32_t bufferSize = ((maxBytesToRead / volumeHeader->blockSize) + 2) * volumeHeader->blockSize; //in order to avoid overflows
    char* readBuffer = new char[bufferSize];
    uint32_t numberOfBytesRead = 0;
    uint32_t reasonForIncompleteRead;
    uint32_t readFileResult = read(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, filePath, readBuffer, startingPosition, maxBytesToRead,
                                   numberOfBytesRead, reasonForIncompleteRead);

    switch (readFileResult)
    {
        case READ_BYTES_FROM_FILE_CAN_NOT_READ_GIVEN_FILE:
            std::cout << "Can not read to given file!\n"; //it may be root, or an FILE_TYPE_FOLDER instead of a FILE_TYPE_REGULAR_FILE as needed
            break;
        case READ_BYTES_FROM_FILE_GIVEN_FILE_DO_NOT_EXIST_OR_SEARCH_FAIL:
            std::cout << "File " << originalFilePath << " do not exist!\n";
            break;
        case READ_BYTES_FROM_FILE_GIVEN_START_EXCEEDS_FILE_SIZE:
            std::cout << "Read starting position exceeds file size!\n";
            break;
        case READ_BYTES_FROM_FILE_FAILED_FOR_OTHER_REASON:
            std::cout << "Could not read any bytes from file!\n";
            break;
        case READ_BYTES_FROM_FILE_SUCCESS:
            std::cout << "Read " << numberOfBytesRead << " from file!\n";
            if(numberOfBytesRead < maxBytesToRead)
            {
                if(reasonForIncompleteRead == INCOMPLETE_BYTES_READ_DUE_TO_NO_FILE_NOT_LONG_ENOUGH)
                    std::cout << "The read was incomplete because the file doesn't contain enough bytes\n";
                else
                    std::cout << "The read was incomplete due to unspecified reasons\n";
            }

            std::cout << originalFilePath << " content:\n\n";
            if(numberOfBytesRead != 0)
            {
                std::cout.write(readBuffer, numberOfBytesRead);
                std::cout << "\n\n";
            }
    }
}

void commandTruncateFile(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, ExtentsFileHeaderNode* extentsFileHeaderNode,
                         std::vector<std::string> commandTokens)
{
    if(commandTokens.size() < 3)
    {
        std::cout << "Insufficient arguments for 'truncate' command!\n";
        return;
    }
    else if(commandTokens.size() > 3)
    {
        std::cout << "Too many arguments for 'truncate' command!\n";
        return;
    }

    char* filePath = new char[100];
    memset(filePath, 0, 100);
    memcpy(filePath, commandTokens[1].c_str(), commandTokens[1].length());

    if(commandTokens[2][0] == '-')
    {
        std::cout << "Can not truncate file to a negative size!\n";
        return;
    }

    uint32_t newSize = atoi(commandTokens[2].c_str());

    uint32_t truncateResult = truncate(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, filePath, newSize);

    switch (truncateResult) {
        case TRUNCATE_FILE_GIVEN_FILE_DO_NOT_EXIST_OR_SEARCH_FAIL:
            std::cout << "Given file do not exist or search fail!\n"; //it may be root, or an ATTR_FOLDER instead of an ATTR_FILE as needed
            break;
        case TRUNCATE_FILE_CAN_NOT_TRUNCATE_GIVEN_FILE_TYPE:
            std::cout << "Can not truncate to given file type!\n"; //it may be root, or an ATTR_FOLDER instead of an ATTR_FILE as needed
            break;
        case TRUNCATE_FILE_NEW_SIZE_GREATER_THAN_ACTUAL_SIZE:
            std::cout << "New truncate size can't be greater than actual file size!\n";
            break;
        case TRUNCATE_FILE_FAILED_FOR_OTHER_REASON:
            std::cout << "Failed to truncate file for unspecified reason!\n";
        case TRUNCATE_FILE_SUCCESS:
            std::cout << "Successfully truncated file!\n";
            break;
    }
}

void commandDeleteDirectory(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, ExtentsFileHeaderNode* extentsFileHeaderNode,
                            std::vector<std::string> commandTokens)
{
    if(commandTokens.size() < 2)
    {
        std::cout << "Insufficient arguments for 'rmdir' command!\n";
        return;
    }
    else if(commandTokens.size() > 2)
    {
        std::cout << "Too many arguments for 'rmdir' command!\n";
        return;
    }

    char* directoryPath = new char[100];
    memset(directoryPath, 0, 100);
    memcpy(directoryPath, commandTokens[1].c_str(), commandTokens[1].length());

    std::string warning;
    uint32_t deleteDirectoryResult = deleteDirectoryByPath(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, directoryPath);

    switch (deleteDirectoryResult)
    {
        case DELETE_DIRECTORY_CAN_NOT_DELETE_ROOT:
            std::cout << "Can not delete root!\n";
            break;
        case DELETE_DIRECTORY_DIRECTORY_DO_NOT_EXIST_OR_SEARCH_FAIL:
            std::cout << "Given directory do not exit or search fail!\n";
            break;
//        case DELETE_DIRECTORY_FAILED_TO_DELETE_DIRECTORY_ENTRY_FROM_PARENT:
//            std::cout << "Failed to delete directory because error on delete directory entry from parent!\n";
//            break;
//        case DELETE_DIRECTORY_FAILED_TO_FREE_BLOCKS:
//            std::cout << "\"Failed to delete directory because error on free blocks!\n";
//            break;
        case DELETE_DIRECTORY_FAILED_FOR_OTHER_REASON:
            std::cout << "Failed to delete this directory for unknown reason!\n";
            break;
        case DELETE_DIRECTORY_SUCCESS:
            std::cout << "Successfully deleted this directory!\n";
    }
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