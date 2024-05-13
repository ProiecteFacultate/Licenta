#include "cstdint"

#ifndef HFS__STRUCTURES_H
#define HFS__STRUCTURES_H

typedef struct
{
    uint32_t startBlock;
    uint32_t blockCount;
} __attribute__((packed)) HFSPlusExtentDescriptor;

typedef struct
{
    uint64_t logicalSize;
    uint32_t clumpSize;
    uint32_t totalBlocks;
    HFSPlusExtentDescriptor extents[8];
} __attribute__((packed)) HFSPlusForkData;

typedef uint32_t HFSCatalogNodeID; //this is nod the id of the nodes but of the records. its name is wrong in documentation probably

typedef struct
{
    uint16_t signature;
    uint16_t version;
    uint32_t attributes;
    uint32_t lastMountedVersion;
    uint32_t journalInfoBlock;
    uint32_t createDate;
    uint32_t modifyDate;
    uint32_t backupDate;
    uint32_t checkedDate;
    uint32_t fileCount;
    uint32_t folderCount;
    uint32_t blockSize;
    uint32_t totalBlocks;
    uint32_t freeBlocks;
    uint32_t nextAllocation;
    uint32_t rsrcClumpSize;
    uint32_t dataClumpSize;
    HFSCatalogNodeID nextCatalogID;
    HFSCatalogNodeID nextExtentsID; //custom
    uint64_t encodingsBitmap;
    uint32_t finderInfo[8];
    HFSPlusForkData allocationFile;
    HFSPlusForkData extentsFile;
    HFSPlusForkData catalogFile;
    HFSPlusForkData attributesFile;
    HFSPlusForkData startupFile;
} __attribute__((packed)) HFSPlusVolumeHeader;

//B-Tree structures
typedef struct
{
    uint32_t fLink;
    uint32_t bLink;
    int8_t kind;
    uint8_t height;
    uint16_t numRecords;
    uint16_t isLeaf;     //normally here is a reserved field but we use it to check if the node is leaf
} __attribute__((packed)) BTNodeDescriptor;

typedef struct
{
    uint16_t treeDepth;
    uint32_t rootNode; //the root number (index) of the root node; it is initially 0 but it might change
    uint32_t leafRecords;
    uint32_t firstLeafNode;
    uint32_t lastLeafNode;
    uint16_t nodeSize;
    uint16_t maxKeyLength;
    uint32_t totalNodes;
    uint32_t freeNodes;
    uint16_t reserved1;
    uint32_t clumpSize;
    uint8_t btreeType;
    uint8_t keyCompareType;
    uint32_t attributes;
    uint32_t reserved3[16];
} __attribute__((packed)) BTHeaderRec;

typedef enum {
    kBTLeafNode       = -1,
    kBTIndexNode      =  0,
    kBTHeaderNode     =  1,
    kBTMapNode        =  2
} NodeType;

//Catalog structures
typedef struct {
    uint16_t length;
    uint8_t chars[510]; //in specification we have unicode so we have an uint16_t unicode[255] instead
} __attribute__((packed)) HFSUniStr255;

typedef struct {
    uint16_t keyLength;
    HFSCatalogNodeID parentID;
    HFSUniStr255 nodeName;
} __attribute__((packed)) HFSPlusCatalogKey;

enum {
    kHFSFolderRecord            = 0x0100,
    kHFSFileRecord              = 0x0200,
    kHFSFolderThreadRecord      = 0x0300,
    kHFSFileThreadRecord        = 0x0400
};

//in documentation there are different structs for folder & file but the attributes that are different are irrelevant for us so we combined them in one structure
typedef struct
{
    int16_t recordType;
    uint16_t flags;
    uint32_t valence;
    HFSCatalogNodeID folderID;
    uint32_t createDate;
    uint32_t contentModDate;
    uint32_t attributeModDate;
    uint32_t accessDate;
    uint32_t backupDate;
    uint8_t permissions[16]; //we don't care
    uint8_t userInfo[16]; //we don't care
    uint8_t finderInfo[16]; //we don't care
    uint32_t textEncoding;
    uint32_t fileSize;   //this was a reserved field but we use it for this
    HFSPlusForkData hfsPlusForkData;
} __attribute__((packed)) HFSPlusCatalogDirectory;

//Custom structure
typedef struct
{
    BTNodeDescriptor nodeDescriptor; //14 bytes
    BTHeaderRec headerRecord; //106 bytes
    uint8_t userDataRecord[128];
    uint16_t mapRecordAndOffsets[388];
} __attribute__((packed)) ExtentsFileHeaderNode;

typedef struct
{
    BTNodeDescriptor nodeDescriptor; //14 bytes
    BTHeaderRec headerRecord; //106 bytes
    uint8_t userDataRecord[128];
    uint16_t mapRecordAndOffsets[1924];
} __attribute__((packed)) CatalogFileHeaderNode;

//multiple of these are placed at the end of a node (after all the CatalogDirectoryRecord in that node) CAUTION they are placed in reverse order
typedef struct
{
    uint32_t nodeNumber;
    uint32_t startBlock;
} __attribute__((packed)) ChildNodeInfo;

typedef struct
{
    HFSPlusCatalogKey catalogKey;
    HFSPlusCatalogDirectory catalogData;
} __attribute__((packed)) CatalogDirectoryRecord;

typedef struct {
    uint16_t keyLength;
    HFSCatalogNodeID parentID;
    HFSUniStr255 nodeName;
    uint32_t extentOverflowIndex; //every extent of a file that is overflowed has an index in normal order 0, 1, 2.. which represents the 9th, 10th.. etch extent of the file
} __attribute__((packed)) ExtentsFileCatalogKey;

typedef struct
{
    HFSCatalogNodeID folderID;
    HFSPlusExtentDescriptor extent;
} __attribute__((packed)) ExtentsFileCatalogDirectory;

typedef struct
{
    ExtentsFileCatalogKey catalogKey;
    ExtentsFileCatalogDirectory catalogData;
} __attribute__((packed)) ExtentsDirectoryRecord;

#endif