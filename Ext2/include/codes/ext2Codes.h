#ifndef EXT2_EXT2CODES_H
#define EXT2_EXT2CODES_H

#define SEARCH_EMPTY_INODE_IN_GROUP_FAILED                                                             0 //TO DEPRECATE
#define SEARCH_EMPTY_INODE_IN_GROUP_SUCCESS                                                            1 //TO DEPRECATE

#define SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_NO_FREE_BLOCKS                                                0
#define SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_FAILED_FOR_OTHER_REASON                                       1
#define SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_SUCCESS                                                       2

#define SEARCH_EMPTY_DATA_BLOCK_NO_FREE_BLOCKS                                                         0
#define SEARCH_EMPTY_DATA_BLOCK_FAILED_FOR_OTHER_REASON                                                1
#define SEARCH_EMPTY_DATA_BLOCK_SUCCESS                                                                2

#define SEARCH_MULTIPLE_DATA_BLOCKS_IN_GROUP_NO_ENOUGH_CONSECUTIVE_FREE_BLOCKS                         0
#define SEARCH_MULTIPLE_DATA_BLOCKS_IN_GROUP_FAILED_FOR_OTHER_REASON                                   1
#define SEARCH_MULTIPLE_DATA_BLOCKS_IN_GROUP_SUCCESS                                                   2

#define SEARCH_MULTIPLE_DATA_BLOCKS_NO_ENOUGH_CONSECUTIVE_FREE_BLOCKS                                  0
#define SEARCH_MULTIPLE_DATA_BLOCKS_FAILED_FOR_OTHER_REASON                                            1
#define SEARCH_MULTIPLE_DATA_BLOCKS_SUCCESS                                                            2

#define GET_GROUP_DESCRIPTOR_FOR_GIVEN_GROUP_FAILED                                                    0
#define GET_GROUP_DESCRIPTOR_FOR_GIVEN_GROUP_SUCCESS                                                   1

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

#define SEARCH_FREE_INODE_NO_FREE_INODES                                                               0
#define SEARCH_FREE_INODE_FAILED_FOR_OTHER_REASON                                                      1
#define SEARCH_FREE_INODE_SUCCESS                                                                      2

#define CREATE_INODE_FAILED_NO_FREE_INODES                                                             0
#define CREATE_INODE_FAILED_FOR_OTHER_REASON                                                           1
#define CREATE_INODE_SUCCESS                                                                           2

#define GET_GROUPS_FREE_INODES_AND_GROUPS_FAILED                                                       0
#define GET_GROUPS_FREE_INODES_AND_GROUPS_SUCCESS                                                      1

#define CALCULATE_GROUP_DEBT_FAILED                                                                    0
#define CALCULATE_GROUP_DEBT_SUCCESS                                                                   1

#define GET_NUMBER_OF_OCCUPIED_INODE_IN_BLOCK_FAILED                                                   0
#define GET_NUMBER_OF_OCCUPIED_INODE_IN_BLOCK_SUCCESS                                                  1

#define SEARCH_FIRST_EMPTY_INODE_IN_GROUP_NO_FREE_INODE_IN_GROUP                                       0
#define SEARCH_FIRST_EMPTY_INODE_IN_GROUP_FAILED_FOR_OTHER_REASON                                      1
#define SEARCH_FIRST_EMPTY_INODE_IN_GROUP_SUCCESS                                                      2

#define UPDATE_GROUP_DESCRIPTOR_FAILED                                                                 0
#define UPDATE_GROUP_DESCRIPTOR_SUCCESS                                                                1

#define UPDATE_VALUE_IN_INODE_BITMAP_FAILED                                                            0
#define UPDATE_VALUE_IN_INODE_BITMAP_SUCCESS                                                           1

#define UPDATE_VALUE_IN_DATA_BLOCK_BITMAP_FAILED                                                       0
#define UPDATE_VALUE_IN_DATA_BLOCK_BITMAP_SUCCESS                                                      1

#define ADD_INODE_TO_INODE_TABLE_FAILED                                                                0
#define ADD_INODE_TO_INODE_TABLE_SUCCESS                                                               1

#define ADD_BLOCK_TO_DIRECTORY_FAILED                                                                  0
#define ADD_BLOCK_TO_DIRECTORY_SUCCESS                                                                 1

#define ADD_DIRECTORY_ENTRY_TO_PARENT_FAILED                                                           0
#define ADD_DIRECTORY_ENTRY_TO_PARENT_SUCCESS                                                          1

#define UPDATE_INODE_FAILED                                                                            0
#define UPDATE_INODE_SUCCESS                                                                           1

#endif
