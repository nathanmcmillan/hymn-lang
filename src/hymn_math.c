/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <math.h>

#include "hymn.h"

static HymnValue math_floor(Hymn *this, int count, HymnValue *arguments) {
    (void)this;
    if (count < 1) return hymn_new_none();
    HymnValue argument = arguments[0];
    if (hymn_is_float(argument)) return hymn_new_float(floor(hymn_as_float(argument)));
    return argument;
}

void hymn_use_math(Hymn *hymn) {
    hymn_add_function(hymn, "math_floor", math_floor);
}
