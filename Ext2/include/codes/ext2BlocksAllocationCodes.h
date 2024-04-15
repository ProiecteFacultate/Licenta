#ifndef EXT2_EXT2BLOCKSALLOCATIONCODES_H
#define EXT2_EXT2BLOCKSALLOCATIONCODES_H

#define ADD_BLOCK_TO_DIRECTORY_NO_FREE_BLOCKS                                              0
#define ADD_BLOCK_TO_DIRECTORY_FAILED_FOR_OTHER_REASON                                     1
#define ADD_BLOCK_TO_DIRECTORY_SUCCESS                                                     9999

#define ADD_HIGHER_ORDER_DATA_BLOCK_NO_FREE_BLOCKS                                         0
#define ADD_HIGHER_ORDER_DATA_FAILED_FOR_OTHER_REASON                                      1
#define ADD_HIGHER_ORDER_DATA_SUCCESS                                                      9999

#define ADD_BLOCK_INDEX_TO_LOWER_ORDER_BLOCK_FAILED                                        0
#define ADD_BLOCK_INDEX_TO_LOWER_ORDER_BLOCK_SUCCESS                                       9999

#define UPDATE_BLOCK_ALLOCATION_FAILED                                                     0
#define UPDATE_BLOCK_ALLOCATION_SUCCESS                                                    9999

#define CHECK_AND_DEALLOCATE_HIGHER_ORDER_TABLE_BLOCK_FAILED                               0
#define CHECK_AND_DEALLOCATE_HIGHER_ORDER_TABLE_BLOCK_SUCCESS                              9999

#define DEALLOCATE_LAST_BLOCK_IN_DIRECTORY_FAILED                                          0
#define DEALLOCATE_LAST_BLOCK_IN_DIRECTORY_SUCCESS                                         9999

#endif