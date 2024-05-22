cmake_minimum_required(VERSION 3.25)
project(DiskEmulationLib)

set(CMAKE_CXX_STANDARD 17)

add_executable(DiskEmulationLib src/main.cpp include/disk.h src/disk.cpp include/diskCodes.h include/diskUtils.h src/diskUtils.cpp)

#to export this as a library
set(SOURCE_FILES include/disk.h src/disk.cpp include/diskUtils.h src/diskUtils.cpp include/diskCodes.h)
add_library(disk_emulation ${SOURCE_FILES})