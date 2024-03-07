#ifndef FAT32_FAT32APIRESPONSECODES_H
#define FAT32_FAT32APIRESPONSECODES_H

#define DIR_CREATION_FAILED                              0
#define DIR_CREATION_SUCCESS                             1
#define DIR_CREATION_INVALID_DIRNAME                     2
#define DIR_CREATION_NEW_NAME_ALREADY_EXISTS             3                                               //a directory with given name for the new dir already exists in parent
#define DIR_CREATION_NO_CLUSTER_AVAILABLE                4

#define GET_SUB_DIRECTORIES_FAILED                       0
#define GET_SUB_DIRECTORIES_SUCCESS                      1

#endif //FAT32_FAT32APIRESPONSECODES_H
