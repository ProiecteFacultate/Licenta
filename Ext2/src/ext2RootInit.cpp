#include "string"
#include "string.h"
#include "vector"
#include "iostream"
#include "windows.h"

#include "../include/disk.h"
#include "../include/diskCodes.h"
#include "../include/structures.h"
#include "../include/ext2FunctionUtils.h"
#include "../include/codes/ext2Codes.h"
#include "../include/codes/ext2Attributes.h"
#include "../include/ext2Utils.h"
#include "../include/codes/ext2ApiResponseCodes.h"
#include "../include/ext2.h"
#include "../include/ext2RootInit.h"

#define BIG_VALUE 99999999

uint32_t addInodeToGroup(DiskInfo* diskInfo, ext2_super_block* superBlock)
{
    //create the new inode (without preallocate blocks yet)
    ext2_inode* newInode = new ext2_inode();
    createNewInode(superBlock, newInode);

    //mark the inode as occupied in the inode bitmap
    uint32_t updateValueInInodeBitmapResult = updateValueInInodeBitmap(diskInfo, superBlock, newInode->i_global_index, 1);
    if(updateValueInInodeBitmapResult == UPDATE_VALUE_IN_INODE_BITMAP_FAILED)
        return DIRECTORY_CREATION_FAILED_FOR_OTHER_REASON;

    uint32_t preallocateNumberOfBlocks = superBlock->s_prealloc_dir_blocks;
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

    return (inodeGroupDescriptorUpdate == UPDATE_GROUP_DESCRIPTOR_SUCCESS && preallocateBlocksDescriptorUpdate == UPDATE_GROUP_DESCRIPTOR_SUCCESS) ? DIRECTORY_CREATION_SUCCESS
                                                                                                        : DIRECTORY_CREATION_FAILED_FOR_OTHER_REASON;
}

void createNewInode(ext2_super_block* superBlock, ext2_inode* newInode)
{
    SYSTEMTIME time;
    GetSystemTime(&time);

    memset(newInode, 0, sizeof(ext2_inode));

    newInode->i_mode = FILE_TYPE_DIRECTORY;
    newInode->i_size = 0;
    //high 7 bits represent how many years since 1900, next 4 for month, next 5 for day and the low 16 represent the second in that day with a granularity of 2 (see in fat)
    newInode->i_atime = ((time.wYear - 1900) << 25) | (time.wMonth << 21) | (time.wDay << 16) | ((time.wHour * 3600 + time.wMinute * 60 + time.wSecond) / 2);
    newInode->i_ctime = newInode->i_atime;
    newInode->i_mtime = newInode->i_atime;
    newInode->i_blocks = superBlock->s_prealloc_dir_blocks;
    newInode->i_file_acl = 0;
    newInode->i_dir_acl = 0;
    newInode->i_faddr = 0;
    newInode->i_group = 0; //the root is in group 0
    newInode->i_global_index = 0; //the group inode is first inode globally
}
