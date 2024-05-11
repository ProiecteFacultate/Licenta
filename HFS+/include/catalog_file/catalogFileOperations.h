#include "../include/disk.h"
#include "../../include/structures.h"

#ifndef HFS__CATALOGFILEOPERATIONS_H
#define HFS__CATALOGFILEOPERATIONS_H

uint32_t findCatalogDirectoryRecordByFullPath(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode,
                                     char* directoryPath, CatalogDirectoryRecord** catalogDirectoryRecord);

uint32_t searchDirectoryRecordByDirectoryNameBeingGivenParentDirectoryRecord(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode,
                                                                             CatalogDirectoryRecord* parentDirectoryRecord, char* searchedDirectoryName,
                                                                             CatalogDirectoryRecord* searchedDirectoryRecord);

////////////////////////////////////

//CRITICAL if this operation fail the whole tree catalog tree might be affected, so the WHOLE FILE SYSTEM MIGHT PE COMPROMISED
//the method doesn't return a status. It either throws an uncaught exception and the program stops, and if not, it means it succeeded
void updateCatalogHeaderNodeOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* updatedCatalogFileHeaderNode);

//CRITICAL
//the method doesn't return a status. It either throws an uncaught exception and the program stops, and if not, it means it succeeded
void updateNodeOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, char* updatedNodeData, uint32_t nodeNumber);

#endif
