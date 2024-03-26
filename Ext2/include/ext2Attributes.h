#ifndef EXT2_EXT2ATTRIBUTES_H
#define EXT2_EXT2ATTRIBUTES_H

#define FILE_TYPE_UNKNOWN                            0                       //from book
#define FILE_TYPE_REGULAR_FILE                       1
#define FILE_TYPE_DIRECTORY                          2
#define FILE_TYPE_CHARACTER_DEVICE                   3
#define FILE_TYPE_BLOCK_DEVICE                       4
#define FILE_TYPE_NAMED_PIPE                         5
#define FILE_TYPE_SOCKET                             6
#define FILE_TYPE_SYMBOLIC_LINK                      7

#define DIRECT_BLOCK                                 0
#define SINGLY_INDIRECT_BLOCK_POINTER                1
#define DOUBLY_INDIRECT_BLOCK_POINTER                2
#define TRIPLY_INDIRECT_BLOCK_POINTER                3

#endif
