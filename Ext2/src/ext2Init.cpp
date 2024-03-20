#include "string"
#include "string.h"
#include "vector"
#include "iostream"

#include "../include/disk.h"
#include "../include/diskUtils.h"
#include "../include/diskCodes.h"
#include "../include/structures.h"
#include "../include/ext2FunctionUtils.h"
#include "../include/ext2Init.h"

void ext2Startup(char* diskDirectory, DiskInfo** diskInfo, uint32_t sectorsNumber, uint32_t sectorSize)
{
    if(checkDiskInitialization(diskDirectory) == false)
        initializeDisk(diskDirectory, diskInfo, sectorsNumber, sectorSize);
    else
        *diskInfo = getDisk(diskDirectory);

    bool ext2AlreadyInitialized = true;
    if(checkExt2FileSystemInitialization(*diskInfo) == false)
    {
        initializeFirstSuperBlockInFirstGroup(*diskInfo);
        ext2AlreadyInitialized = false;
        std::cout << "First super block initialized\n";
    }

    ext2_super_block* firstSuperBlock = readFirstSuperBlock(*diskInfo);

    if(ext2AlreadyInitialized == false)
    {
        initializeGroups(*diskInfo, firstSuperBlock);
        std::cout << "Groups initialized\n";
    }
}

ext2_super_block* readFirstSuperBlock(DiskInfo* diskInfo)
{
    if(diskInfo->diskParameters.sectorSizeBytes <= 1024)
    {
        uint32_t superBlockOccupiedDiskSectors = sizeof(ext2_super_block) / diskInfo->diskParameters.sectorSizeBytes;
        char* readBuffer = new char[sizeof(ext2_super_block)];
        uint32_t numberOfSectorsRead = 0;
        uint32_t readResult = readDiskSectors(diskInfo, superBlockOccupiedDiskSectors, superBlockOccupiedDiskSectors, readBuffer, numberOfSectorsRead);

        if(readResult != EC_NO_ERROR)
            throw std::runtime_error("Failed to read super block");

        return (ext2_super_block*)&readBuffer[0];
    }
    else //a block is >= sector so if a sector > 1024 -> block >= sector >= 1024
    {
        char* readBuffer = new char[diskInfo->diskParameters.sectorSizeBytes];
        uint32_t numberOfSectorsRead = 0;
        uint32_t readResult = readDiskSectors(diskInfo, 1, 0, readBuffer, numberOfSectorsRead);

        if(readResult != EC_NO_ERROR)
            throw std::runtime_error("Failed to read super block");

        return (ext2_super_block*)&readBuffer[1024];
    }
}

//////////////////////////////////////

static bool checkDiskInitialization(char* diskDirectory)
{
    return !(getDisk(diskDirectory) == nullptr);
}

static void initializeDisk(char* diskDirectory, DiskInfo** diskInfo, uint32_t sectorsNumber, uint32_t sectorSize)
{
    *diskInfo = initializeDisk(diskDirectory, sectorsNumber, sectorSize);
    uint32_t batchSize = 1000;
    fillDiskInitialMemory(*diskInfo, batchSize);
    std::cout << "Disk initialized\n";
}

static bool checkExt2FileSystemInitialization(DiskInfo* diskInfo)
{
    char* superBlockBuffer = new char[diskInfo->diskParameters.sectorSizeBytes];
    uint16_t bootSignature;

    if(diskInfo->diskParameters.sectorSizeBytes <= 1024)
    {
        uint32_t sectorToRead = 2;
        if(diskInfo->diskParameters.sectorSizeBytes == 1024)
            sectorToRead = 1;

        uint32_t numberOfSectorsRead = 0;
        int retryReadCount = 2;
        int readResult = readDiskSectors(diskInfo, 1, sectorToRead, superBlockBuffer, numberOfSectorsRead);
        while(readResult != EC_NO_ERROR && retryReadCount > 0)
        {
            readResult = readDiskSectors(diskInfo, 1, sectorToRead, superBlockBuffer, numberOfSectorsRead);
            retryReadCount--;
        }

        if(retryReadCount == 0)
            throw std::runtime_error("Failed to check boot sector initialization");

        bootSignature = *(uint16_t*)&superBlockBuffer[56];
    }
    else
    {
        uint32_t numberOfSectorsRead = 0;
        int retryReadCount = 2;
        int readResult = readDiskSectors(diskInfo, 1, 0, superBlockBuffer, numberOfSectorsRead);
        while(readResult != EC_NO_ERROR && retryReadCount > 0)
        {
            readResult = readDiskSectors(diskInfo, 1, 0, superBlockBuffer, numberOfSectorsRead);
            retryReadCount--;
        }

        if(retryReadCount == 0)
            throw std::runtime_error("Failed to check boot sector initialization");

        bootSignature = *(uint16_t*)&superBlockBuffer[1080];
    }

    delete[] superBlockBuffer;

    return bootSignature == 61267;   //0xEF53
}

static void initializeFirstSuperBlockInFirstGroup(DiskInfo* diskInfo)
{
    ext2_super_block* ext2SuperBlock = new ext2_super_block();
    memset(ext2SuperBlock, 0, sizeof(ext2_super_block)); //always 1024

    uint64_t totalBytesOnDisk = (uint64_t) diskInfo->diskParameters.sectorsNumber * (uint64_t) diskInfo->diskParameters.sectorSizeBytes - 1024; //first 1024 are for boot sector(s)
    uint32_t blocksSize = 1024;
    uint32_t totalNumberOfBlocks = totalBytesOnDisk / blocksSize; //CAUTION change when s_log_block_size changes


    ext2SuperBlock->s_inodes_count = totalBytesOnDisk / 8192; //this is standard; this doesn't mean that files can hava max 8k, or that an inode can represent max 8k
    ext2SuperBlock->s_block_count = totalNumberOfBlocks;
    ext2SuperBlock->s_r_blocks_count = 0; //we don't have a kernel so no reserve
    ext2SuperBlock->s_free_blocks_count = totalBytesOnDisk;
    ext2SuperBlock->s_free_inodes_count = ext2SuperBlock->s_inodes_count;
    ext2SuperBlock->s_first_data_block = 1; //the super block of the first group is this first useful block (ignore data in the name, it is wrong)
    ext2SuperBlock->s_log_block_size = blocksSize;
    ext2SuperBlock->s_log_frag_size = blocksSize; //we don't have fragmentation
    ext2SuperBlock->s_blocks_per_group = 8192;
    ext2SuperBlock->s_frags_per_group = 999;
    ext2SuperBlock->s_inodes_per_group = blocksSize * 8; //1 inode for each bit in inode bitmap, so for block size * 8
    ext2SuperBlock->s_mtime = 999;
    ext2SuperBlock->s_wtime = 999; //TODO
    ext2SuperBlock->s_mnt_count = 999;
    ext2SuperBlock->s_max_mnt_count = 999; //TODO maybe if check is implemented
    ext2SuperBlock->s_magic = 61267; //0xEF53
    ext2SuperBlock->s_state = 1;
    ext2SuperBlock->s_errors = 1;
    ext2SuperBlock->s_minor_rev_level = 1;
    ext2SuperBlock->s_lastcheck = 999; //TODO maybe if check is implemented
    ext2SuperBlock->s_checkinterval = 999; //TODO maybe if check is implemented
    ext2SuperBlock->s_creator_os = 0;
    ext2SuperBlock->s_rev_level = 1;
    ext2SuperBlock->s_def_resuid = 999;
    ext2SuperBlock->s_def_resgid = 999;
    ext2SuperBlock->s_first_ino = 0;
    ext2SuperBlock->s_inode_size = 128;
    ext2SuperBlock->s_block_group_nr = 1;
    ext2SuperBlock->s_feature_compat = 0;
    ext2SuperBlock->s_feature_incompat = 1;
    ext2SuperBlock->s_feature_ro_compat = 1;
    ext2SuperBlock->s_algorithm_usage_bitmap = 0;
    ext2SuperBlock->s_prealloc_blocks = 2;
    ext2SuperBlock->s_prealloc_dir_blocks = 8;
    ext2SuperBlock->s_padding1 = 0;

    if(diskInfo->diskParameters.sectorSizeBytes <= 1024)
    {
        uint32_t superBlockOccupiedDiskSectors = sizeof(ext2_super_block) / diskInfo->diskParameters.sectorSizeBytes;
        char* superBlockBuffer = new char[sizeof(ext2_super_block)];
        memcpy(superBlockBuffer, ext2SuperBlock, sizeof(ext2_super_block));

        uint32_t numberOfSectorsWritten = 0;
        int retryWriteCount = 2;
        int writeResult = writeDiskSectors(diskInfo, superBlockOccupiedDiskSectors, superBlockOccupiedDiskSectors, superBlockBuffer, numberOfSectorsWritten);
        while(writeResult != EC_NO_ERROR && retryWriteCount > 0)
        {
            writeResult = writeDiskSectors(diskInfo, superBlockOccupiedDiskSectors, superBlockOccupiedDiskSectors, superBlockBuffer, numberOfSectorsWritten);
            retryWriteCount--;
        }

        if(retryWriteCount == 0)
        {
            throw std::runtime_error("Failed to initialize super block in first block");
        }

        delete[] superBlockBuffer;
    }
    else //a block is >= sector so if a sector > 1024 -> block >= sector >= 1024
    {
        char* superBlockBuffer = new char[diskInfo->diskParameters.sectorSizeBytes];
        memset(superBlockBuffer, 0, diskInfo->diskParameters.sectorSizeBytes);
        memcpy(superBlockBuffer + 1024, ext2SuperBlock, sizeof(ext2_super_block)); //superBlock of first group is always placed at byte 1024 globally

        uint32_t numberOfSectorsWritten = 0;
        int retryWriteCount = 2;
        int writeResult = writeDiskSectors(diskInfo, 1, 0, superBlockBuffer, numberOfSectorsWritten);
        while(writeResult != EC_NO_ERROR && retryWriteCount > 0)
        {
            writeResult = writeDiskSectors(diskInfo, 1, 0, superBlockBuffer, numberOfSectorsWritten);
            retryWriteCount--;
        }

        if(retryWriteCount == 0)
        {
            throw std::runtime_error("Failed to initialize super block in first block");
        }

        delete[] superBlockBuffer;
    }
}

static void initializeGroups(DiskInfo* diskInfo, ext2_super_block* superBlock)
{
    uint32_t numberOfSectorsWritten;
    uint32_t writeResult;
    char* blockBuffer = new char[superBlock->s_log_block_size];
    uint32_t numOfSectorsPerBlock = getNumberOfSectorsPerBlock(diskInfo, superBlock);
    uint32_t numOfGroupDescriptorsBlocksPerGroup = getNumberOfGroupDescriptorsTablesPerGroup(superBlock);
    char* groupDescriptorsBlocksBuffer = new char[numOfGroupDescriptorsBlocksPerGroup * superBlock->s_log_block_size];

    std::vector<ext2_group_desc*> groupDescriptors;
    initializeGroupDescriptors(diskInfo, superBlock, groupDescriptors);

    for(uint32_t group = 0; group < getNumberOfGroups(superBlock); group++)
    {
        uint32_t firstSectorInGroup = getFirstSectorForGroup(diskInfo, superBlock, group);

        //write in memory the super block for the group
        memcpy(blockBuffer, superBlock, sizeof(ext2_super_block));
        writeResult = writeDiskSectors(diskInfo, numOfSectorsPerBlock, firstSectorInGroup, blockBuffer, numberOfSectorsWritten);

        if(writeResult != EC_NO_ERROR)
            throw std::runtime_error("Failed to initialize blocks");

        //write group descriptors
        uint32_t firstGroupDescriptorSector = firstSectorInGroup + numOfSectorsPerBlock;
        for(uint32_t groupDescriptorIndex = 0; groupDescriptorIndex < groupDescriptors.size(); groupDescriptorIndex++)
            memcpy(groupDescriptorsBlocksBuffer + groupDescriptorIndex * sizeof(ext2_group_desc), groupDescriptors[groupDescriptorIndex], sizeof(ext2_group_desc));

        writeResult = writeDiskSectors(diskInfo, numOfGroupDescriptorsBlocksPerGroup * numOfSectorsPerBlock, firstGroupDescriptorSector,
                                       groupDescriptorsBlocksBuffer, numberOfSectorsWritten);

        if(writeResult != EC_NO_ERROR)
            throw std::runtime_error("Failed to initialize blocks");
    }
}

static void initializeGroupDescriptors(DiskInfo* diskInfo, ext2_super_block* superBlock, std::vector<ext2_group_desc*>& groupDescriptors)
{
    uint32_t numberOfGroups = getNumberOfGroups(superBlock);
    uint32_t numOfGroupDescriptorsTablesPerGroup = getNumberOfGroupDescriptorsTablesPerGroup(superBlock);

    for(uint32_t group = 0; group < numberOfGroups; group++)
    {
        uint32_t firstBlockInGroup = getFirstBlockForGroup(superBlock, group);

        ext2_group_desc* groupDescriptor = new ext2_group_desc();
        groupDescriptor->bg_block_bitmap = firstBlockInGroup + numOfGroupDescriptorsTablesPerGroup + 1;
        groupDescriptor->bg_inode_bitmap = groupDescriptor->bg_block_bitmap + 1;
        groupDescriptor->bg_inode_table = groupDescriptor->bg_inode_bitmap + 1;
        groupDescriptor->bg_free_blocks_count = getNumberOfDataBlocksPerGroup(superBlock);
        groupDescriptor->bg_free_inodes_count = superBlock->s_inodes_per_group;
        groupDescriptor->bg_used_dirs_count = 0;
        groupDescriptor->bg_pad = 0;
        memset(groupDescriptor->bg_reserved, 0, 12);

        groupDescriptors.push_back(groupDescriptor);
    }
}