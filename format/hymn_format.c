/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#define VERSION "0.1.0"

static void help() {
    printf("Hymn Script Formatter\n\n"
           "  -w  Write to file\n"
           "  -c  Format input command\n"
           "  -v  Print version information\n"
           "  -h  Print this help message\n");
}

static size_t file_size(const char *path) {
    FILE *open = fopen(path, "r");
    if (open == NULL) {
        return 0;
    }
    size_t size = 0;
    int ch;
    while ((ch = fgetc(open)) != EOF) {
        size++;
    }
    fclose(open);
    return size;
}

static char *read_file(const char *path) {
    size_t size = file_size(path);
    FILE *open = fopen(path, "r");
    if (open == NULL) {
        return NULL;
    }
    char *source = malloc((size + 1) * sizeof(char));
    for (size_t i = 0; i < size; i++) {
        source[i] = (char)fgetc(open);
    }
    source[size] = '\0';
    fclose(open);
    return source;
}

static void format(const char *source, char **formatted, char **error) {
    *formatted = source;
    *error = NULL;
}

int main(int argc, char **argv) {

    bool write = false;

    char *file = NULL;
    char *code = NULL;

    if (argc >= 2) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
                help();
                return 2;
            } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
                printf("Hymn Script Formatter " VERSION "\n");
                return EXIT_SUCCESS;
            } else if (strcmp(argv[i], "-c") == 0) {
                if (i + 1 < argc) {
                    code = argv[i + 1];
                    i++;
                } else {
                    help();
                    return EXIT_FAILURE;
                }
            } else if (strcmp(argv[i], "-w") == 0) {
                write = true;
            } else {
                file = argv[i];
            }
        }
    }

    if (code != NULL) {
        char *formatted = NULL;
        char *error = NULL;
        format(code, &formatted, &error);
        if (error != NULL) {
            fprintf(stderr, "%s\n", error);
            fflush(stderr);
            free(error);
            return EXIT_FAILURE;
        } else {
            printf("%s\n", formatted);
            free(formatted);
        }
    } else if (file != NULL) {
        char *code = read_file(file);
        if (code == NULL) {
            fprintf(stderr, "File does not exist: %s\n", file);
            return EXIT_FAILURE;
        }
        char *formatted = NULL;
        char *error = NULL;
        format(code, &formatted, &error);
        free(code);
        if (error != NULL) {
            fprintf(stderr, "%s\n", error);
            fflush(stderr);
            free(error);
            return EXIT_FAILURE;
        } else if (write) {
            FILE *open = fopen(file, "w");
            if (open == NULL) {
                fprintf(stderr, "Failed writing to file: %s\n", file);
                free(formatted);
                return EXIT_FAILURE;
            }
            fputs(formatted, open);
            fclose(open);
        } else {
            printf("%s", formatted);
        }
        free(formatted);
    } else {
        help();
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
