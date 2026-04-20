#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmd_read.h"
#include "dir.h"
#include "commands.h"

int cmd_read(FAT32 *fs, tokenlist *tokens) {
    if (tokens->size != 3) {
        printf("Error: Invalid number of arguments. Usage: read [FILENAME] [SIZE]\n");
        return CMD_OK;
    }

    const char *filename = tokens->items[1];
    long size = atol(tokens->items[2]);
    
    if (size <= 0) {
        return CMD_OK; // Or error
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
        // Check if it exists in directory to print specific errors
        DirEntry entry;
        if (dir_find_entry(fs, fs->current_cluster, filename, &entry) != 1) {
            printf("Error: File %s does not exist.\n", filename);
            return CMD_OK;
        }
        if (dir_is_directory(&entry)) {
            printf("Error: %s is a directory.\n", filename);
            return CMD_OK;
        }
        printf("Error: File %s is not opened.\n", filename);
        return CMD_OK;
    }

    if (fs->open_files[slot].mode != MODE_R && fs->open_files[slot].mode != MODE_RW) {
        printf("Error: File %s is not opened for reading.\n", filename);
        return CMD_OK;
    }

    uint32_t offset = fs->open_files[slot].offset;
    uint32_t file_size = fs->open_files[slot].file_size;

    if (offset >= file_size) {
        return CMD_OK; // Nothing to read
    }

    uint32_t to_read = (uint32_t)size;
    if (offset + to_read > file_size) {
        to_read = file_size - offset;
    }

    uint32_t bytes_per_cluster = fs->bs.BPB_BytsPerSec * fs->bs.BPB_SecPerClus;
    uint32_t current_cluster = fs->open_files[slot].first_cluster;

    // Advance to the cluster containing the offset
    uint32_t cluster_idx = offset / bytes_per_cluster;
    for (uint32_t i = 0; i < cluster_idx; i++) {
        current_cluster = fat32_next_cluster(fs, current_cluster);
        if (fat32_is_eoc(current_cluster)) {
            break; // Should not happen if file is well-formed
        }
    }

    uint32_t cluster_offset = offset % bytes_per_cluster;
    uint32_t bytes_read = 0;

    unsigned char *buffer = malloc(to_read + 1);
    if (!buffer) return CMD_OK;

    while (bytes_read < to_read && !fat32_is_eoc(current_cluster)) {
        uint32_t chunk = bytes_per_cluster - cluster_offset;
        if (chunk > to_read - bytes_read) {
            chunk = to_read - bytes_read;
        }

        fseek(fs->fp, cluster_to_offset(fs, current_cluster) + cluster_offset, SEEK_SET);
        fread(buffer + bytes_read, 1, chunk, fs->fp);

        bytes_read += chunk;
        cluster_offset = 0; // Next cluster starts at 0

        if (bytes_read < to_read) {
            current_cluster = fat32_next_cluster(fs, current_cluster);
        }
    }

    buffer[bytes_read] = '\0';
    for(uint32_t i = 0; i < bytes_read; i++){
        putchar(buffer[i]);
    }
    printf("\n");
    free(buffer);

    fs->open_files[slot].offset += bytes_read;
    return CMD_OK;
}
