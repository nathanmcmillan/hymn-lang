/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "main.h"

#ifndef HYMN_TESTING

static HymnValue example_native_test(int count, HymnValue *arguments) {
    if (count == 0) {
        return hymn_new_none();
    }
    i64 i = hymn_as_int(arguments[0]) + 1;
    return hymn_new_int(i);
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    Hymn *hymn = new_hymn();

    hymn_add_function(hymn, "inc", example_native_test);

    char *error = hymn_read(hymn, "test/scripts/test.hm");
    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
        return 1;
    }

    hymn_delete(hymn);
    LOG("END PROGRAM");

    return 0;
}

#endif
