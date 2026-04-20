#include <stdio.h>
#include <string.h>
#include "cmd_mv.h"
#include "dir.h"
#include "commands.h"

int cmd_mv(FAT32 *fs, tokenlist *tokens) {
    if (tokens->size != 3) {
        printf("Error: Invalid number of arguments. Usage: mv [FILENAME/DIRNAME] [NEW_FILENAME/DIRECTORY]\n");
        return CMD_OK;
    }

    const char *origin = tokens->items[1];
    const char *dest = tokens->items[2];

    // Origin file must be closed
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (fs->open_files[i].is_open && strcmp(fs->open_files[i].name, origin) == 0 &&
            strcmp(fs->open_files[i].path, fs->current_path) == 0) {
            printf("Error: File %s is opened.\n", origin);
            return CMD_OK;
        }
    }

    DirEntry origin_entry;
    if (dir_find_entry(fs, fs->current_cluster, origin, &origin_entry) != 1) {
        printf("Error: %s does not exist.\n", origin);
        return CMD_OK;
    }

    DirEntry dest_entry;
    int dest_exists = dir_find_entry(fs, fs->current_cluster, dest, &dest_entry);

    if (dest_exists == 1) {
        if (!dir_is_directory(&dest_entry)) {
            printf("Error: %s is a file entry.\n", dest);
            return CMD_OK;
        }
        
        // Dest is a directory. Insert origin_entry into that directory
        uint32_t target_cluster = dir_entry_cluster(&dest_entry);
        if (target_cluster == 0) target_cluster = fs->bs.BPB_RootClus;
        
        // Prevent moving a directory into itself (basic protection)
        if (target_cluster == dir_entry_cluster(&origin_entry)) {
            printf("Error: Cannot move a directory into itself.\n");
            return CMD_OK;
        }

        if (dir_add_entry(fs, target_cluster, &origin_entry) != 0) {
            printf("Error: Failed to move to directory (it might be full).\n");
            return CMD_OK;
        }
    } else {
        // Renaming in the current directory
        dir_make_fat_name(dest, origin_entry.DIR_Name);
        if (dir_add_entry(fs, fs->current_cluster, &origin_entry) != 0) {
            printf("Error: Failed to rename (directory might be full).\n");
            return CMD_OK;
        }
    }

    // Delete the old entry marking it as 0xE5
    dir_delete_entry(fs, fs->current_cluster, origin);

    return CMD_OK;
}
