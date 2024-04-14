#include "vector"
#include "string"

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
uint32_t searchAndOccupyMultipleBlocks(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t preferredGroup, uint32_t numOfBlocks, std::vector<uint32_t>& newBlocks);

//Being given a group and numOfBlocks, it tries to allocate numOfBlocks consecutive blocks. If it can't find numOfBlocks free consecutive blocks it is a fail. It returns a vector containing
//the GLOBAL indexes of these blocks; If for example numOfBlocks = 8, and we find only 7, it is a fail CAUTION it does NOT look in other groups if the search in the given group fails
//CAUTION it doesn't update the values in data block bitmap, it doesn't reduce the number of free blocks in the corresponding group descriptor, it just finds free blocks
uint32_t searchAndOccupyMultipleBlocksInGivenGroup(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t group, uint32_t numOfBlocks, std::vector<uint32_t>& newBlocks);

uint32_t searchInodeByFullPath(DiskInfo* diskInfo, ext2_super_block* superBlock, char* directoryPath, ext2_inode** inode, bool& isSearchedInodeRoot);

uint32_t searchInodeByDirectoryNameInParent(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* parentInode, char* searchedDirectoryName, ext2_inode* searchedInode,
                                            bool& isSearchedInodeRoot);

uint32_t searchDirectoryWithGivenNameInGivenBlockData(char* searchedName, char* blockBuffer, uint32_t occupiedBytesInBlock, ext2_dir_entry* searchedDirectoryEntry, uint32_t& directoryEntryOffsetInBlock);

uint32_t writeBytesToFileWithTruncate(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* inode, char* dataBuffer, uint32_t maxBytesToWrite,
                                      uint32_t& numberOfBytesWritten, uint32_t& reasonForIncompleteWrite);

uint32_t writeBytesToFileWithAppend(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* inode, char* dataBuffer, uint32_t maxBytesToWrite,
                                    uint32_t& numberOfBytesWritten, uint32_t& reasonForIncompleteWrite);

uint32_t getSubDirectoriesByParentInode(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* parentInode, std::vector<std::pair<ext2_inode*, ext2_dir_entry*>>& subDirectories);

//Being given a parent inode, deletes all its direct and indirect directories's inodes, and also delete this inode
uint32_t deleteInodeOfDirectoryAndChildren(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* inode);

//For this method, it is considered a success if it manages to get all direct and indirect children, and try to deallocate the blocks; if it fails to deallocate blocks, it is not a
//fail, but it will give a warning. this only frees all blocks for direct and indirect children, it does not delete ext2_inodes or ext2_dir_entrys
uint32_t freeBlocksOfDirectoryAndChildren(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* inode, std::string& warning);

//Being given an inode, deletes it. In order to do this is necessary only to mark it with 0 in inode bitmap, and decrease with 1 the number of inodes in the corresponding group descriptor
uint32_t deleteInode(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* inode);

//Being given the inode of a directory, and the inode of its parent, deletes the directory entry from the parent. It also updates the parent inode to reduce its size
uint32_t deleteDirectoryEntryFromParent(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* inodeToBeDeleted, ext2_inode* parentInode);

#endif
