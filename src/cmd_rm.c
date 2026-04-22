#include <stdio.h>
#include <string.h>
#include "cmd_rm.h"
#include "commands.h"
#include "dir.h"

int cmd_rm(FAT32 *fs, tokenlist *tokens) {
    DirEntry entry;

    // rm needs exactly one argument
    if (tokens->size != 2) {
        printf("Error: rm requires exactly one FILENAME.\n");
        return CMD_ERROR;
    }

    const char *filename = tokens->items[1];

    // Check if the file exists in the current directory
    if (!dir_find_entry(fs, fs->current_cluster, filename, &entry)) {
        printf("Error: %s does not exist.\n", filename);
        return CMD_ERROR;
    }

    // Cannot rm a directory
    if (dir_is_directory(&entry)) {
        printf("Error: %s is a directory.\n", filename);
        return CMD_ERROR;
    }

    // Check if file is currently open 
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (fs->open_files[i].is_open &&
            strcmp(fs->open_files[i].name, filename) == 0 &&
            strcmp(fs->open_files[i].path, fs->current_path) == 0) {
            printf("Error: %s is opened.\n", filename);
            return CMD_ERROR;
        }
    }

    // Free the cluster chain if the file has allocated clusters
    uint32_t start_cluster = dir_entry_cluster(&entry);
    if (start_cluster >= 2) {
        fat32_free_cluster_chain(fs, start_cluster);
    }

    // Mark the directory entry as deleted (0xE5)
    dir_delete_entry(fs, fs->current_cluster, filename);

    fflush(fs->fp);
    return CMD_OK;
}