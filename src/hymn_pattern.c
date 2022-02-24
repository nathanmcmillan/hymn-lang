/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "hymn.h"

static bool glob_here(char *pattern, char *text);

static bool glob_star(char *pattern, char *text) {
    while (true) {
        if (glob_here(pattern, text)) {
            return true;
        } else if (text[0] == '\0') {
            return false;
        }
        text++;
    }
}

static bool glob_here(char *pattern, char *text) {
    if (pattern[0] == '\0') {
        return text[0] == '\0';
    } else if (pattern[0] == '*') {
        return glob_star(pattern + 1, text);
    } else if (text[0] != '\0' && (pattern[0] == '?' || pattern[0] == text[0])) {
        return glob_here(pattern + 1, text + 1);
    }
    return false;
}

static HymnValue glob(Hymn *H, int count, HymnValue *arguments) {
    (void)H;
    if (count >= 2) {
        HymnValue expression = arguments[0];
        HymnValue text = arguments[1];
        if (hymn_is_string(expression) && hymn_is_string(text)) {
            bool result = glob_here(hymn_as_string(expression), hymn_as_string(text));
            return hymn_new_bool(result);
        }
    }
    return hymn_new_none();
}

static bool match_here(char *regex, char *text);

static bool match_star(int c, char *regex, char *text) {
    while (true) {
        if (match_here(regex, text)) {
            return true;
        }
        char n = text[0];
        if (n == '\0' || (c != '.' && c != n)) {
            return false;
        }
        text++;
    }
}

static bool match_here(char *regex, char *text) {
    if (regex[0] == '\0') {
        return true;
    } else if (regex[1] == '*') {
        return match_star(regex[0], regex + 2, text);
    } else if (regex[0] == '$' && regex[1] == '\0') {
        return text[0] == '\0';
    } else if (text[0] != '\0' && (regex[0] == '.' || regex[0] == text[0])) {
        return match_here(regex + 1, text + 1);
    }
    return false;
}

static bool match(char *regex, char *text) {
    if (regex[0] == '^') {
        return match_here(regex + 1, text);
    }
    while (true) {
        if (match_here(regex, text)) {
            return true;
        }
        if (text[0] == '\0') {
            break;
        }
        text++;
    }
    return false;
}

static HymnValue pattern(Hymn *H, int count, HymnValue *arguments) {
    (void)H;
    if (count >= 2) {
        HymnValue expression = arguments[0];
        HymnValue text = arguments[1];
        if (hymn_is_string(expression) && hymn_is_string(text)) {
            bool result = match(hymn_as_string(expression), hymn_as_string(text));
            return hymn_new_bool(result);
        }
    }
    return hymn_new_none();
}

// pattern_match
// pattern_replace

void hymn_use_pattern(Hymn *H) {
    hymn_add_function(H, "glob", glob);
    hymn_add_function(H, "pattern", pattern);
}
