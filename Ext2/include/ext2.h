#include "vector"

#include "../include/disk.h"
#include "../include/structures.h"

#ifndef EXT2_EXT2_H
#define EXT2_EXT2_H

//Being given a directory type to create an inode for, respecting the heuristics (from book) searches for a suitable block for the new inode and creates the inode
//CAUTION it doesn't preallocate blocks; it doesn't write the new inode in the inode table, it does not mark as occupied in the bitmap; it doesn't reduce the num of free inodes in its corresponding group desc it just creates the inode
uint32_t createInode(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* parentInode, ext2_inode* createdInode, uint32_t fileType, bool& isParentRoot);

//Being given a group and numOfBlocks, it tries to allocate numOfBlocks consecutive blocks firstly in a given preferredGroup, and if it can't, in looks in any group.
//If it can't find numOfBlocks free consecutive blocks it is a fail. It returns a vector. The searching part for each group is handled by METHOD searchAndOccupyMultipleBlocksInGroup
//It also updates the corresponding group descriptor to reduce the number of free blocks
uint32_t searchAndOccupyMultipleBlocks(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t preferredGroup,  uint32_t numOfBlocks, std::vector<uint32_t>& newBlocks);

//Being given a group and numOfBlocks, it tries to allocate numOfBlocks consecutive blocks. If it can't find numOfBlocks free consecutive blocks it is a fail. It returns a vector containing
//the GLOBAL indexes of these blocks; If for example numOfBlocks = 8, and we find only 7, it is a fail CAUTION it does NOT look in other groups if the search in the given group fails
//CAUTION it doesn't update the values in data block bitmap, it doesn't reduce the number of free blocks in the corresponding group descriptor, it just finds free blocks
uint32_t searchAndOccupyMultipleBlocksInGivenGroup(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t group, uint32_t numOfBlocks, std::vector<uint32_t>& newBlocks);

//Being given a previousBlockGlobalIndex it tries to allocate the block immediately after it. If the block immediately after it is not free, it tries to allocate a block in the same
//group as this block. If this also fails, it searches for a free block in any group. The searching part for each group is handled by METHOD searchAndOccupyFreeDataBlockInGivenGroup
//It also updates the corresponding group descriptor to reduce the number of free blocks
//CAUTION it doesn't update the value in data block bitmap, it doesn't reduce the number of free blocks in the corresponding group descriptor, it just finds a free blocks
uint32_t searchAndOccupyFreeDataBlock(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t preferredGroup, uint32_t previousBlockGlobalIndex, uint32_t& newDataBlock);

//Being given a group and a previousBlockLocalIndex, it firstly tries to allocate the block immediately after the previousBlockLocalIndex if that is empty. If not, it tries to find a
//free block anywhere in the given group. If there is no free block in the given group, the search fail CAUTION it does NOT look in other groups if the search in the given group fails
uint32_t searchAndOccupyFreeDataBlockInGivenGroup(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t group, uint32_t previousBlockLocalIndex, uint32_t& newDataBlock);

uint32_t searchInodeByFullPath(DiskInfo* diskInfo, ext2_super_block* superBlock, char* directoryPath, ext2_inode** inode, bool& isSearchedInodeRoot);

uint32_t searchInodeByDirectoryNameInParent(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* parentInode, char* searchedDirectoryName, ext2_inode* searchedInode,
                                            bool& isSearchedInodeRoot);

uint32_t searchDirectoryWithGivenNameInGivenBlockData(char* searchedName, char* blockBuffer, uint32_t occupiedBytesInBlock, uint32_t& searchedInodeGlobalIndex);

#endif
