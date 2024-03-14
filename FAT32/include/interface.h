#include "vector"
#include "string"

#ifndef FAT32_INTERFACE_H
#define FAT32_INTERFACE_H

//Model: 'mkdir Root/Level_1 NewFile ATTR_FILE' - creates a directory on parent path, with name, and directory attribute
void commandCreateDirectory(DiskInfo* diskInfo, BootSector* bootSector, std::vector<std::string> commandTokens);

//Model: 'ls Root/Level_1' - lists all direct children of given parent //TODO display not only child name, but also its size/size on disk
void commandListSubdirectories(DiskInfo* diskInfo, BootSector* bootSector, std::vector<std::string> commandTokens);

//Model: 'write Root/Level_1 10000 TRUNCATE EOF' and o the next lines is the text to be written to the file for max bytes (10000 in the example).
//Text read stops when encounters the given end of text marker (in or example EOF)
void commandWriteFile(DiskInfo* diskInfo, BootSector* bootSector, std::vector<std::string> commandTokens);

//Model: 'read Root/Level_1 100 10000' - reads starting from byte, up to max bytes from a file and prints them to the console
void commandReadFile(DiskInfo* diskInfo, BootSector* bootSector, std::vector<std::string> commandTokens);

//Model: 'truncate Root/Level_1 50' - sets file size to 50
void commandTruncateFile(DiskInfo* diskInfo, BootSector* bootSector, std::vector<std::string> commandTokens);

//Model 'rmdir Root/File_1' - removes a given directory, freeing all its space; if the given directory is a folder, it also deletes its direct and indirect subdirectories
void commandDeleteDirectory(DiskInfo* diskInfo, BootSector* bootSector, std::vector<std::string> commandTokens);
#endif
