#include "../include/disk.h"
#include "../include/structures.h"

#ifndef EXT2_EXT2FUNCTIONUTILS_H
#define EXT2_EXT2FUNCTIONUTILS_H

uint32_t getNumberOfGroups(ext2_super_block* superBlock);
uint32_t getNumberOfGroupDescriptorsBlocksInFullGroup(ext2_super_block* superBlock);
uint32_t getNumberOfInodesBlocksInFullGroup(ext2_super_block* superBlock);
uint32_t getNumberOfDataBlocksInFullGroup(ext2_super_block* superBlock);

uint32_t getNumberOfBlocksForGivenGroup(ext2_super_block* superBlock, uint32_t group);
uint32_t getFirstBlockForGivenGroup(ext2_super_block* superBlock, uint32_t group);
uint32_t getNumberOfDataBlocksForGivenGroup(ext2_super_block* superBlock, uint32_t group);
uint32_t getFirstDataBlockForGivenGroup(ext2_super_block* superBlock, uint32_t group);
uint32_t getNumberOfGroupDescriptorsBlocksForGivenGroup(ext2_super_block* superBlock, uint32_t group);
uint32_t getFirstGroupDescriptorsBlockForGivenGroup(ext2_super_block* superBlock, uint32_t group);
uint32_t getNumberOfInodesBlocksForGivenGroup(ext2_super_block* superBlock, uint32_t group);
uint32_t getFirstInodeBlockForGivenGroup(ext2_super_block* superBlock, uint32_t group);
uint32_t getDataBitmapBlockForGivenGroup(ext2_super_block* superBlock, uint32_t group);
uint32_t getInodeBitmapBlockForGivenGroup(ext2_super_block* superBlock, uint32_t group);
uint32_t getFirstInodeTableBlockForGivenGroup(ext2_super_block* superBlock, uint32_t group);

uint32_t getNumberOfInodesForGivenGroup(ext2_super_block* superBlock, uint32_t group);

uint32_t getNumberOfSectorsPerBlock(DiskInfo* diskInfo, ext2_super_block* superBlock);
uint32_t getFirstSectorForGivenBlock(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t block);
uint32_t getFirstSectorForGivenGroup(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t group);

//Being given the index of an inode inside a group, calculate the index of that inode globally (if we have 1000 inodes in a group and we want to get global index of local index 50 of
//group 5, then the global index will be 4005)
uint32_t getGlobalIndexOfInode(ext2_super_block* superBlock, uint32_t group, uint32_t localInodeIndex);
//Being given the index of an inode inside a group, get the block where that inode is located (global value of the block)
uint32_t getInodeBlockForInodeIndexInGroup(ext2_super_block* superBlock, uint32_t group, uint32_t localInodeIndex);

//Being given a group, it returns its group descriptor from the main group descriptors (group 0 group descriptors) and also the block (global index) where it is located, and the offset
//inside the block
uint32_t getGroupDescriptorOfGivenGroup(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t group, ext2_group_desc* searchedGroupDescriptor, uint32_t& groupDescriptorBlock,
                                        uint32_t& groupDescriptorOffsetInsideBlock);

//Being given a local index for a data block of a inode (for example the 50321th data block of the inode), calculate its global value, respecting the multi ordering schema
uint32_t getDataBlockGlobalIndexByLocalIndex(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* inode, uint32_t searchedBlockLocalIndexInInode, uint32_t& searchedBlockGlobalIndex);
//Being given a global index of an inode, searches and returns the ext2_inode for the inode
uint32_t getInodeByInodeGlobalIndex(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t inodeGlobalIndex, ext2_inode* searchedInode);
//Being given a group, iterates through its inodes bitmap counting the number of occupied inodes for that group
uint32_t getNumberOfOccupiedInodesInGroup(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t group, uint32_t& numberOfOccupiedInodes);
//Being given the group for which to update the group descriptor, and a change (positive or negative) in free inodes & free blocks, it updates the group descriptor
uint32_t updateGroupDescriptor(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t group, int32_t freeInodesChange, int32_t freeDataBlocksChange);
//Being given an inode, it adds it to it's group inodes table. The inode is automatically taken from the i_group field in the ext2_inode structure
//CAUTION this method only adds the inode tot the inode table, it does not mark its corresponding bit as occupied in the bitmap, nor update the number of free inodes in group descriptor
uint32_t addInodeToInodeTable(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* inode);
//Being given a global index for an inode, and a new value (0 or 1) updates the value in the corresponding inode bitmap
uint32_t updateValueInInodeBitmap(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t inodeGlobalIndex, uint8_t newValue);
//Being given a global index for a data block, and a new value (0 or 1) updates the value in the corresponding data block bitmap
uint32_t updateValueInDataBlockBitmap(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t dataBlockGlobalIndex, uint8_t newValue);

#endif
