/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "hymn.h"
#include "hymn_libs.h"

#ifndef HYMN_TESTING

static void signal_handle(int signum) {
    if (signum != 2) {
        exit(signum);
    }
}

int main(int argc, char **argv) {

    signal(SIGINT, signal_handle);

    Hymn *hymn = new_hymn();

    char *error = NULL;
    if (argc > 1) {
        hymn_use_libs(hymn);
        error = hymn_read(hymn, argv[1]);
    } else {
        printf("Usage: hymn FILE\n");
        printf("Interprets a Hymn script FILE.\n");
    }

    int code = 0;
    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
        fflush(stderr);

        free(error);
        code = 1;
    }

    hymn_delete(hymn);

    return code;
}

#endif
