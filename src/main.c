/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "main.h"

#ifndef HYMN_TESTING

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

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    Hymn *hymn = new_hymn();

    hymn_add_function(hymn, "read", read_file);

    char *error = hymn_read(hymn, "test/scripts/test.hm");
    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
        return 1;
    }

    hymn_delete(hymn);

    return 0;
}

#endif
