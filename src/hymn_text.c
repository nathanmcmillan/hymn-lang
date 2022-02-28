/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "hymn_text.h"

bool hymn_string_ends_with(HymnString *string, const char *using) {
    size_t slen = hymn_string_len(string);
    size_t plen = strlen(using);
    return slen < plen ? false : memcmp(&string[slen - plen], using, plen) == 0;
}

bool hymn_string_contains(HymnString *string, const char *using) {
    size_t slen = hymn_string_len(string);
    size_t plen = strlen(using);
    if (plen > slen) {
        return false;
    }
    size_t diff = slen - plen;
    for (size_t i = 0; i <= diff; i++) {
        if (memcmp(&string[i], using, plen) == 0) {
            return true;
        }
    }
    return false;
}

static bool space(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

void hymn_string_trim(HymnString *string) {
    size_t len = hymn_string_len(string);
    size_t start = 0;
    while (start < len) {
        char c = string[start];
        if (!space(c)) {
            break;
        }
        start++;
    }
    if (start == len) {
        hymn_string_zero(string);
    } else {
        size_t end = len - 1;
        while (end > start) {
            char c = string[end];
            if (!space(c)) {
                break;
            }
            end--;
        }
        end++;
        size_t offset = start;
        size_t size = end - start;
        for (size_t i = 0; i < size; i++) {
            string[i] = string[offset++];
        }
        HymnStringHead *head = hymn_string_head(string);
        head->length = size;
        string[end] = '\0';
    }
}

static HymnValue text_ends(Hymn *H, int count, HymnValue *arguments) {
    (void)H;
    if (count >= 2) {
        HymnValue value = arguments[0];
        HymnValue starts = arguments[1];
        if (hymn_is_string(value) && hymn_is_string(starts)) {
            bool result = hymn_string_ends_with(hymn_as_string(value), hymn_as_string(starts));
            return hymn_new_bool(result);
        }
    }
    return hymn_new_none();
}

static HymnValue text_starts(Hymn *H, int count, HymnValue *arguments) {
    (void)H;
    if (count >= 2) {
        HymnValue value = arguments[0];
        HymnValue starts = arguments[1];
        if (hymn_is_string(value) && hymn_is_string(starts)) {
            bool result = hymn_string_starts_with(hymn_as_string(value), hymn_as_string(starts));
            return hymn_new_bool(result);
        }
    }
    return hymn_new_none();
}

static HymnValue text_replace(Hymn *H, int count, HymnValue *arguments) {
    if (count >= 3) {
        HymnValue value = arguments[0];
        HymnValue find = arguments[1];
        HymnValue replace = arguments[2];
        if (hymn_is_string(value) && hymn_is_string(find) && hymn_is_string(replace)) {
            HymnString *result = hymn_string_replace(hymn_as_string(value), hymn_as_string(find), hymn_as_string(replace));
            HymnObjectString *string = hymn_intern_string(H, result);
            return hymn_new_string_value(string);
        }
    }
    return hymn_new_none();
}

static HymnValue text_trim(Hymn *H, int count, HymnValue *arguments) {
    if (count >= 1) {
        HymnValue value = arguments[0];
        if (hymn_is_string(value)) {
            HymnString *copy = hymn_string_copy(hymn_as_string(value));
            hymn_string_trim(copy);
            HymnObjectString *trim = hymn_intern_string(H, copy);
            return hymn_new_string_value(trim);
        }
    }
    return hymn_new_none();
}

// join (combine list into string with optional delimiter)
// left_strip (remove spaces on left)
// right_strip (remove spaces on right)

void hymn_use_string(Hymn *H) {
    HymnTable *text = hymn_new_table();
    hymn_add_function_to_table(H, text, "ends", text_ends);
    hymn_add_function_to_table(H, text, "starts", text_starts);
    hymn_add_function_to_table(H, text, "replace", text_replace);
    hymn_add_function_to_table(H, text, "trim", text_trim);
    hymn_add_table(H, "text", text);
}