#include "dir.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Converts a cluster number into byte offset in image
uint32_t cluster_to_offset(FAT32 *fs, uint32_t cluster) {
  uint32_t first_data_sector;
  uint32_t first_sector_of_cluster;

  first_data_sector = fat32_first_data_sector(fs);
  first_sector_of_cluster =
      ((cluster - 2) * fs->bs.BPB_SecPerClus) + first_data_sector;

  return first_sector_of_cluster * fs->bs.BPB_BytsPerSec;
}

// Gets the next cluster from FAT
uint32_t fat32_next_cluster(FAT32 *fs, uint32_t cluster) {
  uint32_t fat_offset;
  uint32_t fat_start;
  uint32_t value = 0;

  fat_offset = cluster * 4;
  fat_start = fs->bs.BPB_RsvdSecCnt * fs->bs.BPB_BytsPerSec;

  fseek(fs->fp, fat_start + fat_offset, SEEK_SET);
  fread(&value, sizeof(uint32_t), 1, fs->fp);

  // FAT32 only uses the low 28 bits
  value &= 0x0FFFFFFF;

  return value;
}

// Checks if a cluster value means end of chain
int fat32_is_eoc(uint32_t cluster) { return cluster >= 0x0FFFFFF8; }

// Helper checks for directory entries
int dir_is_unused(const DirEntry *entry) {
  return entry->DIR_Name[0] == 0x00 || entry->DIR_Name[0] == 0xE5;
}

int dir_is_lfn(const DirEntry *entry) { return entry->DIR_Attr == 0x0F; }

int dir_is_directory(const DirEntry *entry) {
  return (entry->DIR_Attr & 0x10) != 0;
}

// Gets the full starting cluster from HI and LO
uint32_t dir_entry_cluster(const DirEntry *entry) {
  return ((uint32_t)entry->DIR_FstClusHI << 16) | entry->DIR_FstClusLO;
}

// Converts the FAT name into string
void dir_entry_name_to_string(const DirEntry *entry, char *out) {
  int i;
  int pos = 0;

  // handling . and .. first
  if (entry->DIR_Name[0] == '.') {
    out[0] = '.';

    if (entry->DIR_Name[1] == '.') {
      out[1] = '.';
      out[2] = '\0';
    } else {
      out[1] = '\0';
    }

    return;
  }

  // copy first 8 chars of name until spaces
  for (i = 0; i < 8 && entry->DIR_Name[i] != ' '; i++) {
    out[pos++] = (char)entry->DIR_Name[i];
  }

  // Add if there is an extension
  if (entry->DIR_Name[8] != ' ') {
    out[pos++] = '.';

    for (i = 8; i < 11 && entry->DIR_Name[i] != ' '; i++) {
      out[pos++] = (char)entry->DIR_Name[i];
    }
  }

  out[pos] = '\0';
}

static void make_fat_name(const char *input, char fat_name[11]) {
  int i;
  int j = 0;

  for (i = 0; i < 11; i++) {
    fat_name[i] = ' ';
  }

  // Handle special dot directories explicitly
  if (strcmp(input, ".") == 0) {
      fat_name[0] = '.';
      return;
  }
  if (strcmp(input, "..") == 0) {
      fat_name[0] = '.';
      fat_name[1] = '.';
      return;
  }

  for (i = 0; input[i] != '\0'; i++) {
    if (input[i] == '.') {
      j = 8;
      continue;
    }

    fat_name[j++] = toupper((unsigned char)input[i]);
  }
}

// Prints the current directory
int dir_find_entry(FAT32 *fs, uint32_t dir_cluster, const char *name,
                   DirEntry *result) {
  DirEntry entry;
  uint32_t cluster;
  uint32_t entries_per_cluster;
  uint32_t i;
  char fat_name[11];

  make_fat_name(name, fat_name);

  cluster = dir_cluster;
  entries_per_cluster =
      (fs->bs.BPB_BytsPerSec * fs->bs.BPB_SecPerClus) / sizeof(DirEntry);

  while (1) {
    fseek(fs->fp, cluster_to_offset(fs, cluster), SEEK_SET); 

    for (i = 0; i < entries_per_cluster; i++) {
      fread(&entry, sizeof(DirEntry), 1, fs->fp);

      // 0x00 means nothing else after this
      if (entry.DIR_Name[0] == 0x00) {
        return 0;
      }

      if (dir_is_unused(&entry) || dir_is_lfn(&entry)) {
        continue;
      }

      if (memcmp(entry.DIR_Name, fat_name, 11) == 0) {
        if (result != NULL) {
          *result = entry;
        }
        return 1;
      }
    }

    cluster = fat32_next_cluster(fs, cluster);

    if (fat32_is_eoc(cluster)) {
      break;
    }
  }

  return 0;
}

// Prints the current directory
void dir_list(FAT32 *fs, uint32_t dir_cluster) {
  DirEntry entry;
  uint32_t cluster;
  uint32_t entries_per_cluster;
  uint32_t i;
  char name[20];

  cluster = dir_cluster;
  entries_per_cluster =
      (fs->bs.BPB_BytsPerSec * fs->bs.BPB_SecPerClus) / sizeof(DirEntry);

  while (1) {
    fseek(fs->fp, cluster_to_offset(fs, cluster), SEEK_SET); 

    for (i = 0; i < entries_per_cluster; i++) {
      fread(&entry, sizeof(DirEntry), 1, fs->fp);

      if (entry.DIR_Name[0] == 0x00) {
        return;
      }

      if (dir_is_unused(&entry) || dir_is_lfn(&entry)) {
        continue;
      }

      dir_entry_name_to_string(&entry, name);
      printf("%s\n", name);
    }

    cluster = fat32_next_cluster(fs, cluster);

    if (fat32_is_eoc(cluster)) {
      break;
    }
  }
}

// Updates the path string for cd
void path_go_into(FAT32 *fs, const char *name) {
  if (strcmp(fs->current_path, "/") == 0) {
    strcat(fs->current_path, name);
  } else {
    strcat(fs->current_path, "/");
    strcat(fs->current_path, name);
  }
}

void path_go_up(FAT32 *fs) {
  char *last_slash;

  if (strcmp(fs->current_path, "/") == 0) {
    return;
  }

  last_slash = strrchr(fs->current_path, '/');

  if (last_slash == fs->current_path) {
    fs->current_path[1] = '\0';
  } else if (last_slash != NULL) {
    *last_slash = '\0';
  }
}

// Public wrapper so other files can use make_fat_name
void dir_make_fat_name(const char *input, char fat_name[11]) {
  make_fat_name(input, fat_name);
}

// Find a free 32-byte slot in the directory's cluster chain and write
//   the new entry there. If no slot exists, allocate a new cluster and
//  append it to the chain. Returns 0 on success, -1 on failure.
int dir_add_entry(FAT32 *fs, uint32_t dir_cluster, const DirEntry *entry) {
  uint32_t cluster = dir_cluster;
  uint32_t entries_per_cluster;
  uint32_t bytes_per_cluster;
  uint32_t i;
  DirEntry existing;
  uint32_t prev_cluster;

  entries_per_cluster =
      (fs->bs.BPB_BytsPerSec * fs->bs.BPB_SecPerClus) / sizeof(DirEntry);
  bytes_per_cluster = fs->bs.BPB_BytsPerSec * fs->bs.BPB_SecPerClus;

  while (1) {
    fseek(fs->fp, cluster_to_offset(fs, cluster), SEEK_SET); 

    for (i = 0; i < entries_per_cluster; i++) {
      fread(&existing, sizeof(DirEntry), 1, fs->fp);

      // 0x00 = never used, 0xE5 = deleted — both are free slots
      if (existing.DIR_Name[0] == 0x00 || existing.DIR_Name[0] == 0xE5) {

        // Seek back to this slot and write the new entry
        fseek(fs->fp, cluster_to_offset(fs, cluster) + (i * sizeof(DirEntry)),
              SEEK_SET);
        fwrite(entry, sizeof(DirEntry), 1, fs->fp);
        return 0;
      }
    }

    // Move to next cluster in chain
    prev_cluster = cluster;
    cluster = fat32_next_cluster(fs, cluster);

    if (fat32_is_eoc(cluster)) {
      // No space left — allocate a new cluster for the directory
      uint32_t new_cluster = fat32_find_free_cluster(fs);
      uint8_t *zeros;

      if (new_cluster == 0) {
        return -1; // disk full
      }

      // Link previous last cluster -> new cluster
      fat32_write_fat_entry(fs, prev_cluster, new_cluster);

      // Mark new cluster as end-of-chain
      fat32_write_fat_entry(fs, new_cluster, 0x0FFFFFF8);

      // Zero out the new cluster
      zeros = calloc(bytes_per_cluster, 1);
      if (zeros == NULL) {
        return -1;
      }
      fseek(fs->fp, cluster_to_offset(fs, new_cluster), SEEK_SET);
      fwrite(zeros, 1, bytes_per_cluster, fs->fp);
      free(zeros);

      // Write the entry at the start of the new cluster
      fseek(fs->fp, cluster_to_offset(fs, new_cluster), SEEK_SET);
      fwrite(entry, sizeof(DirEntry), 1, fs->fp);
      return 0;
    }
  }
}

int dir_update_entry(FAT32 *fs, uint32_t dir_cluster, const char *name, const DirEntry *entry) {
  DirEntry existing;
  uint32_t cluster;
  uint32_t entries_per_cluster;
  uint32_t i;
  char fat_name[11];
  
  dir_make_fat_name(name, fat_name);
  
  cluster = dir_cluster;
  entries_per_cluster = (fs->bs.BPB_BytsPerSec * fs->bs.BPB_SecPerClus) / sizeof(DirEntry);
  
  while (1) {
    fseek(fs->fp, cluster_to_offset(fs, cluster), SEEK_SET); 
    
    for (i = 0; i < entries_per_cluster; i++) {
      fread(&existing, sizeof(DirEntry), 1, fs->fp);
      
      if (existing.DIR_Name[0] == 0x00) {
        return -1; // Not found
      }
      
      if (dir_is_unused(&existing) || dir_is_lfn(&existing)) {
        continue;
      }
      
      if (memcmp(existing.DIR_Name, fat_name, 11) == 0) {
        fseek(fs->fp, cluster_to_offset(fs, cluster) + (i * sizeof(DirEntry)), SEEK_SET);
        fwrite(entry, sizeof(DirEntry), 1, fs->fp);
        return 0; 
      }
    }
    
    cluster = fat32_next_cluster(fs, cluster);
    if (fat32_is_eoc(cluster)) {
      break;
    }
  }
  return -1;
}

int dir_delete_entry(FAT32 *fs, uint32_t dir_cluster, const char *name) {
  DirEntry existing;
  uint32_t cluster;
  uint32_t entries_per_cluster;
  uint32_t i;
  char fat_name[11];
  
  dir_make_fat_name(name, fat_name);
  
  cluster = dir_cluster;
  entries_per_cluster = (fs->bs.BPB_BytsPerSec * fs->bs.BPB_SecPerClus) / sizeof(DirEntry);
  
  while (1) {
    fseek(fs->fp, cluster_to_offset(fs, cluster), SEEK_SET); 
    
    for (i = 0; i < entries_per_cluster; i++) {
      fread(&existing, sizeof(DirEntry), 1, fs->fp);
      
      if (existing.DIR_Name[0] == 0x00) {
        return -1; 
      }
      
      if (dir_is_unused(&existing) || dir_is_lfn(&existing)) {
        continue;
      }
      
      if (memcmp(existing.DIR_Name, fat_name, 11) == 0) {
        existing.DIR_Name[0] = 0xE5; 
        fseek(fs->fp, cluster_to_offset(fs, cluster) + (i * sizeof(DirEntry)), SEEK_SET);
        fwrite(&existing, sizeof(DirEntry), 1, fs->fp);
        return 0; 
      }
    }
    
    cluster = fat32_next_cluster(fs, cluster);
    if (fat32_is_eoc(cluster)) {
      break;
    }
  }
  return -1;
}