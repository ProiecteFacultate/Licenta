#ifndef FAT32_FAT32ATTRIBUTES_H
#define FAT32_FAT32ATTRIBUTES_H

#define ATTR_READ_ONLY                     1
#define ATTR_HIDDEN                        2
#define ATTR_SYSTEM                        4
#define ATTR_VOLUME_ID                     8
#define ATTR_FOLDER                        16
#define ATTR_ARCHIVE                       32
#define ATTR_FILE                          100                                                           //This is not on official documentation, but was added by me

#define WRITE_WITH_TRUNCATE                1
#define WRITE_WITH_APPEND                  2

#endif
