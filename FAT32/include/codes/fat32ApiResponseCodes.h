#ifndef FAT32_FAT32APIRESPONSECODES_H
#define FAT32_FAT32APIRESPONSECODES_H

#define DIR_CREATION_FAILED                              0
#define DIR_CREATION_SUCCESS                             1
#define DIR_CREATION_INVALID_DIRNAME                     2
#define DIR_CREATION_PARENT_DO_NOT_EXIST                 3
#define DIR_CREATION_NEW_NAME_ALREADY_EXISTS             4                                               //a directory with given name for the new dir already exists in parent
#define DIR_CREATION_NO_CLUSTER_AVAILABLE                5
#define DIR_CREATION_PARENT_NOT_A_DIRECTORY              6

#define GET_SUB_DIRECTORIES_FAILED                       0
#define GET_SUB_DIRECTORIES_SUCCESS                      1

#define WRITE_BYTES_TO_FILE_FAILED                       0
#define WRITE_BYTES_TO_FILE_SUCCESS                      1
#define WRITE_BYTES_TO_FILE_CAN_NOT_WRITE_GIVEN_FILE     2                                               //access problems for example is read only, or is a directory(folder)

#define READ_BYTES_FROM_FILE_FAILED                      0
#define READ_BYTES_FROM_FILE_SUCCESS                     1
#define READ_BYTES_FROM_FILE_CAN_NOT_READ_GIVEN_FILE     2

#endif //FAT32_FAT32APIRESPONSECODES_H
