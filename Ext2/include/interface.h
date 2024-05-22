#include "vector"
#include "string"
#include "../include/ext2Structures.h"

#include "diskUtils.h"
#include "../include/ext2Structures.h"

#ifndef EXT2_INTERFACE_H
#define EXT2_INTERFACE_H

//Model: 'mkdir Root/Level_1 NewFile DIRECTORY_TYPE_FILE' - creates a directory on parent path, with name, and directory attribute
void commandCreateDirectory(DiskInfo* diskInfo, ext2_super_block* superBlock, std::vector<std::string> commandTokens);

//Model 1: 'ls Root/Level_1' - lists all direct children of given parent
//Model 2: 'ls -l Root/Level_1' - lists all direct children of given parent with size
void commandListSubdirectories(DiskInfo* diskInfo, ext2_super_block* superBlock, std::vector<std::string> commandTokens);

//Model: 'write Root/Level_1 10000 TRUNCATE EOF' and o the next lines is the text to be written to the file for max bytes (10000 in the example).
//Text read stops when encounters the given end of text marker (in or example EOF)
void commandWriteFile(DiskInfo* diskInfo, ext2_super_block* superBlock, std::vector<std::string> commandTokens);

//Model: 'read Root/Level_1 100 10000' - reads starting from byte, up to max bytes from a file and prints them to the console
void commandReadFile(DiskInfo* diskInfo, ext2_super_block* superBlock, std::vector<std::string> commandTokens);

//Model: 'truncate Root/Level_1 50' - sets file size to 50
void commandTruncateFile(DiskInfo* diskInfo, ext2_super_block* superBlock, std::vector<std::string> commandTokens);

//Model 'rmdir Root/File_1' - removes a given directory, freeing all its space; if the given directory is a folder, it also deletes its direct and indirect subdirectories
void commandDeleteDirectory(DiskInfo* diskInfo, ext2_super_block* superBlock, std::vector<std::string> commandTokens);

//Model 'la Root/File_1' - shows directory attributes
void commandShowDirectoryAttributes(DiskInfo* diskInfo, ext2_super_block* superBlock, std::vector<std::string> commandTokens);

//Model 'preallocate Root/File_1 8000' - allocate enough blocks to accommodate 8000 new bytes (this bytes are added to actual allocated blocks)
void commandPreallocateBlocks(DiskInfo* diskInfo, ext2_super_block* superBlock, std::vector<std::string> commandTokens);

//////////////////////////////////

static void commandListSubdirectoriesWithSize(DiskInfo* diskInfo, ext2_super_block* superBlock, std::vector<std::string> commandTokens);

static void commandListSubdirectoriesWithoutSize(DiskInfo* diskInfo, ext2_super_block* superBlock, std::vector<std::string> commandTokens);


void commandWriteFileTEST(DiskInfo* diskInfo, ext2_super_block* superBlock, std::vector<std::string> commandTokens);

#endif
