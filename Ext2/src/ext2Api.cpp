#include "windows.h"
#include "string.h"
#include "vector"

#include "../include/disk.h"
#include "../include/diskCodes.h"
#include "../include/structures.h"
#include "../include/ext2FunctionUtils.h"
#include "../include/ext2Attributes.h"
#include "../include/codes/ext2Codes.h"
#include "../include/ext2.h"
#include "../include/ext2Api.h"

uint32_t createDirectory(DiskInfo* diskInfo, ext2_super_block* superBlock, char* directoryParentPath, char* newDirectoryName, uint32_t newDirectoryType)
{
    ext2_inode* actualInode = nullptr;

}