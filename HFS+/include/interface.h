#include "vector"
#include "string"
#include "../include/structures.h"

#include "diskUtils.h"
#include "../include/structures.h"

#ifndef HFS__INTERFACE_H
#define HFS__INTERFACE_H

//Model: 'mkdir Root/Level_1 NewFile DIRECTORY_TYPE_FILE' - creates a directory on parent path, with name, and directory attribute
void commandCreateDirectory(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, std::vector<std::string> commandTokens);

//Model 1: 'ls Root/Level_1' - lists all direct children of given parent
//Model 2: 'ls -l Root/Level_1' - lists all direct children of given parent with size
void commandListSubdirectories(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, std::vector<std::string> commandTokens);

//////////////////////////////////

static void commandListSubdirectoriesWithoutSize(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, std::vector<std::string> commandTokens);

#endif
