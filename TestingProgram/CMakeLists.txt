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
        include/time_tests/fat32TimeTests.h
        src/time_tests/fat32TimeTests.cpp
        include/time_tests/ext2TimeTests.h
        src/time_tests/ext2TimeTests.cpp
        include/time_tests/hfsTimeTests.h
        src/time_tests/hfsTimeTests.cpp
        include/space_tests/ext2SpaceTests.h
        include/space_tests/fat32SpaceTests.h
        include/space_tests/hfsSpaceTests.h
        src/space_tests/fat32SpaceTests.cpp
        src/space_tests/ext2SpaceTests.cpp
        src/space_tests/hfsSpaceTests.cpp)

include(FindPkgConfig)

find_package(FAT32 REQUIRED)
include_directories(${FAT32_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${FAT32_LIBRARY})

find_package(EXT2 REQUIRED)
include_directories(${EXT2_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${EXT2_LIBRARY})

find_package(HFS REQUIRED)
include_directories(${HFS_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${HFS_LIBRARY})

find_package(DiskEmulation REQUIRED)
include_directories(${DISKEMULATION_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${DISKEMULATION_LIBRARY})