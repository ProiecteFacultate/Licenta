#ifndef TESTINGPROGRAM_EXT2TESTS_H
#define TESTINGPROGRAM_EXT2TESTS_H

//////////////////WRITE TESTS

//A single 50 Mib file, 2 writes with TRUNCATE, count only the write time, for 2048, 4096, 8192 blockSize
void ext2_test_1();

//Many(100) 500 Kib files, 1 write for each one with TRUNCATE, count only the writes time, for 2048, 4096, 8192 blockSize
void ext2_test_2();

//Multiple rounds(5) of 3 types of files: big 9 Mib, medium 900 Kib, small 90 Kib, count only the write time, for 2048, 4096, 8192 blockSize
void ext2_test_3();

//Write (25 Mib) twice, first with TRUNCATE, second with APPEND (so 50 Mib in total), for 2048, 4096, 8192 blockSize
void ext2_test_4();


//////////////////READ TESTS

//A single 50 Mib file, write and read completely, starting from 0, count only the read time, for 2048, 4096, 8192 blockSize
void ext2_test_5();

//Many(100) 500 Kib files, write and  read completely, starting from 0, count only the read time, for 2048, 4096, 8192 blockSize
void ext2_test_7();

#endif
