#include "string.h"
#include "vector"
#include "math.h"
#include "cstdint"

#include "../include/disk.h"
#include "../include/diskCodes.h"
#include "../include/structures.h"
#include "../include/ext2FunctionUtils.h"
#include "../include/codes/ext2Attributes.h"
#include "../include/codes/ext2Codes.h"
#include "../include/codes/ext2ApiResponseCodes.h"
#include "../include/ext2.h"
#include "../include/utils.h"
#include "../include/codes/ext2BlocksAllocationCodes.h"
#include "../include/ext2BlocksAllocation.h"

#define VALUE_ENTRY sizeof(uint32_t) //we define this as a macro to be easier to replace in testing

#define BIG_VALUE 99999999

uint32_t allocateBlockToDirectory(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* inode, uint32_t& newBlockGlobalIndex, ext2_inode* updatedInode)
{
    uint32_t addHigherOrderDataBlockResult = ADD_BLOCK_TO_DIRECTORY_SUCCESS, secondOrderTableGlobalIndex; //secondOrderTableGlobalIndex keeps its value if already exist (for indexes 12, 13, 14) or a new one if not
    uint32_t entriesInOrderBlock = superBlock->s_log_block_size / VALUE_ENTRY;

    if(inode->i_blocks < 12) //we add a direct block
    {
        uint32_t addDirectDataBlockResult = searchAndOccupyFreeDataBlock(diskInfo, superBlock, inode->i_block[inode->i_blocks - 1], newBlockGlobalIndex);
        if(addDirectDataBlockResult == SEARCH_EMPTY_DATA_BLOCK_NO_FREE_BLOCKS)
            return ADD_BLOCK_TO_DIRECTORY_NO_FREE_BLOCKS;
        else if(addDirectDataBlockResult == SEARCH_EMPTY_DATA_BLOCK_FAILED_FOR_OTHER_REASON)
            return ADD_BLOCK_TO_DIRECTORY_FAILED_FOR_OTHER_REASON;

        updatedInode->i_block[inode->i_blocks] = newBlockGlobalIndex;
    }
    else if(inode->i_blocks <= entriesInOrderBlock + 11) //second order
    {
        addHigherOrderDataBlockResult = addSecondOrderDataBlock(diskInfo, superBlock, inode, newBlockGlobalIndex, secondOrderTableGlobalIndex);
        updatedInode->i_block[12] = secondOrderTableGlobalIndex;
    }
    else if(inode->i_blocks <= entriesInOrderBlock * entriesInOrderBlock + entriesInOrderBlock + 11) //THIRD order
    {
        addHigherOrderDataBlockResult = addThirdOrderDataBlock(diskInfo, superBlock, inode, newBlockGlobalIndex, secondOrderTableGlobalIndex);
        updatedInode->i_block[13] = secondOrderTableGlobalIndex;
    }
    else
    {
        addHigherOrderDataBlockResult = addForthOrderDataBlock(diskInfo, superBlock, inode, newBlockGlobalIndex, secondOrderTableGlobalIndex);
        updatedInode->i_block[14] = secondOrderTableGlobalIndex;
    }

    if(addHigherOrderDataBlockResult == ADD_HIGHER_ORDER_DATA_BLOCK_NO_FREE_BLOCKS)
        return ADD_BLOCK_TO_DIRECTORY_NO_FREE_BLOCKS;
    else if(addHigherOrderDataBlockResult == ADD_HIGHER_ORDER_DATA_FAILED_FOR_OTHER_REASON)
        return ADD_BLOCK_TO_DIRECTORY_FAILED_FOR_OTHER_REASON;

    //the data blocks bitmaps, higher order bitmaps, groups descriptors for added block/tables where updated in methods; now we update the inode
    updatedInode->i_blocks++;
    uint32_t updateInodeResult = updateInode(diskInfo, superBlock, inode, updatedInode);

    return (updateInodeResult == UPDATE_INODE_SUCCESS) ? ADD_BLOCK_TO_DIRECTORY_SUCCESS : ADD_BLOCK_TO_DIRECTORY_FAILED_FOR_OTHER_REASON;
}

/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////

static uint32_t addSecondOrderDataBlock(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* inode, uint32_t& newBlockGlobalIndex, uint32_t& secondOrderTableGlobalIndex)
{
    uint32_t lastBlockInDirectoryGlobalIndex, numberOfSectorsRead, numOfSectorsWritten;
    secondOrderTableGlobalIndex = inode->i_block[12];
    uint32_t lastBlockInDirectoryLocalIndexInsideInode = inode->i_blocks - 1;
    uint32_t getBlockGlobalIndexResult = getDataBlockGlobalIndexByLocalIndexInsideInode(diskInfo, superBlock, inode, lastBlockInDirectoryLocalIndexInsideInode,
                                                                                        lastBlockInDirectoryGlobalIndex);

    if(getBlockGlobalIndexResult != GET_DATA_BLOCK_BY_LOCAL_INDEX_SUCCESS)
        return ADD_HIGHER_ORDER_DATA_FAILED_FOR_OTHER_REASON;

    if(inode->i_blocks <= 12) //it means there is no 2nd order indirect block so we need to add the table
    {
        uint32_t addSecondOrderTableResult = searchAndOccupyFreeDataBlock(diskInfo, superBlock, BIG_VALUE, secondOrderTableGlobalIndex);

        if(addSecondOrderTableResult == SEARCH_EMPTY_DATA_BLOCK_NO_FREE_BLOCKS)
            return ADD_HIGHER_ORDER_DATA_BLOCK_NO_FREE_BLOCKS;
        else if(addSecondOrderTableResult == SEARCH_EMPTY_DATA_BLOCK_FAILED_FOR_OTHER_REASON)
            return ADD_HIGHER_ORDER_DATA_FAILED_FOR_OTHER_REASON;
    }

    uint32_t addDataBlockResult = searchAndOccupyFreeDataBlock(diskInfo, superBlock, lastBlockInDirectoryGlobalIndex, newBlockGlobalIndex);

    if(addDataBlockResult == SEARCH_EMPTY_DATA_BLOCK_NO_FREE_BLOCKS)
        return ADD_HIGHER_ORDER_DATA_BLOCK_NO_FREE_BLOCKS;
    else if(addDataBlockResult == SEARCH_EMPTY_DATA_BLOCK_FAILED_FOR_OTHER_REASON)
        return ADD_HIGHER_ORDER_DATA_FAILED_FOR_OTHER_REASON;

    //now add the new data block to the second order table
    uint32_t addBlockIndexToAnotherResult = addBlockIndexToAnotherBlock(diskInfo, superBlock, secondOrderTableGlobalIndex,
                                                                        (inode->i_blocks - 12) * VALUE_ENTRY, newBlockGlobalIndex);

    return (addBlockIndexToAnotherResult == ADD_BLOCK_INDEX_TO_LOWER_ORDER_BLOCK_SUCCESS) ? ADD_HIGHER_ORDER_DATA_SUCCESS : ADD_HIGHER_ORDER_DATA_FAILED_FOR_OTHER_REASON;
}

static uint32_t addThirdOrderDataBlock(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* inode, uint32_t& newBlockGlobalIndex, uint32_t& secondOrderTableGlobalIndex)
{
    uint32_t thirdOrderTableGlobalIndex, numberOfSectorsRead, lastBlockInDirectoryGlobalIndex;
    secondOrderTableGlobalIndex = inode->i_block[13];
    uint32_t entriesInOrderBlock = superBlock->s_log_block_size / VALUE_ENTRY;
    uint32_t occupiedDataBlocksInThirdLevel = inode->i_blocks - 12 - entriesInOrderBlock; //only the data blocks not the order tables
    int32_t lastBlockInDirectoryThirdOrderTableLocalIndex = occupiedDataBlocksInThirdLevel / entriesInOrderBlock; //in which third order table is the last entry
    if(occupiedDataBlocksInThirdLevel % entriesInOrderBlock == 0)
        lastBlockInDirectoryThirdOrderTableLocalIndex--;

    //check if there is a 2nd order block for level 3 (so no block for i_block[13]
    if(inode->i_blocks <= entriesInOrderBlock + 12) //it means there is no 2nd order block for level 3 (so no block for i_block[13]
    {
        uint32_t addSecondOrderTableResult = searchAndOccupyFreeDataBlock(diskInfo, superBlock, BIG_VALUE, secondOrderTableGlobalIndex);

        if(addSecondOrderTableResult == SEARCH_EMPTY_DATA_BLOCK_NO_FREE_BLOCKS)
            return ADD_HIGHER_ORDER_DATA_BLOCK_NO_FREE_BLOCKS;
        else if(addSecondOrderTableResult == SEARCH_EMPTY_DATA_BLOCK_FAILED_FOR_OTHER_REASON)
            return ADD_HIGHER_ORDER_DATA_FAILED_FOR_OTHER_REASON;
    }

    //check if there is a 3rd order block with free space
    if(occupiedDataBlocksInThirdLevel % entriesInOrderBlock == 0) //we need to add a new 3rd order table
    {
        uint32_t addThirdOrderTableResult = searchAndOccupyFreeDataBlock(diskInfo, superBlock, BIG_VALUE, thirdOrderTableGlobalIndex);

        if(addThirdOrderTableResult == SEARCH_EMPTY_DATA_BLOCK_NO_FREE_BLOCKS)
            return ADD_HIGHER_ORDER_DATA_BLOCK_NO_FREE_BLOCKS;
        else if(addThirdOrderTableResult == SEARCH_EMPTY_DATA_BLOCK_FAILED_FOR_OTHER_REASON)
            return ADD_HIGHER_ORDER_DATA_FAILED_FOR_OTHER_REASON;

        //add the third order table to the second order table
        uint32_t addThirdOrderToSecondOrderResult = addBlockIndexToAnotherBlock(diskInfo, superBlock, secondOrderTableGlobalIndex,
                                                                                (lastBlockInDirectoryThirdOrderTableLocalIndex + 1) * VALUE_ENTRY,
                                                                                thirdOrderTableGlobalIndex);

        if(addThirdOrderToSecondOrderResult == ADD_BLOCK_INDEX_TO_LOWER_ORDER_BLOCK_FAILED)
            return ADD_HIGHER_ORDER_DATA_FAILED_FOR_OTHER_REASON;
    }
    else //we haven't added a new 3rd order block, so we need to retrieve its already existing index
    {
        char* blockBuffer = new char[superBlock->s_log_block_size];
        uint32_t readResult = readDiskSectors(diskInfo, getNumberOfSectorsPerBlock(diskInfo, superBlock),
                                     getFirstSectorForGivenBlock(diskInfo, superBlock, inode->i_block[13]),blockBuffer, numberOfSectorsRead);

        if(readResult != EC_NO_ERROR)
        {
            delete[] blockBuffer;
            return ADD_HIGHER_ORDER_DATA_FAILED_FOR_OTHER_REASON;
        }

        thirdOrderTableGlobalIndex = *(uint32_t*)&blockBuffer[lastBlockInDirectoryThirdOrderTableLocalIndex * VALUE_ENTRY];
        delete[] blockBuffer;
    }

    //now add the data block
    uint32_t getLastBlockInDirectoryGlobalIndexResult = getDataBlockGlobalIndexByLocalIndexInsideInode(diskInfo, superBlock, inode,
                                                                                                       inode->i_blocks - 1, lastBlockInDirectoryGlobalIndex);
    if(getLastBlockInDirectoryGlobalIndexResult != GET_DATA_BLOCK_BY_LOCAL_INDEX_SUCCESS)
        return ADD_HIGHER_ORDER_DATA_FAILED_FOR_OTHER_REASON;

    uint32_t addDataBlockResult = searchAndOccupyFreeDataBlock(diskInfo, superBlock, lastBlockInDirectoryGlobalIndex, newBlockGlobalIndex);

    if(addDataBlockResult == SEARCH_EMPTY_DATA_BLOCK_NO_FREE_BLOCKS)
        return ADD_HIGHER_ORDER_DATA_BLOCK_NO_FREE_BLOCKS;
    else if(addDataBlockResult == SEARCH_EMPTY_DATA_BLOCK_FAILED_FOR_OTHER_REASON)
        return ADD_HIGHER_ORDER_DATA_FAILED_FOR_OTHER_REASON;

    //now add the new data block to the third order table
    uint32_t offset = occupiedDataBlocksInThirdLevel % entriesInOrderBlock;
    uint32_t addBlockIndexToAnotherResult = addBlockIndexToAnotherBlock(diskInfo, superBlock, thirdOrderTableGlobalIndex,
                                                                        offset * VALUE_ENTRY, newBlockGlobalIndex);

    return (addBlockIndexToAnotherResult == ADD_BLOCK_INDEX_TO_LOWER_ORDER_BLOCK_SUCCESS) ? ADD_HIGHER_ORDER_DATA_SUCCESS : ADD_HIGHER_ORDER_DATA_FAILED_FOR_OTHER_REASON;
}

static uint32_t addForthOrderDataBlock(DiskInfo* diskInfo, ext2_super_block* superBlock, ext2_inode* inode, uint32_t& newBlockGlobalIndex, uint32_t& secondOrderTableGlobalIndex)
{
    uint32_t thirdOrderTableGlobalIndex, forthOrderTableGlobalIndex, numberOfSectorsRead, lastBlockInDirectoryGlobalIndex;
    secondOrderTableGlobalIndex = inode->i_block[14];
    uint32_t entriesInOrderBlock = superBlock->s_log_block_size / VALUE_ENTRY;
    uint32_t occupiedDataBlocksInForthLevel = inode->i_blocks - 12 - entriesInOrderBlock - entriesInOrderBlock * entriesInOrderBlock;
    uint32_t offsetInForthLevelBlock = occupiedDataBlocksInForthLevel % entriesInOrderBlock;
    //in which third order table is the CAUTION INDEX FOR THE FORTH ORDER TABLE IN WHICH IS THE LAST DIRECTORY IN DIRECTORY
    int32_t lastBlockInDirectoryThirdOrderTableLocalIndex = occupiedDataBlocksInForthLevel / (entriesInOrderBlock * entriesInOrderBlock); //aka index in second order table
    if(occupiedDataBlocksInForthLevel % (entriesInOrderBlock * entriesInOrderBlock) == 0)
        lastBlockInDirectoryThirdOrderTableLocalIndex--;

    //in which forth order table is the last entry AKA index in third order table
    int32_t lastBlockInDirectoryForthOrderTableLocalIndex = occupiedDataBlocksInForthLevel % (entriesInOrderBlock * entriesInOrderBlock) / entriesInOrderBlock;
    if(occupiedDataBlocksInForthLevel % entriesInOrderBlock == 0)
        lastBlockInDirectoryForthOrderTableLocalIndex--;

    //check if there is a 2nd order block for level 4 (so no block for i_block[13]
    if(inode->i_blocks <= entriesInOrderBlock * entriesInOrderBlock + entriesInOrderBlock + 12) //it means there is no 2nd order block for level 4 (so no block for i_block[14]
    {
        uint32_t addSecondOrderTableResult = searchAndOccupyFreeDataBlock(diskInfo, superBlock, BIG_VALUE, secondOrderTableGlobalIndex);

        if(addSecondOrderTableResult == SEARCH_EMPTY_DATA_BLOCK_NO_FREE_BLOCKS)
            return ADD_HIGHER_ORDER_DATA_BLOCK_NO_FREE_BLOCKS;
        else if(addSecondOrderTableResult == SEARCH_EMPTY_DATA_BLOCK_FAILED_FOR_OTHER_REASON)
            return ADD_HIGHER_ORDER_DATA_FAILED_FOR_OTHER_REASON;
    }

    //check if there is a 3rd order block for level 4 with free space
    if(occupiedDataBlocksInForthLevel % (entriesInOrderBlock * entriesInOrderBlock) == 0) //we need to add a new 3rd order table
    {
        uint32_t addThirdOrderTableResult = searchAndOccupyFreeDataBlock(diskInfo, superBlock, BIG_VALUE, thirdOrderTableGlobalIndex);

        if(addThirdOrderTableResult == SEARCH_EMPTY_DATA_BLOCK_NO_FREE_BLOCKS)
            return ADD_HIGHER_ORDER_DATA_BLOCK_NO_FREE_BLOCKS;
        else if(addThirdOrderTableResult == SEARCH_EMPTY_DATA_BLOCK_FAILED_FOR_OTHER_REASON)
            return ADD_HIGHER_ORDER_DATA_FAILED_FOR_OTHER_REASON;

        //add the third order table to the second order table
        uint32_t addThirdOrderToSecondOrderResult = addBlockIndexToAnotherBlock(diskInfo, superBlock, secondOrderTableGlobalIndex,
                                                                                (lastBlockInDirectoryThirdOrderTableLocalIndex + 1) * VALUE_ENTRY,
                                                                                thirdOrderTableGlobalIndex);

        if(addThirdOrderToSecondOrderResult == ADD_BLOCK_INDEX_TO_LOWER_ORDER_BLOCK_FAILED)
            return ADD_HIGHER_ORDER_DATA_FAILED_FOR_OTHER_REASON;
    }
    else //we haven't added a new 3rd order block, so we need to retrieve its already existing index
    {
        char* blockBuffer = new char[superBlock->s_log_block_size];
        uint32_t readResult = readDiskSectors(diskInfo, getNumberOfSectorsPerBlock(diskInfo, superBlock),
                                              getFirstSectorForGivenBlock(diskInfo, superBlock, inode->i_block[14]),blockBuffer, numberOfSectorsRead);

        if(readResult != EC_NO_ERROR)
        {
            delete[] blockBuffer;
            return ADD_HIGHER_ORDER_DATA_FAILED_FOR_OTHER_REASON;
        }

        thirdOrderTableGlobalIndex = *(uint32_t*)&blockBuffer[lastBlockInDirectoryThirdOrderTableLocalIndex * VALUE_ENTRY];
        delete[] blockBuffer;
    }

    //check if there is a 4th order block with free space
    if(occupiedDataBlocksInForthLevel % entriesInOrderBlock == 0) //we need to add a new 4rd order table (calculation is correct % entriesInOrderBlock is good)
    {
        uint32_t addForthOrderTableResult = searchAndOccupyFreeDataBlock(diskInfo, superBlock, BIG_VALUE, forthOrderTableGlobalIndex);

        if(addForthOrderTableResult == SEARCH_EMPTY_DATA_BLOCK_NO_FREE_BLOCKS)
            return ADD_HIGHER_ORDER_DATA_BLOCK_NO_FREE_BLOCKS;
        else if(addForthOrderTableResult == SEARCH_EMPTY_DATA_BLOCK_FAILED_FOR_OTHER_REASON)
            return ADD_HIGHER_ORDER_DATA_FAILED_FOR_OTHER_REASON;

        //add the forth order table to the third order table
        uint32_t addForthOrderToThirdOrderResult = addBlockIndexToAnotherBlock(diskInfo, superBlock, thirdOrderTableGlobalIndex,
                                                                               (lastBlockInDirectoryForthOrderTableLocalIndex + 1) * VALUE_ENTRY,
                                                                               forthOrderTableGlobalIndex);

        if(addForthOrderToThirdOrderResult == ADD_BLOCK_INDEX_TO_LOWER_ORDER_BLOCK_FAILED)
            return ADD_HIGHER_ORDER_DATA_FAILED_FOR_OTHER_REASON;
    }
    else //we haven't added a new 4th order block, so we need to retrieve its already existing index
    {
        char* blockBuffer = new char[superBlock->s_log_block_size];
        uint32_t readResult = readDiskSectors(diskInfo, getNumberOfSectorsPerBlock(diskInfo, superBlock),
                                              getFirstSectorForGivenBlock(diskInfo, superBlock, thirdOrderTableGlobalIndex),blockBuffer, numberOfSectorsRead);

        if(readResult != EC_NO_ERROR)
        {
            delete[] blockBuffer;
            return ADD_HIGHER_ORDER_DATA_FAILED_FOR_OTHER_REASON;
        }

        forthOrderTableGlobalIndex = *(uint32_t*)&blockBuffer[lastBlockInDirectoryForthOrderTableLocalIndex * VALUE_ENTRY];
        delete[] blockBuffer;
    }

    //now add the data block
    uint32_t getLastBlockInDirectoryGlobalIndexResult = getDataBlockGlobalIndexByLocalIndexInsideInode(diskInfo, superBlock, inode,
                                                                                                       inode->i_blocks - 1, lastBlockInDirectoryGlobalIndex);
    if(getLastBlockInDirectoryGlobalIndexResult != GET_DATA_BLOCK_BY_LOCAL_INDEX_SUCCESS)
        return ADD_HIGHER_ORDER_DATA_FAILED_FOR_OTHER_REASON;

    uint32_t addDataBlockResult = searchAndOccupyFreeDataBlock(diskInfo, superBlock, lastBlockInDirectoryGlobalIndex, newBlockGlobalIndex);

    if(addDataBlockResult == SEARCH_EMPTY_DATA_BLOCK_NO_FREE_BLOCKS)
        return ADD_HIGHER_ORDER_DATA_BLOCK_NO_FREE_BLOCKS;
    else if(addDataBlockResult == SEARCH_EMPTY_DATA_BLOCK_FAILED_FOR_OTHER_REASON)
        return ADD_HIGHER_ORDER_DATA_FAILED_FOR_OTHER_REASON;

    //now add the new data block to the forth order table
    uint32_t addBlockIndexToAnotherResult = addBlockIndexToAnotherBlock(diskInfo, superBlock, forthOrderTableGlobalIndex,
                                                                        offsetInForthLevelBlock * VALUE_ENTRY, newBlockGlobalIndex);

    return (addBlockIndexToAnotherResult == ADD_BLOCK_INDEX_TO_LOWER_ORDER_BLOCK_SUCCESS) ? ADD_HIGHER_ORDER_DATA_SUCCESS : ADD_HIGHER_ORDER_DATA_FAILED_FOR_OTHER_REASON;
}

///////////////////////////////////////
///////////////////////////////////////

static uint32_t addBlockIndexToAnotherBlock(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t blockToAddTheValueTo, uint32_t offsetInBlockToAddTheValueTo, uint32_t valueToAdd)
{
    uint32_t numberOfSectorsRead, numOfSectorsWritten;
    uint32_t numOfSectorsPerBlock = getNumberOfSectorsPerBlock(diskInfo, superBlock);
    char* blockBuffer = new char[superBlock->s_log_block_size];

    uint32_t readResult = readDiskSectors(diskInfo, numOfSectorsPerBlock, getFirstSectorForGivenBlock(diskInfo, superBlock, blockToAddTheValueTo),
                                          blockBuffer, numberOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        delete[] blockBuffer;
        return ADD_BLOCK_INDEX_TO_LOWER_ORDER_BLOCK_FAILED;
    }

    memcpy(blockBuffer + offsetInBlockToAddTheValueTo, &valueToAdd, 4);

    uint32_t writeResult = writeDiskSectors(diskInfo , numOfSectorsPerBlock, getFirstSectorForGivenBlock(diskInfo, superBlock, blockToAddTheValueTo),
                                            blockBuffer, numOfSectorsWritten);

    delete[] blockBuffer;
    return (writeResult == EC_NO_ERROR) ? ADD_BLOCK_INDEX_TO_LOWER_ORDER_BLOCK_SUCCESS : ADD_BLOCK_INDEX_TO_LOWER_ORDER_BLOCK_FAILED;
}

static uint32_t searchAndOccupyFreeDataBlock(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t previousBlockGlobalIndex, uint32_t& newDataBlockGlobalIndex)
{
    uint32_t searchFreeBlockResult;

    if(previousBlockGlobalIndex != BIG_VALUE) //if it is given BIG_VALUE it means we don't care about adding the block after a certain one, or in a certain group
    {
        uint32_t previousBlockGroup = previousBlockGlobalIndex / superBlock->s_blocks_per_group;
        uint32_t previousBlockLocalIndex = previousBlockGlobalIndex % superBlock->s_blocks_per_group;

        searchFreeBlockResult = searchAndOccupyFreeDataBlockInGivenGroup(diskInfo, superBlock, previousBlockGroup, previousBlockLocalIndex, newDataBlockGlobalIndex);

        if (searchFreeBlockResult == SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_SUCCESS)
            return SEARCH_EMPTY_DATA_BLOCK_SUCCESS;
        else if (searchFreeBlockResult == SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_FAILED_FOR_OTHER_REASON)
            return SEARCH_EMPTY_DATA_BLOCK_FAILED_FOR_OTHER_REASON;
    }

    for(uint32_t group = 0; group < getNumberOfGroups(superBlock); group++)
    {
        searchFreeBlockResult = searchAndOccupyFreeDataBlockInGivenGroup(diskInfo, superBlock, group, BIG_VALUE, newDataBlockGlobalIndex);
        if(searchFreeBlockResult == SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_SUCCESS)
        {
            uint32_t updateDataBlockBitmapResult = updateValueInDataBlockBitmap(diskInfo, superBlock, newDataBlockGlobalIndex, 1);
            if(updateDataBlockBitmapResult == UPDATE_VALUE_IN_DATA_BLOCK_BITMAP_FAILED)
                return SEARCH_EMPTY_DATA_BLOCK_FAILED_FOR_OTHER_REASON;

            uint32_t groupDescriptorUpdateResult = updateGroupDescriptor(diskInfo, superBlock, newDataBlockGlobalIndex, 0, 1);
            return (groupDescriptorUpdateResult == UPDATE_GROUP_DESCRIPTOR_SUCCESS) ? SEARCH_EMPTY_DATA_BLOCK_SUCCESS : SEARCH_EMPTY_DATA_BLOCK_FAILED_FOR_OTHER_REASON;
        }
        else if(searchFreeBlockResult == SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_FAILED_FOR_OTHER_REASON)
            return SEARCH_EMPTY_DATA_BLOCK_FAILED_FOR_OTHER_REASON;
    }

    return SEARCH_EMPTY_DATA_BLOCK_NO_FREE_BLOCKS;
}

static uint32_t searchAndOccupyFreeDataBlockInGivenGroup(DiskInfo* diskInfo, ext2_super_block* superBlock, uint32_t group, uint32_t previousBlockLocalIndex, uint32_t& newDataBlock)
{
    uint32_t numOfSectorsPerBlock = getNumberOfSectorsPerBlock(diskInfo, superBlock);
    uint32_t dataBitmapBlock = getDataBitmapBlockForGivenGroup(superBlock, group);

    char* blockBuffer = new char[superBlock->s_log_block_size];
    uint32_t numberOfSectorsRead = 0;
    uint32_t readResult = readDiskSectors(diskInfo, numOfSectorsPerBlock, getFirstSectorForGivenBlock(diskInfo, superBlock, dataBitmapBlock),
                                          blockBuffer, numberOfSectorsRead);

    if(readResult != EC_NO_ERROR)
    {
        delete[] blockBuffer;
        return SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_FAILED_FOR_OTHER_REASON;
    }

    //checks the block after the given previousBlock

    uint32_t preferredLocation = previousBlockLocalIndex + 1;
    uint32_t preferredBlockLocalIndexInDataBlockList = getDataBlockLocalIndexInLocalListOfDataBlocksByGlobalIndex(superBlock, preferredLocation);
    if(previousBlockLocalIndex != BIG_VALUE && getBitFromByte(blockBuffer[preferredBlockLocalIndexInDataBlockList / 8], preferredBlockLocalIndexInDataBlockList % 8) == 0)
    {
        uint8_t newByteValue = changeBitValue(blockBuffer[preferredBlockLocalIndexInDataBlockList / 8], preferredBlockLocalIndexInDataBlockList % 8, 1);
        memset(blockBuffer + preferredBlockLocalIndexInDataBlockList / 8, newByteValue, 1);

        uint32_t numOfSectorsWritten;
        uint32_t writeResult = writeDiskSectors(diskInfo , numOfSectorsPerBlock, getFirstSectorForGivenBlock(diskInfo, superBlock, dataBitmapBlock),
                                                blockBuffer, numOfSectorsWritten);

        newDataBlock = preferredLocation;
        delete[] blockBuffer;

        return (writeResult != EC_NO_ERROR) ? SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_FAILED_FOR_OTHER_REASON : SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_SUCCESS;
    }

    for(uint32_t bitIndex = 0; bitIndex < getNumberOfDataBlocksForGivenGroup(superBlock, group); bitIndex++)
        if(getBitFromByte(blockBuffer[bitIndex / 8], bitIndex % 8) == 0)
        {
            uint8_t newByteValue = changeBitValue(blockBuffer[bitIndex / 8], bitIndex % 8, 1);
            memset(blockBuffer + bitIndex / 8, newByteValue, 1);

            uint32_t numOfSectorsWritten;
            uint32_t writeResult = writeDiskSectors(diskInfo , numOfSectorsPerBlock, getFirstSectorForGivenBlock(diskInfo, superBlock, dataBitmapBlock),
                                                    blockBuffer, numOfSectorsWritten);

            newDataBlock = getFirstDataBlockForGivenGroup(superBlock, group) + bitIndex;
            delete[] blockBuffer;

            return (writeResult != EC_NO_ERROR) ? SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_FAILED_FOR_OTHER_REASON : SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_SUCCESS;
        }

    return SEARCH_EMPTY_DATA_BLOCK_IN_GROUP_NO_FREE_BLOCKS;
}