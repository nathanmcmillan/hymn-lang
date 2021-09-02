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
    usize size = string_len(path);
    if (size < 2) {
        return string_copy(path);
    }
    usize i = size - 2;
    while (true) {
        if (i == 0) break;
        if (path[i] == '/') break;
        i--;
    }
    return new_string_from_substring(path, 0, i);
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

usize file_size(const char *path) {
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

bool file_exists(const char *path) {
    struct stat b;
    return stat(path, &b) == 0;
}

String *cat(const char *path) {
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

static void file_list_add(struct FileList *list, String *file) {
    int count = list->count;
    if (count + 1 > list->capacity) {
        if (list->capacity == 0) {
            list->capacity = 1;
            list->files = safe_malloc(sizeof(String *));
        } else {
            list->capacity *= 2;
            list->files = safe_realloc(list->files, list->capacity * sizeof(String *));
        }
    }
    list->files[count] = file;
    list->count = count + 1;
}

#ifdef __GNUC__
static bool recurse_directories(const char *path, struct FileList *list) {
    DIR *dir = opendir(path);
    if (dir == NULL) {
        return true;
    }
    struct dirent *d;
    char file[PATH_MAX];
    while ((d = readdir(dir)) != NULL) {
        if (strcmp(d->d_name, ".") != 0 && strcmp(d->d_name, "..") != 0) {
            strcpy(file, path);
            strcat(file, PATH_SEP_STRING);
            strcat(file, d->d_name);
            if (recurse_directories(file, list)) {
                file_list_add(list, new_string(file));
            }
        }
    }
    closedir(dir);
    return false;
}
#endif

struct FileList directories(const char *path) {
    struct FileList list = {.count = 0, .capacity = 0, .files = NULL};
#ifdef __GNUC__
    recurse_directories(path, &list);
#elif _MSC_VER
#else
#endif
    return list;
}

void delete_file_list(struct FileList *list) {
    for (int i = 0; i < list->count; i++) {
        string_delete(list->files[i]);
    }
}
