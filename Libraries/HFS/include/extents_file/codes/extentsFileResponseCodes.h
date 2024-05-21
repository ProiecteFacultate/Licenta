#ifndef HFS__EXTENTSFILERESPONSECODES_H
#define HFS__EXTENTSFILERESPONSECODES_H

#define EOF_SEARCH_RECORD_BY_FULL_PATH_PARENT_DO_NOT_EXIST                                                                 0
#define EOF_SEARCH_RECORD_BY_FULL_PATH_FAILED_FOR_OTHER_REASON                                                             1
#define EOF_SEARCH_RECORD_BY_FULL_PATH_SUCCESS                                                                             999

#define EOF_SEARCH_DIRECTORY_RECORD_BY_DIRECTORY_NAME_BEING_GIVEN_PARENT_RECORD_KEY_DO_NOT_EXIT_IN_TREE                    0
#define EOF_SEARCH_DIRECTORY_RECORD_BY_DIRECTORY_NAME_BEING_GIVEN_PARENT_RECORD_FAILED_FOR_OTHER_REASON                    1
#define EOF_SEARCH_DIRECTORY_RECORD_BY_DIRECTORY_NAME_BEING_GIVEN_PARENT_RECORD_SUCCESS                                    999

#define EOF_KEY_1_HIGHER                                                                                                   1
#define EOF_KEYS_EQUAL                                                                                                     0
#define EOF_KEY_2_HIGHER                                                                                                   (-1)

#define EOF_CREATE_DIRECTORY_RECORD_SUCCESS                                                                                999

#define EOF_UPDATE_RECORD_ON_DISK_FAILED                                                                                   0
#define EOF_UPDATE_RECORD_ON_DISK_SUCCESS                                                                                  999

#endif
