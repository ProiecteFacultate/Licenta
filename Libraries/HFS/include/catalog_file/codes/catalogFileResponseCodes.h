#ifndef HFS__CATALOGFILERESPONSECODES_H
#define HFS__CATALOGFILERESPONSECODES_H

#define CF_SEARCH_RECORD_BY_FULL_PATH_PARENT_DO_NOT_EXIST                                                                 0
#define CF_SEARCH_RECORD_BY_FULL_PATH_FAILED_FOR_OTHER_REASON                                                             1
#define CF_SEARCH_RECORD_BY_FULL_PATH_SUCCESS                                                                             999

#define CF_SEARCH_DIRECTORY_RECORD_BY_DIRECTORY_NAME_BEING_GIVEN_PARENT_RECORD_KEY_DO_NOT_EXIT_IN_TREE                    0
#define CF_SEARCH_DIRECTORY_RECORD_BY_DIRECTORY_NAME_BEING_GIVEN_PARENT_RECORD_FAILED_FOR_OTHER_REASON                    1
#define CF_SEARCH_DIRECTORY_RECORD_BY_DIRECTORY_NAME_BEING_GIVEN_PARENT_RECORD_SUCCESS                                    999

#define CF_KEY_1_HIGHER                                                                                                   1
#define CF_KEYS_EQUAL                                                                                                     0
#define CF_KEY_2_HIGHER                                                                                                   (-1)

#define CF_CREATE_DIRECTORY_RECORD_SUCCESS                                                                                999

#define CF_UPDATE_RECORD_ON_DISK_FAILED                                                                                   0
#define CF_UPDATE_RECORD_ON_DISK_SUCCESS                                                                                  999

#endif