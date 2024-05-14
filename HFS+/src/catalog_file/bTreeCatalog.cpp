#include "string.h"
#include "vector"

#include "disk.h"
#include "diskCodes.h"
#include "../../include/structures.h"
#include "../../include/utils.h"
#include "../../include/hfsFunctionUtils.h"
#include "../../include/catalog_file/codes/bTreeResponseCodes.h"
#include "../../include/catalog_file/codes/catalogFileResponseCodes.h"
#include "../../include/hfs.h"
#include "../../include/codes/hfsAttributes.h"
#include "../../include/catalog_file/catalogFileUtils.h"
#include "../../include/catalog_file/catalogFileOperations.h"
#include "../../include/catalog_file/bTreeCatalog.h"

uint32_t cf_searchRecordForGivenNodeDataAndSearchedKey(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, HFSPlusCatalogKey* searchedKey, char* nodeData,
                                                    CatalogDirectoryRecord* searchedRecord, uint32_t& nodeNumberForRecord)
{
    BTNodeDescriptor* nodeDescriptor = (BTNodeDescriptor*)&nodeData[0];
    uint32_t recordFirstByte = sizeof(BTNodeDescriptor);
    CatalogDirectoryRecord* record = (CatalogDirectoryRecord*)&nodeData[recordFirstByte];

    uint32_t recordIndex = 0;
    while(recordIndex < nodeDescriptor->numRecords && cf_compareKeys(searchedKey, &record->catalogKey) > 0)
    {
        recordIndex++;
        recordFirstByte += sizeof(CatalogDirectoryRecord);
        record = (CatalogDirectoryRecord*)&nodeData[recordFirstByte];
    }

    if(recordIndex < nodeDescriptor->numRecords && cf_compareKeys(searchedKey, &record->catalogKey) == 0)
    {
        memcpy(searchedRecord, record, sizeof(CatalogDirectoryRecord));
        return CF_SEARCH_RECORD_IN_GIVEN_DATA_SUCCESS;
    }

    if(nodeDescriptor->isLeaf == NODE_IS_LEAF) //if we haven't found the key and this is a leaf node it means the key doesn't exist in tree
        return CF_SEARCH_RECORD_IN_GIVEN_DATA_KEY_DO_NOT_EXIST_IN_TREE;

    ChildNodeInfo* nextNodeInfo = (ChildNodeInfo*)&nodeData[getCatalogFileNodeSize() - (recordIndex + 1) * sizeof(ChildNodeInfo)];

    uint32_t readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, nodeData, nextNodeInfo->nodeNumber);
    if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
        return CF_SEARCH_RECORD_IN_GIVEN_DATA_FAILED_FOR_OTHER_REASON;

    nodeNumberForRecord = nextNodeInfo->nodeNumber;
    return cf_searchRecordForGivenNodeDataAndSearchedKey(diskInfo, volumeHeader, searchedKey, nodeData, searchedRecord, nodeNumberForRecord);
}

/////////////////////////////insert

uint32_t cf_insertRecordInTree(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, CatalogDirectoryRecord* recordToInsert)
{
    uint32_t newNodeNumber;

    if(catalogFileHeaderNode->headerRecord.freeNodes == catalogFileHeaderNode->headerRecord.totalNodes - 1) //root node do not exist yet
    {
        char* rootNodeData = new char[getCatalogFileNodeSize()];
        uint32_t createNodeOnDiskResult = cf_createNodeOnDisk(diskInfo, volumeHeader, catalogFileHeaderNode, rootNodeData, newNodeNumber, NODE_IS_LEAF);
        delete[] rootNodeData;

        if(createNodeOnDiskResult == CF_CREATE_NODE_ON_DISK_FAILED)
            return CF_INSERT_RECORD_IN_TREE_FAILED;

        uint32_t insertFirstKeyInRootNodeResult = cf_insertRecordInNode(diskInfo, volumeHeader, recordToInsert, 0, 0, true);

        if(insertFirstKeyInRootNodeResult == CF_INSERT_RECORD_IN_NODE_FAILED)
            return CF_INSERT_RECORD_IN_TREE_FAILED;
    }
    else //root node already exists (tree is not empty)
    {
        //read root
        char* rootNodeData = new char[getCatalogFileNodeSize()];
        uint32_t readRootNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, rootNodeData, catalogFileHeaderNode->headerRecord.rootNode);

        if(readRootNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
        {
            delete[] rootNodeData;
            return CF_INSERT_RECORD_IN_NODE_FAILED;
        }

        BTNodeDescriptor* rootNodeDescriptor = (BTNodeDescriptor*)&rootNodeData[0];

        if(rootNodeDescriptor->numRecords == getMaximumNumberOfRecordsPerCatalogFileNode())
        {
            //create new node which will be the new root
            char* newNodeData = new char[getCatalogFileNodeSize()];
            uint32_t createNodeOnDiskResult = cf_createNodeOnDisk(diskInfo, volumeHeader, catalogFileHeaderNode, newNodeData, newNodeNumber, NODE_IS_NOT_LEAF);

            delete[] rootNodeData;
            if(createNodeOnDiskResult == CF_CREATE_NODE_ON_DISK_FAILED)
            {
                delete[] newNodeData;
                return CF_INSERT_RECORD_IN_TREE_FAILED;
            }

            //make old root the child of this new root
            ChildNodeInfo* childNodeInfoToInsert = new ChildNodeInfo();
            childNodeInfoToInsert->nodeNumber = catalogFileHeaderNode->headerRecord.rootNode;
            uint32_t insertChildNodeInfoResult = cf_insertChildNodeInfoInNode(diskInfo, volumeHeader, childNodeInfoToInsert, newNodeNumber, 0);

            if(insertChildNodeInfoResult == CF_INSERT_CHILD_NODE_INFO_IN_NODE_FAILED)
            {
                delete[] newNodeData;
                return CF_INSERT_RECORD_IN_TREE_FAILED;
            }

            //split the old root and move 1 record to the new root
            uint32_t splitChildResult = cf_splitChild(diskInfo, volumeHeader, catalogFileHeaderNode, newNodeNumber, catalogFileHeaderNode->headerRecord.rootNode, 0);

            if(splitChildResult == CF_SPLIT_CHILD_FAILED)
            {
                delete[] newNodeData;
                return CF_INSERT_RECORD_IN_TREE_FAILED;
            }

            //read the new node from disk after the split child
            uint32_t readNewNodeFromDiskAfterUpdateResult = cf_readNodeFromDisk(diskInfo, volumeHeader, newNodeData, newNodeNumber);

            if(readNewNodeFromDiskAfterUpdateResult == CF_READ_NODE_FROM_DISK_FAILED)
            {
                delete[] newNodeData;
                return CF_INSERT_RECORD_IN_NODE_FAILED;
            }

            //new root has 2 children now; decide which of them will have the new record
            int32_t i = 0;
            CatalogDirectoryRecord* newNodeFirstRecord = (CatalogDirectoryRecord*)&newNodeData[sizeof(BTNodeDescriptor)];
            if(cf_compareKeys(&newNodeFirstRecord->catalogKey, &recordToInsert->catalogKey) < 0)
                i++;

            uint32_t byteIndexOfChildNodeInfo = getCatalogFileNodeSize() - (i + 1) * sizeof(ChildNodeInfo);
            ChildNodeInfo* childNodeInfo = (ChildNodeInfo*)&newNodeData[byteIndexOfChildNodeInfo];

            uint32_t insertNonNullResult = cf_insertNonFull(diskInfo, volumeHeader, catalogFileHeaderNode, childNodeInfo->nodeNumber, recordToInsert);
            if(insertNonNullResult == CF_INSERT_NON_FULL_FAILED)
            {
                delete[] newNodeData;
                return CF_INSERT_RECORD_IN_TREE_FAILED;
            }

            //change root
            CatalogFileHeaderNode* updatedCatalogFileHeaderNode = new CatalogFileHeaderNode();
            memcpy(updatedCatalogFileHeaderNode, catalogFileHeaderNode, sizeof(CatalogFileHeaderNode));
            updatedCatalogFileHeaderNode->headerRecord.rootNode = newNodeNumber;

            cf_updateCatalogHeaderNodeOnDisk(diskInfo, volumeHeader, updatedCatalogFileHeaderNode);
            memcpy(catalogFileHeaderNode, updatedCatalogFileHeaderNode, sizeof(CatalogFileHeaderNode));

            delete[] newNodeData;
        }
        else //if root is not full
        {
            uint32_t insertNonNullResult = cf_insertNonFull(diskInfo, volumeHeader, catalogFileHeaderNode, catalogFileHeaderNode->headerRecord.rootNode, recordToInsert);

            delete[] rootNodeData;
            if(insertNonNullResult == CF_INSERT_RECORD_IN_NODE_FAILED)
                return CF_INSERT_RECORD_IN_TREE_FAILED;
        }
    }

    return CF_INSERT_RECORD_IN_TREE_SUCCESS;
}

uint32_t cf_traverseSubtree(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, uint32_t nodeNumberOfNodeToTraverseItsSubtree, HFSCatalogNodeID parentRecordId,
                         std::vector<CatalogDirectoryRecord*>& recordsVector)
{
    //read node to traverse its subtree from disk
    char* nodeToTraverseItsSubtreeData = new char[getCatalogFileNodeSize()];
    uint32_t readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, nodeToTraverseItsSubtreeData, nodeNumberOfNodeToTraverseItsSubtree);

    if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeToTraverseItsSubtreeData;
        return CF_TRAVERSE_SUBTREE_FAILED;
    }

    BTNodeDescriptor* nodeToTraverseItsSubtreeDescriptor = (BTNodeDescriptor*)&nodeToTraverseItsSubtreeData[0];

    int32_t index;

    for(index = 0; index < nodeToTraverseItsSubtreeDescriptor->numRecords; index++)
    {
        uint32_t indexOfRecord = sizeof(BTNodeDescriptor) + index * sizeof(CatalogDirectoryRecord);
        CatalogDirectoryRecord* record = new CatalogDirectoryRecord();
        memcpy(record, &nodeToTraverseItsSubtreeData[indexOfRecord], sizeof(CatalogDirectoryRecord));
        if(record->catalogKey.parentID == parentRecordId)
            recordsVector.push_back(record);
        else
            delete record;

        if(nodeToTraverseItsSubtreeDescriptor->isLeaf == NODE_IS_NOT_LEAF)
        {
            uint32_t indexOfChildNodeInfo = getCatalogFileNodeSize() - (index + 1) * sizeof(ChildNodeInfo);
            ChildNodeInfo* childNodeInfo = (ChildNodeInfo*)&nodeToTraverseItsSubtreeData[indexOfChildNodeInfo];
            uint32_t traverseResult = cf_traverseSubtree(diskInfo, volumeHeader, childNodeInfo->nodeNumber, parentRecordId, recordsVector);

            if(traverseResult == CF_TRAVERSE_SUBTREE_FAILED)
            {
                delete[] nodeToTraverseItsSubtreeData;
                return CF_TRAVERSE_SUBTREE_FAILED;
            }
        }
    }

    if(nodeToTraverseItsSubtreeDescriptor->isLeaf == NODE_IS_NOT_LEAF)
    {
        uint32_t indexOfChildNodeInfo = getCatalogFileNodeSize() - (index + 1) * sizeof(ChildNodeInfo);
        ChildNodeInfo* childNodeInfo = (ChildNodeInfo*)&nodeToTraverseItsSubtreeData[indexOfChildNodeInfo];
        uint32_t traverseResult = cf_traverseSubtree(diskInfo, volumeHeader, childNodeInfo->nodeNumber, parentRecordId, recordsVector);

        delete[] nodeToTraverseItsSubtreeData;
        if(traverseResult == CF_TRAVERSE_SUBTREE_FAILED)
            return CF_TRAVERSE_SUBTREE_FAILED;
    }

    return CF_TRAVERSE_SUBTREE_SUCCESS;
}

uint32_t cf_removeRecordFromTree(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, CatalogDirectoryRecord* recordToRemove)
{
    //There is a case where tree is empty but we can't have it here because we wouldn't find the record if so and we won't reach this method


}

///////////////////////////////////////////

static uint32_t cf_splitChild(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, uint32_t nodeNumberToMoveOneRecordTo,
                    uint32_t nodeNumberToSplit, int32_t indexToMoveTheRecordTo)
{
    uint32_t halfOfTheMaxNumberOfRecordsPerNode = getMaximumNumberOfRecordsPerCatalogFileNode() / 2 + 1; //t (maximum is always odd so it will actually be the great half for 5 will be 3)
    //read the 2 existing nodes from disk
    char* nodeToMoveOneRecordToData = new char[getCatalogFileNodeSize()]; //s
    uint32_t readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, nodeToMoveOneRecordToData, nodeNumberToMoveOneRecordTo);

    if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeToMoveOneRecordToData;
        return CF_SPLIT_CHILD_FAILED;
    }

    BTNodeDescriptor* nodeToMoveOneRecordToNodeDescriptor = (BTNodeDescriptor*)&nodeToMoveOneRecordToData[0];

    char* nodeToSplitData = new char[getCatalogFileNodeSize()]; //y
    readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, nodeToSplitData, nodeNumberToSplit);

    if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeToMoveOneRecordToData, delete[] nodeToSplitData;
        return CF_SPLIT_CHILD_FAILED;
    }

    BTNodeDescriptor* nodeToSplitNodeDescriptor = (BTNodeDescriptor*)&nodeToSplitData[0];

    //create a new node
    char* newNodeData = new char[getCatalogFileNodeSize()]; // z
    uint32_t newNodeNumber;

    uint32_t createNodeOnDiskResult = cf_createNodeOnDisk(diskInfo, volumeHeader, catalogFileHeaderNode, newNodeData, newNodeNumber, nodeToSplitNodeDescriptor->isLeaf);

    if(createNodeOnDiskResult == CF_CREATE_NODE_ON_DISK_FAILED)
    {
        delete[] nodeToMoveOneRecordToData, delete[] nodeToSplitData, delete[] newNodeData;
        return CF_SPLIT_CHILD_FAILED;
    }

    //copy the last t-1 records of nodeToSplit to the new node
    for (int32_t j = 0; j < halfOfTheMaxNumberOfRecordsPerNode - 1; j++)
    {
        uint32_t startingByteOfRecord = sizeof(BTNodeDescriptor) + (j + halfOfTheMaxNumberOfRecordsPerNode) * sizeof(CatalogDirectoryRecord);
        CatalogDirectoryRecord *recordToInsert = (CatalogDirectoryRecord*) &nodeToSplitData[startingByteOfRecord];
        uint32_t insertRecordInNodeResult = cf_insertRecordInNode(diskInfo, volumeHeader, recordToInsert, newNodeNumber, j,
                                                               true);

        if (insertRecordInNodeResult == CF_INSERT_RECORD_IN_NODE_FAILED)
        {
            delete[] nodeToMoveOneRecordToData, delete[] nodeToSplitData, delete[] newNodeData;
            return CF_SPLIT_CHILD_FAILED;
        }
    }

    //copy the last t children of nodeToSplit to the new node
    if(nodeToSplitNodeDescriptor->isLeaf == NODE_IS_NOT_LEAF)
    {
        for (int32_t j = 0; j < halfOfTheMaxNumberOfRecordsPerNode; j++) {
            uint32_t startingByteOfChildNodeInfo = getCatalogFileNodeSize() - (j + halfOfTheMaxNumberOfRecordsPerNode + 1) * sizeof(ChildNodeInfo);
            ChildNodeInfo *childNodeInfoToInsert = (ChildNodeInfo*) &nodeToSplitData[startingByteOfChildNodeInfo];
            uint32_t insertChildNodeInfoInNodeResult = cf_insertChildNodeInfoInNode(diskInfo, volumeHeader, childNodeInfoToInsert, newNodeNumber, j);

            if (insertChildNodeInfoInNodeResult == CF_INSERT_CHILD_NODE_INFO_IN_NODE_FAILED)
            {
                delete[] nodeToMoveOneRecordToData, delete[] nodeToSplitData, delete[] newNodeData;
                return CF_SPLIT_CHILD_FAILED;
            }
        }
    }

    //reduce the number of records in nodeToSplit
    nodeToSplitNodeDescriptor->numRecords = halfOfTheMaxNumberOfRecordsPerNode - 1;
    cf_updateNodeOnDisk(diskInfo, volumeHeader, nodeToSplitData, nodeNumberToSplit);

    //move existing children of nodeNumberToMoveOneRecordTo to right to make space for the new child
    for(int32_t j = nodeToMoveOneRecordToNodeDescriptor->numRecords; j >= indexToMoveTheRecordTo + 1; j--)
    {
        uint32_t startingByteOfChildNodeInfo = getCatalogFileNodeSize() - (j + 1) * sizeof(ChildNodeInfo);
        ChildNodeInfo *childNodeInfoToInsert = (ChildNodeInfo*) &nodeToMoveOneRecordToData[startingByteOfChildNodeInfo];
        uint32_t insertChildNodeInfoResult = cf_insertChildNodeInfoInNode(diskInfo, volumeHeader, childNodeInfoToInsert, nodeNumberToMoveOneRecordTo, j + 1);

        if (insertChildNodeInfoResult == CF_INSERT_CHILD_NODE_INFO_IN_NODE_FAILED) {
            delete[] nodeToMoveOneRecordToData, delete[] nodeToSplitData, delete[] newNodeData;
            return CF_SPLIT_CHILD_FAILED;
        }
    }

    //link the new child (new node) to nodeToMoveOneRecordTo
    ChildNodeInfo *childNodeInfoToInsert = new ChildNodeInfo();
    childNodeInfoToInsert->nodeNumber = newNodeNumber;
    uint32_t insertChildNodeInfoResult = cf_insertChildNodeInfoInNode(diskInfo, volumeHeader, childNodeInfoToInsert, nodeNumberToMoveOneRecordTo,
                                                                   indexToMoveTheRecordTo + 1);

    if (insertChildNodeInfoResult == CF_INSERT_CHILD_NODE_INFO_IN_NODE_FAILED)
    {
        delete[] nodeToMoveOneRecordToData, delete[] nodeToSplitData, delete[] newNodeData;
        return CF_SPLIT_CHILD_FAILED;
    }

    //move all records with greater keys (of nodeToMoveOneRecordTo) than the record to move one space to right to make space for the moved record
    for(int32_t j = nodeToMoveOneRecordToNodeDescriptor->numRecords - 1; j >= indexToMoveTheRecordTo; j--)
    {
        uint32_t startingByteOfRecord = sizeof(BTNodeDescriptor) + j * sizeof(CatalogDirectoryRecord);
        CatalogDirectoryRecord *recordToInsert = (CatalogDirectoryRecord*) &nodeToMoveOneRecordToData[startingByteOfRecord];
        uint32_t insertRecordInNodeResult = cf_insertRecordInNode(diskInfo, volumeHeader, recordToInsert, nodeNumberToMoveOneRecordTo, j + 1,
                                                               false);

        if (insertRecordInNodeResult == CF_INSERT_RECORD_IN_NODE_FAILED)
        {
            delete[] nodeToMoveOneRecordToData, delete[] nodeToSplitData, delete[] newNodeData;
            return CF_SPLIT_CHILD_FAILED;
        }
    }

    //copy the middle record of nodeToSplit to nodeToMoveOneRecordTo
    uint32_t startingByteOfRecord = sizeof(BTNodeDescriptor) + (halfOfTheMaxNumberOfRecordsPerNode - 1) * sizeof(CatalogDirectoryRecord);
    CatalogDirectoryRecord *recordToInsert = (CatalogDirectoryRecord*) &nodeToSplitData[startingByteOfRecord];
    uint32_t insertRecordInNodeResult = cf_insertRecordInNode(diskInfo, volumeHeader, recordToInsert, nodeNumberToMoveOneRecordTo, indexToMoveTheRecordTo, true);

    delete[] nodeToMoveOneRecordToData, delete[] nodeToSplitData, delete[] newNodeData;
    return (insertRecordInNodeResult == CF_INSERT_RECORD_IN_NODE_SUCCESS) ? CF_SPLIT_CHILD_SUCCESS : CF_SPLIT_CHILD_FAILED;
}

static uint32_t cf_insertNonFull(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, uint32_t nodeNumberToInsertRecordInto,
                                CatalogDirectoryRecord* recordToInsert)
{
    //read node to insert record into from disk
    char* nodeToInsertRecordIntoData = new char[getCatalogFileNodeSize()];
    uint32_t readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, nodeToInsertRecordIntoData, nodeNumberToInsertRecordInto);

    if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeToInsertRecordIntoData;
        return CF_INSERT_NON_FULL_FAILED;
    }

    BTNodeDescriptor* nodeToInsertRecordIntoDescriptor = (BTNodeDescriptor*)&nodeToInsertRecordIntoData[0];

    //initialize index as index of rightmost element
    int32_t index = nodeToInsertRecordIntoDescriptor->numRecords - 1;

    if(nodeToInsertRecordIntoDescriptor->isLeaf == NODE_IS_LEAF)
    {
        //find location where the new record to be inserted and move all records with greater key one place at right
        uint32_t byteIndexOfRecord = sizeof(BTNodeDescriptor) + index * sizeof(CatalogDirectoryRecord);
        CatalogDirectoryRecord* record = (CatalogDirectoryRecord*) &nodeToInsertRecordIntoData[byteIndexOfRecord];

        while(index >= 0 && cf_compareKeys(&record->catalogKey, &recordToInsert->catalogKey) > 0)
        {
            uint32_t insertRecordInNodeResult = cf_insertRecordInNode(diskInfo, volumeHeader, record, nodeNumberToInsertRecordInto, index + 1, false);

            if (insertRecordInNodeResult == CF_INSERT_RECORD_IN_NODE_FAILED)
            {
                delete[] nodeToInsertRecordIntoData;
                return CF_INSERT_NON_FULL_FAILED;
            }

            index--;
            byteIndexOfRecord -= sizeof(CatalogDirectoryRecord);
            record = (CatalogDirectoryRecord*)&nodeToInsertRecordIntoData[byteIndexOfRecord];
        }

        //insert the new record at found location
        uint32_t insertRecordInNodeResult = cf_insertRecordInNode(diskInfo, volumeHeader, recordToInsert, nodeNumberToInsertRecordInto, index + 1, true);

        delete[] nodeToInsertRecordIntoData;
        return (insertRecordInNodeResult == CF_INSERT_RECORD_IN_NODE_SUCCESS) ? CF_INSERT_NON_FULL_SUCCESS : CF_INSERT_NON_FULL_FAILED;
    }
    else //node is not leaf
    {
        //find the child which is going to have the new record
        uint32_t byteIndexOfRecord = sizeof(BTNodeDescriptor) + index * sizeof(CatalogDirectoryRecord);
        CatalogDirectoryRecord* record = (CatalogDirectoryRecord*)&nodeToInsertRecordIntoData[byteIndexOfRecord];

        while(index >= 0 && cf_compareKeys(&record->catalogKey, &recordToInsert->catalogKey) > 0)
        {
            index--;
            byteIndexOfRecord -= sizeof(CatalogDirectoryRecord);
            record = (CatalogDirectoryRecord*)&nodeToInsertRecordIntoData[byteIndexOfRecord];
        }

        //read the found child node from disk
        uint32_t startingByteOfChildNodeInfo = getCatalogFileNodeSize() - (index + 2) * sizeof(ChildNodeInfo);
        ChildNodeInfo *childNodeInfo = (ChildNodeInfo*) &nodeToInsertRecordIntoData[startingByteOfChildNodeInfo];

        char* foundChildNodeData = new char[getCatalogFileNodeSize()];
        readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, foundChildNodeData, childNodeInfo->nodeNumber);

        if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
        {
            delete[] nodeToInsertRecordIntoData, delete[] foundChildNodeData;
            return CF_INSERT_NON_FULL_FAILED;
        }

        BTNodeDescriptor* foundChildNodeDescriptor = (BTNodeDescriptor*)&foundChildNodeData[0];

        //see if the found child is full
        if(foundChildNodeDescriptor->numRecords == getMaximumNumberOfRecordsPerCatalogFileNode())
        {
            //if the child is full then split it
            uint32_t splitChildResult = cf_splitChild(diskInfo, volumeHeader, catalogFileHeaderNode, nodeNumberToInsertRecordInto,
                                                   childNodeInfo->nodeNumber, index + 1);

            if(splitChildResult == CF_SPLIT_CHILD_FAILED)
            {
                delete[] nodeToInsertRecordIntoData, delete[] foundChildNodeData;
                return CF_INSERT_NON_FULL_FAILED;
            }

            //we read the node again from disk because it got changed on split
            readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, nodeToInsertRecordIntoData, nodeNumberToInsertRecordInto);

            if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
            {
                delete[] nodeToInsertRecordIntoData;
                return CF_INSERT_NON_FULL_FAILED;
            }

            // after split, the middle record of childNodeInfo[i] goes up and childNodeInfo[i] is split into two. See which of the two is going to have the new record
            byteIndexOfRecord = sizeof(BTNodeDescriptor) + (index + 1) * sizeof(CatalogDirectoryRecord);
            record = (CatalogDirectoryRecord*)&nodeToInsertRecordIntoData[byteIndexOfRecord];

            if(cf_compareKeys(&record->catalogKey, &recordToInsert->catalogKey) < 0)
                index++;
        }

        startingByteOfChildNodeInfo = getCatalogFileNodeSize() - (index + 2) * sizeof(ChildNodeInfo);
        childNodeInfo = (ChildNodeInfo*) &nodeToInsertRecordIntoData[startingByteOfChildNodeInfo];
        uint32_t insertNonNullResult = cf_insertNonFull(diskInfo, volumeHeader, catalogFileHeaderNode, childNodeInfo->nodeNumber, recordToInsert);

        delete[] nodeToInsertRecordIntoData, delete[] foundChildNodeData;
        return (insertNonNullResult == CF_INSERT_NON_FULL_SUCCESS) ? CF_INSERT_NON_FULL_SUCCESS : CF_INSERT_NON_FULL_FAILED;
    }
}

/////REMOVE RECORD

static uint32_t cf_findKey(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, uint32_t nodeNumberOfRootOfSubtree,
                           CatalogDirectoryRecord* recordToFindGreaterThan, uint32_t& indexOfRecordInNode)
{
    indexOfRecordInNode = 0;
    //read root of subtree
    char* nodeData = new char[getCatalogFileNodeSize()];
    uint32_t readRootNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, nodeData, nodeNumberOfRootOfSubtree);

    if(readRootNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData;
        return CF_FIND_KEY_FAILED;
    }

    BTNodeDescriptor* nodeDescriptor = (BTNodeDescriptor*)&nodeData[0];
    uint32_t firstByteOfRecord = sizeof(BTNodeDescriptor);
    CatalogDirectoryRecord* record = (CatalogDirectoryRecord*)&nodeData[firstByteOfRecord];

    while(indexOfRecordInNode < nodeDescriptor->numRecords && cf_compareKeys(&record->catalogKey, &recordToFindGreaterThan->catalogKey) < 0)
    {
        indexOfRecordInNode++;
        firstByteOfRecord += sizeof(CatalogDirectoryRecord);
        record = (CatalogDirectoryRecord*)&nodeData[firstByteOfRecord];
    }

    delete[] nodeData;
    return CF_FIND_KEY_SUCCESS;
}

static uint32_t cf_remove(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, uint32_t nodeNumber,
                          CatalogDirectoryRecord* recordToRemove)
{
    uint32_t index;
    uint32_t findKeyResult = cf_findKey(diskInfo, volumeHeader, catalogFileHeaderNode, nodeNumber, recordToRemove, index);

    if(findKeyResult == CF_FIND_KEY_FAILED)
        return CF_REMOVE_FAILED;

    //the key to be removed is present in this node
    char* nodeData = new char[getCatalogFileNodeSize()];
    uint32_t readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, nodeData, nodeNumber);

    if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData;
        return CF_REMOVE_FAILED;
    }

    BTNodeDescriptor* nodeDescriptor = (BTNodeDescriptor*)&nodeData[0];
    uint32_t firstByteOfRecord = sizeof(BTNodeDescriptor) + index * sizeof(CatalogDirectoryRecord);
    CatalogDirectoryRecord* record = (CatalogDirectoryRecord*)&nodeData[firstByteOfRecord];

    if(index < nodeDescriptor->numRecords && cf_compareKeys(&record->catalogKey, & recordToRemove->catalogKey) == 0)
    {
        delete[] nodeData;

        if(nodeDescriptor->isLeaf == NODE_IS_LEAF)
        {
            uint32_t removeFromLeafResult = cf_removeFromLeaf(diskInfo, volumeHeader, catalogFileHeaderNode, nodeNumber, index);
            return (removeFromLeafResult == CF_REMOVE_FROM_LEAF_SUCCESS) ? CF_REMOVE_SUCCESS : CF_REMOVE_FAILED;
        }
        else
        {
            uint32_t removeFromNonLeafResult = cf_removeFromNonLeaf(diskInfo, volumeHeader, catalogFileHeaderNode, nodeNumber, index);
            return (removeFromNonLeafResult == CF_REMOVE_FROM_NON_LEAF_SUCCESS) ? CF_REMOVE_SUCCESS : CF_REMOVE_FAILED;
        }
    }
    else
    {
        //skip the part where key is leaf and don't exist because this can't happen in our case
        uint32_t halfOfTheMaxNumberOfRecordsPerNode = getMaximumNumberOfRecordsPerCatalogFileNode() / 2 + 1;
        bool flag = ((index == nodeDescriptor->numRecords) ? true : false);

        //read C[idx]
        uint32_t startingByteOfChildNodeInfo = getCatalogFileNodeSize() - (index + 1) * sizeof(ChildNodeInfo);
        ChildNodeInfo *childNodeInfo = (ChildNodeInfo*) &nodeData[startingByteOfChildNodeInfo];
        char* childNodeData = new char[getCatalogFileNodeSize()];
        readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, childNodeData, childNodeInfo->nodeNumber);

        if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
        {
            delete[] nodeData, delete[] childNodeData;
            return CF_REMOVE_FAILED;
        }

        uint32_t childNode_1 = childNodeInfo->nodeNumber;
        BTNodeDescriptor* childNodeDescriptor_1 = new BTNodeDescriptor();
        memcpy(childNodeDescriptor_1, childNodeData, sizeof(BTNodeDescriptor));

        if(childNodeDescriptor_1->numRecords < halfOfTheMaxNumberOfRecordsPerNode)
        {
            uint32_t fillResult = cf_fill(diskInfo, volumeHeader, catalogFileHeaderNode, nodeNumber, index);
            if(fillResult == CF_FILL_FAILED)
            {
                delete[] nodeData, delete[] childNodeData;
                return CF_REMOVE_FAILED;
            }
        }

        if(flag && index > nodeDescriptor->numRecords)
        {
            //read C[idx - 1]
            startingByteOfChildNodeInfo = getCatalogFileNodeSize() - index * sizeof(ChildNodeInfo);
            childNodeInfo = (ChildNodeInfo*) &nodeData[startingByteOfChildNodeInfo];
            readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, childNodeData, childNodeInfo->nodeNumber);

            if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
            {
                delete[] nodeData, delete[] childNodeData;
                return CF_REMOVE_FAILED;
            }

            uint32_t childNode_2 = childNodeInfo->nodeNumber;
            BTNodeDescriptor* childNodeDescriptor_2 = new BTNodeDescriptor();
            memcpy(childNodeDescriptor_2, childNodeData, sizeof(BTNodeDescriptor));
            delete[] nodeData, delete[] childNodeData;

            return cf_remove(diskInfo, volumeHeader, catalogFileHeaderNode, childNode_2, recordToRemove);
        }
        else
        {
            delete[] nodeData, delete[] childNodeData;

            return cf_remove(diskInfo, volumeHeader, catalogFileHeaderNode, childNode_1, recordToRemove);
        }
    }
}

static uint32_t cf_removeFromLeaf(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, uint32_t nodeNumber,
                                  uint32_t indexInNodeOfRecordToRemove)
{
    char* nodeData = new char[getCatalogFileNodeSize()];
    uint32_t readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, nodeData, nodeNumber);

    if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData;
        return CF_REMOVE_FROM_LEAF_FAILED;
    }

    BTNodeDescriptor* nodeDescriptor = (BTNodeDescriptor*)&nodeData[0];
    uint32_t firstByteOfRecord = sizeof(BTNodeDescriptor);
    CatalogDirectoryRecord* record = (CatalogDirectoryRecord*)&nodeData[firstByteOfRecord];

    for(uint32_t i = indexInNodeOfRecordToRemove + 1; i < nodeDescriptor->numRecords; ++i)
    {
        firstByteOfRecord = sizeof(BTNodeDescriptor) + i * sizeof(CatalogDirectoryRecord);
        CatalogDirectoryRecord *recordToInsert = (CatalogDirectoryRecord*) &nodeData[firstByteOfRecord];
        uint32_t insertRecordInNodeResult = cf_insertRecordInNode(diskInfo, volumeHeader, recordToInsert, nodeNumber, i - 1,false);

        if (insertRecordInNodeResult == CF_INSERT_RECORD_IN_NODE_FAILED)
        {
            delete[] nodeData;
            return CF_REMOVE_FROM_LEAF_FAILED;
        }
    }

    nodeDescriptor->numRecords--;
    cf_updateNodeOnDisk(diskInfo, volumeHeader, nodeData, nodeNumber);

    delete[] nodeData;
    return CF_REMOVE_FROM_LEAF_SUCCESS;
}

static uint32_t cf_removeFromNonLeaf(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, uint32_t nodeNumber, uint32_t index)
{
    uint32_t halfOfTheMaxNumberOfRecordsPerNode = getMaximumNumberOfRecordsPerCatalogFileNode() / 2 + 1; //t (maximum is always odd so it will actually be the great half for 5 will be 3)

    char* nodeData = new char[getCatalogFileNodeSize()];
    uint32_t readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, nodeData, nodeNumber);

    if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData;
        return CF_REMOVE_FROM_NON_LEAF_FAILED;
    }

    //make k = keys[idx];
    BTNodeDescriptor* nodeDescriptor = (BTNodeDescriptor*)&nodeData[0];
    uint32_t firstByteOfRecord = sizeof(BTNodeDescriptor) + index * sizeof(CatalogDirectoryRecord);
    CatalogDirectoryRecord* recordToRemove = (CatalogDirectoryRecord*)&nodeData[firstByteOfRecord];

    //read C[idx]
    uint32_t startingByteOfChildNodeInfo = getCatalogFileNodeSize() - (index + 1) * sizeof(ChildNodeInfo);
    ChildNodeInfo *childNodeInfo = (ChildNodeInfo*) &nodeData[startingByteOfChildNodeInfo];
    char* childNodeData = new char[getCatalogFileNodeSize()];
    readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, childNodeData, childNodeInfo->nodeNumber);

    if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData, delete[] childNodeData;
        return CF_REMOVE_FROM_NON_LEAF_FAILED;
    }

    uint32_t childNode_1 = childNodeInfo->nodeNumber;
    BTNodeDescriptor* childNodeDescriptor_1 = new BTNodeDescriptor();
    memcpy(childNodeDescriptor_1, childNodeData, sizeof(BTNodeDescriptor));

    //read C[idx + 1]
    startingByteOfChildNodeInfo = getCatalogFileNodeSize() - (index + 2) * sizeof(ChildNodeInfo);
    childNodeInfo = (ChildNodeInfo*) &nodeData[startingByteOfChildNodeInfo];
    readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, childNodeData, childNodeInfo->nodeNumber);

    if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData, delete[] childNodeData;
        return CF_REMOVE_FROM_NON_LEAF_FAILED;
    }

    uint32_t childNode_2 = childNodeInfo->nodeNumber;
    BTNodeDescriptor* childNodeDescriptor_2 = new BTNodeDescriptor();
    memcpy(childNodeDescriptor_2, childNodeData, sizeof(BTNodeDescriptor));

    //if child that precedes record has .... delete .....
    if(childNodeDescriptor_1->numRecords >= halfOfTheMaxNumberOfRecordsPerNode)
    {
        delete[] nodeData, delete[] childNodeData;

        CatalogDirectoryRecord* predRecord = new CatalogDirectoryRecord();
        uint32_t getPredResult = cf_getPred(diskInfo, volumeHeader, catalogFileHeaderNode, nodeNumber, index, predRecord);
        if(getPredResult == CF_GET_PRED_FAILED)
            return CF_REMOVE_FROM_NON_LEAF_FAILED;

        uint32_t insertRecordInNodeResult = cf_insertRecordInNode(diskInfo, volumeHeader, predRecord, nodeNumber, index,
                                                                  false);

        if (insertRecordInNodeResult == CF_INSERT_RECORD_IN_NODE_FAILED)
            return CF_REMOVE_FROM_NON_LEAF_FAILED;

        uint32_t removeResult = cf_remove(diskInfo, volumeHeader, catalogFileHeaderNode, childNode_1, predRecord);
        return (removeResult == CF_REMOVE_SUCCESS) ? CF_REMOVE_FROM_NON_LEAF_SUCCESS : CF_REMOVE_FROM_NON_LEAF_FAILED;
    }
    else if(childNodeDescriptor_2->numRecords >= halfOfTheMaxNumberOfRecordsPerNode)
    {
        delete[] nodeData, delete[] childNodeData;

        CatalogDirectoryRecord* succRecord = new CatalogDirectoryRecord();
        uint32_t getSuccResult = cf_getSucc(diskInfo, volumeHeader, catalogFileHeaderNode, nodeNumber, index, succRecord);
        if(getSuccResult == CF_GET_PRED_FAILED)
            return CF_REMOVE_FROM_NON_LEAF_FAILED;

        uint32_t insertRecordInNodeResult = cf_insertRecordInNode(diskInfo, volumeHeader, succRecord, nodeNumber, index,
                                                                  false);

        if (insertRecordInNodeResult == CF_INSERT_RECORD_IN_NODE_FAILED)
            return CF_REMOVE_FROM_NON_LEAF_FAILED;

        uint32_t removeResult = cf_remove(diskInfo, volumeHeader, catalogFileHeaderNode, childNode_2, succRecord);
        return (removeResult == CF_REMOVE_SUCCESS) ? CF_REMOVE_FROM_NON_LEAF_SUCCESS : CF_REMOVE_FROM_NON_LEAF_FAILED;
    }
    else
    {
        delete[] nodeData, delete[] childNodeData;

        uint32_t mergeResult = cf_merge(diskInfo, volumeHeader, catalogFileHeaderNode, nodeNumber, index);
        if(mergeResult == CF_MERGE_FAILED)
            return CF_REMOVE_FROM_NON_LEAF_FAILED;

        uint32_t removeResult = cf_remove(diskInfo, volumeHeader, catalogFileHeaderNode, childNode_1, recordToRemove);
        return (removeResult == CF_REMOVE_SUCCESS) ? CF_REMOVE_FROM_NON_LEAF_SUCCESS : CF_REMOVE_FROM_NON_LEAF_FAILED;
    }
}

static uint32_t cf_getPred(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, uint32_t nodeNumber, uint32_t index,
                 CatalogDirectoryRecord* predRecord)
{
    char* nodeData = new char[getCatalogFileNodeSize()];
    uint32_t readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, nodeData, nodeNumber);

    if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData;
        return CF_GET_PRED_FAILED;
    }

    BTNodeDescriptor* nodeDescriptor = (BTNodeDescriptor*)&nodeData[0];

    //keep moving to rightmost node until reach a leaf
    uint32_t startingByteOfChildNodeInfo = getCatalogFileNodeSize() - (index + 1) * sizeof(ChildNodeInfo);
    ChildNodeInfo *childNodeInfo = (ChildNodeInfo*) &nodeData[startingByteOfChildNodeInfo];
    char* childNodeData = new char[getCatalogFileNodeSize()];
    readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, childNodeData, childNodeInfo->nodeNumber);

    if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData, delete[] childNodeData;
        return CF_GET_PRED_FAILED;
    }

    BTNodeDescriptor* childNodeDescriptor = (BTNodeDescriptor*)&childNodeData[0];
    while(childNodeDescriptor->isLeaf == NODE_IS_NOT_LEAF)
    {
        startingByteOfChildNodeInfo = getCatalogFileNodeSize() - (childNodeDescriptor->numRecords + 1) * sizeof(ChildNodeInfo);
        childNodeInfo = (ChildNodeInfo*) &childNodeData[startingByteOfChildNodeInfo];
        readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, childNodeData, childNodeInfo->nodeNumber);

        if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
        {
            delete[] nodeData, delete[] childNodeData;
            return CF_GET_PRED_FAILED;
        }

        childNodeDescriptor = (BTNodeDescriptor*)&childNodeData[0];
    }

    //return last record of leaf
    uint32_t startingByteOfRecord = sizeof(BTNodeDescriptor) + (childNodeDescriptor->numRecords - 1) * sizeof(CatalogDirectoryRecord);
    predRecord = (CatalogDirectoryRecord*) &childNodeData[startingByteOfRecord];

    delete[] nodeData, delete[] childNodeData;
    return CF_GET_PRED_SUCCESS;
}

static uint32_t cf_getSucc(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, uint32_t nodeNumber, uint32_t index,
                 CatalogDirectoryRecord* succRecord)
{
    char* nodeData = new char[getCatalogFileNodeSize()];
    uint32_t readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, nodeData, nodeNumber);

    if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData;
        return CF_GET_SUCC_FAILED;
    }

    BTNodeDescriptor* nodeDescriptor = (BTNodeDescriptor*)&nodeData[0];

    //keep moving to leftmost node until reach a leaf
    ChildNodeInfo *childNodeInfo = (ChildNodeInfo*) &nodeData[0];
    char* childNodeData = new char[getCatalogFileNodeSize()];
    readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, childNodeData, childNodeInfo->nodeNumber);

    if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData, delete[] childNodeData;
        return CF_GET_SUCC_FAILED;
    }

    BTNodeDescriptor* childNodeDescriptor = (BTNodeDescriptor*)&childNodeData[0];
    while(childNodeDescriptor->isLeaf == NODE_IS_NOT_LEAF)
    {
        childNodeInfo = (ChildNodeInfo*) &childNodeData[0];
        readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, childNodeData, childNodeInfo->nodeNumber);

        if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
        {
            delete[] nodeData, delete[] childNodeData;
            return CF_GET_SUCC_FAILED;
        }

        childNodeDescriptor = (BTNodeDescriptor*)&childNodeData[0];
    }

    //return first record of leaf
    succRecord = (CatalogDirectoryRecord*) &childNodeData[0];

    delete[] nodeData, delete[] childNodeData;
    return CF_GET_SUCC_SUCCESS;
}

static uint32_t cf_fill(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, uint32_t nodeNumber, uint32_t index)
{
    uint32_t halfOfTheMaxNumberOfRecordsPerNode = getMaximumNumberOfRecordsPerCatalogFileNode() / 2 + 1; //t (maximum is always odd so it will actually be the great half for 5 will be 3)

    char* nodeData = new char[getCatalogFileNodeSize()];
    uint32_t readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, nodeData, nodeNumber);

    if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData;
        return CF_FILL_FAILED;
    }

    BTNodeDescriptor* nodeDescriptor = (BTNodeDescriptor*)&nodeData[0];

    //read C[idx-1]
    uint32_t startingByteOfChildNodeInfo = getCatalogFileNodeSize() - index * sizeof(ChildNodeInfo);
    ChildNodeInfo *childNodeInfo = (ChildNodeInfo*) &nodeData[startingByteOfChildNodeInfo];
    char* childNodeData = new char[getCatalogFileNodeSize()];
    readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, childNodeData, childNodeInfo->nodeNumber);

    if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData, delete[] childNodeData;
        return CF_FILL_FAILED;
    }

    BTNodeDescriptor* childNodeDescriptor_1 = new BTNodeDescriptor();
    memcpy(childNodeDescriptor_1, childNodeData, sizeof(BTNodeDescriptor));

    //read C[idx+1]
    startingByteOfChildNodeInfo = getCatalogFileNodeSize() - (index + 2) * sizeof(ChildNodeInfo);
    childNodeInfo = (ChildNodeInfo*) &nodeData[startingByteOfChildNodeInfo];
    readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, childNodeData, childNodeInfo->nodeNumber);

    if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData, delete[] childNodeData;
        return CF_FILL_FAILED;
    }

    BTNodeDescriptor* childNodeDescriptor_2 = new BTNodeDescriptor();
    memcpy(childNodeDescriptor_2, childNodeData, sizeof(BTNodeDescriptor));

    //If the previous child(C[idx-1]) has more than t-1 keys, borrow a key from that child
    if(index != 0 && childNodeDescriptor_1->numRecords >= halfOfTheMaxNumberOfRecordsPerNode)
    {
        uint32_t borrowFromPrevResult = cf_borrowFromPrev(diskInfo, volumeHeader, catalogFileHeaderNode, nodeNumber, index);
        delete[] nodeData, delete[] childNodeData;

        return (borrowFromPrevResult == CF_BORROW_FROM_PREV_SUCCESS) ? CF_FILL_SUCCESS : CF_FILL_FAILED;
    }
    else if(index != nodeDescriptor->numRecords && childNodeDescriptor_2->numRecords >= halfOfTheMaxNumberOfRecordsPerNode)
    {
        uint32_t borrowFromNextResult = cf_borrowFromNext(diskInfo, volumeHeader, catalogFileHeaderNode, nodeNumber, index);
        delete[] nodeData, delete[] childNodeData;

        return (borrowFromNextResult == CF_BORROW_FROM_NEXT_SUCCESS) ? CF_FILL_SUCCESS : CF_FILL_FAILED;
    }
    else
    {
        uint32_t mergeResult;

        if(index != nodeDescriptor->numRecords)
            mergeResult = cf_merge(diskInfo, volumeHeader, catalogFileHeaderNode, nodeNumber, index);
        else
            mergeResult = cf_merge(diskInfo, volumeHeader, catalogFileHeaderNode, nodeNumber, index - 1);

        return(mergeResult == CF_MERGE_SUCCESS) ? CF_FILL_SUCCESS : CF_FILL_FAILED;
    }
}

static uint32_t cf_borrowFromPrev(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, uint32_t nodeNumber, uint32_t index)
{
    char* nodeData = new char[getCatalogFileNodeSize()];
    uint32_t readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, nodeData, nodeNumber);

    if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData;
        return CF_BORROW_FROM_PREV_FAILED;
    }

    //get child (c[idx])
    BTNodeDescriptor* nodeDescriptor = (BTNodeDescriptor*)&nodeData[0];

    uint32_t startingByteOfChildNodeInfo = getCatalogFileNodeSize() - (index + 1) * sizeof(ChildNodeInfo);
    ChildNodeInfo *childNodeInfo = (ChildNodeInfo*) &nodeData[startingByteOfChildNodeInfo];
    char* childNodeData = new char[getCatalogFileNodeSize()];
    readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, childNodeData, childNodeInfo->nodeNumber);

    if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData, delete[] childNodeData;
        return CF_BORROW_FROM_PREV_FAILED;
    }

    BTNodeDescriptor* childNodeDescriptor = (BTNodeDescriptor*)&childNodeData[0];

    //get sibling (c[idx - 1])
    startingByteOfChildNodeInfo = getCatalogFileNodeSize() - index * sizeof(ChildNodeInfo);
    ChildNodeInfo *siblingNodeInfo = (ChildNodeInfo*) &nodeData[startingByteOfChildNodeInfo];
    char* siblingNodeData = new char[getCatalogFileNodeSize()];
    readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, siblingNodeData, siblingNodeInfo->nodeNumber);

    if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData, delete[] childNodeData, delete[] siblingNodeData;
        return CF_BORROW_FROM_PREV_FAILED;
    }

    BTNodeDescriptor* siblingNodeDescriptor = (BTNodeDescriptor*)&siblingNodeData[0];

    //move all keys in c[idx] one step ahead
    for(uint32_t i = childNodeDescriptor->numRecords - 1; i >= 0; --i)
    {
        uint32_t firstByteOfRecord = sizeof(BTNodeDescriptor) + i * sizeof(CatalogDirectoryRecord);
        CatalogDirectoryRecord *record = (CatalogDirectoryRecord*) &childNodeData[firstByteOfRecord];
        uint32_t insertRecordInNodeResult = cf_insertRecordInNode(diskInfo, volumeHeader, record, childNodeInfo->nodeNumber, i + 1,
                                                                  false);

        if (insertRecordInNodeResult == CF_INSERT_RECORD_IN_NODE_FAILED)
        {
            delete[] nodeData, delete[] childNodeData, delete[] siblingNodeData;
            return CF_BORROW_FROM_PREV_FAILED;
        }
    }

    //if C[idx] is not a leaf, move all its child pointers one step ahead
    if(childNodeDescriptor->isLeaf == NODE_IS_NOT_LEAF)
    {
        for(uint32_t i = childNodeDescriptor->numRecords; i >= 0; --i)
        {
            startingByteOfChildNodeInfo = getCatalogFileNodeSize() - (i + 1) * sizeof(ChildNodeInfo);
            ChildNodeInfo *childNodeInfoToInsert = (ChildNodeInfo*) &childNodeData[startingByteOfChildNodeInfo];
            uint32_t insertChildNodeInfoResult = cf_insertChildNodeInfoInNode(diskInfo, volumeHeader, childNodeInfoToInsert, childNodeInfo->nodeNumber, i + 1);

            if (insertChildNodeInfoResult == CF_INSERT_CHILD_NODE_INFO_IN_NODE_FAILED) {
                delete[] nodeData, delete[] childNodeData, delete[] siblingNodeData;
                return CF_BORROW_FROM_PREV_FAILED;
            }
        }
    }

    //setting child's first key equal to keys[idx-1] from the current node
    uint32_t firstByteOfRecord = sizeof(BTNodeDescriptor) + (index - 1) * sizeof(CatalogDirectoryRecord);
    CatalogDirectoryRecord *record = (CatalogDirectoryRecord*) &nodeData[firstByteOfRecord];
    uint32_t insertRecordInNodeResult = cf_insertRecordInNode(diskInfo, volumeHeader, record, childNodeInfo->nodeNumber, 0,false);

    if (insertRecordInNodeResult == CF_INSERT_RECORD_IN_NODE_FAILED)
    {
        delete[] nodeData, delete[] childNodeData, delete[] siblingNodeData;
        return CF_BORROW_FROM_PREV_FAILED;
    }

    // Moving sibling's last child as C[idx]'s first child
    if(childNodeDescriptor->isLeaf == NODE_IS_NOT_LEAF)
    {
        startingByteOfChildNodeInfo = getCatalogFileNodeSize() - (siblingNodeDescriptor->numRecords + 1) * sizeof(ChildNodeInfo);
        ChildNodeInfo *childNodeInfoToInsert = (ChildNodeInfo*) &siblingNodeData[startingByteOfChildNodeInfo];
        uint32_t insertChildNodeInfoResult = cf_insertChildNodeInfoInNode(diskInfo, volumeHeader, childNodeInfoToInsert, childNodeInfo->nodeNumber, 0);

        if (insertChildNodeInfoResult == CF_INSERT_CHILD_NODE_INFO_IN_NODE_FAILED) {
            delete[] nodeData, delete[] childNodeData, delete[] siblingNodeData;
            return CF_BORROW_FROM_PREV_FAILED;
        }
    }

    //moving the key from the sibling to the parent; this reduces the number of keys in the sibling
    firstByteOfRecord = sizeof(BTNodeDescriptor) + (siblingNodeDescriptor->numRecords - 1) * sizeof(CatalogDirectoryRecord);
    record = (CatalogDirectoryRecord*) &siblingNodeData[firstByteOfRecord];
    insertRecordInNodeResult = cf_insertRecordInNode(diskInfo, volumeHeader, record, nodeNumber, index - 1,false);

    if (insertRecordInNodeResult == CF_INSERT_RECORD_IN_NODE_FAILED)
    {
        delete[] nodeData, delete[] childNodeData, delete[] siblingNodeData;
        return CF_BORROW_FROM_PREV_FAILED;
    }

    childNodeDescriptor->numRecords++;
    cf_updateNodeOnDisk(diskInfo, volumeHeader, childNodeData, childNodeInfo->nodeNumber);

    siblingNodeDescriptor->numRecords--;
    cf_updateNodeOnDisk(diskInfo, volumeHeader, siblingNodeData, siblingNodeInfo->nodeNumber);

    delete[] nodeData, delete[] childNodeData, delete[] siblingNodeData;
    return CF_BORROW_FROM_PREV_SUCCESS;
}

static uint32_t cf_borrowFromNext(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, uint32_t nodeNumber, uint32_t index)
{
    char* nodeData = new char[getCatalogFileNodeSize()];
    uint32_t readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, nodeData, nodeNumber);

    if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData;
        return CF_BORROW_FROM_NEXT_FAILED;
    }

    //get child (c[idx])
    BTNodeDescriptor* nodeDescriptor = (BTNodeDescriptor*)&nodeData[0];

    uint32_t startingByteOfChildNodeInfo = getCatalogFileNodeSize() - (index + 1) * sizeof(ChildNodeInfo);
    ChildNodeInfo *childNodeInfo = (ChildNodeInfo*) &nodeData[startingByteOfChildNodeInfo];
    char* childNodeData = new char[getCatalogFileNodeSize()];
    readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, childNodeData, childNodeInfo->nodeNumber);

    if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData, delete[] childNodeData;
        return CF_BORROW_FROM_NEXT_FAILED;
    }

    BTNodeDescriptor* childNodeDescriptor = (BTNodeDescriptor*)&childNodeData[0];

    //get sibling (c[idx - 1])
    startingByteOfChildNodeInfo = getCatalogFileNodeSize() - (index + 2) * sizeof(ChildNodeInfo);
    ChildNodeInfo *siblingNodeInfo = (ChildNodeInfo*) &nodeData[startingByteOfChildNodeInfo];
    char* siblingNodeData = new char[getCatalogFileNodeSize()];
    readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, siblingNodeData, siblingNodeInfo->nodeNumber);

    if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData, delete[] childNodeData, delete[] siblingNodeData;
        return CF_BORROW_FROM_NEXT_FAILED;
    }

    BTNodeDescriptor* siblingNodeDescriptor = (BTNodeDescriptor*)&siblingNodeData[0];

    //keys[idx] is inserted as the last key in C[idx]
    uint32_t firstByteOfRecord = sizeof(BTNodeDescriptor) + index * sizeof(CatalogDirectoryRecord);
    CatalogDirectoryRecord *record = (CatalogDirectoryRecord*) &nodeData[firstByteOfRecord];
    uint32_t insertRecordInNodeResult = cf_insertRecordInNode(diskInfo, volumeHeader, record, childNodeInfo->nodeNumber,
                                                              childNodeDescriptor->numRecords,false);

    if (insertRecordInNodeResult == CF_INSERT_RECORD_IN_NODE_FAILED)
    {
        delete[] nodeData, delete[] childNodeData, delete[] siblingNodeData;
        return CF_BORROW_FROM_NEXT_FAILED;
    }

    //sibling's first child is inserted as the last child into C[idx]
    if(childNodeDescriptor->isLeaf == NODE_IS_NOT_LEAF)
    {
        startingByteOfChildNodeInfo = getCatalogFileNodeSize() - sizeof(ChildNodeInfo);
        ChildNodeInfo *childNodeInfoToInsert = (ChildNodeInfo*) &siblingNodeData[startingByteOfChildNodeInfo];
        uint32_t insertChildNodeInfoResult = cf_insertChildNodeInfoInNode(diskInfo, volumeHeader, childNodeInfoToInsert, childNodeInfo->nodeNumber,
                                                                          childNodeDescriptor->numRecords + 1);

        if (insertChildNodeInfoResult == CF_INSERT_CHILD_NODE_INFO_IN_NODE_FAILED) {
            delete[] nodeData, delete[] childNodeData, delete[] siblingNodeData;
            return CF_BORROW_FROM_NEXT_FAILED;
        }
    }

    //The first key from sibling is inserted into keys[idx]
    record = (CatalogDirectoryRecord*) &siblingNodeData[sizeof(BTNodeDescriptor)];
    insertRecordInNodeResult = cf_insertRecordInNode(diskInfo, volumeHeader, record, nodeNumber, index,false);

    if (insertRecordInNodeResult == CF_INSERT_RECORD_IN_NODE_FAILED)
    {
        delete[] nodeData, delete[] childNodeData, delete[] siblingNodeData;
        return CF_BORROW_FROM_NEXT_FAILED;
    }

    //moving all keys in sibling one step behind
    for(uint32_t i = 1; i < siblingNodeDescriptor->numRecords; ++i)
    {
        firstByteOfRecord = sizeof(BTNodeDescriptor) + i * sizeof(CatalogDirectoryRecord);
        record = (CatalogDirectoryRecord*) &siblingNodeData[firstByteOfRecord];
        insertRecordInNodeResult = cf_insertRecordInNode(diskInfo, volumeHeader, record, siblingNodeInfo->nodeNumber, i - 1,
                                                         false);

        if (insertRecordInNodeResult == CF_INSERT_RECORD_IN_NODE_FAILED)
        {
            delete[] nodeData, delete[] childNodeData, delete[] siblingNodeData;
            return CF_BORROW_FROM_NEXT_FAILED;
        }
    }

    //moving the child pointers one step behind
    if(siblingNodeDescriptor->isLeaf == NODE_IS_NOT_LEAF)
    {
        for(uint32_t i = 1; i <= siblingNodeDescriptor->numRecords; ++i)
        {
            startingByteOfChildNodeInfo = getCatalogFileNodeSize() - (i + 1) * sizeof(ChildNodeInfo);
            ChildNodeInfo *childNodeInfoToInsert = (ChildNodeInfo*) &siblingNodeData[startingByteOfChildNodeInfo];
            uint32_t insertChildNodeInfoResult = cf_insertChildNodeInfoInNode(diskInfo, volumeHeader, childNodeInfoToInsert, siblingNodeInfo->nodeNumber,
                                                                              i - 1);

            if (insertChildNodeInfoResult == CF_INSERT_CHILD_NODE_INFO_IN_NODE_FAILED) {
                delete[] nodeData, delete[] childNodeData, delete[] siblingNodeData;
                return CF_BORROW_FROM_NEXT_FAILED;
            }
        }
    }

    childNodeDescriptor->numRecords++;
    cf_updateNodeOnDisk(diskInfo, volumeHeader, childNodeData, childNodeInfo->nodeNumber);

    siblingNodeDescriptor->numRecords--;
    cf_updateNodeOnDisk(diskInfo, volumeHeader, siblingNodeData, siblingNodeInfo->nodeNumber);

    delete[] nodeData, delete[] childNodeData, delete[] siblingNodeData;
    return CF_BORROW_FROM_NEXT_SUCCESS;
}

static uint32_t cf_merge(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, uint32_t nodeNumber, uint32_t index)
{
    uint32_t halfOfTheMaxNumberOfRecordsPerNode = getMaximumNumberOfRecordsPerCatalogFileNode() / 2 + 1; //t (maximum is always odd so it will actually be the great half for 5 will be 3)

    char* nodeData = new char[getCatalogFileNodeSize()];
    uint32_t readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, nodeData, nodeNumber);

    if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData;
        return CF_MERGE_FAILED;
    }

    //get child (c[idx])
    BTNodeDescriptor* nodeDescriptor = (BTNodeDescriptor*)&nodeData[0];

    uint32_t startingByteOfChildNodeInfo = getCatalogFileNodeSize() - (index + 1) * sizeof(ChildNodeInfo);
    ChildNodeInfo *childNodeInfo = (ChildNodeInfo*) &nodeData[startingByteOfChildNodeInfo];
    char* childNodeData = new char[getCatalogFileNodeSize()];
    readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, childNodeData, childNodeInfo->nodeNumber);

    if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData, delete[] childNodeData;
        return CF_MERGE_FAILED;
    }

    BTNodeDescriptor* childNodeDescriptor = (BTNodeDescriptor*)&childNodeData[0];

    //get sibling (c[idx - 1])
    startingByteOfChildNodeInfo = getCatalogFileNodeSize() - (index + 2) * sizeof(ChildNodeInfo);
    ChildNodeInfo *siblingNodeInfo = (ChildNodeInfo*) &nodeData[startingByteOfChildNodeInfo];
    char* siblingNodeData = new char[getCatalogFileNodeSize()];
    readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, siblingNodeData, siblingNodeInfo->nodeNumber);

    if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData, delete[] childNodeData, delete[] siblingNodeData;
        return CF_MERGE_FAILED;
    }

    BTNodeDescriptor* siblingNodeDescriptor = (BTNodeDescriptor*)&siblingNodeData[0];

    //pulling a key from the current node and inserting it into (t-1)th position of C[idx]
    //keys[idx] is inserted as the last key in C[idx]
    uint32_t firstByteOfRecord = sizeof(BTNodeDescriptor) + index * sizeof(CatalogDirectoryRecord);
    CatalogDirectoryRecord *record = (CatalogDirectoryRecord*) &nodeData[firstByteOfRecord];
    uint32_t insertRecordInNodeResult = cf_insertRecordInNode(diskInfo, volumeHeader, record, childNodeInfo->nodeNumber,
                                                              halfOfTheMaxNumberOfRecordsPerNode - 1,false);

    if (insertRecordInNodeResult == CF_INSERT_RECORD_IN_NODE_FAILED)
    {
        delete[] nodeData, delete[] childNodeData, delete[] siblingNodeData;
        return CF_MERGE_FAILED;
    }

    //copying the keys from C[idx+1] to C[idx] at the end
    for(uint32_t i = 0; i < siblingNodeDescriptor->numRecords; ++i)
    {
        firstByteOfRecord = sizeof(BTNodeDescriptor) + i * sizeof(CatalogDirectoryRecord);
        record = (CatalogDirectoryRecord*) &siblingNodeData[firstByteOfRecord];
        insertRecordInNodeResult = cf_insertRecordInNode(diskInfo, volumeHeader, record, childNodeInfo->nodeNumber,
                                                         i + halfOfTheMaxNumberOfRecordsPerNode, false);

        if (insertRecordInNodeResult == CF_INSERT_RECORD_IN_NODE_FAILED)
        {
            delete[] nodeData, delete[] childNodeData, delete[] siblingNodeData;
            return CF_MERGE_FAILED;
        }
    }

    // Copying the child pointers from C[idx+1] to C[idx]
    if(siblingNodeDescriptor->isLeaf == NODE_IS_NOT_LEAF)
    {
        for(uint32_t i = 0; i <= siblingNodeDescriptor->numRecords; ++i)
        {
            startingByteOfChildNodeInfo = getCatalogFileNodeSize() - (i + 1) * sizeof(ChildNodeInfo);
            ChildNodeInfo *childNodeInfoToInsert = (ChildNodeInfo*) &siblingNodeData[startingByteOfChildNodeInfo];
            uint32_t insertChildNodeInfoResult = cf_insertChildNodeInfoInNode(diskInfo, volumeHeader, childNodeInfoToInsert, childNodeInfo->nodeNumber,
                                                                              i + halfOfTheMaxNumberOfRecordsPerNode);

            if (insertChildNodeInfoResult == CF_INSERT_CHILD_NODE_INFO_IN_NODE_FAILED) {
                delete[] nodeData, delete[] childNodeData, delete[] siblingNodeData;
                return CF_MERGE_FAILED;
            }
        }
    }

    //moving all keys after idx in the current node one step before - to fill the gap created by moving keys[idx] to C[idx]
    for(uint32_t i = index + 1; i < nodeDescriptor->numRecords; ++i)
    {
        firstByteOfRecord = sizeof(BTNodeDescriptor) + i * sizeof(CatalogDirectoryRecord);
        record = (CatalogDirectoryRecord*) &nodeData[firstByteOfRecord];
        insertRecordInNodeResult = cf_insertRecordInNode(diskInfo, volumeHeader, record, nodeNumber, i - 1, false);

        if (insertRecordInNodeResult == CF_INSERT_RECORD_IN_NODE_FAILED)
        {
            delete[] nodeData, delete[] childNodeData, delete[] siblingNodeData;
            return CF_MERGE_FAILED;
        }
    }

    //moving the child pointers after (idx+1) in the current node one step before
    for(uint32_t i = index + 2; i <= nodeDescriptor->numRecords; ++i)
    {
        startingByteOfChildNodeInfo = getCatalogFileNodeSize() - (i + 1) * sizeof(ChildNodeInfo);
        ChildNodeInfo *childNodeInfoToInsert = (ChildNodeInfo*) &nodeData[startingByteOfChildNodeInfo];
        uint32_t insertChildNodeInfoResult = cf_insertChildNodeInfoInNode(diskInfo, volumeHeader, childNodeInfoToInsert, nodeNumber, i - 1);

        if (insertChildNodeInfoResult == CF_INSERT_CHILD_NODE_INFO_IN_NODE_FAILED) {
            delete[] nodeData, delete[] childNodeData, delete[] siblingNodeData;
            return CF_MERGE_FAILED;
        }
    }

    childNodeDescriptor->numRecords += siblingNodeDescriptor->numRecords + 1;
    cf_updateNodeOnDisk(diskInfo, volumeHeader, childNodeData, childNodeInfo->nodeNumber);

    nodeDescriptor->numRecords--;
    cf_updateNodeOnDisk(diskInfo, volumeHeader, nodeData, nodeNumber);

    delete[] nodeData, delete[] childNodeData, delete[] siblingNodeData;
    return CF_MERGE_SUCCESS;
}

///////////////////////////

uint32_t cf_createNodeOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, char* newNodeData,
                          uint32_t& newNodeNumber, uint16_t isLeaf)
{
    //the new node number = the number of nodes occupied. node numbers are indexed from 0; 1 is header node and we don't count it
    newNodeNumber = catalogFileHeaderNode->headerRecord.totalNodes - catalogFileHeaderNode->headerRecord.freeNodes - 1;
    uint32_t firstBlockForNode = cf_getFirstBlockForGivenNodeIndex(volumeHeader, newNodeNumber);
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

    uint32_t writeResult = writeDiskSectors(diskInfo, cf_getNumberOfSectorsPerNode(diskInfo, volumeHeader), firstSector, newNodeData,
                                            numberOfSectorsWritten);

    if(writeResult != EC_NO_ERROR)
    {
        delete newNodeDescriptor;
        return CF_CREATE_NODE_ON_DISK_FAILED;
    }

    //mark node as occupied in header node map record and decrease number of free nodes
    CatalogFileHeaderNode* updatedCatalogFileHeaderNode = cf_updateNodeOccupiedInHeaderNodeMapRecord(catalogFileHeaderNode, newNodeNumber, 1);
    updatedCatalogFileHeaderNode->headerRecord.freeNodes--;

    cf_updateCatalogHeaderNodeOnDisk(diskInfo, volumeHeader, updatedCatalogFileHeaderNode);
    memcpy(catalogFileHeaderNode, updatedCatalogFileHeaderNode, sizeof(CatalogFileHeaderNode));

    delete updatedCatalogFileHeaderNode, delete newNodeDescriptor;

    return CF_CREATE_NODE_ON_DISK_SUCCESS;
}

uint32_t cf_insertRecordInNode(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogDirectoryRecord* recordToInsert, uint32_t nodeNumber, uint32_t recordIndexInNode,
                               bool shouldIncreaseNumOfRecordsInNodeDescriptor)
{
    char* nodeData = new char[getCatalogFileNodeSize()];
    uint32_t readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, nodeData, nodeNumber);

    if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData;
        return CF_INSERT_RECORD_IN_NODE_FAILED;
    }

    BTNodeDescriptor* nodeDescriptor = (BTNodeDescriptor*)&nodeData[0];
    uint32_t byteIndexInNodeToInsertTheRecord = sizeof(BTNodeDescriptor) + recordIndexInNode * sizeof(CatalogDirectoryRecord);
    memcpy(nodeData + byteIndexInNodeToInsertTheRecord, recordToInsert, sizeof(CatalogDirectoryRecord));

    if(shouldIncreaseNumOfRecordsInNodeDescriptor == true)
        nodeDescriptor->numRecords++; //making this operation on the pointer also changes the value in nodeData buffer... HOPEFULLY

    cf_updateNodeOnDisk(diskInfo, volumeHeader, nodeData, nodeNumber);

    delete[] nodeData;
    return CF_INSERT_RECORD_IN_NODE_SUCCESS;
}

uint32_t cf_insertChildNodeInfoInNode(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ChildNodeInfo* childNodeInfoToInsert, uint32_t nodeNumber,
                                   uint32_t childNodeInfoIndexInNode)
{
    char* nodeData = new char[getCatalogFileNodeSize()];
    uint32_t readNodeFromDiskResult = cf_readNodeFromDisk(diskInfo, volumeHeader, nodeData, nodeNumber);

    if(readNodeFromDiskResult == CF_READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData;
        return CF_INSERT_CHILD_NODE_INFO_IN_NODE_FAILED;
    }

    uint32_t byteIndexInNodeToInsertChildInfo = getCatalogFileNodeSize() - (childNodeInfoIndexInNode + 1) * sizeof(ChildNodeInfo);
    memcpy(nodeData + byteIndexInNodeToInsertChildInfo, childNodeInfoToInsert, sizeof(ChildNodeInfo));

    cf_updateNodeOnDisk(diskInfo, volumeHeader, nodeData, nodeNumber);

    delete[] nodeData;
    return CF_INSERT_CHILD_NODE_INFO_IN_NODE_SUCCESS;
}

/////////////////////

uint32_t cf_readNodeFromDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, char* nodeBuffer, uint32_t nodeNumber)
{
    uint32_t numberOfSectorsRead;
    uint32_t firstBlockForNode = cf_getFirstBlockForGivenNodeIndex(volumeHeader, nodeNumber);
    uint32_t firstSector = getFirstSectorForGivenBlock(diskInfo, volumeHeader, firstBlockForNode);

    uint32_t readResult = readDiskSectors(diskInfo, cf_getNumberOfSectorsPerNode(diskInfo, volumeHeader), firstSector, nodeBuffer, numberOfSectorsRead);

    return (readResult == EC_NO_ERROR) ? CF_READ_NODE_FROM_DISK_SUCCESS : CF_READ_NODE_FROM_DISK_FAILED;
}

char* readNodeTEST(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, uint32_t nodeNumber)
{
    char* nodeBuffer = new char[getCatalogFileNodeSize()];
    uint32_t numberOfSectorsRead;
    uint32_t firstBlockForNode = cf_getFirstBlockForGivenNodeIndex(volumeHeader, nodeNumber);
    uint32_t firstSector = getFirstSectorForGivenBlock(diskInfo, volumeHeader, firstBlockForNode);

    readDiskSectors(diskInfo, cf_getNumberOfSectorsPerNode(diskInfo, volumeHeader), firstSector, nodeBuffer, numberOfSectorsRead);
    return nodeBuffer;
}