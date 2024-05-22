#ifndef TESTINGPROGRAM_FAT32TESTS_H
#define TESTINGPROGRAM_FAT32TESTS_H

//A single 50 Mib file, 2 writes with TRUNCATE, count only the write time, for 4/8/16 sectors per cluster
void fat32_test_1();

//Many(100) 500 Kib files, 1 write for each one, count only the writes time, for 4/8/16 sectors per cluster
void fat32_test_2();

#endif
