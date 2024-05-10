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
    char* nodeData = new char[getCatalogFileNodeSize()];
    uint32_t newNodeNumber;

    if(catalogFileHeaderNode->headerRecord.freeNodes == catalogFileHeaderNode->headerRecord.totalNodes - 1) //root node do not exist yet //TODO delete root creation from init
    {
        uint32_t createNodeOnDiskResult = createNodeOnDisk(diskInfo, volumeHeader, catalogFileHeaderNode, nodeData, newNodeNumber, NODE_IS_LEAF);
        delete[] nodeData;

        if(createNodeOnDiskResult == CREATE_NODE_ON_DISK_FAILED)
            return INSERT_RECORD_IN_TREE_FAILED;

        uint32_t insertFirstKeyInRootNodeResult = insertRecordInNode(diskInfo, volumeHeader, recordToInsert, 0, 0);

        return (insertFirstKeyInRootNodeResult == INSERT_RECORD_IN_NODE_SUCCESS) ? INSERT_RECORD_IN_TREE_SUCCESS : INSERT_RECORD_IN_TREE_FAILED;
    }
    else //root node already exists (tree is not empty)
    {
        //read root
        uint32_t readRootNodeFromDiskResult = readNodeFromDisk(diskInfo, volumeHeader, nodeData, catalogFileHeaderNode->headerRecord.rootNode);

        if(readRootNodeFromDiskResult == READ_NODE_FROM_DISK_FAILED)
        {
            delete[] nodeData;
            return INSERT_RECORD_IN_NODE_FAILED;
        }

        BTNodeDescriptor* rootNodeDescriptor = (BTNodeDescriptor*)&nodeData[0];

        if(rootNodeDescriptor->numRecords == getMaximumNumberOfRecordsPerCatalogFileNode())
        {
            //create new node which will be the new root
            uint32_t createNodeOnDiskResult = createNodeOnDisk(diskInfo, volumeHeader, catalogFileHeaderNode, nodeData, newNodeNumber, NODE_IS_NOT_LEAF);

            if(createNodeOnDiskResult == CREATE_NODE_ON_DISK_FAILED)
            {
                delete[] nodeData;
                return INSERT_RECORD_IN_TREE_FAILED;
            }

            //make old root the child of this new root
            ChildNodeInfo* childNodeInfoToInsert = new ChildNodeInfo;
            childNodeInfoToInsert->nodeNumber = catalogFileHeaderNode->headerRecord.rootNode;
            uint32_t insertChildNodeInfoResult = insertChildNodeInfoInNode(diskInfo, volumeHeader, childNodeInfoToInsert, newNodeNumber, 0);

            if(insertChildNodeInfoResult == INSERT_CHILD_NODE_INFO_IN_NODE_FAILED)
            {
                delete[] nodeData;
                return INSERT_RECORD_IN_TREE_FAILED;
            }

            //TODO chane root in catalogFileHeaderNode on disk but dont know if here or at the end like on site
        }
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
        return CREATE_NODE_ON_DISK_FAILED;

    //mark node as occupied in header node map record and decrease number of free nodes
    CatalogFileHeaderNode* updatedCatalogFileHeaderNode = updateNodeOccupiedInHeaderNodeMapRecord(catalogFileHeaderNode, newNodeNumber, 1);
    updatedCatalogFileHeaderNode->headerRecord.freeNodes--;

    updateHeaderNodeOnDisk(diskInfo, volumeHeader, updatedCatalogFileHeaderNode);
    memcpy(catalogFileHeaderNode, updatedCatalogFileHeaderNode, sizeof(CatalogFileHeaderNode));

    delete[] updatedCatalogFileHeaderNode;

    return CREATE_NODE_ON_DISK_SUCCESS;
}

uint32_t insertRecordInNode(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, CatalogDirectoryRecord* recordToInsert, uint32_t nodeNumber, uint32_t recordIndexInNode)
{
    char* nodeData = new char[getCatalogFileNodeSize()];
    uint32_t readNodeFromDiskResult = readNodeFromDisk(diskInfo, volumeHeader, nodeData, nodeNumber);

    if(readNodeFromDiskResult == READ_NODE_FROM_DISK_FAILED)
    {
        delete[] nodeData;
        return INSERT_RECORD_IN_NODE_FAILED;
    }

    BTNodeDescriptor* nodeDescriptor = (BTNodeDescriptor*)&nodeData[0];
    uint32_t byteIndexInNodeToInsertTheRecord = sizeof(BTNodeDescriptor) + (recordIndexInNode + 1) * sizeof(CatalogDirectoryRecord);
    memcpy(nodeData + byteIndexInNodeToInsertTheRecord, recordToInsert, sizeof(CatalogDirectoryRecord));
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
    uint32_t firstBlockForNode = getFirstBlockForGivenNodeIndex(volumeHeader, nodeNumber); //root node
    uint32_t firstSector = getFirstSectorForGivenBlock(diskInfo, volumeHeader, firstBlockForNode);

    uint32_t readResult = readDiskSectors(diskInfo, getNumberOfSectorsPerNode(diskInfo, volumeHeader), firstSector, nodeBuffer, numberOfSectorsRead);

    return (readResult == EC_NO_ERROR) ? READ_NODE_FROM_DISK_SUCCESS : READ_NODE_FROM_DISK_FAILED;
}