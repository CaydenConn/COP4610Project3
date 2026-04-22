#include "fat32.h"
#include <stdio.h>
#include <string.h>
#include "dir.h"

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
  uint32_t data_sectors = fat32_total_sectors(fs) - fat32_first_data_sector(fs);

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

uint32_t fat32_find_free_cluster(FAT32 *fs) {
  uint32_t total = fat32_total_clusters(fs);
  uint32_t fat_start = fs->bs.BPB_RsvdSecCnt * fs->bs.BPB_BytsPerSec;
  uint32_t value;

  for (uint32_t i = 2; i < total + 2; i++) {
    uint32_t fat_offset = i * 4;
    fseek(fs->fp, fat_start + fat_offset, SEEK_SET);
    fread(&value, sizeof(uint32_t), 1, fs->fp);
    value &= 0x0FFFFFFF;

    if (value == 0) {
      return i;
    }
  }
  return 0; // No free space
}

void fat32_write_fat_entry(FAT32 *fs, uint32_t cluster, uint32_t value) {
  uint32_t fat_start = fs->bs.BPB_RsvdSecCnt * fs->bs.BPB_BytsPerSec;
  uint32_t fat_offset = cluster * 4;
  uint32_t existing;

  for (int i = 0; i < fs->bs.BPB_NumFATs; i++) {
     uint32_t current_fat_start = fat_start + (i * fat32_fat_size(fs) * fs->bs.BPB_BytsPerSec);
     fseek(fs->fp, current_fat_start + fat_offset, SEEK_SET);
     fread(&existing, sizeof(uint32_t), 1, fs->fp);
     existing = (existing & 0xF0000000) | (value & 0x0FFFFFFF);
     fseek(fs->fp, current_fat_start + fat_offset, SEEK_SET);
     fwrite(&existing, sizeof(uint32_t), 1, fs->fp);
  }
}

// Frees each cluster in a cluster chain by writing 0x00000000 to its FAT entry
void fat32_free_cluster_chain(FAT32 *fs, uint32_t start_cluster) {
  uint32_t current = start_cluster;

  if (current < 2) {
    return; // invalid cluster
  }

  while (1) {
    uint32_t next = fat32_next_cluster(fs, current);
    fat32_write_fat_entry(fs, current, 0x00000000);

    if (fat32_is_eoc(next)) {
      break;
    }
    current = next;
  }
}
