#include "../include/disk.h"
#include "../include/structures.h"

#ifndef EXT2_EXT2API_H
#define EXT2_EXT2API_H

//This is the public api for the file system

uint32_t createDirectory(DiskInfo* diskInfo, ext2_super_block* superBlock, char* parentDirectoryPath, char* newDirectoryName, uint32_t newDirectoryType);

//Being given a directory path, return all its subdirectories (folders and files)
uint32_t getSubDirectories(DiskInfo* diskInfo, ext2_super_block* superBlock, char* directoryPath, std::vector<std::pair<ext2_inode*, ext2_dir_entry*>>& subDirectories);

//Being given a directory path, a buffer with data, and a max number of bytes, writes up to the max number of bytes.
//CAUTION if it writes less than max bytes, it will still return success. This could occur for example when there is insufficient space on disk
uint32_t write(DiskInfo* diskInfo, ext2_super_block* superBlock, char* directoryPath, char* dataBuffer, uint32_t maxBytesToWrite, uint32_t& numberOfBytesWritten, uint32_t writeAttribute,
               uint32_t& reasonForIncompleteWrite);

//Being given a directory path, a buffer to read into, and a max number of bytes, read up to the max number of bytes
uint32_t read(DiskInfo* diskInfo, ext2_super_block* superBlock, char* directoryPath, char* readBuffer, uint32_t startingPosition, uint32_t maxBytesToRead, uint32_t& numberOfBytesRead,
              uint32_t& reasonForIncompleteRead);

#endif
