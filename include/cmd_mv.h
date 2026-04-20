#pragma once

#include "shell.h"
#include "fat32.h"
#include "lexer.h"

int cmd_mv(FAT32 *fs, tokenlist *tokens);
