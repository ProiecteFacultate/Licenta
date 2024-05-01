#include "string.h"
#include "string"
#include "vector"
#include "cstdint"
#include "windows.h"

#include "../include/disk.h"
#include "../include/diskCodes.h"
#include "../include/structures.h"
#include "../include/utils.h"

uint32_t getBitFromByte(uint8_t byte, uint32_t bitIndexInByte)
{
    switch (bitIndexInByte) {
        case 0:
            return (byte & 0x80) >> 7;
        case 1:
            return (byte & 0x40) >> 6;
        case 2:
            return (byte & 0x20) >> 5;
        case 3:
            return (byte & 0x10) >> 4;
        case 4:
            return (byte & 0x08) >> 3;
        case 5:
            return (byte & 0x04) >> 2;
        case 6:
            return (byte & 0x02) >> 1;
        default: //7
            return byte & 0x01;
    }
}

uint32_t changeBitValue(uint32_t byte, uint32_t bitIndexInByte, uint8_t newBitValue)
{
    switch (bitIndexInByte) {
        case 0:
            return (byte & 0x7F) | (newBitValue << 7);
        case 1:
            return (byte & 0xBF) | (newBitValue << 6);
        case 2:
            return (byte & 0xDF) | (newBitValue << 5);
        case 3:
            return (byte & 0xEF) | (newBitValue << 4);
        case 4:
            return (byte & 0xF7) | (newBitValue << 3);
        case 5:
            return (byte & 0xFB) | (newBitValue << 2);
        case 6:
            return (byte & 0xFD) | (newBitValue << 1);
        default: //7
            return (byte & 0xFE) | newBitValue;
    }
}