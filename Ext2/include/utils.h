#include "string"

#ifndef UTILS_H
#define UTILS_H

uint32_t getBitFromByte(uint8_t byte, uint32_t bitIndexInByte);
//CAUTION new bit value is given as a byte, but it should be either 0 or 1
uint32_t changeBitValue(uint32_t byte, uint32_t bitIndexInByte, uint8_t newBitValue);

std::vector<std::string> splitString(const std::string& str, char delimiter);

#endif
