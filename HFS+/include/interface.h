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

//Model: 'write Root/Level_1 10000 TRUNCATE EOF' and o the next lines is the text to be written to the file for max bytes (10000 in the example).
//Text read stops when encounters the given end of text marker (in or example EOF)
void commandWriteFile(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, ExtentsFileHeaderNode* extentsFileHeaderNode,
                      std::vector<std::string> commandTokens);

//Model: 'read Root/Level_1 100 10000' - reads starting from byte, up to max bytes from a file and prints them to the console
void commandReadFile(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, ExtentsFileHeaderNode* extentsFileHeaderNode,
                     std::vector<std::string> commandTokens);

//Model 'rmdir Root/File_1' - removes a given directory, freeing all its space; if the given directory is a folder, it also deletes its direct and indirect subdirectories
void commandDeleteDirectory(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, ExtentsFileHeaderNode* extentsFileHeaderNode,
                            std::vector<std::string> commandTokens);

//////////////////////////////////

static void commandListSubdirectoriesWithoutSize(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, std::vector<std::string> commandTokens);

#endif
