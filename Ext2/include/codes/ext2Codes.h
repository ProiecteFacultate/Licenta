#ifndef EXT2_EXT2CODES_H
#define EXT2_EXT2CODES_H

#define SEARCH_EMPTY_INODE_IN_GROUP_FAILED                                                             0
#define SEARCH_EMPTY_INODE_IN_GROUP_SUCCESS                                                            1

#define SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_FAILED                                                        0
#define SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_SUCCESS                                                       1

#define GET_GROUP_DESCRIPTOR_FOR_GIVEN_GROUP_FAILED                                                    0
#define GET_GROUP_DESCRIPTOR_FOR_GIVEN_GROUP_SUCCESS                                                   1

#define ADD_INODE_TO_GROUP_FAILED_FOR_OTHER_REASON                                                     0
#define ADD_INODE_TO_GROUP_NO_FREE_INODES_IN_GROUP                                                     1
#define ADD_INODE_TO_GROUP_NO_ENOUGH_FREE_DATA_BLOCKS_IN_GROUP                                         2
#define ADD_INODE_TO_GROUP_SUCCESS                                                                     3

#define UPDATE_MAIN_GROUP_DESCRIPTOR_FAILED                                                            0
#define UPDATE_MAIN_GROUP_DESCRIPTOR_SUCCESS                                                           1

#define SEARCH_INODE_BY_FULL_PATH_PARENT_DO_NOT_EXIST                                                  0
#define SEARCH_INODE_BY_FULL_PATH_FAILED                                                               1
#define SEARCH_INODE_BY_FULL_PATH_SUCCESS                                                              2

#define SEARCH_INODE_BY_DIRECTORY_NAME_IN_PARENT_FAILED                                                0
#define SEARCH_INODE_BY_DIRECTORY_NAME_IN_PARENT_INODE_DO_NOT_EXIST                                    1
#define SEARCH_INODE_BY_DIRECTORY_NAME_IN_PARENT_SUCCESS                                               2

#define SEARCH_DIRECTORY_ENTRY_BY_NAME_IN_GIVEN_BLOCK_DATA_FOUND                                       0
#define SEARCH_DIRECTORY_ENTRY_BY_NAME_IN_GIVEN_BLOCK_DATA_NOT_FOUND                                   1

#define GET_DATA_BLOCK_BY_LOCAL_INDEX_FAILED                                                           0
#define GET_DATA_BLOCK_BY_LOCAL_INDEX_SUCCESS                                                          1

#define GET_INODE_BY_INODE_GLOBAL_INDEX_FAILED                                                         0
#define GET_INODE_BY_INODE_GLOBAL_INDEX_SUCCESS                                                        1

#endif
