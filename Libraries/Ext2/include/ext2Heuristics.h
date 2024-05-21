#include "vector"

#ifndef EXT2_EXT2HEURISTICS_H
#define EXT2_EXT2HEURISTICS_H

//For a FOLDER, whose parent is root, search a suitable free inode, respecting the algorithm in the book; return the global index of this inode
//CAUTION this method only returns the index of the inode, it does not mark it as occupied in the bitmap, nor reduce the number of free inodes in its group's group descriptor
uint32_t searchFreeInodeForDirectoryHavingParentRoot(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t& foundInodeGlobalIndex);
//For a FOLDER, whose parent is NOT root, search a suitable free inode, respecting the algorithm in the book; return the global index of this inode
//CAUTION this method only returns the index of the inode, it does not mark it as occupied in the bitmap, nor reduce the number of free inodes in its group's group descriptor
uint32_t searchFreeInodeForNestedDirectory(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* parentInode, uint32_t& foundInodeGlobalIndex);
//For a REGULAR FILE, search a suitable free inode, respecting the algorithm in the book; return the global index of this inode
//CAUTION this method only returns the index of the inode, it does not mark it as occupied in the bitmap, nor reduce the number of free inodes in its group's group descriptor
uint32_t searchFreeInodeForRegularFile(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* parentInode, uint32_t& foundInodeGlobalIndex);

////////////

//Returns a vector with number of free inodes and free blocks for each group
static uint32_t getGroupsFreeInodesAndFreeBlocks(DiskInfo* diskInfo, ext2_super_block* superBlock, std::vector<std::pair<uint16_t, uint16_t>>& groupFreeInodesAndFreeBlocksList);
//Being given a group, returns the global index of the first free inode in the group
static uint32_t searchFirstFreeInodeInGroup(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t group, uint32_t& firstFreeInodeInGroupGlobalIndex);
//Calculates group debt; add 1 for each folder in group, decrease 1 for each file
static uint32_t calculateGroupDebt(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t group, int32_t& debt);

#endif
