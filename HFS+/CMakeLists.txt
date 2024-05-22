cmake_minimum_required(VERSION 3.25)
project(HFS_)

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

add_executable(HFS_ src/main.cpp
        include/hfsStructures.h
        include/hfsInit.h
        src/hsfInit.cpp
        include/hfsFunctionUtils.h
        src/hfsFunctionUtils.cpp
        include/utils.h
        src/utils.cpp
        include/hfsApi.h
        src/hfsApi.cpp
        include/codes/hfsAttributes.h
        include/catalog_file/bTreeCatalog.h
        src/catalog_file/bTreeCatalog.cpp
        include/catalog_file/codes/bTreeResponseCodes.h
        include/catalog_file/catalogFileOperations.h
        src/catalog_file/catalogFileOperations.cpp
        include/catalog_file/catalogFileUtils.h
        src/catalog_file/catalogFileUtils.cpp
        include/catalog_file/codes/catalogFileResponseCodes.h
        include/codes/hfsApiResponseCodes.h
        src/hfs.cpp
        include/hfs.h
        include/interface.h
        src/interface.cpp
        include/codes/hfsCodes.h
        include/extents_file/codes/bTreeResponseCodes.h
        include/extents_file/codes/extentsFileResponseCodes.h
        include/extents_file/bTreeCatalog.h
        include/extents_file/extentsFileOperations.h
        include/extents_file/extentsFileUtils.h
        src/extents_file/bTreeCatalog.cpp
        src/extents_file/extentsFileOperations.cpp
        src/extents_file/extentsFileUtils.cpp
        include/hfsTestApi.h
        src/hfsTestApi.cpp
)

include(FindPkgConfig)
find_package(DiskEmulation REQUIRED)
include_directories(${DISKEMULATION_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${DISKEMULATION_LIBRARY})

#to export this as a library
set(SOURCE_FILES src/main.cpp
        include/hfsStructures.h
        include/hfsInit.h
        src/hsfInit.cpp
        include/hfsFunctionUtils.h
        src/hfsFunctionUtils.cpp
        include/utils.h
        src/utils.cpp
        include/hfsApi.h
        src/hfsApi.cpp
        include/codes/hfsAttributes.h
        include/catalog_file/bTreeCatalog.h
        src/catalog_file/bTreeCatalog.cpp
        include/catalog_file/codes/bTreeResponseCodes.h
        include/catalog_file/catalogFileOperations.h
        src/catalog_file/catalogFileOperations.cpp
        include/catalog_file/catalogFileUtils.h
        src/catalog_file/catalogFileUtils.cpp
        include/catalog_file/codes/catalogFileResponseCodes.h
        include/codes/hfsApiResponseCodes.h
        src/hfs.cpp
        include/hfs.h
        include/interface.h
        src/interface.cpp
        include/codes/hfsCodes.h
        include/extents_file/codes/bTreeResponseCodes.h
        include/extents_file/codes/extentsFileResponseCodes.h
        include/extents_file/bTreeCatalog.h
        include/extents_file/extentsFileOperations.h
        include/extents_file/extentsFileUtils.h
        src/extents_file/bTreeCatalog.cpp
        src/extents_file/extentsFileOperations.cpp
        src/extents_file/extentsFileUtils.cpp
        include/hfsTestApi.h
        src/hfsTestApi.cpp)
add_library(hfs_lib ${SOURCE_FILES})

