/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "hymn.h"

static HymnValue fun(Hymn *H, int count, HymnValue *arguments) {
    (void)H;
    (void)count;
    (void)arguments;
    return hymn_new_int(256);
}

export void hymn_import(Hymn *H) {
    HymnTable *lib = hymn_new_table();
    hymn_add_function_to_table(H, lib, "fun", fun);
    hymn_add_table(H, "dynamic", lib);
}
