#include "windows.h"
#include "string"
#include "string.h"
#include "iostream"

#include "../include/disk.h"
#include "../include/diskUtils.h"
#include "../include/diskCodes.h"
#include "../include/fat32Init.h"
#include "../include/fat32.h"

bool compareFileNames(char* expected, uint8_t actual[])
{
    size_t expectedLength = strlen(expected);

    for(int i = 0; i < expectedLength; i++)
        expected[i] = toupper(expected[i]);

    int index = 0;
    while(index < expectedLength && expected[index] != '.')
    {
        if(expected[index] != actual[index])
            return false;

        index++;
    }

    if(expected[index] == '.')
    {
        int firstIndexAfterDot = 1;
        while(index + firstIndexAfterDot < expectedLength)
        {
            if(expected[index + firstIndexAfterDot] != actual[7 + firstIndexAfterDot])
                return false;
        }
    }

    return true;
}