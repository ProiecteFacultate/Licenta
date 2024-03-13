#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include/diskUtils.h"
#include "../include/fat32Init.h"

#ifndef FAT32_UTILS_H
#define FAT32_UTILS_H

//Minor helper function

bool checkDirectoryNameValidity(const char* directoryName);

bool compareDirectoryNames(char* expected, const char* actual);

void formatDirectoryName(const char* directoryName, char* formattedName);

void extractParentPathFromPath(const char* fullPath, char* parentPath);

#endif
