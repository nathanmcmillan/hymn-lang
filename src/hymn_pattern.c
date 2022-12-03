/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <ctype.h>

#include "hymn.h"

#define MAX_CAPTURES 32

typedef struct Capture Capture;
typedef struct Match Match;

struct Capture {
    const char *begin;
    int size;
};

struct Match {
    char *error;
    int count;
    Capture capture[MAX_CAPTURES];
};

static bool class(int type, int c) {
    if (type == '\0') return false;
    int result;
    switch (tolower(type)) {
    case 'a': result = isalpha(c); break;
    case 'c': result = iscntrl(c); break;
    case 'd': result = isdigit(c); break;
    case 'l': result = islower(c); break;
    case 'p': result = ispunct(c); break;
    case 's': result = isspace(c); break;
    case 'u': result = isupper(c); break;
    case 'w': result = isalnum(c); break;
    case 'x': result = isxdigit(c); break;
    case 'z': result = (c == 0); break;
    default: return (c == type);
    }
    return islower(type) ? result : !result;
}

static char *possible(Match *group, char *pattern) {
    char c = pattern[0];
    pattern++;
    switch (c) {
    case '%': {
        if (pattern[0] == '\0') {
            group->error = "malformed end of class";
        }
        return pattern + 1;
    }
    case '[': {
        while (true) {
            c = pattern[0];
            if (c == '\0') {
                group->error = "malformed end of class";
                return pattern;
            } else if (c == ']') {
                return pattern + 1;
            } else if (c == '%') {
                pattern++;
                if (pattern[0] == '\0') {
                    group->error = "malformed end of class";
                    return pattern;
                }
            }
            pattern++;
        }
    }
    default: return pattern;
    }
}

static bool any(char *pattern, char *end, int c) {
    while (true) {
        pattern++;
        if (pattern == end) {
            return false;
        } else if (pattern[0] == '%') {
            pattern++;
            if (class(pattern[0], c)) {
                return true;
            }
        } else if (pattern[0] == c) {
            return true;
        }
    }
}

static bool check(char *pattern, char *end, int c) {
    if (c == '\0') {
        return false;
    }
    switch (pattern[0]) {
    case '.': return true;
    case '%': return class(pattern[1], c);
    case '[': return any(pattern, end, c);
    default: return pattern[0] == c;
    }
}

static bool match(Match *group, char *pattern, char *text);

static bool greedy(Match *group, char *pattern, char *end, char *text) {
    int count = 0;
    while (check(pattern, end, text[count])) {
        count++;
    }
    while (count >= 0) {
        if (match(group, end + 1, text + count)) {
            return true;
        }
        count--;
    }
    return false;
}

static bool minimal(Match *group, char *pattern, char *end, char *text) {
    while (true) {
        if (match(group, end + 1, text)) {
            return true;
        } else if (check(pattern, end, text[0])) {
            text++;
        } else {
            return false;
        }
    }
}

static bool capturing(Match *group, char *pattern, char *text) {
    int count = group->count;
    if (count >= MAX_CAPTURES) {
        group->error = "too many capture groups";
        return false;
    }
    group->capture[count].begin = text;
    group->capture[count].size = -1;
    group->count = count + 1;
    bool matching = match(group, pattern, text);
    if (!matching) {
        group->count--;
    }
    return matching;
}

static bool captured(Match *group, char *pattern, char *text) {
    int count = group->count;
    if (count > 0) {
        while (--count >= 0) {
            Capture *capture = &group->capture[count];
            if (capture->size == -1) {
                capture->size = (int)(text - capture->begin);
                bool matching = match(group, pattern, text);
                if (!matching) {
                    capture->size = -1;
                }
                return matching;
            }
        }
    }
    group->error = "missing start of capture group";
    return false;
}

static bool match(Match *group, char *pattern, char *text) {
init:
    switch (pattern[0]) {
    case '\0':
        return true;
    case '(':
        return capturing(group, pattern + 1, text);
    case ')':
        return captured(group, pattern + 1, text);
    case '$': {
        if (pattern[1] == '\0') {
            return text[0] == '\0';
        }
    }
    }
    char *end = possible(group, pattern);
    if (group->error != NULL) {
        return false;
    }
    // printf("pattern: %c end: %c text: %c\n", pattern[0], end[0], text[0]);
    bool matching = check(pattern, end, text[0]);
    switch (end[0]) {
    case '-': return minimal(group, pattern, end, text);
    case '*': return greedy(group, pattern, end, text);
    case '+': return matching && greedy(group, pattern, end, text + 1);
    case '?': {
        if (matching && match(group, end + 1, text + 1)) {
            return true;
        } else if (group->error != NULL) {
            return false;
        }
        pattern = end + 1;
        goto init;
    }
    default:
        if (matching) {
            pattern = end;
            text++;
            goto init;
        }
        return false;
    }
}

static bool search(Match *group, char *pattern, char *text) {
    if (pattern[0] == '^') {
        return match(group, pattern + 1, text);
    }
    while (true) {
        if (match(group, pattern, text)) {
            return true;
        } else if (group->error != NULL) {
            return false;
        }
        if (text[0] == '\0') {
            return false;
        }
        text++;
    }
}

static HymnValue find(Hymn *H, int count, HymnValue *arguments) {
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
    HymnString *source = hymn_as_string(text);
    HymnInt start = 0;
    if (count >= 3) {
        HymnValue number = arguments[2];
        if (!hymn_is_int(number)) {
            return hymn_type_exception(H, HYMN_VALUE_INTEGER, number.is);
        }
        start = hymn_as_int(number);
        if (start < 0) {
            start = 0;
        } else if (start >= (HymnInt)hymn_string_len(source)) {
            return hymn_new_bool(false);
        }
        source += start;
    }
    Match group = {0};
    bool result = search(&group, hymn_as_string(pattern), source);
    if (group.error != NULL) {
        return hymn_new_exception(H, group.error);
    }
    for (int i = 0; i < group.count; i++) {
        printf("capture: [%.*s]\n", group.capture[i].size, group.capture[i].begin);
    }
    return hymn_new_bool(result);
}

static HymnValue replace(Hymn *H, int count, HymnValue *arguments) {
    (void)H;
    (void)count;
    (void)arguments;
    return hymn_new_none();
}

void hymn_use_pattern(Hymn *H) {
    HymnTable *pattern = hymn_new_table();
    hymn_add_function_to_table(H, pattern, "find", find);
    hymn_add_function_to_table(H, pattern, "replace", replace);
    hymn_add_table(H, "pattern", pattern);
}
