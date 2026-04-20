#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmd_mkdir.h"
#include "commands.h"
#include "dir.h"

int cmd_mkdir(FAT32 *fs, tokenlist *tokens) {
    DirEntry entry;
    uint32_t new_cluster;
    uint32_t offset;
    uint32_t bytes_per_cluster;
    uint8_t *zeros;
    DirEntry dot;
    DirEntry dotdot;
    DirEntry new_entry;
    uint32_t parent_cluster;
    char fat_name[11];

    // mkdir needs exactly one argument
    if (tokens->size != 2) {
        printf("Error: mkdir requires exactly one DIRNAME.\n");
        return CMD_ERROR;
    }

    // Check if name already exists in current directory
    if (dir_find_entry(fs, fs->current_cluster, tokens->items[1], NULL)) {
        printf("Error: %s already exists.\n", tokens->items[1]);
        return CMD_ERROR;
    }

    // Find a free cluster
    new_cluster = fat32_find_free_cluster(fs);
    if (new_cluster == 0) {
        printf("Error: no free clusters available.\n");
        return CMD_ERROR;
    }

    // Mark the new cluster as end-of-chain in FAT
    fat32_write_fat_entry(fs, new_cluster, 0x0FFFFFF8);

    // Zero out the new cluster
    bytes_per_cluster = fs->bs.BPB_BytsPerSec * fs->bs.BPB_SecPerClus;
    zeros = calloc(bytes_per_cluster, 1);
    if (zeros == NULL) {
        printf("Error: memory allocation failed.\n");
        return CMD_ERROR;
    }

    offset = cluster_to_offset(fs, new_cluster);
    fseek(fs->fp, offset, SEEK_SET);
    fwrite(zeros, 1, bytes_per_cluster, fs->fp);
    free(zeros);

    // Write "." entry — points to itself
    memset(&dot, 0, sizeof(DirEntry));
    dot.DIR_Name[0] = '.';
    memset(&dot.DIR_Name[1], ' ', 10);
    dot.DIR_Attr = 0x10;  // ATTR_DIRECTORY
    dot.DIR_FstClusHI = (uint16_t)(new_cluster >> 16);
    dot.DIR_FstClusLO = (uint16_t)(new_cluster & 0xFFFF);
    dot.DIR_FileSize = 0;

    fseek(fs->fp, offset, SEEK_SET);
    fwrite(&dot, sizeof(DirEntry), 1, fs->fp);

    // Write ".." entry — points to parent (0 if parent is root)
    memset(&dotdot, 0, sizeof(DirEntry));
    dotdot.DIR_Name[0] = '.';
    dotdot.DIR_Name[1] = '.';
    memset(&dotdot.DIR_Name[2], ' ', 9);
    dotdot.DIR_Attr = 0x10;  // ATTR_DIRECTORY

    parent_cluster = fs->current_cluster;
    if (parent_cluster == fs->bs.BPB_RootClus) {
        parent_cluster = 0;
    }
    dotdot.DIR_FstClusHI = (uint16_t)(parent_cluster >> 16);
    dotdot.DIR_FstClusLO = (uint16_t)(parent_cluster & 0xFFFF);
    dotdot.DIR_FileSize = 0;

    fseek(fs->fp, offset + sizeof(DirEntry), SEEK_SET);
    fwrite(&dotdot, sizeof(DirEntry), 1, fs->fp);

    // Build the new directory entry for the parent directory
    memset(&new_entry, 0, sizeof(DirEntry));
    dir_make_fat_name(tokens->items[1], new_entry.DIR_Name);
    new_entry.DIR_Attr = 0x10;  // ATTR_DIRECTORY
    new_entry.DIR_FstClusHI = (uint16_t)(new_cluster >> 16);
    new_entry.DIR_FstClusLO = (uint16_t)(new_cluster & 0xFFFF);
    new_entry.DIR_FileSize = 0;

    // Add it to the current directory
    if (dir_add_entry(fs, fs->current_cluster, &new_entry) != 0) {
        printf("Error: could not add directory entry.\n");
        return CMD_ERROR;
    }

    fflush(fs->fp);
    return CMD_OK;
}