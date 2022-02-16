/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "hymn.h"

static bool glob_here(char *regexp, char *text);

static bool glob_star(int c, char *regexp, char *text) {
    do {
        if (glob_here(regexp, text)) {
            return true;
        }
    } while (*text != '\0' && (*text++ == c || c == '?'));
    return false;
}

static bool glob_here(char *regexp, char *text) {
    if (regexp[0] == '\0') {
        return true;
    }
    if (regexp[1] == '*') {
        return glob_star(regexp[0], regexp + 2, text);
    }
    if (*text != '\0' && (regexp[0] == '?' || regexp[0] == *text)) {
        return glob_here(regexp + 1, text + 1);
    }
    return false;
}

static HymnValue glob(Hymn *H, int count, HymnValue *arguments) {
    (void)H;
    if (count < 2) {
        return hymn_new_none();
    }
    HymnString *expression = hymn_as_string(arguments[0]);
    HymnString *text = hymn_as_string(arguments[1]);
    // TODO, match against '*' (anything) and '?' (any single character)
    bool result = glob_here(expression, text);
    return hymn_new_bool(result);
}

static bool match_here(char *regexp, char *text);

static bool match_star(int c, char *regexp, char *text) {
    do {
        if (match_here(regexp, text)) {
            return true;
        }
    } while (*text != '\0' && (*text++ == c || c == '.'));
    return false;
}

static bool match_here(char *regexp, char *text) {
    if (regexp[0] == '\0') {
        return true;
    }
    if (regexp[1] == '*') {
        return match_star(regexp[0], regexp + 2, text);
    }
    if (regexp[0] == '$' && regexp[1] == '\0') {
        return *text == '\0';
    }
    if (*text != '\0' && (regexp[0] == '.' || regexp[0] == *text)) {
        return match_here(regexp + 1, text + 1);
    }
    return false;
}

static bool match(char *regexp, char *text) {
    if (regexp[0] == '^') {
        return match_here(regexp + 1, text);
    }
    do {
        if (match_here(regexp, text)) {
            return true;
        }
    } while (*text++ != '\0');
    return false;
}

static HymnValue pattern(Hymn *H, int count, HymnValue *arguments) {
    (void)H;
    if (count < 2) {
        return hymn_new_none();
    }
    HymnString *expression = hymn_as_string(arguments[0]);
    HymnString *text = hymn_as_string(arguments[1]);
    bool result = match(expression, text);
    return hymn_new_bool(result);
}

void hymn_use_pattern(Hymn *H) {
    hymn_add_function(H, "glob", glob);
    hymn_add_function(H, "pattern", pattern);
}
