#ifndef TESTINGPROGRAM_COMMONTESTS_H
#define TESTINGPROGRAM_COMMONTESTS_H

//for more file size cases, writes a file of that size, and counts write time
void common_time_test_1();

//for more file size cases, writes a file of that size, and then again with TRUNCATE and counts the second write time
void common_time_test_2();

//for more file size cases, for 100 files, write once and count the write
void common_time_test_3();

#endif
