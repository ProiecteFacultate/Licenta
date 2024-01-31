cmake_minimum_required(VERSION 3.25)
project(FAT32)

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

add_executable(FAT32 src/main.cpp include/fat32Init.h src/fat32Init.cpp include/fat32.h src/fat32.cpp include/fat32MethodUtils.h src/fat32MethodUtils.cpp)

include(FindPkgConfig)
find_package(DiskEmulation REQUIRED)
include_directories(${DISKEMULATION_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${DISKEMULATION_LIBRARY})




