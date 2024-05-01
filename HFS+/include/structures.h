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

typedef uint32_t HFSCatalogNodeID;

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
    uint32_t writeCount;
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
    uint16_t reserved;
} __attribute__((packed)) BTNodeDescriptor;

typedef struct
{
    uint16_t treeDepth;
    uint32_t rootNode;
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
    uint16_t unicode[255];
} __attribute__((packed)) HFSUniStr255;

typedef struct {
    uint16_t              keyLength;
    HFSCatalogNodeID    parentID;
    HFSUniStr255        nodeName;
} __attribute__((packed)) HFSPlusCatalogKey;

enum {
    kHFSFolderRecord            = 0x0100,
    kHFSFileRecord              = 0x0200,
    kHFSFolderThreadRecord      = 0x0300,
    kHFSFileThreadRecord        = 0x0400
};

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
    uint32_t reserved;
} __attribute__((packed)) HFSPlusCatalogFolder;

typedef struct
{
    int16_t recordType;
    int16_t reserved;
    HFSCatalogNodeID parentID;
    uint16_t nodeName[255];
} __attribute__((packed)) HFSPlusCatalogThread;

//Extent overflow structures

typedef struct
{
    uint16_t keyLength;
    uint8_t forkType;
    uint8_t pad;
    HFSCatalogNodeID fileID;
    uint32_t startBlock;
} __attribute__((packed)) HFSPlusExtentKey;

#endif
