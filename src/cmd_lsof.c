#include <stdio.h>
#include <string.h>
#include "cmd_lsof.h"
#include "dir.h"
#include "commands.h"

int cmd_lsof(FAT32 *fs, tokenlist *tokens) {
    if (tokens->size != 1) {
        printf("Error: Invalid number of arguments. Usage: lsof\n");
        return CMD_OK;
    }

    int any_opened = 0;
    
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (fs->open_files[i].is_open) {
            if (!any_opened) {
                 printf("INDEX\tNAME\tMODE\tOFFSET\tPATH\n");
                 any_opened = 1;
            }
            
            const char *mode_str = "";
            if (fs->open_files[i].mode == MODE_R) mode_str = "-r";
            else if (fs->open_files[i].mode == MODE_W) mode_str = "-w";
            else if (fs->open_files[i].mode == MODE_RW) mode_str = "-rw";
            
            printf("%d\t%s\t%s\t%u\t%s\n", 
                i, 
                fs->open_files[i].name, 
                mode_str, 
                fs->open_files[i].offset, 
                fs->open_files[i].path);
        }
    }
    
    if (!any_opened) {
        printf("No opened files.\n");
    }

    return CMD_OK;
}
