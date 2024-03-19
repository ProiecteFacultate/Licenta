#include <iostream>

#include "../include/disk.h"
#include "../include/diskUtils.h"
#include "../include/diskCodes.h"

int main() {

    char* diskDirectory = "D:\\Facultate\\Licenta\\HardDisks\\HardDisk_512Kib";
    DiskInfo* diskInfo = nullptr;
    diskInfo = initializeDisk(diskDirectory, 1024,2 );

    return 0;
}
