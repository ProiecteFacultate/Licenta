#include "vector"
#include "string"
#include "chrono"

#include "../include/ext2Api.h"
#include "../include/ext2Structures.h"
#include "../include/ext2TestApi.h"

uint32_t ext2_create_directory(DiskInfo* diskInfo, ext2_super_block* superBlock, char* parentPath, char* directoryName, int16_t type, int64_t& timeElapsedMilliseconds)
{
    auto start = std::chrono::high_resolution_clock::now();

    uint32_t result = createDirectory(diskInfo, superBlock, parentPath, directoryName, type);

    auto stop = std::chrono::high_resolution_clock::now();
    timeElapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

    return result;
}

uint32_t ext2_get_subdirectories(DiskInfo* diskInfo, ext2_super_block* superBlock, char* directoryPath, std::vector<std::pair<ext2_inode*, ext2_dir_entry*>>& subDirectories,
                                 int64_t& timeElapsedMilliseconds)
{
    auto start = std::chrono::high_resolution_clock::now();

    uint32_t result = getSubDirectoriesByParentPath(diskInfo, superBlock, directoryPath, subDirectories);

    auto stop = std::chrono::high_resolution_clock::now();
    timeElapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

    return result;
}

uint32_t ext2_write_file(DiskInfo* diskInfo, ext2_super_block* superBlock, char* filePath, char* buffer, uint32_t bytesToWrite, uint32_t writeMode, uint32_t& numberOfBytesWritten,
                         uint32_t& reasonForIncompleteWrite, int64_t& timeElapsedMilliseconds)
{
    uint32_t bufferSize = ((bytesToWrite / superBlock->s_log_block_size) + 2) * superBlock->s_log_block_size; //in order to avoid overflows
    char* writeBuffer = new char[bufferSize];
    memcpy(writeBuffer, buffer, bytesToWrite);

    auto start = std::chrono::high_resolution_clock::now();

    uint32_t result = write(diskInfo, superBlock, filePath, writeBuffer, bytesToWrite, numberOfBytesWritten, writeMode,
                            reasonForIncompleteWrite);

    auto stop = std::chrono::high_resolution_clock::now();
    timeElapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

    delete[] writeBuffer;

    return result;
}

uint32_t ext2_read_file(DiskInfo* diskInfo, ext2_super_block* superBlock, char* filePath, char* buffer, uint32_t bytesToRead, uint32_t startingPosition, uint32_t& numberOfBytesRead,
                        uint32_t& reasonForIncompleteRead, int64_t& timeElapsedMilliseconds)
{
    uint32_t bufferSize = ((bytesToRead / superBlock->s_log_block_size) + 2) * superBlock->s_log_block_size; //in order to avoid overflows
    char* readBuffer = new char[bufferSize];

    auto start = std::chrono::high_resolution_clock::now();

    uint32_t result = read(diskInfo, superBlock, filePath, readBuffer, startingPosition, bytesToRead, numberOfBytesRead, reasonForIncompleteRead);

    auto stop = std::chrono::high_resolution_clock::now();
    timeElapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

    memcpy(buffer, readBuffer, bytesToRead);
  //delete[] readBuffer;

    return result;
}

uint32_t ext2_truncate_file(DiskInfo* diskInfo, ext2_super_block* superBlock, char* filePath, uint32_t newSize, int64_t& timeElapsedMilliseconds)
{
    auto start = std::chrono::high_resolution_clock::now();

    uint32_t result = truncateFile(diskInfo, superBlock, filePath, newSize);;

    auto stop = std::chrono::high_resolution_clock::now();
    timeElapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

    return result;
}

uint32_t ext2_delete_directory(DiskInfo* diskInfo, ext2_super_block* superBlock, char* directoryPath, int64_t& timeElapsedMilliseconds)
{
    auto start = std::chrono::high_resolution_clock::now();

    std::string warning;
    uint32_t result = deleteDirectoryByPath(diskInfo, superBlock, directoryPath, warning);

    auto stop = std::chrono::high_resolution_clock::now();
    timeElapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

    return result;
}

uint32_t ext2_get_directory_attributes(DiskInfo* diskInfo, ext2_super_block* superBlock, char* directoryPath, Ext2DirectoryDisplayableAttributes* attributes,
                                       int64_t& timeElapsedMilliseconds)
{
    auto start = std::chrono::high_resolution_clock::now();

    uint32_t result = getDirectoryDisplayableAttributes(diskInfo, superBlock, directoryPath, attributes);

    auto stop = std::chrono::high_resolution_clock::now();
    timeElapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

    return result;
}