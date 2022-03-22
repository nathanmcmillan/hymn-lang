/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hymn.h"
#include "hymn_libs.h"

#if !defined(HYMN_TESTING) && !defined(HYMN_NO_CLI)

static void signal_handle(int signum) {
    if (signum != 2) {
        exit(signum);
    }
}

int main(int argc, char **argv) {

    // TODO: Non ordered options
    // Run a script and open REPL afterwards

    if (argc >= 2) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            printf("Interprets a Hymn script FILE.\n\n");
            printf("  -c  Run FILE as source code\n");
            printf("  -b  Print compiled byte code\n");
            printf("  -v  Prints version information\n");
            printf("  -h  Print this help message\n");
            printf("  -r  Open REPL\n");
            return 2;
        } else if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
            printf("Hymn " HYMN_VERSION "\n");
            return 1;
        }
    }

    signal(SIGINT, signal_handle);

    Hymn *hymn = new_hymn();
    hymn_use_libs(hymn);

    char *error = NULL;

    if (argc <= 1) {
        hymn_repl(hymn);
    } else if (argc >= 3) {
        if (strcmp(argv[1], "-b") == 0) {
            if (argc >= 4) {
                if (strcmp(argv[2], "-c") == 0) {
                    error = hymn_debug(hymn, NULL, argv[3]);
                } else {
                    printf("Unknown second argument: %s\n", argv[2]);
                }
            } else {
                error = hymn_debug(hymn, argv[2], NULL);
            }
        } else if (strcmp(argv[1], "-c") == 0) {
            error = hymn_do(hymn, argv[2]);
        } else {
            printf("Unknown argument: %s\n", argv[1]);
        }
    } else if (argc >= 2 && strcmp(argv[1], "-r") == 0) {
        hymn_repl(hymn);
    } else {
        error = hymn_read(hymn, argv[1]);
    }

    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
        fflush(stderr);
        free(error);
    }

    hymn_delete(hymn);

    return error != NULL ? 1 : 0;
}

#endif
