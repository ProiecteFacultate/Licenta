#include <iostream>
#include <utils.h>
#include <interface.h>

#include "fat32Init.h"
#include "fat32Api.h"
#include "interface.h"
#include "fat32Attributes.h"
#include "utils.h"

int main() {
    char* diskDirectory = "D:\\Facultate\\Licenta\\HardDisks\\HardDisk_512Kib";
    DiskInfo* diskInfo = nullptr;
    BootSector* bootSector = nullptr;
    FsInfo* fsInfo = nullptr;
    fat32Startup(diskDirectory, &diskInfo, &bootSector, &fsInfo, 1024, 512);


    char* filePath = new char[100];
    memset(filePath, 0, 100);
    memcpy(filePath, "Root/File_1", 11);

////WRITE
    uint32_t numberOfBytesWritten = 0;
    uint32_t reasonForIncompleteWrite;
    char* dataBuffer = new char[400000];
    memset(dataBuffer, '1', 400000);
    write(diskInfo, bootSector, filePath, dataBuffer, 400000, numberOfBytesWritten, WRITE_WITH_TRUNCATE, reasonForIncompleteWrite);
    std::cout << "Bytes written: " << numberOfBytesWritten << '\n';

////READ
    memset(filePath, 0, 100);
    memcpy(filePath, "Root/File_1", 11);

    uint32_t numberOfBytesRead = 0;
    uint32_t reasonForIncompleteRead;
    char* readBuffer = new char[100000];
    memset(readBuffer, 0, 100000);
    read(diskInfo, bootSector, filePath, readBuffer, 0, 100000,numberOfBytesRead, reasonForIncompleteRead);
    std::cout << "Bytes read: " << numberOfBytesRead << '\n';
//    std::cout.write(readBuffer, numberOfBytesRead);

    return 0;
}
