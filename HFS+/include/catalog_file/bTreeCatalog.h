#include "disk.h"
#include "vector"
#include "../structures.h"

#ifndef HFS__BTREECATALOG_H
#define HFS__BTREECATALOG_H

/////////////////////////

//HIDDEN FEATURE this method also returns the node number where the record is placed
uint32_t cf_searchRecordForGivenNodeDataAndSearchedKey(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, HFSPlusCatalogKey* searchedKey, char* nodeData,
                                                    CatalogDirectoryRecord* searchedRecord, uint32_t& nodeNumberForRecord);

uint32_t cf_insertRecordInTree(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, CatalogDirectoryRecord* recordToInsert);

uint32_t cf_traverseSubtree(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, uint32_t nodeNumberOfNodeToTraverseItsSubtree, HFSCatalogNodeID parentRecordId,
                         std::vector<CatalogDirectoryRecord*>& recordsVector);

//////////////////////////////////////////////

static uint32_t cf_splitChild(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, uint32_t nodeNumberToMoveOneRecordTo,
                    uint32_t nodeNumberToSplit, int32_t indexToMoveTheRecordTo);

static uint32_t cf_insertNonFull(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, uint32_t nodeNumberToInsertRecordInto,
                              CatalogDirectoryRecord* recordToInsert);

///////////////////////////////////////////

uint32_t cf_readNodeFromDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, char* nodeBuffer, uint32_t nodeNumber);

//Creates a new node on disk HIDDEN FEATURE it also returns the created node buffer through newNodeData, and the number of the new node
uint32_t cf_createNodeOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, char* newNodeData,
                          uint32_t& newNodeNumber, uint16_t isLeaf);

//Being given a node number and a record to insert on a certain record index it inserts it. CAUTION if there is already a record at that index it overwrites it
//shouldIncreaseNumOfRecordsInNodeDescriptor exists because this method is also used for moving to another index without actually adding new ones, just overwriting existing ones
uint32_t cf_insertRecordInNode(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogDirectoryRecord* recordToInsert, uint32_t nodeNumber, uint32_t recordIndexInNode,
                                  bool shouldIncreaseNumOfRecordsInNodeDescriptor);

//Being given a node number and a child node info to insert on a certain info index it inserts it. CAUTION 1: if there is already a child record info at that index it overwrites it
//CAUTION 2: ChildNodeInfo structures are stored in reverse order: if idx = 0 it is at catalogNodeSize - sizeOf(ChildNodeInfo); idx = 1 at catalogNodeSize - 2 * sizeOf(ChildNodeInfo)
uint32_t cf_insertChildNodeInfoInNode(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ChildNodeInfo* childNodeInfoToInsert, uint32_t nodeNumber, uint32_t childNodeInfoIndexInNode);



char* readNodeTEST(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, uint32_t nodeNumber);
#endif
