/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "hymn.h"

static HymnValue os_env(Hymn *H, int count, HymnValue *arguments) {
    if (count < 1) {
        return hymn_new_none();
    }
    HymnString *name = hymn_as_string(arguments[0]);
    char *variable = getenv(name);
    if (variable == NULL) {
        return hymn_new_none();
    }
    HymnObjectString *string = hymn_get_string(H, variable);
    return hymn_new_string_value(string);
}

static HymnValue os_clock(Hymn *H, int count, HymnValue *arguments) {
    (void)H;
    (void)count;
    (void)arguments;
    return hymn_new_float((double)clock() / (double)CLOCKS_PER_SEC);
}

void hymn_use_os(Hymn *H) {
    hymn_add_function(H, "os_env", os_env);
    hymn_add_function(H, "os_clock", os_clock);
    hymn_add(H, "stdout", hymn_new_int(0));
    hymn_add(H, "stderr", hymn_new_int(1));
}
