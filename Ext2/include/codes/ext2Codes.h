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

#define FIND_INODE_BY_PATH_FAILED                                                                      0
#define FIND_INODE_BY_PATH_INVALID_PARENT_NAME                                                         1
#define FIND_INODE_BY_PATH_SUCCESS                                                                     2

#define INODE_FOUND                                                                                    1

#define INODE_SEARCH_FAILED                                                                            0
#define INODE_SEARCH_DO_NOT_EXIST                                                                      1
#define INODE_SEARCH_SUCCESS                                                                           2

#define DIRECTORY_ENTRY_SEARCH_BY_NAME_SEARCH_FOUND                                                    0
#define DIRECTORY_ENTRY_SEARCH_BY_NAME_SEARCH_NOT_FOUND                                                1

#define GET_DATA_BLOCK_BY_LOCAL_INDEX_FAILED                                                           0
#define GET_DATA_BLOCK_BY_LOCAL_INDEX_SUCCESS                                                          1

#endif
