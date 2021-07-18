/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "main.h"

#ifndef HYMN_TESTING

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    Hymn *vm = new_hymn();

    char *error = hymn_read(vm, "test/scripts/test.hm");
    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
        return 1;
    }

    return 0;
}

#endif
