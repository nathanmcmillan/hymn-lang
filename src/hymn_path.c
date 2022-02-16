/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "hymn.h"

static HymnValue path_join(Hymn *H, int count, HymnValue *arguments) {
    HymnString *string = hymn_new_string("");
    for (int a = 0; a < count; a++) {
        if (a != 0) {
            string = hymn_string_append_char(string, '/');
        }
        HymnString *path = hymn_as_string(arguments[a]);
        string = hymn_string_append(string, path);
    }
    return hymn_new_string_value(hymn_intern_string(H, string));
}

// walk directory

// normalize path

void hymn_use_path(Hymn *H) {
    (void)H;
    hymn_add_function(H, "path:cat", path_join);
    hymn_add(H, "path:symbol", hymn_new_string_value(hymn_get_string(H, "/")));
}
