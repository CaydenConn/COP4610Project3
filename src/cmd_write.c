#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmd_write.h"
#include "dir.h"
#include "commands.h"

int cmd_write(FAT32 *fs, tokenlist *tokens) {
    if (tokens->size < 3) {
        printf("Error: Invalid number of arguments. Usage: write [FILENAME] \"[STRING]\"\n");
        return CMD_OK;
    }

    const char *filename = tokens->items[1];

    // Reconstruct the string from tokens which were split by spaces
    char *string_to_write = malloc(1024);
    if (!string_to_write) return CMD_OK;
    string_to_write[0] = '\0';
    for (size_t i = 2; i < tokens->size; i++) {
        strcat(string_to_write, tokens->items[i]);
        if (i < tokens->size - 1) strcat(string_to_write, " ");
    }

    // Strip the quotes
    size_t len = strlen(string_to_write);
    if (len >= 2 && string_to_write[0] == '"' && string_to_write[len - 1] == '"') {
        memmove(string_to_write, string_to_write + 1, len - 2);
        string_to_write[len - 2] = '\0';
        len -= 2;
    } else {
        printf("Error: STRING must be enclosed in \"\"\n");
        free(string_to_write);
        return CMD_OK;
    }

    // Find the file in open_files
    int slot = -1;
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (fs->open_files[i].is_open && strcmp(fs->open_files[i].name, filename) == 0 &&
            strcmp(fs->open_files[i].path, fs->current_path) == 0) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        DirEntry entry;
        if (dir_find_entry(fs, fs->current_cluster, filename, &entry) != 1) {
            printf("Error: File %s does not exist.\n", filename);
            free(string_to_write);
            return CMD_OK;
        }
        if (dir_is_directory(&entry)) {
            printf("Error: %s is a directory.\n", filename);
            free(string_to_write);
            return CMD_OK;
        }
        printf("Error: File %s is not opened.\n", filename);
        free(string_to_write);
        return CMD_OK;
    }

    if (fs->open_files[slot].mode != MODE_W && fs->open_files[slot].mode != MODE_RW) {
        printf("Error: File %s is not opened for writing.\n", filename);
        free(string_to_write);
        return CMD_OK;
    }

    uint32_t offset = fs->open_files[slot].offset;
    uint32_t to_write = (uint32_t)len;

    uint32_t bytes_per_cluster = fs->bs.BPB_BytsPerSec * fs->bs.BPB_SecPerClus;
    uint32_t current_cluster = fs->open_files[slot].first_cluster;
    
    // Check if the file has absolutely NO clusters allocated yet (file_size = 0, first_cluster = 0 potentially)
    // Actually FAT32 represents size 0 files often with first_cluster 0.
    if (current_cluster == 0) {
        // Allocate a new cluster
        current_cluster = fat32_find_free_cluster(fs);
        if (current_cluster == 0) {
            printf("Error: No free clusters available.\n");
            free(string_to_write);
            return CMD_OK;
        }
        fat32_write_fat_entry(fs, current_cluster, 0x0FFFFFF8); // Mark EOC
        fs->open_files[slot].first_cluster = current_cluster;
    }

    // Traverse to the cluster containing the offset
    uint32_t cluster_idx = offset / bytes_per_cluster;
    for (uint32_t i = 0; i < cluster_idx; i++) {
        uint32_t next = fat32_next_cluster(fs, current_cluster);
        if (fat32_is_eoc(next)) {
            // Need to expand the file!
            uint32_t new_cluster = fat32_find_free_cluster(fs);
            if (new_cluster == 0) {
                printf("Error: No free clusters available.\n");
                free(string_to_write);
                return CMD_OK;
            }
            fat32_write_fat_entry(fs, current_cluster, new_cluster);
            fat32_write_fat_entry(fs, new_cluster, 0x0FFFFFF8); // Mark EOC
            next = new_cluster;
        }
        current_cluster = next;
    }

    uint32_t cluster_offset = offset % bytes_per_cluster;
    uint32_t bytes_written = 0;

    while (bytes_written < to_write) {
        uint32_t chunk = bytes_per_cluster - cluster_offset;
        if (chunk > to_write - bytes_written) {
            chunk = to_write - bytes_written;
        }

        fseek(fs->fp, cluster_to_offset(fs, current_cluster) + cluster_offset, SEEK_SET);
        fwrite(string_to_write + bytes_written, 1, chunk, fs->fp);

        bytes_written += chunk;
        cluster_offset = 0;

        if (bytes_written < to_write) {
            uint32_t next = fat32_next_cluster(fs, current_cluster);
            if (fat32_is_eoc(next)) {
                // Must allocate a new cluster to continue writing
                uint32_t new_cluster = fat32_find_free_cluster(fs);
                if (new_cluster == 0) {
                    printf("Error: No free clusters available.\n");
                    break;
                }
                fat32_write_fat_entry(fs, current_cluster, new_cluster);
                fat32_write_fat_entry(fs, new_cluster, 0x0FFFFFF8);
                current_cluster = new_cluster;
            } else {
                current_cluster = next;
            }
        }
    }

    free(string_to_write);

    // Update internal tracking
    fs->open_files[slot].offset += bytes_written;
    if (fs->open_files[slot].offset > fs->open_files[slot].file_size) {
        fs->open_files[slot].file_size = fs->open_files[slot].offset;
    }

    // Now update the directory entry!
    DirEntry entry;
    if (dir_find_entry(fs, fs->open_files[slot].dir_cluster, filename, &entry) == 1) {
        entry.DIR_FileSize = fs->open_files[slot].file_size;
        entry.DIR_FstClusHI = (fs->open_files[slot].first_cluster >> 16) & 0xFFFF;
        entry.DIR_FstClusLO = fs->open_files[slot].first_cluster & 0xFFFF;
        dir_update_entry(fs, fs->open_files[slot].dir_cluster, filename, &entry);
    }

    return CMD_OK;
}
