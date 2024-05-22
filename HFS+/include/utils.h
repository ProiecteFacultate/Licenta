#include "string.h"
#include "string"
#include "vector"
#include "cstdint"
#include "windows.h"

#ifndef HFS__UTILS_H
#define HFS__UTILS_H

uint32_t getExtentsOverflowFileNodeSize(HFSPlusVolumeHeader* volumeHeader);
uint32_t getCatalogFileNodeSize(HFSPlusVolumeHeader* volumeHeader);

uint32_t hfs_getBitFromByte(uint8_t byte, uint32_t bitIndexInByte);

//CAUTION new bit value is given as a byte, but it should be either 0 or 1
uint32_t hfs_changeBitValue(uint32_t byte, uint32_t bitIndexInByte, uint8_t newBitValue);

/////////////////////////

std::vector<std::string> hfs_splitString(const std::string& str, char delimiter);

uint32_t hfs_getCurrentTimeDateAndTimeFormatted();

#endif
