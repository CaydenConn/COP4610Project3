#include <stdio.h>
#include "cmd_exit.h"
#include "commands.h"

int cmd_exit(FAT32 *fs, tokenlist *tokens) {
    (void)fs;

    // exit has 0 argumentd
    if (tokens->size != 1) {
        printf("Error: exit takes no arguments.\n");
        return CMD_ERROR;
    }

    // Tells shell to exit
    return CMD_EXIT;
}
