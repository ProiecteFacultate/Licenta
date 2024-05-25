#ifndef TESTINGPROGRAM_FAT32TIMETESTS_H
#define TESTINGPROGRAM_FAT32TIMETESTS_H

/////////////////WRITE TESTS

//A single 50 Mib file, 2 writes with TRUNCATE, count only the write time, for 4/8/16 sectors per cluster
void fat32_time_test_1();

//Many(100) 500 Kib files, 1 write for each one, count only the writes time, for 4/8/16 sectors per cluster
void fat32_time_test_2();

//Multiple rounds(5) of 3 types of files: big 9 Mib, medium 900 Kib, small 90 Kib, count only the write time, for 4/8/16 sectors per cluster
void fat32_time_test_3();

//Write (25 Mib) twice, first with TRUNCATE, second with APPEND (so 50 Mib in total), for 4/8/16 sectors per cluster
void fat32_time_test_4();

//Write (50 Mib) file in 50 rounds, first truncate and 49 append, and between these rounds add small files. Count only the time for big file, for 4/8/16 sectors per cluster
void fat32_time_test_5();


//////////////////READ TESTS

//A single 50 Mib file, write and read completely, starting from 0, count only the read time, for 4/8/16 sectors per cluster
void fat32_time_test_6();

//A single 50 Mib file, write and read half of the written bytes (from end of quarter 1 to end of quarter 3: 12.5Mib - 37.5Mib), count only the read time, for 4/8/16 sectors per cluster
void fat32_time_test_7();

//Many(100) 500 Kib files, write and  read completely, starting from 0, count only the read time, for 4/8/16 sectors per cluster
void fat32_time_test_8();

//Write (50 Mib) file in 50 rounds, first truncate and 49 append, and between these rounds add small files, and then read only the big file for its full size, starting from 0.
//Count only the read time for big file, for 4/8/16 sectors per cluster
void fat32_time_test_9();

#endif
