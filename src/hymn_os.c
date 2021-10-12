/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <stdlib.h>
#include <time.h>

#include "hymn.h"

static HymnValue os_env(Hymn *this, int count, HymnValue *arguments) {
    (void)this;
    if (count < 1) return hymn_new_none();
    HymnString *name = hymn_as_string(arguments[0]);
    char *variable = getenv(name);
    if (variable == NULL) return hymn_new_none();
    HymnObjectString *object = hymn_new_string_object(variable);
    return hymn_new_string_value(object);
}

static HymnValue os_clock(Hymn *this, int count, HymnValue *arguments) {
    (void)this;
    (void)count;
    (void)arguments;
    return hymn_new_float((double)clock() / (double)CLOCKS_PER_SEC);
}

void hymn_use_os(Hymn *hymn) {
    hymn_add_function(hymn, "os_env", os_env);
    hymn_add_function(hymn, "os_clock", os_clock);
}
