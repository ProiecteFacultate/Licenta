set(FIND_HFS_PATHS D:/Facultate/Licenta/Implementare/Libraries/HFS)

find_path(HFS_INCLUDE_DIR hfsInit.h
        PATH_SUFFIXES include
        PATHS ${FIND_HFS_PATHS})

find_library(HFS_LIBRARY
        NAMES hfs_lib
        PATH_SUFFIXES lib
        PATHS ${FIND_HFS_PATHS})