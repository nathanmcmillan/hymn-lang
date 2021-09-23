/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <inttypes.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "hm_vm.h"

#ifndef HYMN_TESTING

static HymnValue null_pointer(Hymn *this, int count, HymnValue *arguments) {
    (void)this;
    (void)count;
    (void)arguments;
    return hymn_new_pointer(NULL);
}

static HymnValue read_file(Hymn *this, int count, HymnValue *arguments) {
    (void)this;
    if (count != 1) {
        return hymn_new_none();
    }
    String *string = hymn_as_string(arguments[0])->string;
    String *content = cat(string);

    HymnString *object = new_hymn_string(content);
    HymnValue response = (HymnValue){.is = HYMN_VALUE_STRING, .as = {.o = (HymnObject *)object}};

    return response;
}

void signal_handle(int signum) {
    if (signum != 2) {
        exit(signum);
    }
}

int main(int argc, char **argv) {

    signal(SIGINT, signal_handle);

    Hymn *hymn = new_hymn();

    hymn_add_function(hymn, "null", null_pointer);
    hymn_add_function(hymn, "read", read_file);

    char *error = NULL;

    if (argc > 1) {
        error = hymn_read(hymn, argv[1]);
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
