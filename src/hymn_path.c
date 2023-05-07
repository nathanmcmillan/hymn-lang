/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "hymn_path.h"

static void file_list_add(struct HymnPathFileList *list, HymnString *file) {
    int count = list->count;
    if (count >= list->capacity) {
        if (list->capacity == 0) {
            list->capacity = 1;
            list->files = hymn_malloc(sizeof(HymnString *));
        } else {
            list->capacity *= 2;
            list->files = hymn_realloc_int(list->files, list->capacity, sizeof(HymnString *));
        }
    }
    list->files[count] = file;
    list->count = count + 1;
}

#ifdef _MSC_VER
static bool directories(const char *path, int recursive, struct HymnPathFileList *list) {
    HymnString *search = hymn_string_format("%s" PATH_SEP_STRING "*", path);
    WIN32_FIND_DATA find;
    HANDLE handle = FindFirstFile(search, &find);
    if (handle != INVALID_HANDLE_VALUE) {
        char file[PATH_MAX];
        do {
            if (hymn_string_equal(find.cFileName, ".") || hymn_string_equal(find.cFileName, "..")) {
                continue;
            }
            strcpy(file, path);
            strcat(file, PATH_SEP_STRING);
            strcat(file, find.cFileName);
            file_list_add(list, hymn_new_string(file));
            if (recursive > 0 && (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                directories(file, recursive + 1, list);
            }
        } while (FindNextFile(handle, &find));
        FindClose(handle);
    }
    hymn_string_delete(search);
    return false;
}
#else
static bool directories(const char *path, int recursive, struct HymnPathFileList *list) {
    DIR *dir = opendir(path);
    if (dir == NULL) {
        return true;
    }
    struct dirent *d;
    char file[PATH_MAX];
    while ((d = readdir(dir)) != NULL) {
        if (hymn_string_equal(d->d_name, ".") || hymn_string_equal(d->d_name, "..")) {
            continue;
        }
        strcpy(file, path);
        strcat(file, PATH_SEP_STRING);
        strcat(file, d->d_name);
        file_list_add(list, hymn_new_string(file));
        if (recursive > 0) {
            directories(file, recursive + 1, list);
        }
    }
    closedir(dir);
    return false;
}
#endif

struct HymnPathFileList hymn_walk(const char *path, bool recursive) {
    struct HymnPathFileList list = {.count = 0, .capacity = 0, .files = NULL};
    directories(path, recursive ? 1 : -1, &list);
    return list;
}

void hymn_delete_file_list(struct HymnPathFileList *list) {
    for (int i = 0; i < list->count; i++) {
        hymn_string_delete(list->files[i]);
    }
    free(list->files);
}

#define PATH_FUNCTION(fun)                                     \
    if (count == 0) {                                          \
        return hymn_new_exception(H, "missing path");          \
    }                                                          \
    HymnValue path = arguments[0];                             \
    if (!hymn_is_string(path)) {                               \
        return hymn_new_exception(H, "path must be a string"); \
    }                                                          \
    HymnString *string = fun(hymn_as_string(path));            \
    return hymn_new_string_value(hymn_intern_string(H, string));

static HymnValue path_working(Hymn *H, int count, HymnValue *arguments) {
    (void)count;
    (void)arguments;
    HymnString *string = hymn_working_directory();
    return hymn_new_string_value(hymn_intern_string(H, string));
}

static HymnValue path_convert(Hymn *H, int count, HymnValue *arguments) {
    PATH_FUNCTION(hymn_path_convert)
}

static HymnValue path_normalize(Hymn *H, int count, HymnValue *arguments) {
    PATH_FUNCTION(hymn_path_normalize)
}

static HymnValue path_parent(Hymn *H, int count, HymnValue *arguments) {
    PATH_FUNCTION(hymn_path_parent)
}

static HymnValue path_cat(Hymn *H, int count, HymnValue *arguments) {
    HymnString *path = hymn_new_string("");
    for (int a = 0; a < count; a++) {
        if (a != 0) {
            path = hymn_string_append_char(path, PATH_SEP);
        }
        HymnValue value = arguments[a];
        if (hymn_is_string(value)) {
            path = hymn_string_append(path, hymn_as_string(value));
        } else {
            HymnString *string = hymn_value_to_string(value);
            path = hymn_string_append(path, string);
            hymn_string_delete(string);
        }
    }
    return hymn_new_string_value(hymn_intern_string(H, path));
}

static HymnValue path_absolute(Hymn *H, int count, HymnValue *arguments) {
    PATH_FUNCTION(hymn_path_absolute)
}

static HymnValue walk(Hymn *H, int count, HymnValue *arguments, bool recursive) {
    HymnString *path;
    if (count == 0) {
        path = hymn_new_string(".");
    } else {
        HymnValue value = arguments[0];
        if (!hymn_is_string(value)) {
            return hymn_new_exception(H, "path must be a string");
        }
        path = hymn_as_string(value);
    }
    struct HymnPathFileList list = hymn_walk(path, recursive);
    HymnArray *array = hymn_new_array(list.count);
    for (int i = 0; i < list.count; i++) {
        HymnString *file = list.files[i];
        HymnObjectString *item = hymn_intern_string(H, file);
        hymn_reference_string(item);
        array->items[i] = hymn_new_string_value(item);
    }
    free(list.files);
    if (count == 0) {
        hymn_string_delete(path);
    }
    return hymn_new_array_value(array);
}

static HymnValue path_list(Hymn *H, int count, HymnValue *arguments) {
    return walk(H, count, arguments, false);
}

static HymnValue path_walk(Hymn *H, int count, HymnValue *arguments) {
    return walk(H, count, arguments, true);
}

static HymnValue path_base(Hymn *H, int count, HymnValue *arguments) {
    if (count == 0) {
        return hymn_new_exception(H, "missing path");
    }
    HymnValue value = arguments[0];
    if (!hymn_is_string(value)) {
        return hymn_new_exception(H, "path must be a string");
    }
    HymnString *path = hymn_as_string(value);
    size_t size = hymn_string_len(path);
    if (path[size - 1] == PATH_SEP) {
        return hymn_new_string_value(hymn_intern_string(H, hymn_new_string("")));
    }
    if (size < 2) {
        return value;
    }
    size_t index = size - 2;
    while (true) {
        if (index == 0 || path[index] == PATH_SEP) {
            return hymn_new_string_value(hymn_intern_string(H, hymn_substring(path, index + 1, size)));
        }
        index--;
    }
}

static HymnValue path_extension(Hymn *H, int count, HymnValue *arguments) {
    if (count == 0) {
        return hymn_new_exception(H, "missing path");
    }
    HymnValue value = arguments[0];
    if (!hymn_is_string(value)) {
        return hymn_new_exception(H, "path must be a string");
    }
    HymnString *path = hymn_as_string(value);
    size_t size = hymn_string_len(path);
    if (path[size - 1] == '.' || size < 2) {
        return hymn_new_string_value(hymn_intern_string(H, hymn_new_string("")));
    }
    size_t index = size - 2;
    while (true) {
        if (index == 0) {
            return hymn_new_string_value(hymn_intern_string(H, hymn_new_string("")));
        } else if (path[index] == '.') {
            return hymn_new_string_value(hymn_intern_string(H, hymn_substring(path, index + 1, size)));
        }
        index--;
    }
}

void hymn_use_path(Hymn *H) {
    HymnTable *path = hymn_new_table();
    hymn_add_function_to_table(H, path, "working", path_working);
    hymn_add_function_to_table(H, path, "convert", path_convert);
    hymn_add_function_to_table(H, path, "normalize", path_normalize);
    hymn_add_function_to_table(H, path, "parent", path_parent);
    hymn_add_function_to_table(H, path, "cat", path_cat);
    hymn_add_function_to_table(H, path, "absolute", path_absolute);
    hymn_add_function_to_table(H, path, "list", path_list);
    hymn_add_function_to_table(H, path, "walk", path_walk);
    hymn_add_function_to_table(H, path, "base", path_base);
    hymn_add_function_to_table(H, path, "extension", path_extension);
    hymn_add_string_to_table(H, path, "symbol", PATH_SEP_STRING);
    hymn_add_table(H, "path", path);
}
