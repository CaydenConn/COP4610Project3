#include <stdio.h>
#include "cmd_info.h"
#include "commands.h"
#include "fat32.h"

int cmd_info(FAT32 *fs, tokenlist *tokens) {
    // info takes no arguments
    if (tokens->size != 1) {
        printf("Error: info takes no arguments.\n");
        return CMD_ERROR;
    }

    // Print FAT32 info
    fat32_print_info(fs);
    return CMD_OK;
}
