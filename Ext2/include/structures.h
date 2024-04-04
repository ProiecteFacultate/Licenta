#include <stdint.h>

#ifndef EXT2_STRUCTURES_H
#define EXT2_STRUCTURES_H

#define EXT2_N_BLOCKS                         15

typedef struct
{
    uint32_t s_inodes_count;
    uint32_t s_block_count;
    uint32_t s_r_blocks_count;
    uint32_t s_free_blocks_count;
    uint32_t s_free_inodes_count;
    uint32_t s_first_data_block;
    uint32_t s_log_block_size;
    uint32_t s_log_frag_size;
    uint32_t s_blocks_per_group;
    uint32_t s_frags_per_group;
    uint32_t s_inodes_per_group;
    uint32_t s_mtime;
    uint32_t s_wtime;
    uint16_t s_mnt_count;
    uint16_t s_max_mnt_count;
    uint16_t s_magic;
    uint16_t s_state;
    uint16_t s_errors;
    uint16_t s_minor_rev_level;
    uint32_t s_lastcheck;
    uint32_t s_checkinterval;
    uint32_t s_creator_os;
    uint32_t s_rev_level;
    uint16_t s_def_resuid;
    uint16_t s_def_resgid;
    uint32_t s_first_ino;
    uint16_t s_inode_size;
    uint16_t s_block_group_nr;
    uint32_t s_feature_compat;
    uint32_t s_feature_incompat;
    uint32_t s_feature_ro_compat;
    uint8_t s_uuid[16];
    char s_volume_name[16];
    char s_last_mounted[64];
    uint32_t s_algorithm_usage_bitmap;
    uint8_t s_prealloc_blocks;
    uint8_t s_prealloc_dir_blocks;
    uint16_t s_padding1;
    uint32_t s_reserved[204];
} __attribute__((packed)) ext2_super_block;

typedef struct
{
    uint32_t bg_block_bitmap;
    uint32_t bg_inode_bitmap;
    uint32_t bg_inode_table;
    uint16_t bg_free_blocks_count;
    uint16_t bg_free_inodes_count;
    uint16_t bg_used_dirs_count;
    uint16_t bg_pad;
    uint32_t bg_reserved[3];
} __attribute__((packed)) ext2_group_desc;

typedef struct
{
    uint16_t i_mode;
    uint16_t i_uid;
    uint32_t i_size;
    uint32_t i_atime;
    uint32_t i_ctime;
    uint32_t i_mtime;
    uint32_t i_dtime;
    uint16_t i_gid;
    uint16_t i_links_count;
    uint32_t i_blocks;
    uint32_t i_flags;
  //  union osd1;
    uint32_t i_block[EXT2_N_BLOCKS];
    uint32_t i_generation;
    uint32_t i_file_acl;
    uint32_t i_dir_acl;
    uint32_t i_faddr;
  //  union osd2;
    uint32_t i_group; //this field is not part of normal ext2_inode structure, we add it for convenience; we take 2 array entries from i_padding
    uint32_t i_global_index; //this field is not part of normal ext2_inode structure, we add it for convenience; we take 2 array entries from i_padding
    uint16_t i_padding[4]; //padding to 128 bytes
} __attribute__((packed)) ext2_inode;

typedef struct
{
    uint32_t inode;
    uint16_t rec_len;
    uint8_t name_len;
    uint8_t file_type;
    char name[120]; //normally the directory entry structure is of dynamic size, and so does the name, but to make it easier, we make it 128 bytes, and the name max 120
} __attribute__((packed)) ext2_dir_entry;


#endif
