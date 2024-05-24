set(FIND_EXT2_PATHS D:/Facultate/Licenta/Implementare/Libraries/EXT2)

find_path(EXT2_INCLUDE_DIR ext2Init.h
        PATH_SUFFIXES include
        PATHS ${FIND_EXT2_PATHS})

find_library(EXT2_LIBRARY
        NAMES ext2_lib
        PATH_SUFFIXES lib
        PATHS ${FIND_EXT2_PATHS})