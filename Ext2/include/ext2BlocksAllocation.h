#include "../include/disk.h"
#include "../include/structures.h"

#ifndef EXT2_EXT2BLOCKSALLOCATION_H
#define EXT2_EXT2BLOCKSALLOCATION_H

uint32_t allocateBlockToDirectory(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* inode, uint32_t& newBlockGlobalIndex, ext2_inode* updatedInode);

uint32_t deallocateLastBlockInDirectory(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* inode);

//Being given a block global index and a new value allocation updates the bitmap to mark it as free/occupied; It also updates the corresponding group descriptor to increase/decrease
//the free number of blocks
uint32_t updateBlockAllocation(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t blockGlobalIndex, uint32_t newAllocationValue);

//////////////////////////////////

static uint32_t addSecondOrderDataBlock(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* inode, uint32_t& newBlockGlobalIndex, uint32_t& secondOrderTableGlobalIndex);

static uint32_t addThirdOrderDataBlock(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* inode, uint32_t& newBlockGlobalIndex, uint32_t& secondOrderTableGlobalIndex);

static uint32_t addForthOrderDataBlock(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* inode, uint32_t& newBlockGlobalIndex, uint32_t& secondOrderTableGlobalIndex);

//////////////////////////////////

static uint32_t checkAndEventuallyDeallocateSecondOrderTableBlock(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* inode);

static uint32_t checkAndEventuallyDeallocateThirdOrderTableBlock(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* inode);

static uint32_t checkAndEventuallyDeallocateForthOrderTableBlock(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* inode);

/////////////////////////////////

static uint32_t addBlockIndexToAnotherBlock(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t blockToAddTheValueTo, uint32_t offsetInBlockToAddTheValueTo, uint32_t valueToAdd);

//Being given a previousBlockGlobalIndex it tries to allocate the block immediately after it. If the block immediately after it is not free, it tries to allocate a block in the same
//group as this block. If this also fails, it searches for a free block in any group. The searching part for each group is handled by METHOD searchAndOccupyFreeDataBlockInGivenGroup
//It also updates the corresponding group descriptor to reduce the number of free blocks
//CAUTION it also updates the bitmap & number of free blocks in its group descriptor
static uint32_t searchAndOccupyFreeDataBlock(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t previousBlockGlobalIndex, uint32_t& newDataBlockGlobalIndex);

//Being given a group and a previousBlockLocalIndex, it firstly tries to allocate the block immediately after the previousBlockLocalIndex if that is empty. If not, it tries to find a
//free block anywhere in the given group. If there is no free block in the given group, the search fail CAUTION it does NOT look in other groups if the search in the given group fails
static uint32_t searchAndOccupyFreeDataBlockInGivenGroup(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t group, uint32_t previousBlockLocalIndex, uint32_t& newDataBlock);

#endif
