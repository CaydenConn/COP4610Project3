#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmd_lseek.h"
#include "dir.h"
#include "commands.h"

int cmd_lseek(FAT32 *fs, tokenlist *tokens) {
    if (tokens->size != 3) {
        printf("Error: Invalid number of arguments. Usage: lseek [FILENAME] [OFFSET]\n");
        return CMD_OK;
    }

    const char *filename = tokens->items[1];
    long offset = atol(tokens->items[2]);
    
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
            
            if (offset > (long)fs->open_files[i].file_size) {
                printf("Error: Offset %ld is larger than file size.\n", offset);
                return CMD_OK;
            }
            if (offset < 0) {
                printf("Error: Offset cannot be negative.\n");
                return CMD_OK;
            }

            fs->open_files[i].offset = (uint32_t)offset;
            return CMD_OK;
        }
    }

    printf("Error: File %s is not opened.\n", filename);
    return CMD_OK;
}
