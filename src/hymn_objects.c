/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "hymn.h"

void hymn_string_delete(HymnString *string) {
    if (string == NULL) {
        return;
    }
    free((char *)string - sizeof(HymnStringHead));
}

static HymnStringHead *string_head_init(size_t length, size_t capacity) {
    size_t memory = sizeof(HymnStringHead) + capacity + 1;
    HymnStringHead *head = (HymnStringHead *)hymn_malloc(memory);
    memset(head, 0, memory);
    head->length = length;
    head->capacity = capacity;
    return head;
}

HymnString *hymn_new_string_with_capacity(size_t capacity) {
    HymnStringHead *head = string_head_init(0, capacity);
    return (HymnString *)(head + 1);
}

HymnString *hymn_new_string_with_length(const char *chars, size_t length) {
    HymnStringHead *head = string_head_init(length, length);
    char *string = (char *)(head + 1);
    memcpy(string, chars, length);
    return (HymnString *)string;
}

HymnString *hymn_new_empty_string(size_t length) {
    HymnStringHead *head = string_head_init(length, length);
    char *string = (char *)(head + 1);
    return (HymnString *)string;
}

HymnString *hymn_new_string(const char *chars) {
    size_t length = strlen(chars);
    return hymn_new_string_with_length(chars, length);
}

HymnString *hymn_substring(const char *chars, size_t start, size_t end) {
    size_t length = end - start;
    HymnStringHead *head = string_head_init(length, length);
    char *string = (char *)(head + 1);
    memcpy(string, &chars[start], length);
    string[length] = '\0';
    return (HymnString *)string;
}

HymnStringHead *hymn_string_head(HymnString *string) {
    return HYMN_STRING_HEAD(string);
}

HymnString *hymn_string_copy(HymnString *string) {
    HymnStringHead *head = HYMN_STRING_HEAD(string);
    return hymn_new_string_with_length(string, head->length);
}

size_t hymn_string_len(HymnString *string) {
    HymnStringHead *head = HYMN_STRING_HEAD(string);
    return head->length;
}

void hymn_string_zero(HymnString *string) {
    HymnStringHead *head = HYMN_STRING_HEAD(string);
    head->length = 0;
    string[0] = '\0';
}

static HymnStringHead *string_resize(HymnStringHead *head, size_t capacity) {
    size_t memory = sizeof(HymnStringHead) + capacity + 1;
    HymnStringHead *new = hymn_realloc(head, memory);
    new->capacity = capacity;
    return new;
}

HymnString *hymn_string_append(HymnString *string, const char *b) {
    HymnStringHead *head = HYMN_STRING_HEAD(string);
    size_t len_a = head->length;
    size_t len_b = strlen(b);
    size_t len = len_a + len_b;
    if (len > head->capacity) {
        head = string_resize(head, len * 2);
    }
    head->length = len;
    char *s = (char *)(head + 1);
    memcpy(s + len_a, b, len_b + 1);
    s[len] = '\0';
    return (HymnString *)s;
}

HymnString *hymn_string_append_char(HymnString *string, const char b) {
    HymnStringHead *head = HYMN_STRING_HEAD(string);
    size_t len = head->length + 1;
    if (len > head->capacity) {
        head = string_resize(head, len * 2);
    }
    head->length = len;
    char *s = (char *)(head + 1);
    s[len - 1] = b;
    s[len] = '\0';
    return (HymnString *)s;
}

bool hymn_string_equal(HymnString *a, HymnString *b) {
    return 0 == strcmp(a, b);
}

bool hymn_string_starts_with(HymnString *s, const char *using) {
    size_t slen = hymn_string_len(s);
    size_t plen = strlen(using);
    return slen < plen ? false : memcmp(s, using, plen) == 0;
}

static HymnString *string_append_substring(HymnString *string, const char *b, size_t start, size_t end) {
    HymnStringHead *head = HYMN_STRING_HEAD(string);
    size_t len_a = head->length;
    size_t len_b = end - start;
    size_t len = len_a + len_b;
    if (len > head->capacity) {
        head = string_resize(head, len * 2);
    }
    head->length = len;
    char *s = (char *)(head + 1);
    memcpy(s + len_a, &b[start], len_b);
    s[len] = '\0';
    return (HymnString *)s;
}

HymnString *hymn_string_replace(HymnString *string, const char *find, const char *replace) {
    HymnStringHead *head = HYMN_STRING_HEAD(string);
    size_t len = head->length;
    size_t len_sub = strlen(find);
    if (len_sub > len) {
        return hymn_new_string("");
    } else if (len == 0) {
        return hymn_new_string("");
    }
    HymnString *out = hymn_new_string_with_capacity(len);
    size_t end = len - len_sub + 1;
    size_t p = 0;
    for (size_t i = 0; i < end; i++) {
        bool match = true;
        for (size_t k = 0; k < len_sub; k++) {
            if (find[k] != string[i + k]) {
                match = false;
                break;
            }
        }
        if (match) {
            out = string_append_substring(out, string, p, i);
            out = hymn_string_append(out, replace);
            i += len_sub;
            p = i;
        }
    }
    if (p < len) {
        out = string_append_substring(out, string, p, len);
    }
    return out;
}

HymnString *hymn_string_format(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int len = vsnprintf(NULL, 0, format, args);
    va_end(args);
    char *chars = hymn_malloc((len + 1) * sizeof(char));
    va_start(args, format);
    len = vsnprintf(chars, len + 1, format, args);
    va_end(args);
    HymnString *str = hymn_new_string_with_length(chars, len);
    free(chars);
    return str;
}
