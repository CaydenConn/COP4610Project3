#include <stdio.h>
#include <string.h>
#include "cmd_close.h"
#include "dir.h"
#include "commands.h"

int cmd_close(FAT32 *fs, tokenlist *tokens) {
    if (tokens->size != 2) {
        printf("Error: Invalid number of arguments. Usage: close [FILENAME]\n");
        return CMD_OK;
    }

    const char *filename = tokens->items[1];
    
    // Check if the file exists in the directory first
    DirEntry entry;
    if (dir_find_entry(fs, fs->current_cluster, filename, &entry) != 1) {
        printf("Error: File %s does not exist.\n", filename);
        return CMD_OK;
    }

    // Find the file in open_files
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (fs->open_files[i].is_open && strcmp(fs->open_files[i].name, filename) == 0 &&
            strcmp(fs->open_files[i].path, fs->current_path) == 0) {
            fs->open_files[i].is_open = 0; // Mark as closed
            return CMD_OK;
        }
    }

    printf("Error: File %s is not opened.\n", filename);
    return CMD_OK;
}
