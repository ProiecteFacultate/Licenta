#include "string.h"
#include "vector"

#include "../include/disk.h"
#include "../include/diskCodes.h"
#include "../include/structures.h"
#include "../include/utils.h"
#include "../include/hfsFunctionUtils.h"
#include "../include/codes/hfsAttributes.h"
#include "../include/hfs.h"
#include "../include/hfsApi.h"

uint32_t createDirectory(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, char* directoryParentPath,
                         char* newDirectoryName, uint32_t newDirectoryAttribute)
{
    CatalogDirectoryRecord* actualCatalogDirectoryRecord = nullptr;
    uint32_t findCatalogDirectoryRecordResult = findCatalogDirectoryRecordByFullPath(diskInfo, volumeHeader, directoryParentPath,
                                                                     &actualCatalogDirectoryRecord);
    if(findCatalogDirectoryRecordEntryResult != FIND_DIRECTORY_ENTRY_BY_PATH_SUCCESS)
        return DIR_CREATION_PARENT_DO_NOT_EXIST;

    uint32_t parentAlreadyContainsDirectoryWithGivenName = findDirectoryEntryByDirectoryName(diskInfo,
                                                                                             bootSector,
                                                                                             actualDirectoryEntry,
                                                                                             newDirectoryName,
                                                                                             new DirectoryEntry());

}