#include "string"
#include "cstdint"

#ifndef UTILS_H
#define UTILS_H

uint32_t ext2_getBitFromByte(uint8_t byte, uint32_t bitIndexInByte);

//CAUTION new bit value is given as a byte, but it should be either 0 or 1
uint32_t ext2_changeBitValue(uint32_t byte, uint32_t bitIndexInByte, uint8_t newBitValue);

//The maximum number of bytes a file can have considering the 4 level schema CAUTION the size might be a long (64 bits)
uint64_t getMaximumFileSize(ext2_super_block* superBlock);

////////////////////////////////////////

std::vector<std::string> ext2_splitString(const std::string& str, char delimiter);

void ext2_extractParentPathFromPath(const char* fullPath, char* parentPath);

uint32_t ext2_getCurrentTimeDateAndTimeFormatted();

#endif
