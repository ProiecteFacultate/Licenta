#include <stdint.h>

#ifndef FAT32_STRUCTURES_H
#define FAT32_STRUCTURES_H

typedef struct
{                                                //144MiB -> 288.000 sectors -> 4,500 clusters (64 sectors/cluster)
    //512Kib-> 1024 sectors -> 64 clusters (16 sectors/cluster)
    // bios parameter block
    uint8_t BootJumpInstruction[3];
    uint8_t OemIdentifier[8];                    //'MSWIN4.1'         -fixed
    uint16_t BytesPerSector;                     //512
    uint8_t SectorsPerCluster;                   //16                 //TODO constantly changing
    uint16_t ReservedSectors;                    //32                 -fixed
    uint8_t FatCount;                            //2                  -fixed
    uint16_t RootDirEntryCount;                  //0                  -fixed
    uint16_t TotalSectors;                       //0                  -fixed
    uint8_t MediaDescriptorType;                 //99                 -fixed (doesn't matter)
    uint16_t X_SectorsPerFat;                    //0                  -fixed
    uint16_t SectorsPerTrack;                    //99                 -doesn't matter
    uint16_t Heads;                              //99                 -doesn't matter
    uint32_t HiddenSectors;                      //0                  -since we don't have partitions
    uint32_t LargeSectorCount;                   //0                  -may change

    // extended boot record
    uint32_t SectorsPerFat;                      //36
    uint16_t Flags;                              //20224 (0x4F00)     -fixed
    uint16_t FatVersion;                         //0 ??               -fixed (doesn't matter)
    uint32_t RootDirCluster;                     //2                  -usually 2 but can change (keep always 2 or things will break)
    uint16_t FsInfoSector;                       //1                  -usually 1 but can change
    uint16_t BackupBootSector;                   //6                  -usually 6 but can change
    uint8_t Reserved_1[12];                      //0
    uint8_t Drive;                               //0                  -doesn't matter
    uint8_t FlagsWinNT;                          //0                  -doesn't matter
    uint8_t Signature;                           //0x99               -fixed (doesn't matter)
    uint32_t VolumeId;                           //0                  -doesn't matter
    uint8_t VolumeLabel[11];                     //LABEL              -doesn't matter
    uint8_t SystemId[8];                         //FAT32              -fixed
    uint16_t BootSignature;                      //43605 (0xAA55)     -fixed (end of sector)

} __attribute__((packed)) BootSector;

typedef struct
{
    uint32_t LeadSignature;                      //0x41615252     -fixed (beginning of sector)
    uint32_t AnotherSignature;                   //0x61417272     -fixed
    uint32_t LastKnownFreeClusterCount;          //0xFFFFFFFF     -changes at runtime
    uint32_t FirstClusterToCheckAvailability;    //0xFFFFFFFF     -changes at runtime
    uint8_t Reserved[12];                        //0              -doesn't matter
    uint32_t TrailSignature;                     //0xAA550000     -fixed

} __attribute((packed)) FsInfo;

typedef struct
{
    uint8_t FileName[11];
    uint8_t Attributes;
    uint8_t Reserved;
    uint8_t CreationTimeTenths;
    uint16_t CreationTime;
    uint16_t CreationDate;
    uint16_t LastAccessedDate;
    uint16_t FirstClusterHigh;
    uint16_t LastWriteTime;
    uint16_t LastWriteDate;
    uint16_t FirstClusterLow;
    uint32_t FileSize; //for directories containing subdirectories, this counts only the bytes in the clusters corresponding to this directory entry, without its subdirectories size
} __attribute__((packed)) DirectoryEntry;

typedef struct
{
    uint32_t FileSize;
    uint32_t FileSizeOnDisk;
    uint16_t CreationYear;
    uint16_t CreationMonth;
    uint16_t CreationDay;
    uint16_t CreationHour;
    uint16_t CreationMinute;
    uint16_t CreationSecond;
    uint16_t LastAccessedYear;
    uint16_t LastAccessedMonth;
    uint16_t LastAccessedDay;
    uint16_t LastWriteYear;
    uint16_t LastWriteMonth;
    uint16_t LastWriteDay;
    uint16_t LastWriteHour;
    uint16_t LastWriteMinute;
    uint16_t LastWriteSecond;
} __attribute__((packed)) DirectoryDisplayableAttributes;

#endif
