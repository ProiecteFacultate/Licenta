#include "windows.h"
#include "string.h"
#include "vector"

#include "../include/disk.h"
#include "../include/diskCodes.h"
#include "../include/structures.h"
#include "../include/ext2FunctionUtils.h"
#include "../include/codes/ext2Attributes.h"
#include "../include/codes/ext2Codes.h"
#include "../include/codes/ext2ApiResponseCodes.h"
#include "../include/ext2.h"
#include "../include/ext2Api.h"

uint32_t createDirectory(DiskInfo* diskInfo, ext2_super_block* superBlock, char* parentDirectoryPath, char* newDirectoryName, uint32_t newDirectoryType)
{
    ext2_inode* actualInode = nullptr;
    bool isSearchedInodeRoot;
    uint32_t searchInodeResult = searchInodeByFullPath(diskInfo, superBlock, parentDirectoryPath, &actualInode, isSearchedInodeRoot);

    if(searchInodeResult != SEARCH_INODE_BY_FULL_PATH_SUCCESS)
        return DIRECTORY_CREATION_PARENT_DO_NOT_EXIST;

    uint32_t parentAlreadyContainsDirectoryWithGivenName = searchInodeByDirectoryNameInParent(diskInfo, superBlock, actualInode,
                                                                                              newDirectoryName, new ext2_inode(), isSearchedInodeRoot);

    if(actualInode->i_mode != FILE_TYPE_DIRECTORY)
    {
        delete actualInode;
        return DIRECTORY_CREATION_PARENT_NOT_A_FOLDER;
    }

    if(parentAlreadyContainsDirectoryWithGivenName == SEARCH_INODE_BY_DIRECTORY_NAME_IN_PARENT_SUCCESS)
    {
        if(actualInode != nullptr)
            delete actualInode;
        return DIRECTORY_CREATION_NEW_NAME_ALREADY_EXISTS;
    }

    return DIRECTORY_CREATION_SUCCESS;

    //TODO search empty blocks .... search empty inodes
}