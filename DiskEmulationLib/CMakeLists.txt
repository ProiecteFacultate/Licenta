cmake_minimum_required(VERSION 3.25)
project(DiskEmulationLib)

set(CMAKE_CXX_STANDARD 17)

add_executable(DiskEmulationLib src/main.cpp src/diskInitialization.cpp)