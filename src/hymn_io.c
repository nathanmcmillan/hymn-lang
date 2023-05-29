/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "hymn_io.h"

#ifdef _MSC_VER
#define IS_DIRECTORY(F) (((F)&S_IFMT) == S_IFDIR)
#else
#define IS_DIRECTORY(F) S_ISDIR(F)
#endif

#define PATH_STRING                                            \
    if (count < 1) {                                           \
        return hymn_new_exception(H, "missing path");          \
    }                                                          \
    HymnValue value = arguments[0];                            \
    if (!hymn_is_string(value)) {                              \
        return hymn_new_exception(H, "path must be a string"); \
    }                                                          \
    HymnString *path = hymn_as_string(value);

static HymnValue io_size(Hymn *H, int count, HymnValue *arguments) {
    PATH_STRING
    size_t size = hymn_file_size(path);
    return hymn_new_int((int64_t)size);
}

static HymnValue io_read(Hymn *H, int count, HymnValue *arguments) {
    PATH_STRING
    HymnString *string = hymn_read_file(path);
    if (string == NULL) {
        return hymn_new_exception(H, "fopen null pointer");
    }
    HymnObjectString *object = hymn_intern_string(H, string);
    return hymn_new_string_value(object);
}

static HymnValue io_read_lines(Hymn *H, int count, HymnValue *arguments) {
    PATH_STRING
    HymnString *string = hymn_read_file(path);
    if (string == NULL) {
        return hymn_new_exception(H, "fopen null pointer");
    }
    HymnArray *array = hymn_new_array(0);
    size_t size = hymn_string_len(string);
    size_t from = 0;
    for (size_t to = 0; to < size; to++) {
        char c = string[to];
        if (c == '\n' || (c == '\r' && to + 1 < size && string[to + 1] == '\n')) {
            HymnString *sub = hymn_substring(string, from, to);
            HymnObjectString *object = hymn_intern_string(H, sub);
            hymn_reference_string(object);
            hymn_array_push(array, hymn_new_string_value(object));
            from = to + 1;
            if (c == '\r') {
                from++;
            }
        }
    }
    if (from < size) {
        HymnString *sub = hymn_substring(string, from, size);
        HymnObjectString *object = hymn_intern_string(H, sub);
        hymn_reference_string(object);
        hymn_array_push(array, hymn_new_string_value(object));
    }
    hymn_string_delete(string);
    return hymn_new_array_value(array);
}

static HymnValue writing(Hymn *H, HymnString *path, HymnString *content, const char *mode) {
    FILE *open = fopen(path, mode);
    if (open == NULL) {
        return hymn_new_exception(H, "fopen null pointer");
    }
    fputs(content, open);
    fclose(open);
    return hymn_new_bool(true);
}

static HymnValue io_write(Hymn *H, int count, HymnValue *arguments) {
    if (count < 2) {
        return hymn_new_exception(H, "missing path and content");
    }
    HymnValue a = arguments[0];
    HymnValue b = arguments[1];
    if (!hymn_is_string(a) || !hymn_is_string(b)) {
        return hymn_new_exception(H, "path and content must be strings");
    }
    HymnString *path = hymn_as_string(a);
    HymnString *content = hymn_as_string(b);
    return writing(H, path, content, "w");
}

static HymnValue io_append(Hymn *H, int count, HymnValue *arguments) {
    if (count < 2) {
        return hymn_new_exception(H, "missing path and content");
    }
    HymnValue a = arguments[0];
    HymnValue b = arguments[1];
    if (!hymn_is_string(a) || !hymn_is_string(b)) {
        return hymn_new_exception(H, "path and content must be strings");
    }
    HymnString *path = hymn_as_string(a);
    HymnString *content = hymn_as_string(b);
    return writing(H, path, content, "a");
}

static HymnValue io_exists(Hymn *H, int count, HymnValue *arguments) {
    PATH_STRING
    bool exists = hymn_file_exists(path);
    return hymn_new_bool(exists);
}

static HymnValue io_stats(Hymn *H, int count, HymnValue *arguments) {
    PATH_STRING
    struct stat b;
    bool exists = stat(path, &b) == 0;
    if (!exists) {
        return hymn_new_exception(H, "path does not exist");
    }
    HymnTable *table = hymn_new_table();
    hymn_set_property_const(H, table, "directory", hymn_new_bool(IS_DIRECTORY(b.st_mode)));
    hymn_set_property_const(H, table, "mode", hymn_new_int(b.st_mode));
    hymn_set_property_const(H, table, "nlink", hymn_new_int((HymnInt)b.st_nlink));
    hymn_set_property_const(H, table, "uid", hymn_new_int(b.st_uid));
    hymn_set_property_const(H, table, "gid", hymn_new_int(b.st_gid));
    hymn_set_property_const(H, table, "size", hymn_new_int(b.st_size));
    hymn_set_property_const(H, table, "atime", hymn_new_int(b.st_atime));
    hymn_set_property_const(H, table, "mtime", hymn_new_int(b.st_mtime));
    hymn_set_property_const(H, table, "ctime", hymn_new_int(b.st_ctime));
    return hymn_new_table_value(table);
}

static HymnValue io_input(Hymn *H, int count, HymnValue *arguments) {
    (void)count;
    (void)arguments;
    HymnString *string = hymn_new_string("");
    while (true) {
        int c = getchar();
        if (c == '\n' || c == EOF) {
            break;
        }
        string = hymn_string_append_char(string, (char)c);
    }
    return hymn_new_string_value(hymn_intern_string(H, string));
}

static HymnValue io_move(Hymn *H, int count, HymnValue *arguments) {
    if (count < 2) {
        return hymn_new_exception(H, "missing source and destination paths");
    }
    HymnValue a = arguments[0];
    HymnValue b = arguments[1];
    if (!hymn_is_string(a) || !hymn_is_string(b)) {
        return hymn_new_exception(H, "source and destination must be strings");
    }
    HymnString *source = hymn_as_string(a);
    HymnString *target = hymn_as_string(b);
    int renamed = rename(source, target);
    return hymn_new_bool(renamed != -1);
}

static HymnValue io_copy(Hymn *H, int count, HymnValue *arguments) {
    if (count < 2) {
        return hymn_new_exception(H, "missing source and destination paths");
    }
    HymnValue a = arguments[0];
    HymnValue b = arguments[1];
    if (!hymn_is_string(a) || !hymn_is_string(b)) {
        return hymn_new_exception(H, "source and destination must be strings");
    }
    HymnString *source = hymn_as_string(a);
    HymnString *target = hymn_as_string(b);
    size_t size = hymn_file_size(source);
    FILE *from = fopen(source, "r");
    if (from == NULL) {
        return hymn_new_exception(H, "fopen null pointer");
    }
    FILE *to = fopen(target, "w");
    if (to == NULL) {
        return hymn_new_exception(H, "fopen null pointer");
    }
    for (size_t i = 0; i < size; i++) {
        fputc(fgetc(from), to);
    }
    fclose(from);
    fclose(to);
    return hymn_new_bool(true);
}

static HymnValue io_remove(Hymn *H, int count, HymnValue *arguments) {
    PATH_STRING
    int removed = remove(path);
    return hymn_new_bool(removed != -1);
}

static HymnValue io_mkdir(Hymn *H, int count, HymnValue *arguments) {
    PATH_STRING
#ifdef _MSC_VER
    int result = mkdir(path);
#else
    int result = mkdir(path, 0755);
#endif
    return hymn_new_bool(result != -1);
}

void hymn_use_io(Hymn *H) {
    HymnTable *io = hymn_new_table();
    hymn_add_function_to_table(H, io, "size", io_size);
    hymn_add_function_to_table(H, io, "read", io_read);
    hymn_add_function_to_table(H, io, "read-lines", io_read_lines);
    hymn_add_function_to_table(H, io, "write", io_write);
    hymn_add_function_to_table(H, io, "append", io_append);
    hymn_add_function_to_table(H, io, "exists", io_exists);
    hymn_add_function_to_table(H, io, "stats", io_stats);
    hymn_add_function_to_table(H, io, "input", io_input);
    hymn_add_function_to_table(H, io, "move", io_move);
    hymn_add_function_to_table(H, io, "copy", io_copy);
    hymn_add_function_to_table(H, io, "remove", io_remove);
    hymn_add_function_to_table(H, io, "mkdir", io_mkdir);
    hymn_add_table(H, "io", io);
}
