set(FIND_FAT32_PATHS D:/Facultate/Licenta/Implementare/Libraries/FAT32)

find_path(FAT32_INCLUDE_DIR fat32Init.h
        PATH_SUFFIXES include
        PATHS ${FIND_FAT32_PATHS})

find_library(FAT32_LIBRARY
        NAMES fat32_lib
        PATH_SUFFIXES lib
        PATHS ${FIND_FAT32_PATHS})