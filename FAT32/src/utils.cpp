#include "windows.h"
#include "string"
#include "string.h"
#include "algorithm"
#include "iostream"

#include "../include/structures.h"
#include "../include/fat32.h"

bool checkDirectoryNameValidity(const char* directoryName) //CAUTION directoryName MUST end with null
{
    if(strlen(directoryName) > 11)
        return false;

    char invalidChars[] = {0x22, 0x2A, 0x2B, 0x2C, 0x2F, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x5B, 0x5C, 0x5D};

    int index;
    for(index = 0; index < strlen(directoryName); index++) //we can use strlen since NULL is an invalid char in the name anyway
        if((directoryName[index] < 0x20 && !(index == 0 && directoryName[index] == 0x05)) || std::find(invalidChars, invalidChars + 14, directoryName[index]) != invalidChars + 14)
            return false;

    if(directoryName[0] == '.')
        return false;

    int nameSize = 0;
    index = 0;
    while(directoryName[index] < strlen(directoryName) && directoryName[index] != '.')
    {
        nameSize++;
        index++;
    }

    if(nameSize > 8)
        return false;

    int extensionSize = 0;
    bool containsExtension = false; //if it has dot it contains extension, otherwise not
    index = strlen(directoryName) - 1;
    while(directoryName[index] >= 0)
    {
        extensionSize++;
        index--;
        if(directoryName[index] == '.')
        {
            containsExtension = true;
            break;
        }
    }

    if(containsExtension && extensionSize > 3)
        return false;

    return true;
}

bool compareDirectoryNames(char* expected, const char* actual) //CAUTION pass expected with null in the end!!!!
{
    if(strcmp(expected, actual) == 0) //this is used for the case where expected is taken from a directoryEntry->FileName so already has required format
        return true;

    int index;

    for(index = 0; index < strlen(expected); index++)
        expected[index] = toupper(expected[index]);

    bool containsDot = false;
    int firstIndexAfterNamePart = -1;  //let's consider only the cases where there is no more than one dot !! DO NOT ADD MORE THAN 1 DOT
    for(index = 0; index < strlen(expected); index++)
        if(expected[index] == '.')
        {
            firstIndexAfterNamePart = index;
            containsDot = true;
        }


    if(firstIndexAfterNamePart == -1)
        firstIndexAfterNamePart = strlen(expected);

    if(firstIndexAfterNamePart > 8)
        return false;  //it means that the expected name (non extension part) is bigger than 8

    for(index = 0; index < firstIndexAfterNamePart; index++)
        if(expected[index] != actual[index])
            return false;

    for(index = firstIndexAfterNamePart; index < 8; index++)  //in case names match on first part, but then after name (without extension) ends in expected, it is continued in actual
        if(actual[index] != 0x20)
            return false;

    if(!containsDot) //it means that we have not dot, so we stop after checking the beginning
        firstIndexAfterNamePart--; //because we add 1 below

    int indexInExtensionPart = 0;
    for(index = firstIndexAfterNamePart + 1; index < strlen(expected); index++, indexInExtensionPart++)
        if(expected[index] != actual[8 + indexInExtensionPart])
            return false;

    if(indexInExtensionPart > 3) //it means that extension in expected was > 3
        return false;

    for(index = indexInExtensionPart; index < 3; index++)  //in case extension match on first part, but then it continues in actual (after ending in expected)
        if(actual[8 + index] != 0x20)
            return false;

    return true;
}

void formatDirectoryName(const char* directoryName, char* formattedName)
{
    memset(formattedName, 0x20, 11);

    int index;
    for(index = 0; directoryName[index] != '\0' && directoryName[index] != '.'; index++)
        formattedName[index] = toupper(directoryName[index]);

    if(directoryName[index] == '.')
    {
        index++;
        int extensionIndex = 0;
        while(directoryName[index] != '\0')
        {
            formattedName[8 + extensionIndex] = toupper(directoryName[index]);
            extensionIndex++;
            index++;
        }
    }
}

void extractParentPathFromPath(const char* fullPath, char* parentPath)
{
    std::string fullPathAsString = fullPath;
    size_t pos = fullPathAsString.find_last_of('/');
    strcpy(parentPath, fullPathAsString.substr(0, pos).c_str());
    parentPath[pos] = '\0';
}


////////////////////Utils that are not directly related with FAT32

std::vector<std::string> splitString(const std::string& str, char delimiter)
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

uint16_t getCurrentDateFormatted()
{
    SYSTEMTIME time;
    GetSystemTime(&time);
    return ((time.wYear - 1900) << 9) | (time.wMonth << 5) | time.wDay; //high 7 bits represent how many years since 1900, next 4 for month, last 5 for day
}

uint16_t getCurrentTimeFormatted()
{
    SYSTEMTIME time;
    GetSystemTime(&time);
    return (time.wHour * 3600 + time.wMinute * 60 + time.wSecond) / 2; //we have granularity of 2 secs, otherwise 16 bits is not enough. Multiply with 2 for real value in sec
}