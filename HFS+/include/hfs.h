#include "disk.h"
#include "structures.h"

#ifndef HFS__HFS_H
#define HFS__HFS_H

//CRITICAL
void updateVolumeHeaderNodeOnDisk(DiskInfo* diskInfo, HFSPlusVolumeHeader* volumeHeader, HFSPlusVolumeHeader* updatedVolumeHeader);

#endif