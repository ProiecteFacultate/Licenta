#include "string.h"

#include "../include/disk.h"
#include "../../include/structures.h"
#include "../../include/utils.h"
#include "../../include/hfsFunctionUtils.h"
#include "../../include/catalog_file/catalogFileUtils.h"
#include "../../include/catalog_file/codes/catalogFileResponseCodes.h"

int32_t compareKeys(HFSPlusCatalogKey* key1, HFSPlusCatalogKey* key2)
{
    if(key1->parentID > key2->parentID)
        return KEY_1_HIGHER;

    if(key1->parentID < key2->parentID)
        return KEY_2_HIGHER;

    //else we have equality in parentId so we compare based on nodeName
    char* name1 = new char[key1->nodeName.length + 1];
    memcpy(name1, key1->nodeName.chars, key1->nodeName.length);
    name1[key1->nodeName.length] = 0;

    char* name2 = new char[key2->nodeName.length + 1];
    memcpy(name2, key2->nodeName.chars, key2->nodeName.length);
    name2[key2->nodeName.length] = 0;

    if(strcmp(name1, name2) > 0)
        return KEY_1_HIGHER;
    else if(strcmp(name1, name2) < 0)
        return KEY_2_HIGHER;

    return KEYS_EQUAL;
}

CatalogFileHeaderNode* updateNodeOccupiedInHeaderNodeMapRecord(CatalogFileHeaderNode* catalogFileHeaderNode, uint32_t nodeNumber, uint8_t newValue)
{
    CatalogFileHeaderNode* updatedCatalogFileHeaderNode = new CatalogFileHeaderNode();
    memcpy(updatedCatalogFileHeaderNode, catalogFileHeaderNode, sizeof(CatalogFileHeaderNode));

    uint32_t byteIndexForNodeNumber = nodeNumber / 8;
    uint32_t bitIndexForNodeNumber = nodeNumber % 8;

    uint8_t newByteValueForNodes = changeBitValue(byteIndexForNodeNumber, bitIndexForNodeNumber, newValue);
    updatedCatalogFileHeaderNode->mapRecordAndOffsets[byteIndexForNodeNumber] = newByteValueForNodes;

    return updatedCatalogFileHeaderNode;
}

uint32_t getNumberOfBlocksPerNode(HFSPlusVolumeHeader* volumeHeader)
{
    return getCatalogFileNodeSize() / volumeHeader->blockSize;
}

uint32_t getNumberOfSectorsPerNode(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader)
{
    return getNumberOfBlocksPerNode(volumeHeader) * getNumberOfSectorsPerBlock(diskInfo, volumeHeader);
}

uint32_t getFirstBlockForGivenNodeIndex(HFSPlusVolumeHeader* volumeHeader, uint32_t nodeNumber)
{
    return volumeHeader->catalogFile.extents[0].startBlock + (nodeNumber + 1) * getNumberOfBlocksPerNode(volumeHeader); //we add 1 because root is nodeNumber 0 but there is also header node
}

uint32_t getSectorForGivenNodeIndex(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, uint32_t nodeNumber)
{
    return getFirstBlockForGivenNodeIndex(volumeHeader, nodeNumber) * getNumberOfSectorsPerBlock(diskInfo, volumeHeader);
}
