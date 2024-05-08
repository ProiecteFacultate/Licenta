cmake_minimum_required(VERSION 3.28)
project(HFS_)

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

add_executable(HFS_ src/main.cpp
        include/structures.h
        include/hfsInit.h
        src/hsfInit.cpp
        include/hfsFunctionUtils.h
        src/hfsFunctionUtils.cpp
        include/utils.h
        src/utils.cpp
        include/hfsApi.h
        src/hfsApi.cpp
        include/hfs.h
        src/hfs.cpp
        include/codes/hfsAttributes.h
        include/bTreeCatalog.h
        src/bTreeCatalog.cpp
        include/codes/bTreeResponseCodes.h)

include(FindPkgConfig)
find_package(DiskEmulation REQUIRED)
include_directories(${DISKEMULATION_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${DISKEMULATION_LIBRARY})
