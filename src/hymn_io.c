/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "hymn.h"

// #if _POSIX_C_SOURCE >= 2
// #define HYMN_POPEN_SUPPORTED
// #endif

static HymnValue io_popen(Hymn *this, int count, HymnValue *arguments) {
    (void)this;
#ifdef HYMN_POPEN_SUPPORTED
    if (count < 2) return hymn_new_none();
    HymnString *file = hymn_as_string(arguments[0]);
    HymnString *mode = hymn_as_string(arguments[1]);
    FILE *f = popen(file, mode);
    return f == NULL ? hymn_new_none() : hymn_new_pointer(f);
#else
    (void)count;
    (void)arguments;
    return hymn_new_none();
#endif
}

void hymn_use_io(Hymn *hymn) {
    hymn_add_function(hymn, "io_popen", io_popen);
}
