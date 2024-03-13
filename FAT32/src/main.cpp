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
#include "../include/fat32Attributes.h"

bool creatingDiskAndFileSystem = false;
bool debugMode = true;

int initialLoad(DiskInfo** diskInfo, BootSector** bootSector, FsInfo** fsInfo)
{
    char* diskDirectory = "D:\\Facultate\\Licenta\\HardDisks\\HardDisk_512Kib";
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

    if(!checkBootSectorsInitialized(*diskInfo))
    {
        std::cout << "Initializing boot sectors...\n";
        initializeBootSectors(*diskInfo);
    }
    else
    {
        std::cout << "Boot sectors already initialized\n";
    }

    *bootSector = readBootSector(*diskInfo);
    *fsInfo = readFsInfo(*diskInfo, *bootSector);

    if(creatingDiskAndFileSystem == true)
    {
        initializeFat(*diskInfo, *bootSector);   //TODO initialize only when file system created
    }
}

int main() {
    DiskInfo* diskInfo = nullptr;
    BootSector* bootSector = nullptr;
    FsInfo* fsInfo = nullptr;

    initialLoad(&diskInfo, &bootSector, &fsInfo);

    if(debugMode == true)
    {
        uint32_t numOfSectorsRead = 0;
        char* rootSectorData = new char[bootSector->BytesPerSector];
        readDiskSectors(diskInfo, 1, 104, rootSectorData, numOfSectorsRead);
        char* sectorData1 = new char[bootSector->BytesPerSector];
        readDiskSectors(diskInfo, 1, 32, sectorData1, numOfSectorsRead);
        char* sectorData2 = new char[bootSector->BytesPerSector];
        readDiskSectors(diskInfo, 1, 112, sectorData2, numOfSectorsRead);
        char* sectorData3 = new char[bootSector->BytesPerSector];
        readDiskSectors(diskInfo, 1, 108, sectorData3, numOfSectorsRead);

        DirectoryEntry* dir = (DirectoryEntry*)&rootSectorData[0];
        return 0;
    }

    if(creatingDiskAndFileSystem == false)
    {
//////////////Directories creation
        char* parentPath = new char[100];
        char* newDir = new char[10];
        memcpy(parentPath, "Root\0", 5);
        char* originalParentPath = new char[100];
        memcpy(originalParentPath, parentPath, 100);
        std::cout << '\n';
        memset(newDir, '\0', 10);

        for(int x = 0; x <= 0; x++)
        {
            memcpy(parentPath, originalParentPath, 100);
            memcpy(newDir, "Dir_", 4);
            std::string xAsString = (x < 100 ? "0" : "") + ((x < 10 ? "0" : "") + std::to_string(x));
            memcpy(newDir + 4, &xAsString[0], 3);
            int createDirectoryResult = createDirectory(diskInfo, bootSector, parentPath, newDir, ATTR_DIRECTORY);
            std::cout << "Directory creation for " << newDir << " : " << createDirectoryResult << "\n";
        }
//
///////////////Subdirectories list
//        std::vector<DirectoryEntry*> subDirectories;
//        int getSubdirectoriesResult = getSubDirectories(diskInfo, bootSector, originalParentPath, subDirectories);
//        std::string prompt1 = "\nGet subdirectories result: ";
//        std::cout << prompt1 << getSubdirectoriesResult << "\n";
//        for(auto dir : subDirectories)
//        {
//            char* fileName = new char[12];
//            memcpy(fileName, dir->FileName, 11);
//            fileName[11] = 0;
//            std::cout << fileName << "\n";
//        }

/////////File write
//        std::ifstream in("../fin.txt");
//        in.seekg(0, std::ios::end);
//        std::streampos fileSize = in.tellg();
//        in.seekg(0, std::ios::beg);
//        char *text = new char[fileSize];
//        in.read(text, fileSize);
////        std::cout << text << std::endl;
//
//        char* fileParentPath = new char[100];
//        memcpy(fileParentPath, "Root/Level_1/\0", 21);
//        char* newFile= new char[100];
//        memcpy(newFile, "Level_2_2\0", 10);
//        int createFileResult = createDirectory(diskInfo, bootSector, fileParentPath, newFile, ATTR_DIRECTORY);
//        std::cout << "File creation for " << newFile << " : " << createFileResult << "\n";
//
//        char* filePath = new char[100];
//        memcpy(filePath, "Root/File_1\0", 12);
//        uint32_t numberOfBytesWritten = 0;
//        uint32_t reasonForIncompleteWrite;
//        memset(text + 6000, 0, 1000);
//        uint32_t writeFileResult = write(diskInfo, bootSector, filePath, text, 6000, numberOfBytesWritten, WRITE_WITH_TRUNCATE,
//                                         reasonForIncompleteWrite);
//        std::string prompt2 = "\nWrite file result: ";
//        std::cout << prompt2 << writeFileResult << " : number of bytes written: " << numberOfBytesWritten << "\n";
//        if(numberOfBytesWritten < 6000)
//        {
//            std::string prompt3 = "\nReason for incomplete bytes write: ";
//            std::cout << prompt3 << reasonForIncompleteWrite << "\n";
//        }


///////////////Read file
//        filePath = new char[100];
//        memcpy(filePath, "Root/File_1\0", 12);
//        uint32_t numberOfBytesRead = 0;
//        uint32_t reasonForIncompleteRead;
//        char* readBuffer = new char[7001];
//        uint32_t readFileResult = read(diskInfo, bootSector, filePath, readBuffer, 7000, numberOfBytesRead, reasonForIncompleteRead);
//        std::string prompt3 = "\nRead file result: ";
//        std::cout << prompt3 << readFileResult << " : number of bytes read: " << numberOfBytesRead << "\n";
//        if(numberOfBytesRead < 7000)
//        {
//            std::string prompt4 = "\nReason for incomplete bytes read: ";
//            std::cout << prompt4 << reasonForIncompleteRead << "\n";
//        }
//
//        readBuffer[7000] = '\0';
//        std::cout << "\n\n" << readBuffer << "\n\n";

///////////Truncate file
//        filePath = new char[100];
//        memcpy(filePath, "Root/File_1\0", 12);
//        uint32_t truncateResult = truncateFile(diskInfo, bootSector, filePath);
//        std::string prompt5 = "\nTruncate result: ";
//        std::cout << prompt5 << truncateResult << "\n";

///////////Delete directory
        char* filePathToDelete = new char[100];
        memcpy(filePathToDelete, "Root/Level_1\0", 20);
        uint32_t deleteDirectoryResult = deleteDirectoryByPath(diskInfo, bootSector, filePathToDelete);
        std::string prompt6 = "\nDelete directory result: ";
        std::cout << prompt6 << deleteDirectoryResult << "\n";
    }

    return 0;
}
