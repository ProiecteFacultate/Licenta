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

//Being given a group, it returns its group descriptor from the main group descriptors (group 0 group descriptors) and also the block where it is located, and the offset inside the block
uint32_t getGroupDescriptorOfGivenGroup(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t group, ext2_group_desc* searchedGroupDescriptor, uint32_t& groupDescriptorBlock,
                                        uint32_t& groupDescriptorOffsetInsideBlock);

uint32_t getBitFromByte(uint8_t byte, uint32_t bitIndexInByte);
//CAUTION new bit value is given as a byte, but it should be either 0 or 1
uint32_t changeBitValue(uint32_t byte, uint32_t bitIndexInByte, uint8_t newBitValue);

//Being given a local index for a data block of a inode (for example the 50321th data block of the inode), calculate its global value, respecting the multi ordering schema
uint32_t getDataBlockGlobalIndexByLocalIndex(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* inode, uint32_t searchedBlockLocalIndexInInode, uint32_t& searchedBlockGlobalIndex);

#endif
