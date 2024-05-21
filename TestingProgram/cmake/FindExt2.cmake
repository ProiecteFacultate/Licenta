set(FIND_Ext2_PATHS D:/Facultate/Licenta/Implementare/Libraries/Ext2)

find_path(Ext2_INCLUDE_DIR ext2Init.h
        PATH_SUFFIXES include
        PATHS ${FIND_Ext2_PATHS})

find_library(Ext2_LIBRARY
        NAMES ext2_lib
        PATH_SUFFIXES lib
        PATHS ${FIND_Ext2_PATHS})