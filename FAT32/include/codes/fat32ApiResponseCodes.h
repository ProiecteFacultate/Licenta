#ifndef FAT32_FAT32APIRESPONSECODES_H
#define FAT32_FAT32APIRESPONSECODES_H

#define DIR_CREATION_FAILED                                                    0
#define DIR_CREATION_SUCCESS                                                   999
#define DIR_CREATION_INVALID_DIRNAME                                           2
#define DIR_CREATION_PARENT_DO_NOT_EXIST                                       3
#define DIR_CREATION_NEW_NAME_ALREADY_EXISTS                                   4                                               //a directory with given name for the new dir already exists in parent
#define DIR_CREATION_NO_CLUSTER_AVAILABLE                                      5
#define DIR_CREATION_PARENT_NOT_A_FOLDER                                       6

#define GET_SUB_DIRECTORIES_FAILED                                             0
#define GET_SUB_DIRECTORIES_SUCCESS                                            999
#define GET_SUB_DIRECTORIES_GIVEN_DIRECTORY_CAN_NOT_CONTAIN_SUBDIRECTORIES     2                                               //if the given directory is a file

#define WRITE_BYTES_TO_FILE_FAILED                                             0
#define WRITE_BYTES_TO_FILE_SUCCESS                                            999
#define WRITE_BYTES_TO_FILE_CAN_NOT_WRITE_GIVEN_FILE                           2                                               //access problems for example is read only, or is a directory(folder)

#define READ_BYTES_FROM_FILE_FAILED                                            0
#define READ_BYTES_FROM_FILE_SUCCESS                                           999
#define READ_BYTES_FROM_FILE_CAN_NOT_READ_GIVEN_FILE                           2
#define READ_BYTES_FROM_FILE_GIVEN_FILE_DO_NOT_EXIST                           3
#define READ_BYTES_FROM_FILE_GIVEN_START_EXCEEDS_FILE_SIZE                     4

#define TRUNCATE_FILE_GIVEN_FILE_DO_NOT_EXIST_OR_SEARCH_FAIL                   0
#define TRUNCATE_FILE_CAN_NOT_TRUNCATE_GIVEN_FILE_TYPE                         1
#define TRUNCATE_FILE_NEW_SIZE_GREATER_THAN_ACTUAL_SIZE                        2
#define TRUNCATE_FILE_FAILED_FOR_OTHER_REASON                                  3
#define TRUNCATE_FILE_SUCCESS                                                  999


#define DELETE_DIRECTORY_FAILED_FOR_OTHER_REASON                               0
#define DELETE_DIRECTORY_SUCCESS                                               999
#define DELETE_DIRECTORY_CAN_NOT_DELETE_ROOT                                   2

#define DIR_GET_FULL_SIZE_FAILED                                               0
#define DIR_GET_FULL_SIZE_SUCCESS                                              999

#define DIR_GET_DISPLAYABLE_ATTRIBUTES_FAILED                                  0
#define DIR_GET_DISPLAYABLE_ATTRIBUTES_SUCCESS                                 999
#define DIR_GET_DISPLAYABLE_ATTRIBUTES_FAILED_GIVEN_DIRECTORY_DO_NOT_EXIST     2

#endif //FAT32_FAT32APIRESPONSECODES_H
