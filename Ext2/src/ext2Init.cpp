#include "string"
#include "string.h"
#include "iostream"

#include "../include/disk.h"
#include "../include/diskUtils.h"
#include "../include/diskCodes.h"
#include "../include/structures.h"
#include "../include/ext2Init.h"

bool checkDiskInitialization(char* diskDirectory)
{
    return !(getDisk(diskDirectory) == nullptr);
}

void initializeDisk(char* diskDirectory, DiskInfo** diskInfo, uint32_t sectorsNumber, uint32_t sectorSize)
{
    *diskInfo = initializeDisk(diskDirectory, sectorsNumber, sectorSize);
    fillDiskInitialMemory(*diskInfo);
    std::cout << "Disk initialized\n";
}

bool checkExt2FileSystemInitialization(DiskInfo* diskInfo)
{
    char* firstBootSectorBuffer = new char[diskInfo->diskParameters.sectorSizeBytes];

    uint32_t numberOfSectorsRead = 0;
    int retryReadCount = 2;
    int readResult = readDiskSectors(diskInfo, 1, 0, firstBootSectorBuffer, numberOfSectorsRead);
    while(readResult != EC_NO_ERROR && retryReadCount > 0)
    {
        readResult = readDiskSectors(diskInfo, 1, 0, firstBootSectorBuffer, numberOfSectorsRead);
        retryReadCount--;
    }

    if(retryReadCount == 0)
    {
        throw std::runtime_error("Failed to check boot sector initialization");
    }

    uint16_t bootSignature = *(uint16_t*)&firstBootSectorBuffer[510];

    delete[] firstBootSectorBuffer;

    return bootSignature == 61267;   //0xEF53
}

void initializeFirstSectors(DiskInfo* diskInfo)
{
    ext2_super_block* ext2SuperBlock = new ext2_super_block();
    memset(ext2SuperBlock, 0, sizeof(ext2_super_block)); //always 1024
    uint32_t totalNumberOfBlocks = (diskInfo->diskParameters.sectorsNumber * diskInfo->diskParameters.sectorSizeBytes) / 1024; //CAUTION change when s_log_block_size changes


    ext2SuperBlock->s_inodes_count = 10;
    ext2SuperBlock->s_block_count = totalNumberOfBlocks;
    ext2SuperBlock->s_r_blocks_count = totalNumberOfBlocks / 20; //~5% of total number of blocks; this is the standard
    ext2SuperBlock->s_free_blocks_count = 10;
    ext2SuperBlock->s_free_inodes_count = 10;
    ext2SuperBlock->s_first_data_block = 1;
    ext2SuperBlock->s_log_block_size = 0; //it is represented in power of 2 using 1024 as unit so 0 means 1024 bytes (from book)
    ext2SuperBlock->s_log_frag_size = 0;
    ext2SuperBlock->s_blocks_per_group = 10;
    ext2SuperBlock->s_frags_per_group = 999;
    ext2SuperBlock->s_inodes_per_group = 8192; //1 inode for each bit in inode bitmap, so for block size 1024 -> 8192
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
    ext2SuperBlock->s_first_ino = 11;
    ext2SuperBlock->s_inode_size = 128;
    ext2SuperBlock->s_block_group_nr = 1;
    ext2SuperBlock->s_feature_compat = 0;
    ext2SuperBlock->s_feature_incompat = 1;
    ext2SuperBlock->s_feature_ro_compat = 1;
    ext2SuperBlock->s_algorithm_usage_bitmap = 0;
    ext2SuperBlock->s_prealloc_blocks = 10;
    ext2SuperBlock->s_prealloc_dir_blocks = 10;
    ext2SuperBlock->s_padding1 = 0;
}