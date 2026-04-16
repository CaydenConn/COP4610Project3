#pragma once

#include <stdlib.h>
#include <stdbool.h>

// Stores the list of tokens from user input
typedef struct {
    char ** items;
    size_t size;
} tokenlist;

// Read input
char * get_input(void);

// Split input into tokens
tokenlist * get_tokens(char *input);

// Create empty token list
tokenlist * new_tokenlist(void);

// Add one token
void add_token(tokenlist *tokens, char *item);

// Free token list memory
void free_tokens(tokenlist *tokens);
