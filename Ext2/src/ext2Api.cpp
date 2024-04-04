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
    bool isParentRoot;
    uint32_t searchParentInodeResult = searchInodeByFullPath(diskInfo, superBlock, parentDirectoryPath, &actualInode, isParentRoot);

    if(searchParentInodeResult != SEARCH_INODE_BY_FULL_PATH_SUCCESS)
        return DIRECTORY_CREATION_PARENT_DO_NOT_EXIST;

    uint32_t parentAlreadyContainsDirectoryWithGivenName = searchInodeByDirectoryNameInParent(diskInfo, superBlock, actualInode,
                                                                                              newDirectoryName, new ext2_inode(), isParentRoot);

    if(actualInode->i_mode != FILE_TYPE_DIRECTORY)
        return DIRECTORY_CREATION_PARENT_NOT_A_FOLDER;

    if(parentAlreadyContainsDirectoryWithGivenName == SEARCH_INODE_BY_DIRECTORY_NAME_IN_PARENT_SUCCESS)
        return DIRECTORY_CREATION_NEW_NAME_ALREADY_EXISTS;

    //create the new inode (without preallocate blocks yet)
    ext2_inode* newInode = new ext2_inode();
    uint32_t createInodeResult = createInode(diskInfo, superBlock, actualInode, newInode, newDirectoryType, isParentRoot);

    if(createInodeResult == CREATE_INODE_FAILED_NO_FREE_INODES)
        return DIRECTORY_CREATION_NO_FREE_INODES;
    else if(createInodeResult == CREATE_INODE_FAILED_FOR_OTHER_REASON)
        return DIRECTORY_CREATION_FAILED_FOR_OTHER_REASON;

    //mark the inode as occupied in the inode bitmap
    uint32_t updateValueInInodeBitmapResult = updateValueInInodeBitmap(diskInfo, superBlock, newInode->i_global_index, 1);

    if(updateValueInInodeBitmapResult == UPDATE_VALUE_IN_INODE_BITMAP_FAILED)
        return DIRECTORY_CREATION_FAILED_FOR_OTHER_REASON;

    //now we look for blocks to preallocate
    uint32_t preallocateNumberOfBlocks;
    if(newDirectoryType == FILE_TYPE_DIRECTORY)
        preallocateNumberOfBlocks = superBlock->s_prealloc_dir_blocks;
    else if(newDirectoryType == FILE_TYPE_REGULAR_FILE)
        preallocateNumberOfBlocks = superBlock->s_prealloc_blocks;

    std::vector<uint32_t> preallocateBlocks;
    uint32_t searchPreallocateBlocksResult;
    do {
        searchPreallocateBlocksResult = searchAndOccupyMultipleBlocks(diskInfo, superBlock, newInode->i_group, preallocateNumberOfBlocks, preallocateBlocks);
        preallocateNumberOfBlocks /= 2;
    } while (searchPreallocateBlocksResult != SEARCH_MULTIPLE_DATA_BLOCKS_SUCCESS && preallocateNumberOfBlocks > 0);

    if(searchPreallocateBlocksResult == SEARCH_MULTIPLE_DATA_BLOCKS_NO_ENOUGH_CONSECUTIVE_FREE_BLOCKS)
        return DIRECTORY_CREATION_NO_FREE_DATA_BLOCKS;
    else  if(searchPreallocateBlocksResult == SEARCH_MULTIPLE_DATA_BLOCKS_FAILED_FOR_OTHER_REASON)
        return DIRECTORY_CREATION_FAILED_FOR_OTHER_REASON;

    //mark the preallocate blocks as occupied in the data blocks bitmap
    for(int i = 0; i < preallocateBlocks.size(); i++)
    {
        uint32_t updateDataBlockBitmapResult = updateValueInDataBlockBitmap(diskInfo, superBlock, preallocateBlocks[i], 1);
        if(updateDataBlockBitmapResult == UPDATE_VALUE_IN_DATA_BLOCK_BITMAP_FAILED)
            return DIRECTORY_CREATION_FAILED_FOR_OTHER_REASON;
    }

    newInode->i_blocks = preallocateBlocks.size();
    for(int i = 0; i < preallocateBlocks.size(); i++)
        newInode->i_block[i] = preallocateBlocks[i];

    //now add the inode to the inode table
    uint32_t addInodeToInodeTableResult = addInodeToInodeTable(diskInfo, superBlock, newInode);

    if(addInodeToInodeTableResult == ADD_INODE_TO_INODE_TABLE_FAILED)
        return DIRECTORY_CREATION_FAILED_FOR_OTHER_REASON;

    //update inode's group and preallocate blocks' group groups descriptor
    uint32_t preallocateBlocksGroup = preallocateBlocks[0] / superBlock->s_blocks_per_group;
    uint32_t inodeGroupDescriptorUpdate = updateGroupDescriptor(diskInfo, superBlock, newInode->i_group, -1, 0);
    uint32_t preallocateBlocksDescriptorUpdate = updateGroupDescriptor(diskInfo, superBlock, preallocateBlocksGroup, 0, -preallocateBlocks.size());

    if(inodeGroupDescriptorUpdate == UPDATE_GROUP_DESCRIPTOR_FAILED || preallocateBlocksDescriptorUpdate == UPDATE_GROUP_DESCRIPTOR_FAILED)
        return DIRECTORY_CREATION_FAILED_FOR_OTHER_REASON;

    uint32_t addDirectoryEntryToParentResult = addDirectoryEntryToParent(diskInfo, superBlock, actualInode, newInode, newDirectoryName);

    return (addDirectoryEntryToParentResult == ADD_DIRECTORY_ENTRY_TO_PARENT_SUCCESS) ? DIRECTORY_CREATION_SUCCESS : DIRECTORY_CREATION_FAILED_FOR_OTHER_REASON;
}