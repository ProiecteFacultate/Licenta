#include "vector"
#include "chrono"

#include "../include/fat32Api.h"
#include "../include/fat32Structures.h"
#include "../include/fat32FunctionUtils.h"
#include "../include/fat32.h"
#include "../include/codes/fat32ApiResponseCodes.h"
#include "../include/fat32TestApi.h"

uint32_t fat32_create_directory(DiskInfo* diskInfo, BootSector* bootSector, char* parentPath, char* directoryName, int16_t type, int64_t& timeElapsedMilliseconds)
{
    auto start = std::chrono::high_resolution_clock::now();

    uint32_t result = createDirectory(diskInfo, bootSector, parentPath, directoryName, type);

    auto stop = std::chrono::high_resolution_clock::now();
    timeElapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

    return result;
}

uint32_t fat32_get_subdirectories(DiskInfo* diskInfo, BootSector* bootSector, char* directoryPath, std::vector<std::pair<DirectoryEntry*, DirectorySizeDetails*>>& subDirectories,
                                int64_t& timeElapsedMilliseconds)
{
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<DirectoryEntry*> subDirectoriesDiEntry;
    uint32_t result = getSubDirectoriesByParentPath(diskInfo, bootSector, directoryPath, subDirectoriesDiEntry);

    if(result != GET_SUB_DIRECTORIES_SUCCESS)
        return result;

    for(DirectoryEntry* child: subDirectoriesDiEntry)
    {
        char* fileName = new char[12];
        memcpy(fileName, child->FileName, 11);
        fileName[11] = 0;

        uint32_t numOfClustersOccupiedClusters = child->FileSize / getClusterSize(bootSector);
        if(child->FileSize % getClusterSize(bootSector) == 0) //in case the size occupies the last sector at maximum
            numOfClustersOccupiedClusters--;

        uint32_t sizeOnDisk;
        uint32_t size = child->FileSize;
        uint32_t getDirectorySizeResult = getDirectoryDetailsByDirectoryEntry(diskInfo, bootSector, child, size,
                                                                        sizeOnDisk);

        if(getDirectorySizeResult != DIR_GET_FULL_SIZE_SUCCESS)
            return getDirectorySizeResult;

        DirectorySizeDetails* directorySizeDetails = new DirectorySizeDetails();
        directorySizeDetails->FileSize = size;
        directorySizeDetails->SizeOnDisk = sizeOnDisk;

        subDirectories.push_back(std::make_pair(child, directorySizeDetails));
    }

    auto stop = std::chrono::high_resolution_clock::now();
    timeElapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

    return result;
}

uint32_t fat32_write_file(DiskInfo* diskInfo, BootSector* bootSector, char* filePath, char* buffer, uint32_t bytesToWrite, uint32_t writeMode, uint32_t& numberOfBytesWritten,
                        uint32_t& reasonForIncompleteWrite, int64_t& timeElapsedMilliseconds)
{
    auto start = std::chrono::high_resolution_clock::now();

    uint32_t result = write(diskInfo, bootSector, filePath, buffer, bytesToWrite, numberOfBytesWritten, writeMode,
                                     reasonForIncompleteWrite);

    auto stop = std::chrono::high_resolution_clock::now();
    timeElapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

    return result;
}

uint32_t fat32_read_file(DiskInfo* diskInfo, BootSector* bootSector, char* filePath, char* buffer, uint32_t bytesToRead, uint32_t startingPosition, uint32_t& numberOfBytesRead,
                       uint32_t& reasonForIncompleteRead, int64_t& timeElapsedMilliseconds)
{
    auto start = std::chrono::high_resolution_clock::now();

    uint32_t result = read(diskInfo, bootSector, filePath, buffer, startingPosition, bytesToRead, numberOfBytesRead, reasonForIncompleteRead);

    auto stop = std::chrono::high_resolution_clock::now();
    timeElapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

    return result;
}

uint32_t fat32_truncate_file(DiskInfo* diskInfo, BootSector* bootSector, char* filePath, uint32_t newSize, int64_t& timeElapsedMilliseconds)
{
    auto start = std::chrono::high_resolution_clock::now();

    uint32_t result = truncateFile(diskInfo, bootSector, filePath, newSize);

    auto stop = std::chrono::high_resolution_clock::now();
    timeElapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

    return result;
}

uint32_t fat32_delete_directory(DiskInfo* diskInfo, BootSector* bootSector, char* directoryPath, int64_t& timeElapsedMilliseconds)
{
    auto start = std::chrono::high_resolution_clock::now();

    uint32_t result = deleteDirectoryByPath(diskInfo, bootSector, directoryPath);

    auto stop = std::chrono::high_resolution_clock::now();
    timeElapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

    return result;
}

uint32_t fat32_get_directory_attributes(DiskInfo* diskInfo, BootSector* bootSector, char* directoryPath, Fat32DirectoryDisplayableAttributes* attributes, int64_t& timeElapsedMilliseconds)
{
    auto start = std::chrono::high_resolution_clock::now();

    uint32_t result =  getDirectoryDisplayableAttributes(diskInfo, bootSector, directoryPath, attributes);

    auto stop = std::chrono::high_resolution_clock::now();
    timeElapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

    return result;
}