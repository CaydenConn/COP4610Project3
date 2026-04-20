#pragma once

#include "shell.h"
#include "fat32.h"
#include "lexer.h"

int cmd_read(FAT32 *fs, tokenlist *tokens);
