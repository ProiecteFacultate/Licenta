#ifndef HFS__EXTENTSFILEOPERATIONS_H
#define HFS__EXTENTSFILEOPERATIONS_H

//CRITICAL if this operation fail the whole tree catalog tree might be affected, so the WHOLE FILE SYSTEM MIGHT PE COMPROMISED
//the method doesn't return a status. It either throws an uncaught exception and the program stops, and if not, it means it succeeded
void eof_updateExtentsHeaderNodeOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsFileHeaderNode* updatedExtentsFileHeaderNode);

void eof_updateNodeOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, char* updatedNodeData, uint32_t nodeNumber);

//CRITICAL the method uses updateNodeOnDisk which is another critical method and if that throws this also throws
uint32_t eof_updateRecordOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsDirectoryRecord* directoryRecord, ExtentsDirectoryRecord* updatedDirectoryRecord,
                            uint32_t nodeNumberOfRecord);

#endif
