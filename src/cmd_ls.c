#include <stdio.h>
#include "cmd_ls.h"
#include "commands.h"
#include "dir.h"

int cmd_ls(FAT32 *fs, tokenlist *tokens) {
    // ls has 0 arguments
    if (tokens->size != 1) {
        printf("Error: ls takes no arguments.\n");
        return CMD_ERROR;
    }

    // Print current directory
    dir_list(fs, fs->current_cluster);
    return CMD_OK;
}
