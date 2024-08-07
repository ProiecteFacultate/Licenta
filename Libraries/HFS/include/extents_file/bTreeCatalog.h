#include "disk.h"
#include "vector"
#include "../hfsStructures.h"

#ifndef HFS__EXTENTS_BTREECATALOG_H
#define HFS__EXTENTS_BTREECATALOG_H

//HIDDEN FEATURE this method also returns the node number where the record is placed
uint32_t eof_searchRecordForGivenNodeDataAndSearchedKey(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsFileCatalogKey* searchedKey, char* nodeData,
                                                    ExtentsDirectoryRecord* searchedRecord, uint32_t& nodeNumberForRecord);

uint32_t eof_insertRecordInTree(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsFileHeaderNode* extentsFileHeaderNode, ExtentsDirectoryRecord* recordToInsert);

uint32_t eof_traverseSubtree(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, uint32_t nodeNumberOfNodeToTraverseItsSubtree, HFSCatalogNodeID fileId,
                             std::vector<ExtentsDirectoryRecord*>& recordsVector);

uint32_t eof_removeRecordFromTree(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsFileHeaderNode* extentsFileHeaderNode, ExtentsDirectoryRecord* recordToRemove);

//////////////////////////////////////////////

static uint32_t eof_splitChild(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsFileHeaderNode* extentsFileHeaderNode, uint32_t nodeNumberToMoveOneRecordTo,
                           uint32_t nodeNumberToSplit, int32_t indexToMoveTheRecordTo);

static uint32_t eof_insertNonFull(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsFileHeaderNode* extentsFileHeaderNode, uint32_t nodeNumberToInsertRecordInto,
                              ExtentsDirectoryRecord* recordToInsert);

//////////REMOVE RECORD

static uint32_t eof_findKey(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, uint32_t nodeNumber, ExtentsDirectoryRecord* recordToFindGreaterThan, uint32_t& index);

static uint32_t eof_remove(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsFileHeaderNode* extentFileHeaderNode, uint32_t nodeNumber,
                          ExtentsDirectoryRecord* recordToRemove);

static uint32_t eof_removeFromLeaf(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, uint32_t nodeNumber, uint32_t index);

static uint32_t eof_removeFromNonLeaf(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsFileHeaderNode* extentsFileHeaderNode, uint32_t nodeNumber, uint32_t index);

static uint32_t eof_getPred(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, uint32_t nodeNumber, uint32_t index, ExtentsDirectoryRecord* predRecord);

static uint32_t eof_getSucc(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, uint32_t nodeNumber, uint32_t index, ExtentsDirectoryRecord* succRecord);

static uint32_t eof_fill(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsFileHeaderNode* extentsFileHeaderNode, uint32_t nodeNumber, uint32_t index);

static uint32_t eof_borrowFromPrev(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, uint32_t nodeNumber, uint32_t index);

static uint32_t eof_borrowFromNext(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, uint32_t nodeNumber, uint32_t index);

static uint32_t eof_merge(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsFileHeaderNode* extentsFileHeaderNode, uint32_t nodeNumber, uint32_t index);

///////////////////////////////////////////

uint32_t eof_readNodeFromDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, char* nodeBuffer, uint32_t nodeNumber);

//Creates a new node on disk HIDDEN FEATURE it also returns the created node buffer through newNodeData, and the number of the new node
uint32_t eof_createNodeOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsFileHeaderNode* extentsFileHeaderNode, char* newNodeData,
                          uint32_t& newNodeNumber, uint16_t isLeaf);

//Being given a node number and a record to insert on a certain record index it inserts it. CAUTION if there is already a record at that index it overwrites it
//shouldIncreaseNumOfRecordsInNodeDescriptor exists because this method is also used for moving to another index without actually adding new ones, just overwriting existing ones
uint32_t eof_insertRecordInNode(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsDirectoryRecord* recordToInsert, uint32_t nodeNumber, uint32_t recordIndexInNode,
                            bool shouldIncreaseNumOfRecordsInNodeDescriptor);

//Being given a node number and a child node info to insert on a certain info index it inserts it. CAUTION 1: if there is already a child record info at that index it overwrites it
//CAUTION 2: ChildNodeInfo structures are stored in reverse order: if idx = 0 it is at catalogNodeSize - sizeOf(ChildNodeInfo); idx = 1 at catalogNodeSize - 2 * sizeOf(ChildNodeInfo)
uint32_t eof_insertChildNodeInfoInNode(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ChildNodeInfo* childNodeInfoToInsert, uint32_t nodeNumber, uint32_t childNodeInfoIndexInNode);

#endif