#include "vector"
#include "string"
#include "../include/structures.h"

#include "diskUtils.h"
#include "../include/structures.h"

#ifndef EXT2_INTERFACE_H
#define EXT2_INTERFACE_H

//Model: 'mkdir Root/Level_1 NewFile FILE_TYPE_REGULAR_FILE' - creates a directory on parent path, with name, and directory attribute
void commandCreateDirectory(DiskInfo* diskInfo, ext2_super_block* superBlock, std::vector<std::string> commandTokens);
//Model 1: 'ls Root/Level_1' - lists all direct children of given parent
//Model 2: 'ls -l Root/Level_1' - lists all direct children of given parent with size
void commandListSubdirectories(DiskInfo* diskInfo, ext2_super_block* superBlock, std::vector<std::string> commandTokens);

//////////////////////////////////

static void commandListSubdirectoriesWithoutSize(DiskInfo* diskInfo, ext2_super_block* superBlock, std::vector<std::string> commandTokens);

#endif
