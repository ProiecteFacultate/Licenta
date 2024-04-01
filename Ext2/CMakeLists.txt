cmake_minimum_required(VERSION 3.25)
project(Ext2)

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

add_executable(Ext2 src/main.cpp include/structures.h include/ext2Init.h src/ext2Init.cpp include/ext2FunctionUtils.h src/ext2FunctionUtils.cpp include/codes/ext2Attributes.h include/ext2.h src/ext2.cpp include/codes/ext2Codes.h include/ext2Api.h src/ext2Api.cpp
        include/codes/ext2ApiResponseCodes.h
        include/ext2Heuristics.h
        src/ext2Heuristics.cpp
        include/ext2Utils.h
        src/ext2Utils.cpp
        include/ext2RootInit.h
        src/ext2RootInit.cpp)

include(FindPkgConfig)
find_package(DiskEmulation REQUIRED)
include_directories(${DISKEMULATION_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${DISKEMULATION_LIBRARY})