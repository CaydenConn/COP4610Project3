#ifndef CMD_MKDIR_H
#define CMD_MKDIR_H
 
#include "fat32.h"
#include "lexer.h"
 
int cmd_mkdir(FAT32 *fs, tokenlist *tokens);
 
#endif