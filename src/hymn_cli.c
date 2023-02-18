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

static void help(void) {
    printf("Hymn Script\n\n"
           "  -c  Run command\n"
           "  -i  Open interactive mode\n"
           "  -s  Open server mode\n"
           "  -b  Print compiled byte code\n"
           "  -v  Print version information\n"
           "  -h  Print this help message\n"
           "  --  End of options\n");
}

int main(int argc, char **argv) {

    char mode = 0;
    bool byte = false;

    char *file = NULL;
    char *code = NULL;

    if (argc >= 2) {
        for (int i = 1; i < argc; i++) {
            if (hymn_string_equal(argv[i], "--")) {
                if (i + 1 < argc) {
                    file = argv[i + 1];
                }
                break;
            } else if (hymn_string_equal(argv[i], "-h") || hymn_string_equal(argv[i], "--help")) {
                help();
                return 2;
            } else if (hymn_string_equal(argv[i], "-v") || hymn_string_equal(argv[i], "--version")) {
                printf("Hymn " HYMN_VERSION "\n");
                return EXIT_SUCCESS;
            } else if (hymn_string_equal(argv[i], "-c")) {
                if (i + 1 < argc) {
                    code = argv[i + 1];
                    i++;
                } else {
                    help();
                    return EXIT_FAILURE;
                }
            } else if (hymn_string_equal(argv[i], "-b")) {
                byte = true;
            } else if (hymn_string_equal(argv[i], "-i")) {
                mode = 1;
            } else if (hymn_string_equal(argv[i], "-s")) {
                mode = 2;
            } else {
                file = argv[i];
            }
        }
    }

    signal(SIGINT, signal_handle);

    Hymn *hymn = new_hymn();
    hymn_use_libs(hymn);

    int exit = EXIT_SUCCESS;

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
            exit = EXIT_FAILURE;
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
            exit = EXIT_FAILURE;
        }
    }

#ifdef HYMN_NO_REPL
    (void)mode;
    fprintf(stderr, "interactive mode not available\n");
#else
    if (mode == 2) {
        hymn_server(hymn);
    } else if (mode == 1 || (file == NULL && code == NULL)) {
        hymn_repl(hymn);
    }
#endif

    hymn_delete(hymn);

    return exit;
}

#endif
