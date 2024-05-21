#ifndef HFS__EXTENTS_BTREERESPONSECODES_H
#define HFS__EXTENTS_BTREERESPONSECODES_H

#define EOF_READ_NODE_FROM_DISK_FAILED                                                                                       0
#define EOF_READ_NODE_FROM_DISK_SUCCESS                                                                                      999

#define EOF_SEARCH_RECORD_IN_GIVEN_DATA_KEY_DO_NOT_EXIST_IN_TREE                                                             0
#define EOF_SEARCH_RECORD_IN_GIVEN_DATA_FAILED_FOR_OTHER_REASON                                                              1
#define EOF_SEARCH_RECORD_IN_GIVEN_DATA_SUCCESS                                                                              999

#define EOF_INSERT_RECORD_IN_TREE_FAILED                                                                                     0
#define EOF_INSERT_RECORD_IN_TREE_SUCCESS                                                                                    999

#define EOF_REMOVE_RECORD_FROM_TREE_FAILED                                                                                    0
#define EOF_REMOVE_RECORD_FROM_TREE_SUCCESS                                                                                   999

#define EOF_SPLIT_CHILD_FAILED                                                                                               0
#define EOF_SPLIT_CHILD_SUCCESS                                                                                              999

#define EOF_INSERT_NON_FULL_FAILED                                                                                           0
#define EOF_INSERT_NON_FULL_SUCCESS                                                                                          999

#define EOF_CREATE_NODE_ON_DISK_FAILED                                                                                       0
#define EOF_CREATE_NODE_ON_DISK_SUCCESS                                                                                      999

#define EOF_INSERT_RECORD_IN_NODE_FAILED                                                                                     0
#define EOF_INSERT_RECORD_IN_NODE_SUCCESS                                                                                    999

#define EOF_INSERT_CHILD_NODE_INFO_IN_NODE_FAILED                                                                            0
#define EOF_INSERT_CHILD_NODE_INFO_IN_NODE_SUCCESS                                                                           999

#define EOF_TRAVERSE_SUBTREE_FAILED                                                                                          0
#define EOF_TRAVERSE_SUBTREE_SUCCESS                                                                                         999

#define EOF_FIND_KEY_FAILED                                                                                                   0
#define EOF_FIND_KEY_SUCCESS                                                                                                  999

#define EOF_REMOVE_FAILED                                                                                                     0
#define EOF_REMOVE_SUCCESS                                                                                                    999

#define EOF_REMOVE_FROM_LEAF_FAILED                                                                                           0
#define EOF_REMOVE_FROM_LEAF_SUCCESS                                                                                          999

#define EOF_REMOVE_FROM_NON_LEAF_FAILED                                                                                       0
#define EOF_REMOVE_FROM_NON_LEAF_SUCCESS                                                                                      999

#define EOF_GET_PRED_FAILED                                                                                                   0
#define EOF_GET_PRED_SUCCESS                                                                                                  999

#define EOF_GET_SUCC_FAILED                                                                                                   0
#define EOF_GET_SUCC_SUCCESS                                                                                                  999

#define EOF_FILL_FAILED                                                                                                       0
#define EOF_FILL_SUCCESS                                                                                                      999

#define EOF_BORROW_FROM_PREV_FAILED                                                                                           0
#define EOF_BORROW_FROM_PREV_SUCCESS                                                                                          999

#define EOF_BORROW_FROM_NEXT_FAILED                                                                                           0
#define EOF_BORROW_FROM_NEXT_SUCCESS                                                                                          999

#define EOF_MERGE_FAILED                                                                                                      0
#define EOF_MERGE_SUCCESS                                                                                                     999

#endif
