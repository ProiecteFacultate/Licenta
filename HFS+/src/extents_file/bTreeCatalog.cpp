#include "string.h"
#include "vector"

#include "disk.h"
#include "diskCodes.h"
#include "../../include/structures.h"
#include "../../include/utils.h"
#include "../../include/hfsFunctionUtils.h"
#include "../../include/extents_file/codes/bTreeResponseCodes.h"
#include "../../include/hfs.h"
#include "../../include/codes/hfsAttributes.h"
#include "../../include/extents_file/extentsFileUtils.h"
#include "../../include/extents_file/extentsFileOperations.h"
#include "../../include/extents_file/bTreeCatalog.h"

uint32_t eof_searchRecordForGivenNodeDataAndSearchedKey(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsFileCatalogKey* searchedKey, char* nodeData,
                                                    ExtentsDirectoryRecord* searchedRecord, uint32_t& nodeNumberForRecord)
{
    BTNodeDescriptor* nodeDescriptor = (BTNodeDescriptor*)&nodeData[0];
    uint32_t recordFirstByte = sizeof(BTNodeDescriptor);
    ExtentsDirectoryRecord* record = (ExtentsDirectoryRecord*)&nodeData[recordFirstByte];

    uint32_t recordIndex = 0;
    while(recordIndex < nodeDescriptor->numRecords && eof_compareKeys(searchedKey, &record->catalogKey) > 0)
    {
        recordIndex++;
        recordFirstByte += sizeof(ExtentsDirectoryRecord);
        record = (ExtentsDirectoryRecord*)&nodeData[recordFirstByte];
    }

    if(recordIndex < nodeDescriptor->numRecords && eof_compareKeys(searchedKey, &record->catalogKey) == 0)
    {
        memcpy(searchedRecord, record, sizeof(ExtentsDirectoryRecord));
        return EOF_SEARCH_RECORD_IN_GIVEN_DATA_SUCCESS;
    }

    if(nodeDescriptor->isLeaf == NODE_IS_LEAF) //if we haven't found the key and this is a leaf node it means the key doesn't exist in tree
        return EOF_SEARCH_RECORD_IN_GIVEN_DATA_KEY_DO_NOT_EXIST_IN_TREE;

    ChildNodeInfo* nextNodeInfo = (ChildNodeInfo*)&nodeData[getExtentsOverflowFileNodeSize() - (recordIndex + 1) * sizeof(ChildNodeInfo)];

    uint32_t readNodeFromDiskResult = eof_readNodeFromDisk(diskInfo, volumeHeader, nodeData, nextNodeInfo->nodeNumber);
    if(readNodeFromDiskResult == EOF_READ_NODE_FROM_DISK_FAILED)
        return EOF_SEARCH_RECORD_IN_GIVEN_DATA_FAILED_FOR_OTHER_REASON;

    nodeNumberForRecord = nextNodeInfo->nodeNumber;
    return eof_searchRecordForGivenNodeDataAndSearchedKey(diskInfo, volumeHeader, searchedKey, nodeData, searchedRecord, nodeNumberForRecord);
}

/////////////////////////////insert

uint32_t eof_insertRecordInTree(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsFileHeaderNode* extentsFileHeaderNode, ExtentsDirectoryRecord* recordToInsert)
{
    uint32_t newNodeNumber;

    if(extentsFileHeaderNode->headerRecord.freeNodes == extentsFileHeaderNode->headerRecord.totalNodes - 1) //root node do not exist yet
    {
        char* rootNodeData = new char[getExtentsOverflowFileNodeSize()];
        uint32_t createNodeOnDiskResult = eof_createNodeOnDisk(diskInfo, volumeHeader, extentsFileHeaderNode, rootNodeData, newNodeNumber, NODE_IS_LEAF);
        delete[] rootNodeData;

        if(createNodeOnDiskResult == EOF_CREATE_NODE_ON_DISK_FAILED)
            return EOF_INSERT_RECORD_IN_TREE_FAILED;

        uint32_t insertFirstKeyInRootNodeResult = eof_insertRecordInNode(diskInfo, volumeHeader, recordToInsert, 0, 0, true);

        if(insertFirstKeyInRootNodeResult == EOF_INSERT_RECORD_IN_NODE_FAILED)
            return EOF_INSERT_RECORD_IN_TREE_FAILED;
    }
    else //root node already exists (tree is not empty)
    {
        //read root
        char* rootNodeData = new char[getExtentsOverflowFileNodeSize()];
        uint32_t readRootNodeFromDiskResult = eof_readNodeFromDisk(diskInfo, volumeHeader, rootNodeData, extentsFileHeaderNode->headerRecord.rootNode);

        if(readRootNodeFromDiskResult == EOF_READ_NODE_FROM_DISK_FAILED)
        {
            delete[] rootNodeData;
            return EOF_INSERT_RECORD_IN_NODE_FAILED;
        }

        BTNodeDescriptor* rootNodeDescriptor = (BTNodeDescriptor*)&rootNodeData[0];

        if(rootNodeDescriptor->numRecords == getMaximumNumberOfRecordsPerExtentsFileNode())
        {
            //create new node which will be the new root
            char* newNodeData = new char[getExtentsOverflowFileNodeSize()];
            uint32_t createNodeOnDiskResult = eof_createNodeOnDisk(diskInfo, volumeHeader, extentsFileHeaderNode, newNodeData, newNodeNumber, NODE_IS_NOT_LEAF);

            delete[] rootNodeData;
            if(createNodeOnDiskResult == EOF_CREATE_NODE_ON_DISK_FAILED)
            {
                delete[] newNodeData;
                return EOF_INSERT_RECORD_IN_TREE_FAILED;
            }

            //make old root the child of this new root
            ChildNodeInfo* childNodeInfoToInsert = new ChildNodeInfo();
            childNodeInfoToInsert->nodeNumber = extentsFileHeaderNode->headerRecord.rootNode;
            uint32_t insertChildNodeInfoResult = eof_insertChildNodeInfoInNode(diskInfo, volumeHeader, childNodeInfoToInsert, newNodeNumber, 0);

            if(insertChildNodeInfoResult == EOF_INSERT_CHILD_NODE_INFO_IN_NODE_FAILED)
            {
                delete[] newNodeData;
                return EOF_INSERT_RECORD_IN_TREE_FAILED;
            }

            //split the old root and move 1 record to the new root
            uint32_t splitChildResult = eof_splitChild(diskInfo, volumeHeader, extentsFileHeaderNode, newNodeNumber, extentsFileHeaderNode->headerRecord.rootNode, 0);

            if(splitChildResult == EOF_SPLIT_CHILD_FAILED)
            {
                delete[] newNodeData;
                return EOF_INSERT_RECORD_IN_TREE_FAILED;
            }

            //read the new node from disk after the split child
            uint32_t readNewNodeFromDiskAfterUpdateResult = eof_readNodeFromDisk(diskInfo, volumeHeader, newNodeData, newNodeNumber);

            if(readNewNodeFromDiskAfterUpdateResult == EOF_READ_NODE_FROM_DISK_FAILED)
            {
                delete[] newNodeData;
                return EOF_INSERT_RECORD_IN_NODE_FAILED;
            }

            //new root has 2 children now; decide which of them will have the new record
            int32_t i = 0;
            ExtentsDirectoryRecord* newNodeFirstRecord = (ExtentsDirectoryRecord*)&newNodeData[sizeof(BTNodeDescriptor)];
            if(eof_compareKeys(&newNodeFirstRecord->catalogKey, &recordToInsert->catalogKey) < 0)
                i++;

            uint32_t byteIndexOfChildNodeInfo = getExtentsOverflowFileNodeSize() - (i + 1) * sizeof(ChildNodeInfo);
            ChildNodeInfo* childNodeInfo = (ChildNodeInfo*)&newNodeData[byteIndexOfChildNodeInfo];

            uint32_t insertNonNullResult = eof_insertNonFull(diskInfo, volumeHeader, extentsFileHeaderNode, childNodeInfo->nodeNumber, recordToInsert);
            if(insertNonNullResult == EOF_INSERT_NON_FULL_FAILED)
            {
                delete[] newNodeData;
                return EOF_INSERT_RECORD_IN_TREE_FAILED;
            }

            //change root
            ExtentsFileHeaderNode* updatedExtentsFileHeaderNode = new ExtentsFileHeaderNode();
            memcpy(updatedExtentsFileHeaderNode, extentsFileHeaderNode, sizeof(ExtentsFileHeaderNode));
            updatedExtentsFileHeaderNode->headerRecord.rootNode = newNodeNumber;

            eof_updateExtentsHeaderNodeOnDisk(diskInfo, volumeHeader, updatedExtentsFileHeaderNode);
            memcpy(extentsFileHeaderNode, updatedExtentsFileHeaderNode, sizeof(ExtentsFileHeaderNode));

            delete[] newNodeData;
        }
        else //if root is not full
        {
            uint32_t insertNonNullResult = eof_insertNonFull(diskInfo, volumeHeader, extentsFileHeaderNode, extentsFileHeaderNode->headerRecord.rootNode, recordToInsert);

            delete[] rootNodeData;
            if(insertNonNullResult == EOF_INSERT_RECORD_IN_NODE_FAILED)
                return EOF_INSERT_RECORD_IN_TREE_FAILED;
        }
    }

    HFSPlusVolumeHeader* updatedVolumeHeader = new HFSPlusVolumeHeader();
    memcpy(updatedVolumeHeader, volumeHeader, sizeof(HFSPlusVolumeHeader));
    updatedVolumeHeader->nextExtentsID++;
    updateVolumeHeaderNodeOnDisk(diskInfo, volumeHeader, updatedVolumeHeader);
    memcpy(volumeHeader, updatedVolumeHeader, sizeof(HFSPlusVolumeHeader));

    return EOF_INSERT_RECORD_IN_TREE_SUCCESS;
}

uint32_t eof_traverseSubtree(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, uint32_t nodeNumberOfNodeToTraverseItsSubtree, HFSCatalogNodeID fileId,
                         std::vector<ExtentsDirectoryRecord*>& recordsVector)
{
    //read node to traverse its subtree from disk
    char* nodeToTraverseItsSubtreeData = new char[getExtentsOverflowFileNodeSize()];
    uint32_t readNodeFromDiskResult = eof_readNodeFromDisk(diskInfo, volumeHeader, nodeToTraverseItsSubtreeData, nodeNumberOfNodeToTraverseItsSubtree);

    if(readNodeFromDiskResult == EOF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeToTraverseItsSubtreeData;
        return EOF_TRAVERSE_SUBTREE_FAILED;
    }

    BTNodeDescriptor* nodeToTraverseItsSubtreeDescriptor = (BTNodeDescriptor*)&nodeToTraverseItsSubtreeData[0];

    int32_t index;

    for(index = 0; index < nodeToTraverseItsSubtreeDescriptor->numRecords; index++)
    {
        uint32_t indexOfRecord = sizeof(BTNodeDescriptor) + index * sizeof(ExtentsDirectoryRecord);
        ExtentsDirectoryRecord* record = new ExtentsDirectoryRecord();
        memcpy(record, &nodeToTraverseItsSubtreeData[indexOfRecord], sizeof(ExtentsDirectoryRecord));
        if(record->catalogKey.fileId == fileId)
            recordsVector.push_back(record);
        else
            delete record;

        if(nodeToTraverseItsSubtreeDescriptor->isLeaf == NODE_IS_NOT_LEAF)
        {
            uint32_t indexOfChildNodeInfo = getExtentsOverflowFileNodeSize() - (index + 1) * sizeof(ChildNodeInfo);
            ChildNodeInfo* childNodeInfo = (ChildNodeInfo*)&nodeToTraverseItsSubtreeData[indexOfChildNodeInfo];
            uint32_t traverseResult = eof_traverseSubtree(diskInfo, volumeHeader, childNodeInfo->nodeNumber, fileId, recordsVector);

            delete childNodeInfo;
            if(traverseResult == EOF_TRAVERSE_SUBTREE_FAILED)
            {
                delete[] nodeToTraverseItsSubtreeData, delete nodeToTraverseItsSubtreeDescriptor;
                return EOF_TRAVERSE_SUBTREE_FAILED;
            }
        }
    }

    if(nodeToTraverseItsSubtreeDescriptor->isLeaf == NODE_IS_NOT_LEAF)
    {
        uint32_t indexOfChildNodeInfo = getExtentsOverflowFileNodeSize() - (index + 1) * sizeof(ChildNodeInfo);
        ChildNodeInfo* childNodeInfo = (ChildNodeInfo*)&nodeToTraverseItsSubtreeData[indexOfChildNodeInfo];
        uint32_t traverseResult = eof_traverseSubtree(diskInfo, volumeHeader, childNodeInfo->nodeNumber, fileId, recordsVector);

        delete[] nodeToTraverseItsSubtreeData, delete nodeToTraverseItsSubtreeDescriptor, delete childNodeInfo;
        if(traverseResult == EOF_TRAVERSE_SUBTREE_FAILED)
            return EOF_TRAVERSE_SUBTREE_FAILED;
    }

    return EOF_TRAVERSE_SUBTREE_SUCCESS;
}

///////////////////////////////////////////

static uint32_t eof_splitChild(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsFileHeaderNode* extentsFileHeaderNode, uint32_t nodeNumberToMoveOneRecordTo,
                           uint32_t nodeNumberToSplit, int32_t indexToMoveTheRecordTo)
{
    uint32_t halfOfTheMaxNumberOfRecordsPerNode = getMaximumNumberOfRecordsPerCatalogFileNode() / 2 + 1; //t (maximum is always odd so it will actually be the great half for 5 will be 3)
    //read the 2 existing nodes from disk
    char* nodeToMoveOneRecordToData = new char[getExtentsOverflowFileNodeSize()]; //s
    uint32_t readNodeFromDiskResult = eof_readNodeFromDisk(diskInfo, volumeHeader, nodeToMoveOneRecordToData, nodeNumberToMoveOneRecordTo);

    if(readNodeFromDiskResult == EOF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeToMoveOneRecordToData;
        return EOF_SPLIT_CHILD_FAILED;
    }

    BTNodeDescriptor* nodeToMoveOneRecordToNodeDescriptor = (BTNodeDescriptor*)&nodeToMoveOneRecordToData[0];

    char* nodeToSplitData = new char[getExtentsOverflowFileNodeSize()]; //y
    readNodeFromDiskResult = eof_readNodeFromDisk(diskInfo, volumeHeader, nodeToSplitData, nodeNumberToSplit);

    if(readNodeFromDiskResult == EOF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeToMoveOneRecordToData, delete[] nodeToSplitData;
        return EOF_SPLIT_CHILD_FAILED;
    }

    BTNodeDescriptor* nodeToSplitNodeDescriptor = (BTNodeDescriptor*)&nodeToSplitData[0];

    //create a new node
    char* newNodeData = new char[getExtentsOverflowFileNodeSize()]; // z
    uint32_t newNodeNumber;

    uint32_t createNodeOnDiskResult = eof_createNodeOnDisk(diskInfo, volumeHeader, extentsFileHeaderNode, newNodeData, newNodeNumber, nodeToSplitNodeDescriptor->isLeaf);

    if(createNodeOnDiskResult == EOF_CREATE_NODE_ON_DISK_FAILED)
    {
        delete[] nodeToMoveOneRecordToData, delete[] nodeToSplitData, delete[] newNodeData;
        return EOF_SPLIT_CHILD_FAILED;
    }

    //copy the last t-1 records of nodeToSplit to the new node
    for (int32_t j = 0; j < halfOfTheMaxNumberOfRecordsPerNode - 1; j++)
    {
        uint32_t startingByteOfRecord = sizeof(BTNodeDescriptor) + (j + halfOfTheMaxNumberOfRecordsPerNode) * sizeof(ExtentsDirectoryRecord);
        ExtentsDirectoryRecord *recordToInsert = (ExtentsDirectoryRecord*) &nodeToSplitData[startingByteOfRecord];
        uint32_t insertRecordInNodeResult = eof_insertRecordInNode(diskInfo, volumeHeader, recordToInsert, newNodeNumber, j,
                                                               true);

        if (insertRecordInNodeResult == EOF_INSERT_RECORD_IN_NODE_FAILED)
        {
            delete[] nodeToMoveOneRecordToData, delete[] nodeToSplitData, delete[] newNodeData;
            return EOF_SPLIT_CHILD_FAILED;
        }
    }

    //copy the last t children of nodeToSplit to the new node
    if(nodeToSplitNodeDescriptor->isLeaf == NODE_IS_NOT_LEAF)
    {
        for (int32_t j = 0; j < halfOfTheMaxNumberOfRecordsPerNode; j++) {
            uint32_t startingByteOfChildNodeInfo = getExtentsOverflowFileNodeSize() - (j + halfOfTheMaxNumberOfRecordsPerNode + 1) * sizeof(ChildNodeInfo);
            ChildNodeInfo *childNodeInfoToInsert = (ChildNodeInfo*) &nodeToSplitData[startingByteOfChildNodeInfo];
            uint32_t insertChildNodeInfoInNodeResult = eof_insertChildNodeInfoInNode(diskInfo, volumeHeader, childNodeInfoToInsert, newNodeNumber, j);

            if (insertChildNodeInfoInNodeResult == EOF_INSERT_CHILD_NODE_INFO_IN_NODE_FAILED)
            {
                delete[] nodeToMoveOneRecordToData, delete[] nodeToSplitData, delete[] newNodeData;
                return EOF_SPLIT_CHILD_FAILED;
            }
        }
    }

    //reduce the number of records in nodeToSplit
    nodeToSplitNodeDescriptor->numRecords = halfOfTheMaxNumberOfRecordsPerNode - 1;
    eof_updateNodeOnDisk(diskInfo, volumeHeader, nodeToSplitData, nodeNumberToSplit);

    //move existing children of nodeNumberToMoveOneRecordTo to right to make space for the new child
    for(int32_t j = nodeToMoveOneRecordToNodeDescriptor->numRecords; j >= indexToMoveTheRecordTo + 1; j--)
    {
        uint32_t startingByteOfChildNodeInfo = getExtentsOverflowFileNodeSize() - (j + 1) * sizeof(ChildNodeInfo);
        ChildNodeInfo *childNodeInfoToInsert = (ChildNodeInfo*) &nodeToMoveOneRecordToData[startingByteOfChildNodeInfo];
        uint32_t insertChildNodeInfoResult = eof_insertChildNodeInfoInNode(diskInfo, volumeHeader, childNodeInfoToInsert, nodeNumberToMoveOneRecordTo, j + 1);

        if (insertChildNodeInfoResult == EOF_INSERT_CHILD_NODE_INFO_IN_NODE_FAILED) {
            delete[] nodeToMoveOneRecordToData, delete[] nodeToSplitData, delete[] newNodeData;
            return EOF_SPLIT_CHILD_FAILED;
        }
    }

    //link the new child (new node) to nodeToMoveOneRecordTo
    ChildNodeInfo *childNodeInfoToInsert = new ChildNodeInfo();
    childNodeInfoToInsert->nodeNumber = newNodeNumber;
    uint32_t insertChildNodeInfoResult = eof_insertChildNodeInfoInNode(diskInfo, volumeHeader, childNodeInfoToInsert, nodeNumberToMoveOneRecordTo,
                                                                   indexToMoveTheRecordTo + 1);

    if (insertChildNodeInfoResult == EOF_INSERT_CHILD_NODE_INFO_IN_NODE_FAILED)
    {
        delete[] nodeToMoveOneRecordToData, delete[] nodeToSplitData, delete[] newNodeData;
        return EOF_SPLIT_CHILD_FAILED;
    }

    //move all records with greater keys (of nodeToMoveOneRecordTo) than the record to move one space to right to make space for the moved record
    for(int32_t j = nodeToMoveOneRecordToNodeDescriptor->numRecords - 1; j >= indexToMoveTheRecordTo; j--)
    {
        uint32_t startingByteOfRecord = sizeof(BTNodeDescriptor) + j * sizeof(ExtentsDirectoryRecord);
        ExtentsDirectoryRecord* recordToInsert = (ExtentsDirectoryRecord*) &nodeToMoveOneRecordToData[startingByteOfRecord];
        uint32_t insertRecordInNodeResult = eof_insertRecordInNode(diskInfo, volumeHeader, recordToInsert, nodeNumberToMoveOneRecordTo, j + 1,
                                                               false);

        if (insertRecordInNodeResult == EOF_INSERT_RECORD_IN_NODE_FAILED)
        {
            delete[] nodeToMoveOneRecordToData, delete[] nodeToSplitData, delete[] newNodeData;
            return EOF_SPLIT_CHILD_FAILED;
        }
    }

    //copy the middle record of nodeToSplit to nodeToMoveOneRecordTo
    uint32_t startingByteOfRecord = sizeof(BTNodeDescriptor) + (halfOfTheMaxNumberOfRecordsPerNode - 1) * sizeof(ExtentsDirectoryRecord);
    ExtentsDirectoryRecord* recordToInsert = (ExtentsDirectoryRecord*) &nodeToSplitData[startingByteOfRecord];
    uint32_t insertRecordInNodeResult = eof_insertRecordInNode(diskInfo, volumeHeader, recordToInsert, nodeNumberToMoveOneRecordTo, indexToMoveTheRecordTo, true);

    delete[] nodeToMoveOneRecordToData, delete[] nodeToSplitData, delete[] newNodeData;
    return (insertRecordInNodeResult == EOF_INSERT_RECORD_IN_NODE_SUCCESS) ? EOF_SPLIT_CHILD_SUCCESS : EOF_SPLIT_CHILD_FAILED;
}

static uint32_t eof_insertNonFull(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsFileHeaderNode* extentsFileHeaderNode, uint32_t nodeNumberToInsertRecordInto,
                              ExtentsDirectoryRecord* recordToInsert)
{
    //read node to insert record into from disk
    char* nodeToInsertRecordIntoData = new char[getExtentsOverflowFileNodeSize()];
    uint32_t readNodeFromDiskResult = eof_readNodeFromDisk(diskInfo, volumeHeader, nodeToInsertRecordIntoData, nodeNumberToInsertRecordInto);

    if(readNodeFromDiskResult == EOF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeToInsertRecordIntoData;
        return EOF_INSERT_NON_FULL_FAILED;
    }

    BTNodeDescriptor* nodeToInsertRecordIntoDescriptor = (BTNodeDescriptor*)&nodeToInsertRecordIntoData[0];

    //initialize index as index of rightmost element
    int32_t index = nodeToInsertRecordIntoDescriptor->numRecords - 1;

    if(nodeToInsertRecordIntoDescriptor->isLeaf == NODE_IS_LEAF)
    {
        //find location where the new record to be inserted and move all records with greater key one place at right
        uint32_t byteIndexOfRecord = sizeof(BTNodeDescriptor) + index * sizeof(ExtentsDirectoryRecord);
        ExtentsDirectoryRecord* record = (ExtentsDirectoryRecord*) &nodeToInsertRecordIntoData[byteIndexOfRecord];

        while(index >= 0 && eof_compareKeys(&record->catalogKey, &recordToInsert->catalogKey) > 0)
        {
            uint32_t insertRecordInNodeResult = eof_insertRecordInNode(diskInfo, volumeHeader, record, nodeNumberToInsertRecordInto, index + 1, false);

            if (insertRecordInNodeResult == EOF_INSERT_RECORD_IN_NODE_FAILED)
            {
                delete[] nodeToInsertRecordIntoData;
                return EOF_INSERT_NON_FULL_FAILED;
            }

            index--;
            byteIndexOfRecord -= sizeof(ExtentsDirectoryRecord);
            record = (ExtentsDirectoryRecord*)&nodeToInsertRecordIntoData[byteIndexOfRecord];
        }

        //insert the new record at found location
        uint32_t insertRecordInNodeResult = eof_insertRecordInNode(diskInfo, volumeHeader, recordToInsert, nodeNumberToInsertRecordInto, index + 1, true);

        delete[] nodeToInsertRecordIntoData;
        return (insertRecordInNodeResult == EOF_INSERT_RECORD_IN_NODE_SUCCESS) ? EOF_INSERT_NON_FULL_SUCCESS : EOF_INSERT_NON_FULL_FAILED;
    }
    else //node is not leaf
    {
        //find the child which is going to have the new record
        uint32_t byteIndexOfRecord = sizeof(BTNodeDescriptor) + index * sizeof(ExtentsDirectoryRecord);
        ExtentsDirectoryRecord* record = (ExtentsDirectoryRecord*)&nodeToInsertRecordIntoData[byteIndexOfRecord];

        while(index >= 0 && eof_compareKeys(&record->catalogKey, &recordToInsert->catalogKey) > 0)
        {
            index--;
            byteIndexOfRecord -= sizeof(ExtentsDirectoryRecord);
            record = (ExtentsDirectoryRecord*)&nodeToInsertRecordIntoData[byteIndexOfRecord];
        }

        //read the found child node from disk
        uint32_t startingByteOfChildNodeInfo = getExtentsOverflowFileNodeSize() - (index + 2) * sizeof(ChildNodeInfo);
        ChildNodeInfo *childNodeInfo = (ChildNodeInfo*) &nodeToInsertRecordIntoData[startingByteOfChildNodeInfo];

        char* foundChildNodeData = new char[getExtentsOverflowFileNodeSize()];
        readNodeFromDiskResult = eof_readNodeFromDisk(diskInfo, volumeHeader, foundChildNodeData, childNodeInfo->nodeNumber);

        if(readNodeFromDiskResult == EOF_READ_NODE_FROM_DISK_FAILED)
        {
            delete[] nodeToInsertRecordIntoData, delete[] foundChildNodeData;
            return EOF_INSERT_NON_FULL_FAILED;
        }

        BTNodeDescriptor* foundChildNodeDescriptor = (BTNodeDescriptor*)&foundChildNodeData[0];

        //see if the found child is full
        if(foundChildNodeDescriptor->numRecords == getMaximumNumberOfRecordsPerExtentsFileNode())
        {
            //if the child is full then split it
            uint32_t splitChildResult = eof_splitChild(diskInfo, volumeHeader, extentsFileHeaderNode, nodeNumberToInsertRecordInto,
                                                   childNodeInfo->nodeNumber, index + 1);

            if(splitChildResult == EOF_SPLIT_CHILD_FAILED)
            {
                delete[] nodeToInsertRecordIntoData, delete[] foundChildNodeData;
                return EOF_INSERT_NON_FULL_FAILED;
            }

            //we read the node again from disk because it got changed on split
            readNodeFromDiskResult = eof_readNodeFromDisk(diskInfo, volumeHeader, nodeToInsertRecordIntoData, nodeNumberToInsertRecordInto);

            if(readNodeFromDiskResult == EOF_READ_NODE_FROM_DISK_FAILED)
            {
                delete[] nodeToInsertRecordIntoData;
                return EOF_INSERT_NON_FULL_FAILED;
            }

            // after split, the middle record of childNodeInfo[i] goes up and childNodeInfo[i] is split into two. See which of the two is going to have the new record
            byteIndexOfRecord = sizeof(BTNodeDescriptor) + (index + 1) * sizeof(CatalogDirectoryRecord);
            record = (ExtentsDirectoryRecord*)&nodeToInsertRecordIntoData[byteIndexOfRecord];

            if(eof_compareKeys(&record->catalogKey, &recordToInsert->catalogKey) < 0)
                index++;
        }

        startingByteOfChildNodeInfo = getExtentsOverflowFileNodeSize() - (index + 2) * sizeof(ChildNodeInfo);
        childNodeInfo = (ChildNodeInfo*) &nodeToInsertRecordIntoData[startingByteOfChildNodeInfo];
        uint32_t insertNonNullResult = eof_insertNonFull(diskInfo, volumeHeader, extentsFileHeaderNode, childNodeInfo->nodeNumber, recordToInsert);

        delete[] nodeToInsertRecordIntoData, delete[] foundChildNodeData;
        return (insertNonNullResult == EOF_INSERT_NON_FULL_SUCCESS) ? EOF_INSERT_NON_FULL_SUCCESS : EOF_INSERT_NON_FULL_FAILED;
    }
}

///////////////////////////

uint32_t eof_createNodeOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsFileHeaderNode* extentsFileHeaderNode, char* newNodeData,
                          uint32_t& newNodeNumber, uint16_t isLeaf)
{
    //the new node number = the number of nodes occupied. node numbers are indexed from 0; 1 is header node and we don't count it
    newNodeNumber = extentsFileHeaderNode->headerRecord.totalNodes - extentsFileHeaderNode->headerRecord.freeNodes - 1;
    uint32_t firstBlockForNode = eof_getFirstBlockForGivenNodeIndex(volumeHeader, newNodeNumber);
    uint32_t firstSector = getFirstSectorForGivenBlock(diskInfo, volumeHeader, firstBlockForNode);

    BTNodeDescriptor* newNodeDescriptor = new BTNodeDescriptor();
    newNodeDescriptor->fLink = 0;
    newNodeDescriptor->bLink = 0;
    newNodeDescriptor->kind = kBTLeafNode;
    newNodeDescriptor->height = 0;
    newNodeDescriptor->numRecords = 0;
    newNodeDescriptor->isLeaf = isLeaf;

    //write node descriptor in memory
    uint32_t numberOfSectorsWritten;
    memcpy(newNodeData, newNodeDescriptor, sizeof(BTNodeDescriptor));

    uint32_t writeResult = writeDiskSectors(diskInfo, eof_getNumberOfSectorsPerNode(diskInfo, volumeHeader), firstSector, newNodeData,
                                            numberOfSectorsWritten);

    if(writeResult != EC_NO_ERROR)
    {
        delete newNodeDescriptor;
        return EOF_CREATE_NODE_ON_DISK_FAILED;
    }

    //mark node as occupied in header node map record and decrease number of free nodes
    ExtentsFileHeaderNode* updatedExtentsFileHeaderNode = eof_updateNodeOccupiedInHeaderNodeMapRecord(extentsFileHeaderNode, newNodeNumber, 1);
    updatedExtentsFileHeaderNode->headerRecord.freeNodes--;

    eof_updateExtentsHeaderNodeOnDisk(diskInfo, volumeHeader, updatedExtentsFileHeaderNode);
    memcpy(extentsFileHeaderNode, updatedExtentsFileHeaderNode, sizeof(ExtentsFileHeaderNode));

    delete updatedExtentsFileHeaderNode, delete newNodeDescriptor;

    return EOF_CREATE_NODE_ON_DISK_SUCCESS;
}

uint32_t eof_insertRecordInNode(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ExtentsDirectoryRecord* recordToInsert, uint32_t nodeNumber, uint32_t recordIndexInNode,
                            bool shouldIncreaseNumOfRecordsInNodeDescriptor)
{
    char* nodeData = new char[getExtentsOverflowFileNodeSize()];
    uint32_t readNodeFromDiskResult = eof_readNodeFromDisk(diskInfo, volumeHeader, nodeData, nodeNumber);

    if(readNodeFromDiskResult == EOF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData;
        return EOF_INSERT_RECORD_IN_NODE_FAILED;
    }

    BTNodeDescriptor* nodeDescriptor = (BTNodeDescriptor*)&nodeData[0];
    uint32_t byteIndexInNodeToInsertTheRecord = sizeof(BTNodeDescriptor) + recordIndexInNode * sizeof(ExtentsDirectoryRecord);
    memcpy(nodeData + byteIndexInNodeToInsertTheRecord, recordToInsert, sizeof(ExtentsDirectoryRecord));

    if(shouldIncreaseNumOfRecordsInNodeDescriptor == true)
        nodeDescriptor->numRecords++; //making this operation on the pointer also changes the value in nodeData buffer... HOPEFULLY

    eof_updateNodeOnDisk(diskInfo, volumeHeader, nodeData, nodeNumber);

    delete[] nodeData;
    return EOF_INSERT_RECORD_IN_NODE_SUCCESS;
}

uint32_t eof_insertChildNodeInfoInNode(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ChildNodeInfo* childNodeInfoToInsert, uint32_t nodeNumber,
                                   uint32_t childNodeInfoIndexInNode)
{
    char* nodeData = new char[getExtentsOverflowFileNodeSize()];
    uint32_t readNodeFromDiskResult = eof_readNodeFromDisk(diskInfo, volumeHeader, nodeData, nodeNumber);

    if(readNodeFromDiskResult == EOF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData;
        return EOF_INSERT_CHILD_NODE_INFO_IN_NODE_FAILED;
    }

    uint32_t byteIndexInNodeToInsertChildInfo = getExtentsOverflowFileNodeSize() - (childNodeInfoIndexInNode + 1) * sizeof(ChildNodeInfo);
    memcpy(nodeData + byteIndexInNodeToInsertChildInfo, childNodeInfoToInsert, sizeof(ChildNodeInfo));

    eof_updateNodeOnDisk(diskInfo, volumeHeader, nodeData, nodeNumber);

    delete[] nodeData;
    return EOF_INSERT_CHILD_NODE_INFO_IN_NODE_SUCCESS;
}

/////////////////////

uint32_t eof_readNodeFromDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, char* nodeBuffer, uint32_t nodeNumber)
{
    uint32_t numberOfSectorsRead;
    uint32_t firstBlockForNode = eof_getFirstBlockForGivenNodeIndex(volumeHeader, nodeNumber);
    uint32_t firstSector = getFirstSectorForGivenBlock(diskInfo, volumeHeader, firstBlockForNode);

    uint32_t readResult = readDiskSectors(diskInfo, eof_getNumberOfSectorsPerNode(diskInfo, volumeHeader), firstSector, nodeBuffer,
                                          numberOfSectorsRead);

    return (readResult == EC_NO_ERROR) ? EOF_READ_NODE_FROM_DISK_SUCCESS : EOF_READ_NODE_FROM_DISK_FAILED;
}