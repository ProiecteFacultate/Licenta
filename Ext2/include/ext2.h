#include "../include/disk.h"
#include "../include/structures.h"

#ifndef EXT2_EXT2_H
#define EXT2_EXT2_H

uint32_t addInodeToGroup(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t group, uint32_t fileType);

//Being given a group, it looks into its data blocks bitmap for the first free data block and returns the global index of this first free data block in group, and also the global index
//if it finds this block, it also marks it as occupied
uint32_t searchAndOccupyFirstFreeDataBlockInGroup(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t group, uint32_t& firstFreeDataBlock);

//Being given a group number, returns the block (global number of the block) where there is the first free inode in the group, and also the offset inside this block, where the inode entry
//of this free inode is located
uint32_t searchAndOccupyEmptyInodeInGroup(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t group, uint32_t& blockWithFirstFreeInodeInInodeTable, uint32_t& offsetOfFirstFreeInode);

uint32_t createNewInode(ext2_super_block* superBlock, ext2_inode* newInode, uint32_t fileType, std::vector<uint32_t>& blocks);

uint32_t updateMainGroupDescriptor(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_group_desc* newGroupDescriptor, uint32_t groupDescriptorBlock, uint32_t groupDescriptorOffset);

uint32_t findInodeByFullPath(DiskInfo* diskInfo, ext2_super_block* superBlock, char* directoryPath, ext2_inode** inode);

uint32_t findInodeByDirectoryNameInParent(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* parentInode, char* searchedDirectoryName, ext2_inode* searchedInode);
#endif
