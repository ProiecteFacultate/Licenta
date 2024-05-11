#include "string.h"

#include "disk.h"
#include "diskCodes.h"
#include "../../include/structures.h"
#include "../../include/utils.h"
#include "../../include/hfsFunctionUtils.h"
#include "../../include/catalog_file/codes/bTreeResponseCodes.h"
#include "../../include/catalog_file/codes/catalogFileResponseCodes.h"
#include "../../include/codes/hfsAttributes.h"
#include "../../include/catalog_file/catalogFileUtils.h"
#include "../../include/catalog_file/catalogFileOperations.h"
#include "../../include/catalog_file/bTreeCatalog.h"

uint32_t searchRecordForGivenNodeDataAndSearchedKey(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, HFSPlusCatalogKey* searchedKey, char* nodeData,
                                                    CatalogDirectoryRecord* searchedRecord)
{
    BTNodeDescriptor* nodeDescriptor = (BTNodeDescriptor*)&nodeData[0];
    uint32_t recordFirstByte = sizeof(BTNodeDescriptor);
    CatalogDirectoryRecord* record = (CatalogDirectoryRecord*)&nodeData[recordFirstByte];

    uint32_t recordIndex = 0;
    while(recordIndex < nodeDescriptor->numRecords && compareKeys(searchedKey, &record->catalogKey) > 0)
    {
        recordIndex++;
        recordFirstByte += sizeof(CatalogDirectoryRecord);
        record = (CatalogDirectoryRecord*)&nodeData[recordFirstByte];
    }

    if(recordIndex < nodeDescriptor->numRecords && compareKeys(searchedKey, &record->catalogKey) == 0)
    {
        memcpy(searchedRecord, record, sizeof(CatalogDirectoryRecord));
        return SEARCH_RECORD_IN_GIVEN_DATA_SUCCESS;
    }

    if(nodeDescriptor->isLeaf == NODE_IS_LEAF) //if we haven't found the key and this is a leaf node it means the key doesn't exist in tree
        return SEARCH_RECORD_IN_GIVEN_DATA_KEY_DO_NOT_EXIST_IN_TREE;

    ChildNodeInfo* nextNodeInfo = (ChildNodeInfo*)&nodeData[getCatalogFileNodeSize() - (recordIndex + 1) * sizeof(ChildNodeInfo)];

    uint32_t readNodeFromDiskResult = readNodeFromDisk(diskInfo, volumeHeader, nodeData, nextNodeInfo->nodeNumber);
    if(readNodeFromDiskResult == READ_NODE_FROM_DISK_FAILED)
        return SEARCH_RECORD_IN_GIVEN_DATA_FAILED_FOR_OTHER_REASON;

    return searchRecordForGivenNodeDataAndSearchedKey(diskInfo, volumeHeader, searchedKey, nodeData, searchedRecord);
}

/////////////////////////////insert

uint32_t insertRecordInTree(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, CatalogDirectoryRecord* recordToInsert)
{
    uint32_t newNodeNumber;

    if(catalogFileHeaderNode->headerRecord.freeNodes == catalogFileHeaderNode->headerRecord.totalNodes - 1) //root node do not exist yet //TODO delete root creation from init
    {
        char* rootNodeData = new char[getCatalogFileNodeSize()];
        uint32_t createNodeOnDiskResult = createNodeOnDisk(diskInfo, volumeHeader, catalogFileHeaderNode, rootNodeData, newNodeNumber, NODE_IS_LEAF);
        delete[] rootNodeData;

        if(createNodeOnDiskResult == CREATE_NODE_ON_DISK_FAILED)
            return INSERT_RECORD_IN_TREE_FAILED;

        uint32_t insertFirstKeyInRootNodeResult = insertRecordInNode(diskInfo, volumeHeader, recordToInsert, 0, 0, true);

        return (insertFirstKeyInRootNodeResult == INSERT_RECORD_IN_NODE_SUCCESS) ? INSERT_RECORD_IN_TREE_SUCCESS : INSERT_RECORD_IN_TREE_FAILED;
    }
    else //root node already exists (tree is not empty)
    {
        //read root
        char* rootNodeData = new char[getCatalogFileNodeSize()];
        uint32_t readRootNodeFromDiskResult = readNodeFromDisk(diskInfo, volumeHeader, rootNodeData, catalogFileHeaderNode->headerRecord.rootNode);

        if(readRootNodeFromDiskResult == READ_NODE_FROM_DISK_FAILED)
        {
            delete[] rootNodeData;
            return INSERT_RECORD_IN_NODE_FAILED;
        }

        BTNodeDescriptor* rootNodeDescriptor = (BTNodeDescriptor*)&rootNodeData[0];

        if(rootNodeDescriptor->numRecords == getMaximumNumberOfRecordsPerCatalogFileNode())
        {
            //create new node which will be the new root
            char* newNodeData = new char[getCatalogFileNodeSize()];
            uint32_t createNodeOnDiskResult = createNodeOnDisk(diskInfo, volumeHeader, catalogFileHeaderNode, newNodeData, newNodeNumber, NODE_IS_NOT_LEAF);

            delete[] rootNodeData;
            if(createNodeOnDiskResult == CREATE_NODE_ON_DISK_FAILED)
            {
                delete[] newNodeData;
                return INSERT_RECORD_IN_TREE_FAILED;
            }

            //make old root the child of this new root
            ChildNodeInfo* childNodeInfoToInsert = new ChildNodeInfo();
            childNodeInfoToInsert->nodeNumber = catalogFileHeaderNode->headerRecord.rootNode;
            uint32_t insertChildNodeInfoResult = insertChildNodeInfoInNode(diskInfo, volumeHeader, childNodeInfoToInsert, newNodeNumber, 0);

            if(insertChildNodeInfoResult == INSERT_CHILD_NODE_INFO_IN_NODE_FAILED)
            {
                delete[] newNodeData;
                return INSERT_RECORD_IN_TREE_FAILED;
            }

            //split the old root and move 1 record to the new root
            uint32_t splitChildResult = splitChild(diskInfo, volumeHeader, catalogFileHeaderNode, newNodeNumber, catalogFileHeaderNode->headerRecord.rootNode, 0);

            if(splitChildResult == SPLIT_CHILD_FAILED)
            {
                delete[] newNodeData;
                return INSERT_RECORD_IN_TREE_FAILED;
            }

            //read the new node from disk after the split child
            uint32_t readNewNodeFromDiskAfterUpdateResult = readNodeFromDisk(diskInfo, volumeHeader, newNodeData, newNodeNumber);

            if(readNewNodeFromDiskAfterUpdateResult == READ_NODE_FROM_DISK_FAILED)
            {
                delete[] newNodeData;
                return INSERT_RECORD_IN_NODE_FAILED;
            }

            //new root has 2 children now; decide which of them will have the new record
            uint32_t i = 0;
            CatalogDirectoryRecord* newNodeFirstRecord = (CatalogDirectoryRecord*)&newNodeData[sizeof(BTNodeDescriptor)];
            if(compareKeys(&newNodeFirstRecord->catalogKey, &recordToInsert->catalogKey) < 0)
                i++;

            uint32_t byteIndexOfChildNodeInfo = getCatalogFileNodeSize() - (i + 1) * sizeof(ChildNodeInfo);
            ChildNodeInfo* childNodeInfo = (ChildNodeInfo*)&newNodeData[byteIndexOfChildNodeInfo];
            delete[] newNodeData;

            uint32_t insertNonNullResult = insertNonFull(diskInfo, volumeHeader, catalogFileHeaderNode, childNodeInfo->nodeNumber, recordToInsert);
            if(insertNonNullResult == INSERT_NON_FULL_FAILED)
                return INSERT_RECORD_IN_TREE_FAILED;

            //change root
            CatalogFileHeaderNode* updatedCatalogFileHeaderNode = new CatalogFileHeaderNode();
            memcpy(updatedCatalogFileHeaderNode, catalogFileHeaderNode, sizeof(CatalogFileHeaderNode));
            updatedCatalogFileHeaderNode->headerRecord.rootNode = newNodeNumber;

            updateHeaderNodeOnDisk(diskInfo, volumeHeader, updatedCatalogFileHeaderNode);
            memcpy(catalogFileHeaderNode, updatedCatalogFileHeaderNode, sizeof(CatalogFileHeaderNode));

            delete rootNodeDescriptor, delete childNodeInfoToInsert, delete newNodeFirstRecord, delete childNodeInfo, delete updatedCatalogFileHeaderNode;
            return INSERT_RECORD_IN_TREE_SUCCESS;
        }
        else //if root is not full
        {
            uint32_t insertNonNullResult = insertNonFull(diskInfo, volumeHeader, catalogFileHeaderNode, catalogFileHeaderNode->headerRecord.rootNode, recordToInsert);

            delete[] rootNodeData, delete rootNodeDescriptor;
            return (insertNonNullResult == INSERT_NON_FULL_SUCCESS) ? INSERT_RECORD_IN_TREE_SUCCESS : INSERT_RECORD_IN_TREE_FAILED;
        }
    }
}

static uint32_t splitChild(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, uint32_t nodeNumberToMoveOneRecordTo,
                    uint32_t nodeNumberToSplit, uint32_t indexToMoveTheRecordTo)
{
    uint32_t halfOfTheMaxNumberOfRecordsPerNode = getMaximumNumberOfRecordsPerCatalogFileNode() / 2 + 1; //t (maximum is always odd so it will actually be the great half for 5 will be 3)
    //read the 2 existing nodes from disk
    char* nodeToMoveOneRecordToData = new char[getCatalogFileNodeSize()]; //s
    uint32_t readNodeFromDiskResult = readNodeFromDisk(diskInfo, volumeHeader, nodeToMoveOneRecordToData, nodeNumberToMoveOneRecordTo);

    if(readNodeFromDiskResult == READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeToMoveOneRecordToData;
        return SPLIT_CHILD_FAILED;
    }

    BTNodeDescriptor* nodeToMoveOneRecordToNodeDescriptor = (BTNodeDescriptor*)&nodeToMoveOneRecordToData[0];

    char* nodeToSplitData = new char[getCatalogFileNodeSize()]; //y
    readNodeFromDiskResult = readNodeFromDisk(diskInfo, volumeHeader, nodeToSplitData, nodeNumberToSplit);

    if(readNodeFromDiskResult == READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeToMoveOneRecordToData, delete[] nodeToSplitData;
        return SPLIT_CHILD_FAILED;
    }

    BTNodeDescriptor* nodeToSplitNodeDescriptor = (BTNodeDescriptor*)&nodeToSplitData[0];

    //create a new node
    char* newNodeData = new char[getCatalogFileNodeSize()]; // z
    uint32_t newNodeNumber;

    uint32_t createNodeOnDiskResult = createNodeOnDisk(diskInfo, volumeHeader, catalogFileHeaderNode, newNodeData, newNodeNumber, nodeToSplitNodeDescriptor->isLeaf);

    if(createNodeOnDiskResult == CREATE_NODE_ON_DISK_FAILED)
    {
        delete[] nodeToMoveOneRecordToData, delete[] nodeToSplitData, delete[] newNodeData;
        return SPLIT_CHILD_FAILED;
    }

    //copy the last t-1 records of nodeToSplit to the new node
    for (uint32_t j = 0; j < halfOfTheMaxNumberOfRecordsPerNode - 1; j++)
    {
        uint32_t startingByteOfRecord = sizeof(BTNodeDescriptor) + (j + halfOfTheMaxNumberOfRecordsPerNode) * sizeof(CatalogDirectoryRecord);
        CatalogDirectoryRecord *recordToInsert = (CatalogDirectoryRecord*) &nodeToSplitData[startingByteOfRecord];
        uint32_t insertRecordInNodeResult = insertRecordInNode(diskInfo, volumeHeader, recordToInsert, newNodeNumber, j,
                                                               false);

        if (insertRecordInNodeResult == INSERT_RECORD_IN_NODE_FAILED) {
            delete[] nodeToMoveOneRecordToData, delete[] nodeToSplitData, delete[] newNodeData;
            return SPLIT_CHILD_FAILED;
        }
    }

    //copy the last t children of nodeToSplit to the new node
    if(nodeToSplitNodeDescriptor->isLeaf == NODE_IS_NOT_LEAF)
    {
        for (uint32_t j = 0; j < halfOfTheMaxNumberOfRecordsPerNode; j++) {
            uint32_t startingByteOfChildNodeInfo = getCatalogFileNodeSize() - (j + halfOfTheMaxNumberOfRecordsPerNode + 1) * sizeof(ChildNodeInfo);
            ChildNodeInfo *childNodeInfoToInsert = (ChildNodeInfo*) &nodeToSplitData[startingByteOfChildNodeInfo];
            uint32_t insertChildNodeInfoInNodeResult = insertChildNodeInfoInNode(diskInfo, volumeHeader, childNodeInfoToInsert, newNodeNumber, j);

            if (insertChildNodeInfoInNodeResult == INSERT_CHILD_NODE_INFO_IN_NODE_FAILED) {
                delete[] nodeToMoveOneRecordToData, delete[] nodeToSplitData, delete[] newNodeData;
                return SPLIT_CHILD_FAILED;
            }
        }
    }

    //reduce the number of records in nodeToSplit
    nodeToSplitNodeDescriptor->numRecords = halfOfTheMaxNumberOfRecordsPerNode - 1;
    updateNodeOnDisk(diskInfo, volumeHeader, nodeToSplitData, nodeNumberToSplit);

    //move existing children of nodeNumberToMoveOneRecordTo to right to make space for the new child
    for(uint32_t j = nodeToMoveOneRecordToNodeDescriptor->numRecords; j >= indexToMoveTheRecordTo + 1; j--)
    {
        uint32_t startingByteOfChildNodeInfo = getCatalogFileNodeSize() - (j + 1) * sizeof(ChildNodeInfo);
        ChildNodeInfo *childNodeInfoToInsert = (ChildNodeInfo*) &nodeToMoveOneRecordToData[startingByteOfChildNodeInfo];
        uint32_t insertChildNodeInfoResult = insertChildNodeInfoInNode(diskInfo, volumeHeader, childNodeInfoToInsert, nodeNumberToMoveOneRecordTo, j + 1);

        if (insertChildNodeInfoResult == INSERT_CHILD_NODE_INFO_IN_NODE_FAILED) {
            delete[] nodeToMoveOneRecordToData, delete[] nodeToSplitData, delete[] newNodeData;
            return SPLIT_CHILD_FAILED;
        }
    }

    //link the new child (new node) to nodeToMoveOneRecordTo
    ChildNodeInfo *childNodeInfoToInsert = new ChildNodeInfo();
    childNodeInfoToInsert->nodeNumber = newNodeNumber;
    uint32_t insertChildNodeInfoResult = insertChildNodeInfoInNode(diskInfo, volumeHeader, childNodeInfoToInsert, nodeNumberToMoveOneRecordTo,
                                                                   indexToMoveTheRecordTo + 1);

    if (insertChildNodeInfoResult == INSERT_CHILD_NODE_INFO_IN_NODE_FAILED)
    {
        delete[] nodeToMoveOneRecordToData, delete[] nodeToSplitData, delete[] newNodeData;
        return SPLIT_CHILD_FAILED;
    }

    //move all records with greater keys (of nodeToMoveOneRecordTo) than the record to move one space to right to make space for the moved record
    for(uint32_t j = nodeToMoveOneRecordToNodeDescriptor->numRecords - 1; j >= indexToMoveTheRecordTo; j--)
    {
        uint32_t startingByteOfRecord = sizeof(BTNodeDescriptor) + j * sizeof(CatalogDirectoryRecord);
        CatalogDirectoryRecord *recordToInsert = (CatalogDirectoryRecord*) &nodeToMoveOneRecordToData[startingByteOfRecord];
        uint32_t insertRecordInNodeResult = insertRecordInNode(diskInfo, volumeHeader, recordToInsert, nodeNumberToMoveOneRecordTo, j + 1,
                                                               false);

        if (insertRecordInNodeResult == INSERT_RECORD_IN_NODE_FAILED) {
            delete[] nodeToMoveOneRecordToData, delete[] nodeToSplitData, delete[] newNodeData;
            return SPLIT_CHILD_FAILED;
        }
    }

    //copy the middle record of nodeToSplit to nodeToMoveOneRecordTo
    uint32_t startingByteOfRecord = sizeof(BTNodeDescriptor) + (halfOfTheMaxNumberOfRecordsPerNode - 1) * sizeof(CatalogDirectoryRecord);
    CatalogDirectoryRecord *recordToInsert = (CatalogDirectoryRecord*) &nodeToSplitData[startingByteOfRecord];
    uint32_t insertRecordInNodeResult = insertRecordInNode(diskInfo, volumeHeader, recordToInsert, nodeNumberToMoveOneRecordTo, indexToMoveTheRecordTo, true);

    delete[] nodeToMoveOneRecordToData, delete[] nodeToSplitData, delete[] newNodeData;
    return (insertRecordInNodeResult == INSERT_RECORD_IN_NODE_SUCCESS) ? SPLIT_CHILD_SUCCESS : SPLIT_CHILD_FAILED;
}

static uint32_t insertNonFull(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, uint32_t nodeNumberToInsertRecordInto,
                                CatalogDirectoryRecord* recordToInsert)
{
    //read node to insert record into from disk
    char* nodeToInsertRecordIntoData = new char[getCatalogFileNodeSize()];
    uint32_t readNodeFromDiskResult = readNodeFromDisk(diskInfo, volumeHeader, nodeToInsertRecordIntoData, nodeNumberToInsertRecordInto);

    if(readNodeFromDiskResult == READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeToInsertRecordIntoData;
        return INSERT_NON_FULL_FAILED;
    }

    BTNodeDescriptor* nodeToInsertRecordIntoDescriptor = (BTNodeDescriptor*)&nodeToInsertRecordIntoData[0];

    //initialize index as index of rightmost element
    uint32_t index = nodeToInsertRecordIntoDescriptor->numRecords - 1;

    if(nodeToInsertRecordIntoDescriptor->isLeaf == NODE_IS_LEAF)
    {
        //find location where the new record to be inserted and move all records with greater key one place at right
        uint32_t byteIndexOfRecord = sizeof(BTNodeDescriptor) + index * sizeof(CatalogDirectoryRecord);
        CatalogDirectoryRecord* record = (CatalogDirectoryRecord*) &nodeToInsertRecordIntoData[byteIndexOfRecord];

        while(index >= 0 && compareKeys(&record->catalogKey, &recordToInsert->catalogKey) > 0)
        {
            uint32_t insertRecordInNodeResult = insertRecordInNode(diskInfo, volumeHeader, record, nodeNumberToInsertRecordInto, index + 1, false);

            if (insertRecordInNodeResult == INSERT_RECORD_IN_NODE_FAILED)
            {
                delete[] nodeToInsertRecordIntoData;
                return INSERT_NON_FULL_FAILED;
            }

            index--;
            byteIndexOfRecord -= sizeof(CatalogDirectoryRecord);
            record = (CatalogDirectoryRecord*)&nodeToInsertRecordIntoData[byteIndexOfRecord];
        }

        //insert the new record at found location
        uint32_t insertRecordInNodeResult = insertRecordInNode(diskInfo, volumeHeader, recordToInsert, nodeNumberToInsertRecordInto, index + 1, true);

        delete[] nodeToInsertRecordIntoData;
        return (insertRecordInNodeResult == INSERT_RECORD_IN_NODE_SUCCESS) ? INSERT_NON_FULL_SUCCESS : INSERT_NON_FULL_FAILED;
    }
    else //node is not leaf
    {
        //find the child which is going to have the new record
        uint32_t byteIndexOfRecord = sizeof(BTNodeDescriptor) + index * sizeof(CatalogDirectoryRecord);
        CatalogDirectoryRecord* record = (CatalogDirectoryRecord*)&nodeToInsertRecordIntoData[byteIndexOfRecord];

        while(index >= 0 && compareKeys(&record->catalogKey, &recordToInsert->catalogKey) > 0)
        {
            index--;
            byteIndexOfRecord -= sizeof(CatalogDirectoryRecord);
            record = (CatalogDirectoryRecord*)&nodeToInsertRecordIntoData[byteIndexOfRecord];
        }

        //read the found child node from disk
        uint32_t startingByteOfChildNodeInfo = getCatalogFileNodeSize() - (index + 2) * sizeof(ChildNodeInfo);
        ChildNodeInfo *childNodeInfo = (ChildNodeInfo*) &nodeToInsertRecordIntoData[startingByteOfChildNodeInfo];

        char* foundChildNodeData = new char[getCatalogFileNodeSize()];
        readNodeFromDiskResult = readNodeFromDisk(diskInfo, volumeHeader, foundChildNodeData, childNodeInfo->nodeNumber);

        if(readNodeFromDiskResult == READ_NODE_FROM_DISK_FAILED)
        {
            delete[] nodeToInsertRecordIntoData, delete[] foundChildNodeData;
            return INSERT_NON_FULL_FAILED;
        }

        BTNodeDescriptor* foundChildNodeDescriptor = (BTNodeDescriptor*)&foundChildNodeData[0];

        //see if the found child is full
        if(foundChildNodeDescriptor->numRecords == getMaximumNumberOfRecordsPerCatalogFileNode())
        {
            //if the child is full then split it
            uint32_t splitChildResult = splitChild(diskInfo, volumeHeader, catalogFileHeaderNode, nodeNumberToInsertRecordInto,
                                                   childNodeInfo->nodeNumber, index + 1);

            if(splitChildResult == SPLIT_CHILD_FAILED)
            {
                delete[] nodeToInsertRecordIntoData, delete[] foundChildNodeData;
                return INSERT_NON_FULL_FAILED;
            }

            //we read the node again from disk because it got changed on split
            readNodeFromDiskResult = readNodeFromDisk(diskInfo, volumeHeader, nodeToInsertRecordIntoData, nodeNumberToInsertRecordInto);

            if(readNodeFromDiskResult == READ_NODE_FROM_DISK_FAILED)
            {
                delete[] nodeToInsertRecordIntoData;
                return INSERT_NON_FULL_FAILED;
            }

            // after split, the middle record of childNodeInfo[i] goes up and childNodeInfo[i] is split into two. See which of the two is going to have the new record
            byteIndexOfRecord = sizeof(BTNodeDescriptor) + (index + 1) * sizeof(CatalogDirectoryRecord);
            record = (CatalogDirectoryRecord*)&nodeToInsertRecordIntoData[byteIndexOfRecord];

            if(compareKeys(&record->catalogKey, &recordToInsert->catalogKey) < 0)
                index++;
        }

        startingByteOfChildNodeInfo = getCatalogFileNodeSize() - (index + 2) * sizeof(ChildNodeInfo);
        childNodeInfo = (ChildNodeInfo*) &nodeToInsertRecordIntoData[startingByteOfChildNodeInfo];
        uint32_t insertNonNullResult = insertNonFull(diskInfo, volumeHeader, catalogFileHeaderNode, childNodeInfo->nodeNumber, recordToInsert);

        delete[] nodeToInsertRecordIntoData, delete[] foundChildNodeData;
        return (insertNonNullResult == INSERT_NON_FULL_SUCCESS) ? INSERT_NON_FULL_SUCCESS : INSERT_NON_FULL_FAILED;
    }
}

///////////////////////////

uint32_t createNodeOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogFileHeaderNode* catalogFileHeaderNode, char* newNodeData,
                          uint32_t& newNodeNumber, uint16_t isLeaf)
{
    //the new node number = the number of nodes occupied. node numbers are indexed from 0; 1 is header node and we don't count it
    newNodeNumber = catalogFileHeaderNode->headerRecord.totalNodes - catalogFileHeaderNode->headerRecord.freeNodes - 1;
    uint32_t firstBlockForNode = getFirstBlockForGivenNodeIndex(volumeHeader, newNodeNumber);
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

    uint32_t writeResult = writeDiskSectors(diskInfo, getNumberOfSectorsPerNode(diskInfo, volumeHeader), firstSector, newNodeData,
                                            numberOfSectorsWritten);

    if(writeResult != EC_NO_ERROR)
    {
        delete newNodeDescriptor;
        return CREATE_NODE_ON_DISK_FAILED;
    }

    //mark node as occupied in header node map record and decrease number of free nodes
    CatalogFileHeaderNode* updatedCatalogFileHeaderNode = updateNodeOccupiedInHeaderNodeMapRecord(catalogFileHeaderNode, newNodeNumber, 1);
    updatedCatalogFileHeaderNode->headerRecord.freeNodes--;

    updateHeaderNodeOnDisk(diskInfo, volumeHeader, updatedCatalogFileHeaderNode);
    memcpy(catalogFileHeaderNode, updatedCatalogFileHeaderNode, sizeof(CatalogFileHeaderNode));

    delete updatedCatalogFileHeaderNode, delete newNodeDescriptor;

    return CREATE_NODE_ON_DISK_SUCCESS;
}

uint32_t insertRecordInNode(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogDirectoryRecord* recordToInsert, uint32_t nodeNumber, uint32_t recordIndexInNode,
                               bool shouldIncreaseNumOfRecordsInNodeDescriptor)
{
    char* nodeData = new char[getCatalogFileNodeSize()];
    uint32_t readNodeFromDiskResult = readNodeFromDisk(diskInfo, volumeHeader, nodeData, nodeNumber);

    if(readNodeFromDiskResult == READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData;
        return INSERT_RECORD_IN_NODE_FAILED;
    }

    BTNodeDescriptor* nodeDescriptor = (BTNodeDescriptor*)&nodeData[0];
    uint32_t byteIndexInNodeToInsertTheRecord = sizeof(BTNodeDescriptor) + recordIndexInNode * sizeof(CatalogDirectoryRecord);
    memcpy(nodeData + byteIndexInNodeToInsertTheRecord, recordToInsert, sizeof(CatalogDirectoryRecord));

    if(shouldIncreaseNumOfRecordsInNodeDescriptor == true)
        nodeDescriptor->numRecords++; //making this operation on the pointer also changes the value in nodeData buffer... HOPEFULLY

    updateNodeOnDisk(diskInfo, volumeHeader, nodeData, nodeNumber);

    delete[] nodeData;
    return INSERT_RECORD_IN_NODE_SUCCESS;
}

uint32_t insertChildNodeInfoInNode(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, ChildNodeInfo* childNodeInfoToInsert, uint32_t nodeNumber,
                                   uint32_t childNodeInfoIndexInNode)
{
    char* nodeData = new char[getCatalogFileNodeSize()];
    uint32_t readNodeFromDiskResult = readNodeFromDisk(diskInfo, volumeHeader, nodeData, nodeNumber);

    if(readNodeFromDiskResult == READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData;
        return INSERT_CHILD_NODE_INFO_IN_NODE_FAILED;
    }

    uint32_t byteIndexInNodeToInsertChildInfo = getCatalogFileNodeSize() - (childNodeInfoIndexInNode + 1) * sizeof(ChildNodeInfo);
    memcpy(nodeData + byteIndexInNodeToInsertChildInfo, childNodeInfoToInsert, sizeof(ChildNodeInfo));

    updateNodeOnDisk(diskInfo, volumeHeader, nodeData, nodeNumber);

    delete[] nodeData;
    return INSERT_CHILD_NODE_INFO_IN_NODE_SUCCESS;
}

/////////////////////

uint32_t readNodeFromDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, char* nodeBuffer, uint32_t nodeNumber)
{
    uint32_t numberOfSectorsRead;
    uint32_t firstBlockForNode = getFirstBlockForGivenNodeIndex(volumeHeader, nodeNumber);
    uint32_t firstSector = getFirstSectorForGivenBlock(diskInfo, volumeHeader, firstBlockForNode);

    uint32_t readResult = readDiskSectors(diskInfo, getNumberOfSectorsPerNode(diskInfo, volumeHeader), firstSector, nodeBuffer, numberOfSectorsRead);

    return (readResult == EC_NO_ERROR) ? READ_NODE_FROM_DISK_SUCCESS : READ_NODE_FROM_DISK_FAILED;
}