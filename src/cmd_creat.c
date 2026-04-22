#include <stdio.h>
#include <string.h>
#include "cmd_creat.h"
#include "commands.h"
#include "dir.h"

int cmd_creat(FAT32 *fs, tokenlist *tokens) {
    DirEntry new_entry;

    // creat needs exactly one argument
    if (tokens->size != 2) {
        printf("Error: creat requires exactly one DIRNAME.\n");
        return CMD_ERROR;
    }

    // Check if name already exists in current directory
    if (dir_find_entry(fs, fs->current_cluster, tokens->items[1], NULL)) {
        printf("Error: %s already exists.\n", tokens->items[1]);
        return CMD_ERROR;
    }

    // Build a file entry: cluster 0, size 0, ATTR_ARCHIVE
    memset(&new_entry, 0, sizeof(DirEntry));
    dir_make_fat_name(tokens->items[1], new_entry.DIR_Name);
    new_entry.DIR_Attr = 0x20;  // ATTR_ARCHIVE
    new_entry.DIR_FstClusHI = 0;
    new_entry.DIR_FstClusLO = 0;
    new_entry.DIR_FileSize = 0;

    // Add it to the current directory
    if (dir_add_entry(fs, fs->current_cluster, &new_entry) != 0) {
        printf("Error: could not add file entry.\n");
        return CMD_ERROR;
    }

    fflush(fs->fp);
    return CMD_OK;
}