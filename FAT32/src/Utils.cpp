#include "windows.h"
#include "string"
#include "string.h"
#include "algorithm"
#include "iostream"

#include "../include/fat32.h"

bool checkDirectoryNameValidity(const char* directoryName)
{
    char invalidChars[] = {0x22, 0x2A, 0x2B, 0x2C, 0x2F, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x5B, 0x5C, 0x5D};

    int index;
    for(index = 0; index < strlen(directoryName); index++) //we can use strlen since NULL is an invalid char in the name anyway
        if((directoryName[index] < 0x20 && !(index == 0 && directoryName[index] == 0x05)) || std::find(invalidChars, invalidChars + 14, directoryName[index]))
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
    index = strlen(directoryName) - 1;
    while(directoryName[index] >= 0 && directoryName[index] != '.')
    {
        extensionSize++;
        index--;
    }

    if(extensionSize > 3)
        return false;

    return true;
}

bool compareDirectoryNames(char* expected, const char* actual) //WARN pass expected with null in the end!!!!
{
    int index;

    for(index = 0; index < strlen(expected); index++)
        expected[index] = toupper(expected[index]);

    int dotIndexInExpected = 12;  //let's consider only the cases where there is no more than one dot !! DO NOT ADD MORE THAN 1 DOT
    for(index = 0; index < strlen(expected); index++)
        if(expected[index] == '.')
            dotIndexInExpected = index;

    if(dotIndexInExpected > 8)
        return false;  //it means that the expected name (non extension part) is bigger than 8

    for(index = 0; index < dotIndexInExpected; index++)
        if(expected[index] != actual[index])
            return false;

    int indexInExtensionPart = 0;
    for(index = dotIndexInExpected + 1; index < strlen(expected); index++, indexInExtensionPart++)
        if(expected[index] != actual[8 + indexInExtensionPart])
            return false;

    if(indexInExtensionPart > 3) //it means that extension in expected was > 3
        return false;

    for(index = indexInExtensionPart; index < 3; index++) //same as on the comment below but for extension
        if(actual[8 + index] != 0x20)
            return false;

    for(index = dotIndexInExpected; index < 8; index++)  //in case names match on first part, but then after name (without extension) ends in expected, it is continued in actual
        if(actual[index] != 0x20)
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