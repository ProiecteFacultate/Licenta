set(FIND_DISKEMULATION_PATHS D:/Facultate/Licenta/Implementare/Libraries/DiskEmulation)

find_path(DISKEMULATION_INCLUDE_DIR disk.h
        PATH_SUFFIXES include
        PATHS ${FIND_DISKEMULATION_PATHS})

find_library(DISKEMULATION_LIBRARY
        NAMES disk_emulation
        PATH_SUFFIXES lib
        PATHS ${FIND_DISKEMULATION_PATHS})