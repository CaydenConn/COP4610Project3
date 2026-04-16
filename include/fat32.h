#pragma once

#include <stdint.h>
#include <stdio.h>

// Stores the FAT32 boot sector fields
typedef struct __attribute__((packed)) {
    uint8_t  BS_jmpBoot[3];
    uint8_t  BS_OEMName[8];
    uint16_t BPB_BytsPerSec;     
    uint8_t  BPB_SecPerClus;     
    uint16_t BPB_RsvdSecCnt;     
    uint8_t  BPB_NumFATs;        
    uint16_t BPB_RootEntCnt;
    uint16_t BPB_TotSec16;
    uint8_t  BPB_Media;
    uint16_t BPB_FATSz16;
    uint16_t BPB_SecPerTrk;
    uint16_t BPB_NumHeads;
    uint32_t BPB_HiddSec;
    uint32_t BPB_TotSec32;       
    uint32_t BPB_FATSz32;        
    uint16_t BPB_ExtFlags;
    uint16_t BPB_FSVer;
    uint32_t BPB_RootClus;       
    uint16_t BPB_FSInfo;
    uint16_t BPB_BkBootSec;
    uint8_t  BPB_Reserved[12];
    uint8_t  BS_DrvNum;
    uint8_t  BS_Reserved1;
    uint8_t  BS_BootSig;
    uint32_t BS_VolID;
    uint8_t  BS_VolLab[11];
    uint8_t  BS_FilSysType[8];
} FAT32BootSector;

// Keeps track of the opened image and shell state
typedef struct {
    FILE *fp;
    FAT32BootSector bs;
    char image_name[256];
    char current_path[256];
    uint32_t current_cluster;
} FAT32;

// Open the image and read the boot sector
int fat32_open(FAT32 *fs, const char *filename);

// Close image
void fat32_close(FAT32 *fs);

// Get total image size in bytes
long fat32_image_size(FAT32 *fs);

// Get total number of sectors
uint32_t fat32_total_sectors(FAT32 *fs);

// Get FAT size in sectors
uint32_t fat32_fat_size(FAT32 *fs);

// Get first data sector
uint32_t fat32_first_data_sector(FAT32 *fs);

// Get total number of clusters in data region
uint32_t fat32_total_clusters(FAT32 *fs);

// Get number of FAT entries
uint32_t fat32_entries_per_fat(FAT32 *fs);

// Print the values needed for the info command
void fat32_print_info(FAT32 *fs);
