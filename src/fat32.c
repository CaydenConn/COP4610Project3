#include <stdio.h>
#include <string.h>
#include "fat32.h"

int fat32_open(FAT32 *fs, const char *filename) {
    // Clear the struct first
    memset(fs, 0, sizeof(FAT32));

    // Open the image in read/write mode
    fs->fp = fopen(filename, "rb+");
    if (fs->fp == NULL) {
        fprintf(stderr, "Error: %s does not exist.\n", filename);
        return -1;
    }

    // Read the boot sector
    if (fread(&fs->bs, sizeof(FAT32BootSector), 1, fs->fp) != 1) {
        fprintf(stderr, "Error: failed to read boot sector.\n");
        fclose(fs->fp);
        fs->fp = NULL;
        return -1;
    }

    // Save image name and start at root path
    strncpy(fs->image_name, filename, sizeof(fs->image_name) - 1);
    strcpy(fs->current_path, "/");
    fs->current_cluster = fs->bs.BPB_RootClus;

    return 0;
}

void fat32_close(FAT32 *fs) {
    // Only close if it is open
    if (fs->fp != NULL) {
        fclose(fs->fp);
        fs->fp = NULL;
    }
}

long fat32_image_size(FAT32 *fs) {
    // Save current file position
    long old_pos = ftell(fs->fp);

    // Move to end to get file size
    fseek(fs->fp, 0, SEEK_END);
    long size = ftell(fs->fp);

    // Go back to old position
    fseek(fs->fp, old_pos, SEEK_SET);

    return size;
}

uint32_t fat32_total_sectors(FAT32 *fs) {
    // if Fat32 is TotSec16
    if (fs->bs.BPB_TotSec16 != 0) {
        return fs->bs.BPB_TotSec16;
    }
    return fs->bs.BPB_TotSec32;
}

uint32_t fat32_fat_size(FAT32 *fs) {
    // If FAT32 uses FATSz16
    if (fs->bs.BPB_FATSz16 != 0) {
        return fs->bs.BPB_FATSz16;
    }
    return fs->bs.BPB_FATSz32;
}

uint32_t fat32_first_data_sector(FAT32 *fs) {
    // First data sector = reserved sectors + all FAT sectors
    return fs->bs.BPB_RsvdSecCnt + (fs->bs.BPB_NumFATs * fat32_fat_size(fs));
}

uint32_t fat32_total_clusters(FAT32 *fs) {
    // Data sectors = total sectors - first data sector
    uint32_t data_sectors =
        fat32_total_sectors(fs) - fat32_first_data_sector(fs);

    // Total clusters = data sectors / sectors per cluster
    return data_sectors / fs->bs.BPB_SecPerClus;
}

// Returns Fat byte by entry
uint32_t fat32_entries_per_fat(FAT32 *fs) {
    uint32_t fat_bytes = fat32_fat_size(fs) * fs->bs.BPB_BytsPerSec;
    return fat_bytes / 4;
}

// Prints fat32 info
void fat32_print_info(FAT32 *fs) {
    printf("position of root cluster (in cluster #): %u\n", fs->bs.BPB_RootClus);
    printf("bytes per sector: %u\n", fs->bs.BPB_BytsPerSec);
    printf("sectors per cluster: %u\n", fs->bs.BPB_SecPerClus);
    printf("total # of clusters in data region: %u\n", fat32_total_clusters(fs));
    printf("# of entries in one FAT: %u\n", fat32_entries_per_fat(fs));
    printf("size of image (in bytes): %ld\n", fat32_image_size(fs));
}
