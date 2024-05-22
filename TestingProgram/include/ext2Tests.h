#ifndef TESTINGPROGRAM_EXT2TESTS_H
#define TESTINGPROGRAM_EXT2TESTS_H

//A single 50 Mib file, 2 writes with TRUNCATE, count only the write time, for 2048, 4096, 8192 blockSize
void ext2_test_1();

//Many(100) 500 Kib files, 1 write for each one with TRUNCATE, count only the writes time, for 2048, 4096, 8192 blockSize
void ext2_test_2();

#endif
