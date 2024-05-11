#ifndef HFS__BTREERESPONSECODES_H
#define HFS__BTREERESPONSECODES_H

#define READ_NODE_FROM_DISK_FAILED                                                                                       0
#define READ_NODE_FROM_DISK_SUCCESS                                                                                      999

#define SEARCH_RECORD_IN_GIVEN_DATA_KEY_DO_NOT_EXIST_IN_TREE                                                             0
#define SEARCH_RECORD_IN_GIVEN_DATA_FAILED_FOR_OTHER_REASON                                                              1
#define SEARCH_RECORD_IN_GIVEN_DATA_SUCCESS                                                                              999

#define INSERT_RECORD_IN_TREE_FAILED                                                                                     0
#define INSERT_RECORD_IN_TREE_SUCCESS                                                                                    999

#define SPLIT_CHILD_FAILED                                                                                               0
#define SPLIT_CHILD_SUCCESS                                                                                              999

#define INSERT_NON_FULL_FAILED                                                                                           0
#define INSERT_NON_FULL_SUCCESS                                                                                          999

#define CREATE_NODE_ON_DISK_FAILED                                                                                       0
#define CREATE_NODE_ON_DISK_SUCCESS                                                                                      999

#define INSERT_RECORD_IN_NODE_FAILED                                                                                     0
#define INSERT_RECORD_IN_NODE_SUCCESS                                                                                    999

#define INSERT_CHILD_NODE_INFO_IN_NODE_FAILED                                                                            0
#define INSERT_CHILD_NODE_INFO_IN_NODE_SUCCESS                                                                           999

#define TRAVERSE_SUBTREE_FAILED                                                                                          0
#define TRAVERSE_SUBTREE_SUCCESS                                                                                         999

#endif
