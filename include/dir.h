#pragma once

#include <stdint.h>
#include "fat32.h"

// FAT32 directory entry
typedef struct __attribute__((packed)) {
    uint8_t  DIR_Name[11];
    uint8_t  DIR_Attr;
    uint8_t  DIR_NTRes;
    uint8_t  DIR_CrtTimeTenth;
    uint16_t DIR_CrtTime;
    uint16_t DIR_CrtDate;
    uint16_t DIR_LstAccDate;
    uint16_t DIR_FstClusHI;
    uint16_t DIR_WrtTime;
    uint16_t DIR_WrtDate;
    uint16_t DIR_FstClusLO;
    uint32_t DIR_FileSize;
} DirEntry;


uint32_t cluster_to_offset(FAT32 *fs, uint32_t cluster);

uint32_t fat32_next_cluster(FAT32 *fs, uint32_t cluster);

int fat32_is_eoc(uint32_t cluster);

int dir_is_unused(const DirEntry *entry);
int dir_is_lfn(const DirEntry *entry);
int dir_is_directory(const DirEntry *entry);

uint32_t dir_entry_cluster(const DirEntry *entry);

void dir_entry_name_to_string(const DirEntry *entry, char *out);

int dir_find_entry(FAT32 *fs, uint32_t dir_cluster, const char *name, DirEntry *result);

void dir_list(FAT32 *fs, uint32_t dir_cluster);


void path_go_into(FAT32 *fs, const char *name);
void path_go_up(FAT32 *fs);

// Convert user input name to FAT32 8.3 format
void dir_make_fat_name(const char *input, char fat_name[11]);

// Add a 32-byte directory entry to a directory's cluster chain (returns 0 on success, -1 on failure)
int dir_add_entry(FAT32 *fs, uint32_t dir_cluster, const DirEntry *entry);