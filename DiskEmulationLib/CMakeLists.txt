cmake_minimum_required(VERSION 3.25)
project(DiskEmulationLib)

set(CMAKE_CXX_STANDARD 17)

add_executable(DiskEmulationLib src/main.cpp src/disk.cpp include/diskCodes.h include/diskUtils.h src/diskUtils.cpp)

set(SOURCE_FILES src/disk.cpp src/diskUtils.cpp include/diskCodes.h)
add_library(disk_emulation ${SOURCE_FILES})