/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "file_io.h"

String *working_directory() {
    char path[PATH_MAX];
    if (getcwd(path, sizeof(path)) != NULL) {
        return new_string(path);
    }
    return NULL;
}

String *path_normalize(String *path) {
    usize i = 0;
    usize size = string_len(path);
    if (size > 1 && path[0] == '.') {
        if (path[1] == '.') {
            if (size > 2 && path[2] == '/') {
                i = 3;
            }
        } else if (path[1] == '/') {
            i = 2;
        }
    }

    usize n = 0;
    char normal[PATH_MAX];

    while (i < size) {

        if (path[i] == '/') {
            if (i + 2 < size) {
                if (path[i + 1] == '.' && path[i + 2] == '/') {
                    i += 2;
                    continue;
                } else if (path[i + 2] == '.' && i + 3 < size && path[i + 3] == '/') {
                    if (n > 0) {
                        n--;
                        while (n > 0) {
                            if (normal[n] == '/') {
                                break;
                            }
                            n--;
                        }
                    }
                    i += 3;
                    continue;
                }
            }
        }

        normal[n] = path[i];
        n++;
        i++;
    }

    normal[n] = '\0';
    return new_string(normal);
}

String *path_parent(String *path) {
    return path;
}

String *path_join(String *path, String *child) {
    String *new = string_copy(path);
    new = string_append_char(new, '/');
    return string_append(new, child);
}

String *path_absolute(String *path) {
    String *working = working_directory();
    if (string_starts_with(path, working)) {
        string_delete(working);
        return path_normalize(path);
    }
    working = string_append_char(working, '/');
    working = string_append(working, path);
    String *normal = path_normalize(working);
    string_delete(working);
    return normal;
}

usize file_size(char *path) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file: %s\n", path);
        exit(1);
    }
    usize num = 0;
    int ch;
    while ((ch = fgetc(fp)) != EOF) {
        num++;
    }
    fclose(fp);
    return num;
}

usize file_binary_size(char *path) {
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file: %s\n", path);
        exit(1);
    }
    usize num = 0;
    int ch;
    while ((ch = fgetc(fp)) != EOF) {
        num++;
    }
    fclose(fp);
    return num;
}

String *cat(char *path) {
    usize size = file_size(path);
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file: %s\n", path);
        exit(1);
    }
    char *content = safe_malloc((size + 1) * sizeof(char));
    for (usize i = 0; i < size; i++) {
        content[i] = (char)fgetc(fp);
    }
    fclose(fp);
    String *s = new_string_with_length(content, size);
    free(content);
    return s;
}

void core_write(char *path, char *content) {
    FILE *fp = fopen(path, "a");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file: %s\n", path);
        exit(1);
    }
    fputs(content, fp);
    fclose(fp);
}

char *read_binary(char *path, usize *size_pointer) {
    usize size = file_binary_size(path);
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file: %s\n", path);
        exit(1);
    }
    char *content = safe_malloc(size * sizeof(char));
    for (usize i = 0; i < size; i++) {
        content[i] = (char)fgetc(fp);
    }
    fclose(fp);
    *size_pointer = size;
    return content;
}
