#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include/diskUtils.h"
#include "../include/fat32Init.h"

#ifndef FAT32_FAT32METHODUTILS_H
#define FAT32_FAT32METHODUTILS_H

bool compareFileNames(char* expected, uint8_t actual[]);

#endif
