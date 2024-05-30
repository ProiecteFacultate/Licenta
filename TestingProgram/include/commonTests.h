#ifndef TESTINGPROGRAM_COMMONTESTS_H
#define TESTINGPROGRAM_COMMONTESTS_H

//for more file size cases, writes a file of that size, and counts write time
void common_time_test_1();

//for more file size cases, writes a file of that size, and then again with TRUNCATE and counts the second write time
void common_time_test_2();

//for more file size cases, for 100 files, write once and count the write
void common_time_test_3();

//for more file size cases, writes a file of that size, and then again with APPEND and counts the second write time. The size of append is the same as TRUNCATE, and in total will be
//double the previous tests
void common_time_test_4();

//for more file size cases, write a big file in 16 rounds (first truncate, 15 append), and between the writes put a small file, count the total time only for the big file
void common_time_test_5();

//for more file size cases, writes a file of that size, reads it, and counts only the read time
void common_time_test_6();

//for more file size cases, for 100 files, write and read once, and count the read time of all
void common_time_test_7();

#endif
