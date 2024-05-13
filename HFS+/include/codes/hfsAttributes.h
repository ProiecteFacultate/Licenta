#ifndef HFS__HFSATTRIBUTES_H
#define HFS__HFSATTRIBUTES_H

#define DIRECTORY_TYPE_FOLDER                                           0
#define DIRECTORY_TYPE_FILE                                             1

#define NODE_IS_LEAF                                                    1     //Don't put 0 for any of them. 0 is default for root node when it is not yet created
#define NODE_IS_NOT_LEAF                                                2

#define WRITE_WITH_TRUNCATE                                             0
#define WRITE_WITH_APPEND                                               1

#endif
