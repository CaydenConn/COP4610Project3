#include <stdio.h>
#include "fat32.h"
#include "shell.h"

int main(int argc, char *argv[]) {
    FAT32 fs;

    // Need img file as argument
    if (argc != 2) {
        fprintf(stderr, "Usage: ./filesys <FAT32 image>\n");
        return 1;
    }

    // Open the image
    if (fat32_open(&fs, argv[1]) != 0) {
        return 1;
    }

    // Start shell
    run_shell(&fs);

    // Close the image before exiting
    fat32_close(&fs);

    return 0;
}
