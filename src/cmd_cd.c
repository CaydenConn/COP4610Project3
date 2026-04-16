#include <stdio.h>
#include <string.h>
#include "cmd_cd.h"
#include "commands.h"
#include "dir.h"

int cmd_cd(FAT32 *fs, tokenlist *tokens) {
    DirEntry entry;
    uint32_t next_cluster;

    // cd needs only one directory
    if (tokens->size != 2) {
        printf("Error: cd requires exactly one argument.\n");
        return CMD_ERROR;
    }

    // cd "." stays put
    if (strcmp(tokens->items[1], ".") == 0) {
        return CMD_OK;
    }

    // cd .. moves up one directory
    if (strcmp(tokens->items[1], "..") == 0) {
        if (dir_find_entry(fs, fs->current_cluster, "..", &entry)) {
            next_cluster = dir_entry_cluster(&entry);

            // If cluster is 0, treat it as root
            if (next_cluster == 0) {
                next_cluster = fs->bs.BPB_RootClus;
            }

            fs->current_cluster = next_cluster;
            path_go_up(fs);
            return CMD_OK;
        }

        // If .. is not found, just go to root
        fs->current_cluster = fs->bs.BPB_RootClus;
        strcpy(fs->current_path, "/");
        return CMD_OK;
    }

    // Look for the name in the current directory
    if (!dir_find_entry(fs, fs->current_cluster, tokens->items[1], &entry)) {
        printf("Error: directory does not exist.\n");
        return CMD_ERROR;
    }

    // Checking if a directory
    if (!dir_is_directory(&entry)) {
        printf("Error: not a directory.\n");
        return CMD_ERROR;
    }

    // Move into that directory
    next_cluster = dir_entry_cluster(&entry);

    if (next_cluster == 0) {
        next_cluster = fs->bs.BPB_RootClus;
    }

    fs->current_cluster = next_cluster;
    path_go_into(fs, tokens->items[1]);

    return CMD_OK;
}
