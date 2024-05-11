#ifndef HFS__HFSAPIRESPONSECODES_H
#define HFS__HFSAPIRESPONSECODES_H

#define DIRECTORY_CREATION_PARENT_DO_NOT_EXIST                                                                           0
#define DIRECTORY_CREATION_PARENT_NOT_A_FOLDER                                                                           1
#define DIRECTORY_CREATION_NEW_NAME_ALREADY_EXISTS                                                                       2
#define DIRECTORY_CREATION_PARENT_FAILED_TO_INSERT_RECORD_IN_CATALOG_TREE                                                3
#define DIRECTORY_CREATION_FAILED_FOR_OTHER_REASON                                                                       4
#define DIRECTORY_CREATION_SUCCESS                                                                                       999

#define GET_SUBDIRECTORIES_GIVEN_DIRECTORY_DO_NOT_EXIST                                                                  0
#define GET_SUBDIRECTORIES_GIVEN_DIRECTORY_NOT_A_FOLDER                                                                  1
#define GET_SUBDIRECTORIES_FAILED_FOR_OTHER_REASON                                                                       2
#define GET_SUBDIRECTORIES_SUCCESS                                                                                       999

#endif
