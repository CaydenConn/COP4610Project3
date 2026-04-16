#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *get_input(void) {
    char *buffer = NULL;
    int bufsize = 0;
    char line[5];

    // Read the input by few characters 
    while (fgets(line, 5, stdin) != NULL) {
        int addby = 0;
        char *newln = strchr(line, '\n');

        // If newline is found, only copy up to it
        if (newln != NULL) {
            addby = (int)(newln - line);
        } else {
            addby = 4;
        }

        // Grow buffer and copy new piece into it
        buffer = (char *)realloc(buffer, bufsize + addby);
        memcpy(&buffer[bufsize], line, addby);
        bufsize += addby;

        // Stop once whole line is read
        if (newln != NULL) {
            break;
        }
    }

    // Add null terminator
    buffer = (char *)realloc(buffer, bufsize + 1);
    buffer[bufsize] = '\0';

    return buffer;
}

tokenlist *new_tokenlist(void) {
    // Make a new empty token list
    tokenlist *tokens = (tokenlist *)malloc(sizeof(tokenlist));
    tokens->size = 0;
    tokens->items = (char **)malloc(sizeof(char *));
    tokens->items[0] = NULL;
    return tokens;
}

void add_token(tokenlist *tokens, char *item) {
    size_t i = tokens->size;

    // Make room for new token and NULL at the end
    tokens->items = (char **)realloc(tokens->items, (i + 2) * sizeof(char *));
    tokens->items[i] = (char *)malloc(strlen(item) + 1);
    tokens->items[i + 1] = NULL;

    // Copy token text
    strcpy(tokens->items[i], item);

    tokens->size += 1;
}

tokenlist *get_tokens(char *input) {
    // Make a copy
    char *buf = (char *)malloc(strlen(input) + 1);
    strcpy(buf, input);

    tokenlist *tokens = new_tokenlist();
    char *tok = strtok(buf, " ");

    // Split by spaces
    while (tok != NULL) {
        add_token(tokens, tok);
        tok = strtok(NULL, " ");
    }

    free(buf);
    return tokens;
}

void free_tokens(tokenlist *tokens) {
    size_t i;

    // Free each token
    for (i = 0; i < tokens->size; i++) {
        free(tokens->items[i]);
    }

    // freeing list
    free(tokens->items);
    free(tokens);
}
