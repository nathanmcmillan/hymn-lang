/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "hymn_io.h"

HymnChar *working_directory() {
    char path[PATH_MAX];
    if (getcwd(path, sizeof(path)) != NULL) {
        return new_string(path);
    }
    return NULL;
}

HymnChar *path_normalize(HymnChar *path) {
    size_t i = 0;
    size_t size = string_len(path);
    if (size > 1 && path[0] == '.') {
        if (path[1] == '.') {
            if (size > 2 && path[2] == PATH_SEP) {
                i = 3;
            }
        } else if (path[1] == PATH_SEP) {
            i = 2;
        }
    }

    size_t n = 0;
    char normal[PATH_MAX];

    while (i < size) {
        if (path[i] == PATH_SEP) {
            if (i + 2 < size) {
                if (path[i + 1] == '.' && path[i + 2] == PATH_SEP) {
                    i += 2;
                    continue;
                } else if (path[i + 2] == '.' && i + 3 < size && path[i + 3] == PATH_SEP) {
                    if (n > 0) {
                        n--;
                        while (n > 0) {
                            if (normal[n] == PATH_SEP) {
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

HymnChar *path_parent(HymnChar *path) {
    size_t size = string_len(path);
    if (size < 2) {
        return string_copy(path);
    }
    size_t i = size - 2;
    while (true) {
        if (i == 0) break;
        if (path[i] == PATH_SEP) break;
        i--;
    }
    return new_string_from_substring(path, 0, i);
}

HymnChar *path_join(HymnChar *path, HymnChar *child) {
    HymnChar *new = string_copy(path);
    new = string_append_char(new, PATH_SEP);
    return string_append(new, child);
}

HymnChar *path_absolute(HymnChar *path) {
    HymnChar *working = working_directory();
    if (string_starts_with(path, working)) {
        string_delete(working);
        return path_normalize(path);
    }
    working = string_append_char(working, PATH_SEP);
    working = string_append(working, path);
    HymnChar *normal = path_normalize(working);
    string_delete(working);
    return normal;
}

size_t file_size(const char *path) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file: %s\n", path);
        return 0;
    }
    size_t num = 0;
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

HymnChar *cat(const char *path) {
    size_t size = file_size(path);
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file: %s\n", path);
        return new_string("");
    }
    char *content = hymn_malloc((size + 1) * sizeof(char));
    for (size_t i = 0; i < size; i++) {
        content[i] = (char)fgetc(fp);
    }
    fclose(fp);
    HymnChar *s = new_string_with_length(content, size);
    free(content);
    return s;
}

static void file_list_add(struct FileList *list, HymnChar *file) {
    int count = list->count;
    if (count + 1 > list->capacity) {
        if (list->capacity == 0) {
            list->capacity = 1;
            list->files = hymn_malloc(sizeof(HymnChar *));
        } else {
            list->capacity *= 2;
            list->files = hymn_realloc(list->files, list->capacity * sizeof(HymnChar *));
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
        if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0) {
            continue;
        }
        strcpy(file, path);
        strcat(file, PATH_SEP_STRING);
        strcat(file, d->d_name);
        if (recurse_directories(file, list)) {
            file_list_add(list, new_string(file));
        }
    }
    closedir(dir);
    return false;
}
#elif _MSC_VER
#include <windows.h>
static bool recurse_directories(const char *path, struct FileList *list) {
    HymnChar *search = string_format("%s" PATH_SEP_STRING "*", path);
    WIN32_FIND_DATA find;
    HANDLE handle = FindFirstFile(search, &find);
    if (handle == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Find files failed: %d\n", GetLastError());
    } else {
        char file[PATH_MAX];
        do {
            if (strcmp(find.cFileName, ".") == 0 || strcmp(find.cFileName, "..") == 0) {
                continue;
            }
            strcpy(file, path);
            strcat(file, PATH_SEP_STRING);
            strcat(file, find.cFileName);
            if (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                recurse_directories(file, list);
            } else {
                file_list_add(list, new_string(file));
            }
        } while (FindNextFile(handle, &find));
        FindClose(handle);
    }
    string_delete(search);
    return false;
}
#endif

struct FileList directories(const char *path) {
    struct FileList list = {.count = 0, .capacity = 0, .files = NULL};
    recurse_directories(path, &list);
    return list;
}

void delete_file_list(struct FileList *list) {
    for (int i = 0; i < list->count; i++) {
        string_delete(list->files[i]);
    }
    free(list->files);
}
