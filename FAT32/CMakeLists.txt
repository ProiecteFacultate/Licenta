cmake_minimum_required(VERSION 3.25)
project(FAT32)

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

add_executable(FAT32 src/main.cpp include/fat32Init.h src/fat32Init.cpp include/fat32.h src/fat32.cpp include/utils.h src/utils.cpp include/codes/fat32Codes.h include/fat32Attributes.h include/fat32Api.h src/fat32Api.cpp include/fat32FunctionUtils.h src/fat32FunctionUtils.cpp include/codes/fat32ApiResponseCodes.h include/interface.h src/interface.cpp include/structures.h)

include(FindPkgConfig)
find_package(DiskEmulation REQUIRED)
include_directories(${DISKEMULATION_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${DISKEMULATION_LIBRARY})

#to export this as a library
set(SOURCE_FILES include/fat32Init.h src/fat32Init.cpp include/fat32.h src/fat32.cpp include/utils.h src/utils.cpp include/codes/fat32Codes.h include/fat32Attributes.h include/fat32Api.h src/fat32Api.cpp include/fat32FunctionUtils.h src/fat32FunctionUtils.cpp include/codes/fat32ApiResponseCodes.h include/interface.h src/interface.cpp include/structures.h)
add_library(fat32_lib ${SOURCE_FILES})


