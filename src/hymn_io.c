/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "hymn.h"

static HymnValue io_size(Hymn *H, int count, HymnValue *arguments) {
    (void)H;
    if (count < 1) {
        return hymn_new_none();
    }
    HymnString *path = hymn_as_string(arguments[0]);
    size_t size = hymn_file_size(path);
    return hymn_new_int((int64_t)size);
}

static HymnValue io_read(Hymn *H, int count, HymnValue *arguments) {
    if (count < 1) {
        return hymn_new_none();
    }
    HymnString *path = hymn_as_string(arguments[0]);
    HymnString *string = hymn_read_file(path);
    if (string == NULL) {
        return hymn_new_none();
    }
    HymnObjectString *object = hymn_intern_string(H, string);
    return hymn_new_string_value(object);
}

static HymnValue io_exists(Hymn *H, int count, HymnValue *arguments) {
    (void)H;
    if (count < 1) {
        return hymn_new_none();
    }
    HymnString *path = hymn_as_string(arguments[0]);
    bool exists = hymn_file_exists(path);
    return hymn_new_bool(exists);
}

static HymnValue io_input(Hymn *H, int count, HymnValue *arguments) {
    (void)H;
    (void)count;
    (void)arguments;
    HymnString *string = hymn_new_string("");
    while (true) {
        char c = getchar();
        if (c == '\n' || c == EOF) {
            break;
        }
        string = hymn_string_append_char(string, c);
    }
    return hymn_new_string_value(hymn_intern_string(H, string));
}

void hymn_use_io(Hymn *H) {
    hymn_add_function(H, "io:size", io_size);
    hymn_add_function(H, "io:read", io_read);
    hymn_add_function(H, "io:exists", io_exists);
    hymn_add_function(H, "io:input", io_input);
}
