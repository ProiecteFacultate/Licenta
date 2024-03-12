#ifndef FAT32_FAT32CODES_H
#define FAT32_FAT32CODES_H

#define FAT_VALUE_FREE                                                    0x00000000
#define FAT_VALUE_RESERVED_1                                              0x00000001                               //Reserved for internal purposes
#define FAT_VALUE_USED                                                    0x00000002                               //CAUTION this is a not assignable value!!!!!!!!
#define FAT_VALUE_RESERVED_2                                              0x0FFFFFF0                               //Reserved in some contexts
#define FAT_VALUE_RESERVED_3                                              0x0FFFFFF6                               //Reserved; do not use
#define FAT_VALUE_BAD_SECTOR                                              0x0FFFFFF7
#define FAT_VALUE_EOC                                                     0x0FFFFFFF

#define DIR_ENTRY_NO_MORE_ENTRIES                                         0
#define DIR_ENTRY_FOUND                                                   1
#define DIR_ENTRY_NOT_FOUND_IN_CLUSTER                                    2
#define DIR_ENTRY_DO_NOT_EXIST                                            3
#define DIR_ENTRY_SEARCH_ERROR                                            4

#define CLUSTER_SEARCH_FAILED                                             0
#define CLUSTER_SEARCH_SUCCESS                                            1
#define CLUSTER_SEARCH_NO_FREE_CLUSTERS                                   2

#define FAT_UPDATE_FAILED                                                 0
#define FAT_UPDATE_SUCCESS                                                1

#define DIR_ENTRY_ADD_FAILED                                              0
#define DIR_ENTRY_ADD_SUCCESS                                             1
#define DIR_ENTRY_ADD_NO_CLUSTER_AVAILABLE                                2

#define DIR_SETUP_FIRST_CLUSTER_FAILED                                    0
#define DIR_SETUP_FIRST_CLUSTER_SUCCESS                                   1

#define FAT_VALUE_RETRIEVE_FAILED                                         (-1)

#define DIR_ADD_NEW_CLUSTER_FAILED                                        0
#define DIR_ADD_NEW_CLUSTER_SUCCESS                                       1
#define DIR_ADD_NEW_CLUSTER_NO_CLUSTER_AVAILABLE                          2

#define CLUSTER_SEARCH_IN_CHAN_FAILED                                     0
#define CLUSTER_SEARCH_IN_CHAN_SUCCESS                                    1
#define CLUSTER_SEARCH_IN_CHAN_EOC                                        2                                         //if we reach eoc, before reaching our cluster in chain

#define DIRECTORY_ENTRY_UPDATE_FAILED                                     0
#define DIRECTORY_ENTRY_UPDATE_SUCCESS                                    1

#define DIRECTORY_UPDATE_SUBDIRECTORIES_DOT_DOT_FAILED                    0
#define DIRECTORY_UPDATE_SUBDIRECTORIES_DOT_DOT_SUCCESS                   1

#define FIND_DIRECTORY_ENTRY_BY_PATH_FAILED                               0
#define FIND_DIRECTORY_ENTRY_BY_PATH_SUCCESS                              1

#define WRITE_BYTES_TO_FILE_FAILED                                        0
#define WRITE_BYTES_TO_FILE_SUCCESS                                       1


#define WRITE_BYTES_TO_FILE_FAILED                                        0
#define WRITE_BYTES_TO_FILE_SUCCESS                                       1

#define INCOMPLETE_BYTES_WRITE_DUE_TO_NO_CLUSTERS_AVAILABLE               0                              //we want to specifically know about this case
#define INCOMPLETE_BYTES_WRITE_DUE_TO_OTHER                               1



#define FREE_CLUSTERS_IN_CHAIN_FAILED                                     0
#define FREE_CLUSTERS_IN_CHAIN_SUCCESS                                    1

#endif
