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

static void help() {
    printf("Hymn Script\n\n"
           "  -c  Run input as source code\n"
           "  -r  Open interactive mode\n"
           "  -b  Print compiled byte code\n"
           "  -v  Print version information\n"
           "  -h  Print this help message\n"
           "  --  End of options\n");
}

int main(int argc, char **argv) {

    bool repl = false;
    bool byte = false;
    char *file = NULL;
    char *code = NULL;

    if (argc >= 2) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--") == 0) {
                if (i + 1 < argc) {
                    file = argv[i + 1];
                }
                break;
            } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
                help();
                return 2;
            } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
                printf("Hymn " HYMN_VERSION "\n");
                return 0;
            } else if (strcmp(argv[i], "-c") == 0) {
                if (i + 1 < argc) {
                    code = argv[i + 1];
                    i++;
                } else {
                    help();
                    return 1;
                }
            } else if (strcmp(argv[i], "-b") == 0) {
                byte = true;
            } else if (strcmp(argv[i], "-r") == 0) {
                repl = true;
            } else {
                file = argv[i];
            }
        }
    }

    signal(SIGINT, signal_handle);

    Hymn *hymn = new_hymn();
    hymn_use_libs(hymn);

    int exit = 0;

    if (file != NULL) {
        char *error;
        if (byte) {
            error = hymn_debug(hymn, file, NULL);
        } else {
            error = hymn_read(hymn, file);
        }
        if (error != NULL) {
            fprintf(stderr, "%s\n", error);
            fflush(stderr);
            free(error);
            exit = 1;
        }
    }

    if (code != NULL) {
        char *error;
        if (byte) {
            error = hymn_debug(hymn, NULL, code);
        } else {
            error = hymn_do(hymn, code);
        }
        if (error != NULL) {
            fprintf(stderr, "%s\n", error);
            fflush(stderr);
            free(error);
            exit = 1;
        }
    }

    if (repl || (file == NULL && code == NULL)) {
        hymn_repl(hymn);
    }

    hymn_delete(hymn);

    return exit;
}

#endif
