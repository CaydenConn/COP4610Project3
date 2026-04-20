#include <stdio.h>
#include <string.h>
#include "cmd_open.h"
#include "dir.h"
#include "commands.h"

int cmd_open(FAT32 *fs, tokenlist *tokens) {
    if (tokens->size != 3) {
        printf("Error: Invalid number of arguments. Usage: open [FILENAME] [FLAGS]\n");
        return CMD_OK;
    }

    const char *filename = tokens->items[1];
    const char *flags = tokens->items[2];

    int mode = 0;
    if (strcmp(flags, "-r") == 0) mode = MODE_R;
    else if (strcmp(flags, "-w") == 0) mode = MODE_W;
    else if (strcmp(flags, "-rw") == 0 || strcmp(flags, "-wr") == 0) mode = MODE_RW;
    else {
        printf("Error: Invalid mode.\n");
        return CMD_OK;
    }

    // Check if file is already open
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (fs->open_files[i].is_open && strcmp(fs->open_files[i].name, filename) == 0 &&
            strcmp(fs->open_files[i].path, fs->current_path) == 0) {
            printf("Error: File is already open.\n");
            return CMD_OK;
        }
    }

    // Find the file in current directory
    DirEntry entry;
    if (dir_find_entry(fs, fs->current_cluster, filename, &entry) != 1) {
        printf("Error: File %s does not exist.\n", filename);
        return CMD_OK;
    }

    if (dir_is_directory(&entry)) {
        printf("Error: %s is a directory.\n", filename);
        return CMD_OK;
    }

    // Find an empty slot
    int slot = -1;
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (!fs->open_files[i].is_open) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        printf("Error: Maximum number of open files reached.\n");
        return CMD_OK;
    }

    // Add to open files
    fs->open_files[slot].is_open = 1;
    strncpy(fs->open_files[slot].name, filename, sizeof(fs->open_files[slot].name) - 1);
    fs->open_files[slot].name[sizeof(fs->open_files[slot].name) - 1] = '\0';
    strncpy(fs->open_files[slot].path, fs->current_path, sizeof(fs->open_files[slot].path) - 1);
    fs->open_files[slot].path[sizeof(fs->open_files[slot].path) - 1] = '\0';
    fs->open_files[slot].mode = mode;
    fs->open_files[slot].offset = 0;
    fs->open_files[slot].first_cluster = dir_entry_cluster(&entry);
    fs->open_files[slot].file_size = entry.DIR_FileSize;
    fs->open_files[slot].dir_cluster = fs->current_cluster;

    return CMD_OK;
}
