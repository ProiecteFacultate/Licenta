cmake_minimum_required(VERSION 3.25)
project(Ext2)

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

add_executable(Ext2 src/main.cpp include/structures.h include/ext2Init.h src/ext2Init.cpp include/ext2FunctionUtils.h src/ext2FunctionUtils.cpp include/codes/ext2Attributes.h include/ext2.h src/ext2.cpp include/codes/ext2Codes.h include/ext2Api.h src/ext2Api.cpp
        include/codes/ext2ApiResponseCodes.h
        include/ext2Heuristics.h
        src/ext2Heuristics.cpp
        include/utils.h
        src/utils.cpp
        include/ext2RootInit.h
        src/ext2RootInit.cpp
        include/interface.h
        src/interface.cpp
        include/ext2BlocksAllocation.h
        src/ext2BlocksAllocation.cpp
        include/codes/ext2BlocksAllocationCodes.h
        include/ext2TestApi.h
        src/ext2TestApi.cpp)

include(FindPkgConfig)
find_package(DiskEmulation REQUIRED)
include_directories(${DISKEMULATION_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${DISKEMULATION_LIBRARY})

#to export this as a library
set(SOURCE_FILES include/structures.h include/ext2Init.h src/ext2Init.cpp include/ext2FunctionUtils.h src/ext2FunctionUtils.cpp include/codes/ext2Attributes.h include/ext2.h src/ext2.cpp include/codes/ext2Codes.h include/ext2Api.h src/ext2Api.cpp
        include/codes/ext2ApiResponseCodes.h
        include/ext2Heuristics.h
        src/ext2Heuristics.cpp
        include/utils.h
        src/utils.cpp
        include/ext2RootInit.h
        src/ext2RootInit.cpp
        include/interface.h
        src/interface.cpp
        include/ext2BlocksAllocation.h
        src/ext2BlocksAllocation.cpp
        include/codes/ext2BlocksAllocationCodes.h
        include/ext2TestApi.h
        src/ext2TestApi.cpp
)

add_library(ext2_lib ${SOURCE_FILES})