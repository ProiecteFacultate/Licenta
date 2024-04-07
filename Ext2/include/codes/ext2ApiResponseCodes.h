#ifndef EXT2_EXT2APIRESPONSECODES_H
#define EXT2_EXT2APIRESPONSECODES_H

#define DIRECTORY_CREATION_PARENT_DO_NOT_EXIST                                                       0
#define DIRECTORY_CREATION_PARENT_NOT_A_FOLDER                                                       1
#define DIRECTORY_CREATION_NEW_NAME_ALREADY_EXISTS                                                   2
#define DIRECTORY_CREATION_NO_FREE_INODES                                                            3
#define DIRECTORY_CREATION_NO_FREE_DATA_BLOCKS                                                       4
#define DIRECTORY_CREATION_FAILED_FOR_OTHER_REASON                                                   5
#define DIRECTORY_CREATION_SUCCESS                                                                   9999

#define GET_SUBDIRECTORIES_GIVEN_DIRECTORY_DO_NOT_EXIST                                              0
#define GET_SUBDIRECTORIES_GIVEN_DIRECTORY_NOT_A_FOLDER                                              1
#define GET_SUBDIRECTORIES_FAILED_FOR_OTHER_REASON                                                   2
#define GET_SUBDIRECTORIES_SUCCESS                                                                   9999

#define WRITE_BYTES_TO_FILE_CAN_NOT_WRITE_GIVEN_FILE                                                 0
#define WRITE_BYTES_TO_FILE_GIVEN_FILE_DO_NOT_EXIST_OR_SEARCH_FAIL                                   1
#define WRITE_BYTES_TO_FILE_FAILED_FOR_OTHER_REASON                                                  2
#define WRITE_BYTES_TO_FILE_SUCCESS                                                                  9999

#define READ_BYTES_FROM_FILE_CAN_NOT_READ_GIVEN_FILE                                                 0
#define READ_BYTES_FROM_FILE_GIVEN_FILE_DO_NOT_EXIST_OR_SEARCH_FAIL                                                 1
#define READ_BYTES_FROM_FILE_GIVEN_START_EXCEEDS_FILE_SIZE                                           2
#define READ_BYTES_FROM_FILE_FAILED_FOR_OTHER_REASON                                                 3
#define READ_BYTES_FROM_FILE_SUCCESS                                                                 4

#endif
