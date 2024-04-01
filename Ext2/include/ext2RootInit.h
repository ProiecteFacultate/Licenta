#ifndef EXT2_EXT2ROOTINIT_H
#define EXT2_EXT2ROOTINIT_H

//JUST FOR INITIALIZING THE ROOT. IF IT WORKS IT WORKS! DON'T CHANGE!

uint32_t addInodeToGroup(DiskInfo* diskInfo, ext2_super_block* superBlock);

void createNewInode(ext2_super_block* superBlock, ext2_inode* newInode);

#endif