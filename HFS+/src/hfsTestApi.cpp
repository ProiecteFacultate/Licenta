#include "vector"
#include "chrono"

#include "../include/hfsApi.h"
#include "../include/structures.h"
#include "../include/hfsTestApi.h"

uint32_t hfs_create_directory(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, char* parentPath, char* directoryName,
                              int16_t type, int64_t& timeElapsedMilliseconds)
{
    auto start = std::chrono::high_resolution_clock::now();

    uint32_t result = createDirectory(diskInfo, volumeHeader, catalogFileHeaderNode, parentPath, directoryName, type);

    auto stop = std::chrono::high_resolution_clock::now();
    timeElapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

    return result;
}

uint32_t hfs_get_subdirectories(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, char* directoryPath,
                                std::vector<CatalogDirectoryRecord*>& subDirectories, int64_t& timeElapsedMilliseconds)
{
    auto start = std::chrono::high_resolution_clock::now();

    uint32_t result = getSubDirectoriesByParentPath(diskInfo, volumeHeader, catalogFileHeaderNode, directoryPath, subDirectories);

    auto stop = std::chrono::high_resolution_clock::now();
    timeElapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

    return result;
}

uint32_t hfs_write_file(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, ExtentsFileHeaderNode* extentsFileHeaderNode, char* filePath,
                        char* buffer, uint32_t bytesToWrite, uint32_t writeMode, uint32_t& numberOfBytesWritten, uint32_t& reasonForIncompleteWrite, int64_t& timeElapsedMilliseconds)
{
    uint32_t bufferSize = ((bytesToWrite / volumeHeader->blockSize) + 1) * volumeHeader->blockSize; //in order to avoid overflows
    char* writeBuffer = new char[bufferSize];
    memcpy(writeBuffer, buffer, bytesToWrite);

    auto start = std::chrono::high_resolution_clock::now();

    uint32_t result = write(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, filePath, writeBuffer, bytesToWrite, numberOfBytesWritten,
                            writeMode, reasonForIncompleteWrite);

    auto stop = std::chrono::high_resolution_clock::now();
    timeElapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

    delete[] writeBuffer;

    return result;
}

uint32_t hfs_read_file(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, ExtentsFileHeaderNode* extentsFileHeaderNode, char* filePath,
                       char* buffer, uint32_t bytesToRead, uint32_t startingPosition, uint32_t& numberOfBytesRead, uint32_t& reasonForIncompleteRead, int64_t& timeElapsedMilliseconds)
{
    uint32_t bufferSize = ((bytesToRead / volumeHeader->blockSize) + 1) * volumeHeader->blockSize; //in order to avoid overflows
    char* readBuffer = new char[bufferSize];
    memcpy(readBuffer, buffer, bytesToRead);

    auto start = std::chrono::high_resolution_clock::now();

    uint32_t result = read(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, filePath, readBuffer, startingPosition, bytesToRead,
                                   numberOfBytesRead, reasonForIncompleteRead);

    auto stop = std::chrono::high_resolution_clock::now();
    timeElapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

    delete[] readBuffer;

    return result;
}

uint32_t hfs_truncate_file(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, ExtentsFileHeaderNode* extentsFileHeaderNode, char* filePath,
                           uint32_t newSize, int64_t& timeElapsedMilliseconds)
{
    auto start = std::chrono::high_resolution_clock::now();

    uint32_t result = truncate(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, filePath, newSize);

    auto stop = std::chrono::high_resolution_clock::now();
    timeElapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

    return result;
}

uint32_t hfs_delete_directory(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, ExtentsFileHeaderNode* extentsFileHeaderNode, char* directoryPath,
                              int64_t& timeElapsedMilliseconds)
{
    auto start = std::chrono::high_resolution_clock::now();

    uint32_t result = deleteDirectoryByPath(diskInfo, volumeHeader, catalogFileHeaderNode, extentsFileHeaderNode, directoryPath);

    auto stop = std::chrono::high_resolution_clock::now();
    timeElapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

    return result;
}

uint32_t hfs_get_directory_attributes(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, char* directoryPath, HFSDirectoryDisplayableAttributes* attributes,
                                      int64_t& timeElapsedMilliseconds)
{
    auto start = std::chrono::high_resolution_clock::now();

    uint32_t result = getDirectoryDisplayableAttributes(diskInfo, volumeHeader, catalogFileHeaderNode, directoryPath, attributes);

    auto stop = std::chrono::high_resolution_clock::now();
    timeElapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

    return result;
}