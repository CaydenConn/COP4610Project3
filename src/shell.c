#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shell.h"
#include "lexer.h"
#include "commands.h"
#include "cmd_info.h"
#include "cmd_exit.h"
#include "cmd_ls.h"
#include "cmd_cd.h"

// Print shell prompt
static void print_prompt(FAT32 *fs) {
    printf("[%s]%s> ", fs->image_name, fs->current_path);
    fflush(stdout);
}

void run_shell(FAT32 *fs) {
    while (1) {
        print_prompt(fs);

        // Read input
        char *input = get_input();
        if (input == NULL) {
            break;
        }

        // Skip empty input
        if (strlen(input) == 0) {
            free(input);
            continue;
        }

        // Split input into tokens
        tokenlist *tokens = get_tokens(input);

        if (tokens->size > 0) {
            int status = CMD_OK;

            // Check which command the user typed
            if (strcmp(tokens->items[0], "info") == 0) {
                status = cmd_info(fs, tokens);
            } else if (strcmp(tokens->items[0], "exit") == 0) {
                status = cmd_exit(fs, tokens);
            } else if (strcmp(tokens->items[0], "cd") == 0) {
                status = cmd_cd(fs, tokens);
            } else if (strcmp(tokens->items[0], "ls") == 0) {
                status = cmd_ls(fs, tokens);
            } else {
                printf("Error: unsupported command.\n");
            }

            // Exit the shell if exit command was used
            if (status == CMD_EXIT) {
                free(input);
                free_tokens(tokens);
                break;
            }
        }

        // Free memory
        free(input);
        free_tokens(tokens);
    }
}
