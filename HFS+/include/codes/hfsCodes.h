#ifndef HFS__HFSCODES_H
#define HFS__HFSCODES_H

#define INCOMPLETE_BYTES_WRITE_DUE_TO_NO_FREE_BLOCKS                                                                     0
#define INCOMPLETE_BYTES_WRITE_DUE_TO_UNKNOWN                                                                            1

#define INCOMPLETE_BYTES_READ_DUE_TO_NO_FILE_NOT_LONG_ENOUGH                                                             0
#define INCOMPLETE_BYTES_READ_DUE_TO_UNKNOWN                                                                             1

#define SEARCH_FREE_EXTENT_NO_EXTENT_WITH_DESIRED_NUMBER_OF_BLOCKS                                                       0
#define SEARCH_FREE_EXTENT_FAILED_FOR_OTHER_REASON                                                                       1
#define SEARCH_FREE_EXTENT_SUCCESS                                                                                       999

#define CHANGE_BLOCK_ALLOCATION_FAILED                                                                                   0
#define CHANGE_BLOCK_ALLOCATION_SUCCESS                                                                                  999

#define WRITE_DATA_TO_EXTENT_FAILED                                                                                      0
#define WRITE_DATA_TO_EXTENT_SUCCESS                                                                                     999

#define SET_EXTENTS_FOR_DIRECTORY_RECORD_FAILED                                                                          0
#define SET_EXTENTS_FOR_DIRECTORY_RECORD_SUCCESS                                                                         999

#define GET_ALL_EXTENTS_FOR_DIRECTORY_RECORD_FAILED                                                                      0
#define GET_ALL_EXTENTS_FOR_DIRECTORY_RECORD_SUCCESS                                                                     999

#define DELETE_DIRECTORY_RECORD_AND_RELATED_DATA_FAILED                                                                  0
#define DELETE_DIRECTORY_RECORD_AND_RELATED_DATA_SUCCESS                                                                 999

#define DELETE_DIRECTORY_AND_CHILDREN_FAILED                                                                             0
#define DELETE_DIRECTORY_AND_CHILDREN_SUCCESS                                                                            999

#endif
