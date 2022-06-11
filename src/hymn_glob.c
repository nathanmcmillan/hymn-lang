/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "hymn.h"

static bool match(char *pattern, char *text) {
init:
    if (pattern[0] == '\0') {
        return text[0] == '\0';
    } else if (pattern[0] == '*') {
        pattern++;
        while (true) {
            if (match(pattern, text)) {
                return true;
            } else if (text[0] == '\0') {
                return false;
            }
            text++;
        }
    } else if (text[0] != '\0' && (pattern[0] == '?' || pattern[0] == text[0])) {
        pattern++;
        text++;
        goto init;
    }
    return false;
}

static HymnValue glob(Hymn *H, int count, HymnValue *arguments) {
    if (count < 2) {
        return hymn_arity_exception(H, 2, count);
    }
    HymnValue pattern = arguments[0];
    if (!hymn_is_string(pattern)) {
        return hymn_type_exception(H, HYMN_VALUE_STRING, pattern.is);
    }
    HymnValue text = arguments[1];
    if (!hymn_is_string(text)) {
        return hymn_type_exception(H, HYMN_VALUE_STRING, text.is);
    }
    bool result = match(hymn_as_string(pattern), hymn_as_string(text));
    return hymn_new_bool(result);
}

void hymn_use_glob(Hymn *H) {
    hymn_add_function(H, "glob", glob);
}
