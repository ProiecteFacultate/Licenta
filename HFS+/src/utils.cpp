#include "string.h"
#include "string"
#include "vector"
#include "cstdint"
#include "windows.h"

#include "../include/disk.h"
#include "../include/diskCodes.h"
#include "../include/hfsStructures.h"
#include "../include/utils.h"

uint32_t getExtentsOverflowFileNodeSize(HFSPlusVolumeHeader* volumeHeader) { return volumeHeader->blockSize; } //1024 standard

uint32_t getCatalogFileNodeSize(HFSPlusVolumeHeader* volumeHeader)
{
    if(volumeHeader->blockSize >= 4096)
        return volumeHeader->blockSize;

    return 4096;
} //4096 standard

uint32_t hfs_getBitFromByte(uint8_t byte, uint32_t bitIndexInByte)
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

uint32_t hfs_changeBitValue(uint32_t byte, uint32_t bitIndexInByte, uint8_t newBitValue)
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

////////////////////Utils that are not directly related with HFS

std::vector<std::string> hfs_splitString(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    size_t start = 0;
    size_t end = str.find(delimiter);

    while (end != std::string::npos) {
        token = str.substr(start, end - start);
        tokens.push_back(token);
        start = end + 1;
        end = str.find(delimiter, start);
    }

    // Push the last token after the last delimiter
    token = str.substr(start);
    tokens.push_back(token);

    return tokens;
}

uint32_t hfs_getCurrentTimeDateAndTimeFormatted()
{
    SYSTEMTIME time;
    GetSystemTime(&time);
    return ((time.wYear - 1900) << 25) | (time.wMonth << 21) | (time.wDay << 16) | ((time.wHour * 3600 + time.wMinute * 60 + time.wSecond) / 2);
}