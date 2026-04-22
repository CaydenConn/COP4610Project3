#include <stdio.h>
#include <string.h>
#include "cmd_rmdir.h"
#include "commands.h"
#include "dir.h"

// Check if a directory cluster chain contains only "." and ".." entries
static int dir_is_empty(FAT32 *fs, uint32_t dir_cluster) {
    DirEntry entry;
    uint32_t cluster = dir_cluster;
    uint32_t entries_per_cluster =
        (fs->bs.BPB_BytsPerSec * fs->bs.BPB_SecPerClus) / sizeof(DirEntry);

    while (1) {
        fseek(fs->fp, cluster_to_offset(fs, cluster), SEEK_SET);

        for (uint32_t i = 0; i < entries_per_cluster; i++) {
            fread(&entry, sizeof(DirEntry), 1, fs->fp);

            // End of directory entries
            if (entry.DIR_Name[0] == 0x00) {
                return 1; // empty
            }

            // Skip deleted entries and LFN entries
            if (dir_is_unused(&entry) || dir_is_lfn(&entry)) {
                continue;
            }

            // Skip "." and ".."
            if (entry.DIR_Name[0] == '.' && entry.DIR_Name[1] == ' ') {
                continue;
            }
            if (entry.DIR_Name[0] == '.' && entry.DIR_Name[1] == '.') {
                continue;
            }

            // Found an entry
            return 0;
        }

        cluster = fat32_next_cluster(fs, cluster);
        if (fat32_is_eoc(cluster)) {
            break;
        }
    }

    return 1; // empty
}

int cmd_rmdir(FAT32 *fs, tokenlist *tokens) {
    DirEntry entry;

    // rmdir needs exactly one argument
    if (tokens->size != 2) {
        printf("Error: rmdir requires exactly one DIRNAME.\n");
        return CMD_ERROR;
    }

    const char *dirname = tokens->items[1];

    // Cannot remove "." or ".."
    if (strcmp(dirname, ".") == 0 || strcmp(dirname, "..") == 0) {
        printf("Error: cannot remove \".\" or \"..\".\n");
        return CMD_ERROR;
    }

    // Check if the directory exists
    if (!dir_find_entry(fs, fs->current_cluster, dirname, &entry)) {
        printf("Error: %s does not exist.\n", dirname);
        return CMD_ERROR;
    }

    // Must be a directory
    if (!dir_is_directory(&entry)) {
        printf("Error: %s is not a directory.\n", dirname);
        return CMD_ERROR;
    }

    uint32_t target_cluster = dir_entry_cluster(&entry);
    if (target_cluster == 0) {
        target_cluster = fs->bs.BPB_RootClus;
    }

    // Cannot remove the current working directory
    if (target_cluster == fs->current_cluster) {
        printf("Error: cannot remove the current working directory.\n");
        return CMD_ERROR;
    }

    // Check if any file is opened inside this directory
    // Build the full path of the directory to compare against open files
    char dir_path[256];
    if (strcmp(fs->current_path, "/") == 0) {
        snprintf(dir_path, sizeof(dir_path), "/%s", dirname);
    } else {
        snprintf(dir_path, sizeof(dir_path), "%s/%s", fs->current_path, dirname);
    }

    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (fs->open_files[i].is_open &&
            strcmp(fs->open_files[i].path, dir_path) == 0) {
            printf("Error: a file is opened in %s.\n", dirname);
            return CMD_ERROR;
        }
    }

    // Check if the directory is empty
    if (!dir_is_empty(fs, target_cluster)) {
        printf("Error: directory is not empty.\n");
        return CMD_ERROR;
    }

    // Free the directory's cluster chain
    fat32_free_cluster_chain(fs, target_cluster);

    // Mark the directory entry as deleted in the parent
    dir_delete_entry(fs, fs->current_cluster, dirname);

    fflush(fs->fp);
    return CMD_OK;
}