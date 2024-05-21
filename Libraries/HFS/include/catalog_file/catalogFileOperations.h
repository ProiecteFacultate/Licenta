#include "../include/disk.h"
#include "../../include/structures.h"

#ifndef HFS__CATALOGFILEOPERATIONS_H
#define HFS__CATALOGFILEOPERATIONS_H

uint32_t cf_findCatalogDirectoryRecordByFullPath(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode,
                                     char* directoryPath, CatalogDirectoryRecord** catalogDirectoryRecord, uint32_t& nodeOfNewRecord);

//HIDDEN FEATURE this method also returns the node number where the record is placed
uint32_t cf_searchDirectoryRecordByDirectoryNameBeingGivenParentDirectoryRecord(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode,
                                                                             CatalogDirectoryRecord* parentDirectoryRecord, char* searchedDirectoryName,
                                                                             CatalogDirectoryRecord* searchedDirectoryRecord, uint32_t& nodeNumberForRecord);

////////////////////////////////////

//CRITICAL if this operation fail the whole tree catalog tree might be affected, so the WHOLE FILE SYSTEM MIGHT PE COMPROMISED
//the method doesn't return a status. It either throws an uncaught exception and the program stops, and if not, it means it succeeded
void cf_updateCatalogHeaderNodeOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* updatedCatalogFileHeaderNode);

void cf_updateNodeOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, char* updatedNodeData, uint32_t nodeNumber);

//CRITICAL the method uses updateNodeOnDisk which is another critical method and if that throws this also throws
uint32_t cf_updateRecordOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogDirectoryRecord* directoryRecord, CatalogDirectoryRecord* updatedDirectoryRecord,
                            uint32_t nodeNumberOfRecord);

#endif
