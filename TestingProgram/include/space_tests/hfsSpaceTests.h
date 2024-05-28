#include "cstdint"
#include "vector"

#include "hfsStructures.h"

#ifndef TESTINGPROGRAM_HFSSPACETESTS_H
#define TESTINGPROGRAM_HFSSPACETESTS_H

//shows how much space (in bytes and as percentage) occupies the structures of the file system
void hfs_space_test_1();

//writes one big file and shows its internal fragmentation (it will be only for its last cluster), for 2048, 4096, 8192 blockSize
void hfs_space_test_2();

//writes many (100) medium files (500kIB) each, and then sum their total internal fragmentation, for 2048, 4096, 8192 blockSize
void hfs_space_test_3();

//write many (100) small files 2 times with truncate, once with the block size + 1 byte, so fragmentation will be almost 50%, and second time 1 byte less, so equal to block size,
//and fragmentation will be 0, for 2048, 4096, 8192 blockSize
void hfs_space_test_ignore_1();

//Multiple rounds (5) of 3 types of files: big 9 Mib, medium 900 Kib, small 90 Kib, count total internal fragmentation for all of them, for 2048, 4096, 8192 blockSize
void hfs_space_test_4();

//////////////////////

void calculateInternalFragmentation(HFSPlusVolumeHeader* volumeHeader, std::vector<CatalogDirectoryRecord*> directories, uint32_t& totalSizeOnDisk, uint32_t& totalFilesSizes,
                                    uint32_t& totalInternalFragmentation);

#endif
