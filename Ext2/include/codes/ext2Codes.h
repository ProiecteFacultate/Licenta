#ifndef EXT2_EXT2CODES_H
#define EXT2_EXT2CODES_H

#define SEARCH_EMPTY_INODE_IN_GROUP_OTHER_ERROR                                                        0
#define SEARCH_EMPTY_INODE_IN_GROUP_NO_FREE_INODES_IN_GROUP                                            1
#define SEARCH_EMPTY_INODE_IN_GROUP_SUCCESS                                                            2

#define SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_FAILED                                                        0
#define SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_SUCCESS                                                       1

#define GET_GROUP_DESCRIPTOR_FOR_GIVEN_GROUP_FAILED                                                    0
#define GET_GROUP_DESCRIPTOR_FOR_GIVEN_GROUP_SUCCESS                                                   1

#define ADD_INODE_TO_GROUP_FAILED_FOR_OTHER_REASON                                                     0
#define ADD_INODE_TO_GROUP_NO_FREE_INODES_IN_GROUP                                                     1
#define ADD_INODE_TO_GROUP_NO_ENOUGH_FREE_DATA_BLOCKS_IN_GROUP                                         2
#define ADD_INODE_TO_GROUP_SUCCESS                                                                     3

#endif
