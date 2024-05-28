#include "cstdint"
#include "vector"

#include "ext2Structures.h"

#ifndef TESTINGPROGRAM_EXT2SPACETESTS_H
#define TESTINGPROGRAM_EXT2SPACETESTS_H

//shows how much space (in bytes and as percentage) occupies the structures of the file system
void ext2_space_test_1();

//writes one big file and shows its internal fragmentation (it will be only for its last cluster), for 2048, 4096, 8192 blockSize
void ext2_space_test_2();

//writes many (100) medium files (500kIB) each, and then sum their total internal fragmentation, for 2048, 4096, 8192 blockSize
void ext2_space_test_3();

//write many (100) small files 2 times with truncate, once with the 8 * block size (8 are preallocated) + 1 byte, so fragmentation will be higher, and second time 1 byte less,
//so equal to block size * 8, and fragmentation will be 0, for 2048, 4096, 8192 blockSize
void ext2_space_test_ignore_1();

//Multiple rounds (5) of 3 types of files: big 9 Mib, medium 900 Kib, small 90 Kib, count total internal fragmentation for all of them, for 2048, 4096, 8192 blockSize
void ext2_space_test_4();

//////////////////////

void calculateInternalFragmentation(ext2_super_block* superBlock, std::vector<std::pair<ext2_inode*, ext2_dir_entry*>> directories, uint32_t& totalSizeOnDisk, uint32_t& totalFilesSizes,
                                    uint32_t& totalInternalFragmentation);

#endif
