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
    if (signum == SIGINT) {
        signal(SIGINT, signal_handle);
    } else {
        exit(signum);
    }
}

static void help(void) {
    printf("hymn script v" HYMN_VERSION "\n\n"
           "  -c  run command\n"
           "  -i  open interactive mode\n"
           "  -b  print compiled byte code\n"
           "  -v  print version information\n"
           "  -h  print this help message\n"
           "  --  end of options\n");
}

int main(int argc, char **argv) {

    bool repl = false;
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
                printf("hymn v" HYMN_VERSION "\n");
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
                repl = true;
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
            error = hymn_script(hymn, file);
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
            error = hymn_command(hymn, code);
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
    if (repl || (file == NULL && code == NULL)) {
        hymn_repl(hymn);
    }
#endif

    hymn_delete(hymn);

    return exit;
}

#endif
