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
#define READ_BYTES_FROM_FILE_GIVEN_FILE_DO_NOT_EXIST_OR_SEARCH_FAIL                                  1
#define READ_BYTES_FROM_FILE_GIVEN_START_EXCEEDS_FILE_SIZE                                           2
#define READ_BYTES_FROM_FILE_FAILED_FOR_OTHER_REASON                                                 3
#define READ_BYTES_FROM_FILE_SUCCESS                                                                 4

#define TRUNCATE_FILE_GIVEN_FILE_DO_NOT_EXIST_OR_SEARCH_FAIL                                         0
#define TRUNCATE_FILE_CAN_NOT_TRUNCATE_GIVEN_FILE_TYPE                                               1
#define TRUNCATE_FILE_NEW_SIZE_GREATER_THAN_ACTUAL_SIZE                                              2
#define TRUNCATE_FILE_FAILED_FOR_OTHER_REASON                                                        3
#define TRUNCATE_FILE_SUCCESS                                                                        9999

#define DELETE_DIRECTORY_CAN_NOT_DELETE_ROOT                                                         0
#define DELETE_DIRECTORY_FAILED_TO_FIND_GIVEN_DIRECTORY                                              1
#define DELETE_DIRECTORY_FAILED_FOR_OTHER_REASON                                                     2
#define DELETE_DIRECTORY_FAILED_TO_DELETE_DIRECTORY_ENTRY_FROM_PARENT                                3
#define DELETE_DIRECTORY_FAILED_TO_DELETE_INODE                                                      4
#define DELETE_DIRECTORY_FAILED_TO_FREE_BLOCKS                                                       5
#define DELETE_DIRECTORY_SUCCESS                                                                     9999

#define DIRECTORY_GET_DISPLAYABLE_ATTRIBUTES_FAILED_GIVEN_DIRECTORY_DO_NOT_EXIST                     0
#define DIRECTORY_GET_DISPLAYABLE_ATTRIBUTES_FAILED_FOR_OTHER_REASON                                 1
#define DIRECTORY_GET_DISPLAYABLE_ATTRIBUTES_SUCCESS                                                 9999

#define PREALLOCATE_BLOCKS_INCOMPLETE_ALLOCATION                                                     0
#define PREALLOCATE_BLOCKS_FILE_DO_NOT_EXIST_OR_SEARCH_FAIL                                          1
#define PREALLOCATE_BLOCKS_CAN_NOT_PREALLOCATE_TO_GIVEN_FILE_TYPE                                    2
#define PREALLOCATE_BLOCKS_SUCCESS                                                                   9999

#endif
