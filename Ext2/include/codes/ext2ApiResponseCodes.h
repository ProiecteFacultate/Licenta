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

#endif
