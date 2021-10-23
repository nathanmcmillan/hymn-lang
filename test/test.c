/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#ifdef __GNUC__
#include <dirent.h>
#include <linux/limits.h>
#include <unistd.h>
#define PATH_SEP '/'
#define PATH_SEP_STRING "/"
#elif _MSC_VER
#include <direct.h>
#include <windows.h>
#define getcwd _getcwd
#define PATH_MAX FILENAME_MAX
#define PATH_SEP '\\'
#define PATH_SEP_STRING "\\"
#endif

#include "hymn.h"

int tests_success = 0;
int tests_fail = 0;
int tests_count = 0;

struct FileList {
    int count;
    int capacity;
    HymnString **files;
};

static void file_list_add(struct FileList *list, HymnString *file) {
    int count = list->count;
    if (count + 1 > list->capacity) {
        if (list->capacity == 0) {
            list->capacity = 1;
            list->files = hymn_malloc(sizeof(HymnString *));
        } else {
            list->capacity *= 2;
            list->files = hymn_realloc(list->files, list->capacity * sizeof(HymnString *));
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
            file_list_add(list, hymn_new_string(file));
        }
    }
    closedir(dir);
    return false;
}
#elif _MSC_VER
static bool recurse_directories(const char *path, struct FileList *list) {
    HymnString *search = hymn_string_format("%s" PATH_SEP_STRING "*", path);
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
                file_list_add(list, hymn_new_string(file));
            }
        } while (FindNextFile(handle, &find));
        FindClose(handle);
    }
    hymn_string_delete(search);
    return false;
}
#endif

static struct FileList directories(const char *path) {
    struct FileList list = {.count = 0, .capacity = 0, .files = NULL};
    recurse_directories(path, &list);
    return list;
}

static void delete_file_list(struct FileList *list) {
    for (int i = 0; i < list->count; i++) {
        hymn_string_delete(list->files[i]);
    }
    free(list->files);
}

static HymnString *out;

static void console(const char *format, ...) {
    va_list args;

    va_start(args, format);
    int len = vsnprintf(NULL, 0, format, args);
    va_end(args);
    char *chars = hymn_malloc((len + 1) * sizeof(char));
    va_start(args, format);
    vsnprintf(chars, len + 1, format, args);
    va_end(args);
    out = hymn_string_append(out, chars);
    free(chars);
}

static HymnString *parse_expected(HymnString *source) {
    HymnString *expected = hymn_new_string("");
    size_t size = hymn_string_len(source);
    for (size_t pos = 0; pos < size; pos++) {
        char c = source[pos];
        if (c == '-' && pos + 2 < size && source[pos + 1] == '-') {
            if (source[pos + 2] == ' ') {
                pos += 3;
                while (pos < size) {
                    c = source[pos];
                    expected = hymn_string_append_char(expected, c);
                    if (c == '\n') {
                        break;
                    }
                    pos++;
                }
                continue;
            } else if (source[pos + 2] == '\n') {
                pos += 2;
                expected = hymn_string_append_char(expected, '\n');
                continue;
            }
        }
        break;
    }
    return hymn_string_trim(expected);
}

static HymnString *test_source(HymnString *script) {
    HymnString *source = hymn_read_file(script);
    HymnString *expected = parse_expected(source);
    HymnString *result = NULL;
    if (strcmp(expected, "") != 0) {
        Hymn *hymn = new_hymn();
        hymn->print = console;
        hymn_string_zero(out);
        char *error = hymn_do(hymn, script, source);
        hymn_delete(hymn);
        if (strcmp(expected, "@Exception") == 0) {
            if (error == NULL) {
                result = hymn_new_string("Expected an error.\n");
            } else {
                free(error);
            }
        } else {
            hymn_string_trim(out);
            if (error != NULL) {
                result = hymn_new_string(error);
                free(error);
                hymn_string_trim(result);
            } else if (hymn_string_starts_with(expected, "@Starts")) {
                HymnString *start = hymn_substring(expected, 8, hymn_string_len(expected));
                if (!hymn_string_starts_with(out, start)) {
                    result = hymn_string_format("Expected start:\n%s\n\nBut was:\n%s", start, out);
                }
                hymn_string_delete(start);
            } else if (!hymn_string_equal(out, expected)) {
                result = hymn_string_format("Expected:\n%s\n\nBut was:\n%s", expected, out);
            }
        }
    }
    hymn_string_delete(source);
    hymn_string_delete(expected);
    return result;
}

static void test_hymn(const char *filter) {
    out = hymn_new_string("");

    struct FileList all = directories("test" PATH_SEP_STRING "language");

    HymnString *end = hymn_new_string(".hm");
    struct HymnFilterList scripts = hymn_string_filter_ends_with(all.files, all.count, end);

    for (int i = 0; i < scripts.count; i++) {
        HymnString *script = scripts.filtered[i];
        if (filter != NULL && !hymn_string_contains(script, filter)) {
            continue;
        }
        tests_count++;
        printf("%s\n", script);
        HymnString *result = test_source(script);
        if (result != NULL) {
            printf("⨯ %s\n\n", result);
            tests_fail++;
        } else {
            // printf(" ✓\n");
            tests_success++;
        }
        hymn_string_delete(result);
    }

    delete_file_list(&all);
    hymn_delete_filter_list(&scripts);
    hymn_string_delete(end);

    hymn_string_delete(out);
}

int main(int argc, char **argv) {
    const char *filter = NULL;
    if (argc >= 2) {
        filter = argv[1];
    }

    printf("\n");
    clock_t start = clock();
    test_hymn(filter);
    double elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
    printf("\nSuccess: %d, Failed: %d, Total: %d, Time: %g s\n\n", tests_success, tests_fail, tests_count, elapsed);

    return 0;
}
