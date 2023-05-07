/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <ctype.h>

#include "hymn_pattern.h"

#define MAX_CAPTURES 16

typedef struct Capture Capture;
typedef struct Match Match;

struct Capture {
    const char *begin;
    size_t size;
};

struct Match {
    const char *error;
    int count;
    char padding[4];
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
    case '[': return any(pattern, end - 1, c);
    default: return pattern[0] == c;
    }
}

static char *match(Match *group, char *pattern, char *text);

static char *greedy(Match *group, char *pattern, char *end, char *text) {
    int count = 0;
    while (check(pattern, end, text[count])) {
        count++;
    }
    while (count >= 0) {
        char *stop = match(group, end + 1, text + count);
        if (stop != NULL) {
            return stop;
        }
        count--;
    }
    return NULL;
}

static char *minimal(Match *group, char *pattern, char *end, char *text) {
    while (true) {
        char *stop = match(group, end + 1, text);
        if (stop != NULL) {
            return stop;
        } else if (check(pattern, end, text[0])) {
            text++;
        } else {
            return NULL;
        }
    }
}

static char *capturing(Match *group, char *pattern, char *text) {
    int count = group->count;
    if (count >= MAX_CAPTURES) {
        group->error = "too many capture groups";
        return false;
    }
    group->capture[count].begin = text;
    group->capture[count].size = SIZE_MAX;
    group->count = count + 1;
    char *stop = match(group, pattern, text);
    if (stop == NULL) {
        group->count--;
    }
    return stop;
}

static char *captured(Match *group, char *pattern, char *text) {
    int count = group->count;
    if (count > 0) {
        while (--count >= 0) {
            Capture *capture = &group->capture[count];
            if (capture->size == SIZE_MAX) {
                capture->size = (size_t)(text - capture->begin);
                char *stop = match(group, pattern, text);
                if (stop == NULL) {
                    capture->size = SIZE_MAX;
                }
                return stop;
            }
        }
    }
    group->error = "missing start of capture group";
    return NULL;
}

static char *match(Match *group, char *pattern, char *text) {
init:
    switch (pattern[0]) {
    case '\0':
        return text;
    case '(':
        return capturing(group, pattern + 1, text);
    case ')':
        return captured(group, pattern + 1, text);
    case '$': {
        if (pattern[1] == '\0') {
            return text[0] == '\0' ? text : NULL;
        }
        break;
    }
    default:
        break;
    }
    char *end = possible(group, pattern);
    if (group->error != NULL) {
        return NULL;
    }
    bool matching = check(pattern, end, text[0]);
    switch (end[0]) {
    case '-': return minimal(group, pattern, end, text);
    case '*': return greedy(group, pattern, end, text);
    case '+': return matching ? greedy(group, pattern, end, text + 1) : NULL;
    case '?': {
        if (matching) {
            char *stop = match(group, end + 1, text + 1);
            if (stop != NULL) {
                return stop;
            }
        }
        if (group->error != NULL) {
            return NULL;
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
        return NULL;
    }
}

static HymnValue get(Hymn *H, int count, HymnValue *arguments) {
    if (count < 2) {
        return hymn_arity_exception(H, 2, count);
    }
    HymnValue source = arguments[0];
    if (!hymn_is_string(source)) {
        return hymn_type_exception(H, HYMN_VALUE_STRING, source.is);
    }
    HymnValue expression = arguments[1];
    if (!hymn_is_string(expression)) {
        return hymn_type_exception(H, HYMN_VALUE_STRING, expression.is);
    }
    HymnString *text = hymn_as_string(source);
    if (count >= 3) {
        HymnValue number = arguments[2];
        if (!hymn_is_int(number)) {
            return hymn_type_exception(H, HYMN_VALUE_INTEGER, number.is);
        }
        HymnInt start = hymn_as_int(number);
        if (start < 0) {
            start = 0;
        } else if (start >= (HymnInt)hymn_string_len(text)) {
            return hymn_new_none();
        }
        text += start;
    }
    HymnString *pattern = hymn_as_string(expression);
    char *begin = NULL;
    char *end = NULL;
    Match group = {0};
    if (pattern[0] == '^') {
        end = match(&group, pattern + 1, text);
        if (end != NULL) {
            begin = text;
        }
    } else {
        while (true) {
            end = match(&group, pattern, text);
            if (end != NULL) {
                begin = text;
                break;
            } else if (group.error != NULL || text[0] == '\0') {
                break;
            }
            text++;
        }
    }
    if (group.error != NULL) {
        return hymn_new_exception(H, group.error);
    } else if (begin == NULL) {
        return hymn_new_none();
    }
    HymnArray *array = hymn_new_array(0);
    HymnString *whole = hymn_string_format("%.*s", end - begin, begin);
    HymnObjectString *have = hymn_intern_string(H, whole);
    hymn_reference_string(have);
    hymn_array_push(array, hymn_new_string_value(have));
    for (int i = 0; i < group.count; i++) {
        HymnString *sub = hymn_string_format("%.*s", group.capture[i].size, group.capture[i].begin);
        HymnObjectString *object = hymn_intern_string(H, sub);
        hymn_reference_string(object);
        hymn_array_push(array, hymn_new_string_value(object));
    }
    return hymn_new_array_value(array);
}

static HymnValue find(Hymn *H, int count, HymnValue *arguments) {
    if (count < 2) {
        return hymn_arity_exception(H, 2, count);
    }
    HymnValue source = arguments[0];
    if (!hymn_is_string(source)) {
        return hymn_type_exception(H, HYMN_VALUE_STRING, source.is);
    }
    HymnValue expression = arguments[1];
    if (!hymn_is_string(expression)) {
        return hymn_type_exception(H, HYMN_VALUE_STRING, expression.is);
    }
    HymnString *original = hymn_as_string(source);
    HymnString *text = original;
    if (count >= 3) {
        HymnValue number = arguments[2];
        if (!hymn_is_int(number)) {
            return hymn_type_exception(H, HYMN_VALUE_INTEGER, number.is);
        }
        HymnInt start = hymn_as_int(number);
        if (start < 0) {
            start = 0;
        } else if (start >= (HymnInt)hymn_string_len(text)) {
            return hymn_new_none();
        }
        text += start;
    }
    HymnString *pattern = hymn_as_string(expression);
    char *begin = NULL;
    char *end = NULL;
    Match group = {0};
    if (pattern[0] == '^') {
        end = match(&group, pattern + 1, text);
        if (end != NULL) {
            begin = text;
        }
    } else {
        while (true) {
            end = match(&group, pattern, text);
            if (end != NULL) {
                begin = text;
                break;
            } else if (group.error != NULL || text[0] == '\0') {
                break;
            }
            text++;
        }
    }
    if (group.error != NULL) {
        return hymn_new_exception(H, group.error);
    } else if (begin == NULL) {
        return hymn_new_none();
    }
    HymnArray *array = hymn_new_array(0);
    hymn_array_push(array, hymn_new_int((HymnInt)(begin - original)));
    hymn_array_push(array, hymn_new_int((HymnInt)(end - original)));
    for (int i = 0; i < group.count; i++) {
        HymnInt start = (HymnInt)(group.capture[i].begin - original);
        hymn_array_push(array, hymn_new_int(start));
        hymn_array_push(array, hymn_new_int(start + (HymnInt)group.capture[i].size));
    }
    return hymn_new_array_value(array);
}

static HymnValue is_match(Hymn *H, int count, HymnValue *arguments) {
    if (count < 2) {
        return hymn_arity_exception(H, 2, count);
    }
    HymnValue source = arguments[0];
    if (!hymn_is_string(source)) {
        return hymn_type_exception(H, HYMN_VALUE_STRING, source.is);
    }
    HymnValue expression = arguments[1];
    if (!hymn_is_string(expression)) {
        return hymn_type_exception(H, HYMN_VALUE_STRING, expression.is);
    }
    HymnString *original = hymn_as_string(source);
    if (count >= 3) {
        HymnValue number = arguments[2];
        if (!hymn_is_int(number)) {
            return hymn_type_exception(H, HYMN_VALUE_INTEGER, number.is);
        }
        HymnInt start = hymn_as_int(number);
        if (start < 0) {
            start = 0;
        } else if (start >= (HymnInt)hymn_string_len(original)) {
            return hymn_new_bool(false);
        }
        original += start;
    }
    HymnString *text = original;
    HymnString *pattern = hymn_as_string(expression);
    char *begin = NULL;
    char *end = NULL;
    Match group = {0};
    if (pattern[0] == '^') {
        end = match(&group, pattern + 1, text);
        if (end != NULL) {
            begin = text;
        }
    } else {
        while (true) {
            end = match(&group, pattern, text);
            if (end != NULL) {
                begin = text;
                break;
            } else if (group.error != NULL || text[0] == '\0') {
                break;
            }
            text++;
        }
    }
    if (group.error != NULL) {
        return hymn_new_exception(H, group.error);
    }
    return hymn_new_bool(begin != NULL && begin == original && end[0] == '\0');
}

static HymnString *replacer(HymnString *updated, char *begin, char *end, Match *group, HymnString *replacement) {
    if (updated == NULL) {
        updated = hymn_new_string_with_capacity(hymn_string_len(replacement));
    }
    HymnString *text = replacement;
    while (text[0] != '\0') {
        if (text[0] == '%') {
            char c = text[1];
            if (c == '%') {
                updated = hymn_string_append_char(updated, '%');
                text += 2;
                continue;
            } else if (c >= '0' && c <= '9') {
                int i = c - '0';
                if (i == 0) {
                    updated = hymn_string_append_substring(updated, begin, 0, (size_t)(end - begin));
                    text += 2;
                    continue;
                } else {
                    i--;
                    if (i < group->count) {
                        updated = hymn_string_append_substring(updated, group->capture[i].begin, 0, group->capture[i].size);
                        text += 2;
                        continue;
                    }
                }
            }
        }
        updated = hymn_string_append_char(updated, text[0]);
        text++;
    }
    return updated;
}

static HymnValue replace(Hymn *H, int count, HymnValue *arguments) {
    if (count < 3) {
        return hymn_arity_exception(H, 3, count);
    }
    HymnValue source = arguments[0];
    if (!hymn_is_string(source)) {
        return hymn_type_exception(H, HYMN_VALUE_STRING, source.is);
    }
    HymnValue expression = arguments[1];
    if (!hymn_is_string(expression)) {
        return hymn_type_exception(H, HYMN_VALUE_STRING, expression.is);
    }
    HymnValue substitute = arguments[2];
    if (!hymn_is_string(substitute)) {
        return hymn_type_exception(H, HYMN_VALUE_STRING, substitute.is);
    }
    HymnString *original = hymn_as_string(source);
    HymnString *text = original;
    HymnString *pattern = hymn_as_string(expression);
    HymnString *replacement = hymn_as_string(substitute);
    HymnString *updated = NULL;
    char *last = NULL;
    Match group = {0};
    if (pattern[0] == '^') {
        char *end = match(&group, pattern + 1, text);
        if (group.error == NULL && end != NULL) {
            updated = hymn_string_copy(replacement);
            last = end;
        }
    } else {
        while (true) {
            char *end = match(&group, pattern, text);
            if (group.error != NULL) {
                break;
            } else if (end != NULL) {
                if (updated == NULL) {
                    if (text == original) {
                        updated = replacer(updated, text, end, &group, replacement);
                    } else {
                        updated = hymn_string_format("%.*s", text - original, original);
                        updated = replacer(updated, text, end, &group, replacement);
                    }
                } else {
                    if (last != text) {
                        HymnString *behind = hymn_string_format("%.*s", text - last, last);
                        updated = hymn_string_append(updated, behind);
                        hymn_string_delete(behind);
                    }
                    updated = replacer(updated, text, end, &group, replacement);
                }
                last = end;
                if (end[0] == '\0') {
                    break;
                }
                text = end;
            } else {
                text++;
                if (text[0] == '\0') {
                    break;
                }
            }
        }
    }
    if (group.error != NULL) {
        if (updated != NULL) {
            hymn_string_delete(updated);
        }
        return hymn_new_exception(H, group.error);
    }
    if (updated == NULL) {
        return source;
    } else if (last != NULL) {
        updated = hymn_string_append(updated, last);
    }
    return hymn_new_string_value(hymn_intern_string(H, updated));
}

void hymn_use_pattern(Hymn *H) {
    HymnTable *pattern = hymn_new_table();
    hymn_add_function_to_table(H, pattern, "get", get);
    hymn_add_function_to_table(H, pattern, "find", find);
    hymn_add_function_to_table(H, pattern, "match", is_match);
    hymn_add_function_to_table(H, pattern, "replace", replace);
    hymn_add_table(H, "pattern", pattern);
}
