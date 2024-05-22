cmake_minimum_required(VERSION 3.25)
project(TestingProgram)

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

add_executable(TestingProgram main.cpp
        include/utils.h
        src/utils.cpp
        include/commonTests.h
        src/commonTests.cpp
        include/attributes.h
        include/fat32Tests.h
        src/fat32Tests.cpp
        include/ext2Tests.h
        src/ext2Tests.cpp
        include/hfsTests.h
        src/hfsTests.cpp)

include(FindPkgConfig)

find_package(FAT32 REQUIRED)
include_directories(${FAT32_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${FAT32_LIBRARY})

find_package(Ext2 REQUIRED)
include_directories(${Ext2_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${Ext2_LIBRARY})

find_package(HFS REQUIRED)
include_directories(${HFS_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${HFS_LIBRARY})

find_package(DiskEmulation REQUIRED)
include_directories(${DISKEMULATION_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${DISKEMULATION_LIBRARY})