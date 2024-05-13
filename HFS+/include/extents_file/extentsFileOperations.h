#ifndef HFS__EXTENTSFILEOPERATIONS_H
#define HFS__EXTENTSFILEOPERATIONS_H

uint32_t eof_findCatalogDirectoryRecordByFullPath(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsFileHeaderNode* extentsFileHeaderNode,
                                              char* directoryPath, ExtentsDirectoryRecord** extentsDirectoryRecord, uint32_t& nodeOfNewRecord, uint32_t extentOverflowIndex);

//HIDDEN FEATURE this method also returns the node number where the record is placed
uint32_t eof_searchDirectoryRecordByDirectoryNameBeingGivenParentDirectoryRecord(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsFileHeaderNode* extentsFileHeaderNode,
                                                                             ExtentsDirectoryRecord* parentDirectoryRecord, char* searchedDirectoryName,
                                                                             ExtentsDirectoryRecord* searchedDirectoryRecord, uint32_t& nodeNumberForRecord,
                                                                             uint32_t extentOverflowIndex);

////////////////////////////////////

//CRITICAL if this operation fail the whole tree catalog tree might be affected, so the WHOLE FILE SYSTEM MIGHT PE COMPROMISED
//the method doesn't return a status. It either throws an uncaught exception and the program stops, and if not, it means it succeeded
void eof_updateExtentsHeaderNodeOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsFileHeaderNode* updatedExtentsFileHeaderNode);

void eof_updateNodeOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, char* updatedNodeData, uint32_t nodeNumber);

//CRITICAL the method uses updateNodeOnDisk which is another critical method and if that throws this also throws
uint32_t eof_updateRecordOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsDirectoryRecord* directoryRecord, ExtentsDirectoryRecord* updatedDirectoryRecord,
                            uint32_t nodeNumberOfRecord);

#endif
