/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "hymn.h"

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

static HymnValue io_size(Hymn *H, int count, HymnValue *arguments) {
    (void)H;
    if (count < 1) {
        return hymn_new_none();
    }
    HymnString *path = hymn_as_string(arguments[0]);
    size_t size = file_size(path);
    return hymn_new_int((int64_t)size);
}

static HymnValue io_read(Hymn *H, int count, HymnValue *arguments) {
    if (count < 1) {
        return hymn_new_none();
    }
    HymnString *path = hymn_as_string(arguments[0]);
    size_t size = file_size(path);
    FILE *open = fopen(path, "r");
    if (open == NULL) {
        return hymn_new_none();
    }
    char *content = hymn_malloc((size + 1) * sizeof(char));
    for (size_t i = 0; i < size; i++) {
        content[i] = (char)fgetc(open);
    }
    fclose(open);
    HymnObjectString *string = hymn_get_string_with_length(H, content, size);
    free(content);
    return hymn_new_string_value(string);
}

void hymn_use_io(Hymn *H) {
    hymn_add_function(H, "io_size", io_size);
    hymn_add_function(H, "io_read", io_read);
}
