/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "hymn_util.h"

void *hymn_malloc(size_t size) {
    void *mem = malloc(size);
    if (mem) {
        return mem;
    }
    fprintf(stderr, "malloc failed.\n");
    exit(1);
}

void *hymn_calloc(size_t members, size_t member_size) {
    void *mem = calloc(members, member_size);
    if (mem) {
        return mem;
    }
    fprintf(stderr, "calloc failed.\n");
    exit(1);
}

void *hymn_realloc(void *mem, size_t size) {
    mem = realloc(mem, size);
    if (mem) {
        return mem;
    }
    fprintf(stderr, "realloc failed.\n");
    exit(1);
}

static HymnStringHead *string_head_init(size_t length, size_t capacity) {
    size_t memory = sizeof(HymnStringHead) + capacity + 1;
    HymnStringHead *head = (HymnStringHead *)hymn_malloc(memory);
    memset(head, 0, memory);
    head->length = length;
    head->capacity = capacity;
    return head;
}

HymnString *new_string_with_capacity(size_t capacity) {
    HymnStringHead *head = string_head_init(0, capacity);
    return (HymnString *)(head + 1);
}

HymnString *new_string_with_length(const char *init, size_t length) {
    HymnStringHead *head = string_head_init(length, length);
    char *string = (char *)(head + 1);
    memcpy(string, init, length);
    string[length] = '\0';
    return (HymnString *)string;
}

HymnString *new_string_from_substring(const char *init, size_t start, size_t end) {
    size_t length = end - start;
    HymnStringHead *head = string_head_init(length, length);
    char *string = (char *)(head + 1);
    memcpy(string, &init[start], length);
    string[length] = '\0';
    return (HymnString *)string;
}

HymnString *new_string(const char *init) {
    size_t len = strlen(init);
    return new_string_with_length(init, len);
}

HymnString *string_copy(HymnString *this) {
    HymnStringHead *head = (HymnStringHead *)((char *)this - sizeof(HymnStringHead));
    return new_string_with_length(this, head->length);
}

size_t string_len(HymnString *this) {
    HymnStringHead *head = (HymnStringHead *)((char *)this - sizeof(HymnStringHead));
    return head->length;
}

void string_delete(HymnString *this) {
    if (this == NULL) {
        return;
    }
    free((char *)this - sizeof(HymnStringHead));
}

HymnString *substring(HymnString *this, size_t start, size_t end) {
    size_t len = end - start;
    HymnStringHead *head = string_head_init(len, len);
    char *s = (char *)(head + 1);
    memcpy(s, this + start, len);
    s[len] = '\0';
    return (HymnString *)s;
}

HymnString *string_trim(HymnString *this) {
    size_t len = string_len(this);
    size_t start = 0;
    while (start < len) {
        char c = this[start];
        if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
            break;
        }
        start++;
    }
    if (start == len) {
        string_zero(this);
    } else {
        size_t end = len - 1;
        while (end > start) {
            char c = this[end];
            if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
                break;
            }
            end--;
        }
        end++;
        size_t offset = start;
        size_t size = end - start;
        for (size_t i = 0; i < size; i++) {
            this[i] = this[offset++];
        }
        HymnStringHead *head = (HymnStringHead *)((char *)this - sizeof(HymnStringHead));
        head->length = size;
        this[end] = '\0';
    }
    return this;
}

static HymnStringHead *string_resize(HymnStringHead *head, size_t capacity) {
    size_t memory = sizeof(HymnStringHead) + capacity + 1;
    HymnStringHead *new = hymn_realloc(head, memory);
    new->capacity = capacity;
    return new;
}

HymnString *string_append(HymnString *this, const char *b) {
    HymnStringHead *head = (HymnStringHead *)((char *)this - sizeof(HymnStringHead));
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

HymnString *string_append_char(HymnString *this, const char b) {
    HymnStringHead *head = (HymnStringHead *)((char *)this - sizeof(HymnStringHead));
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

HymnString *string_append_substring(HymnString *this, const char *b, size_t start, size_t end) {
    HymnStringHead *head = (HymnStringHead *)((char *)this - sizeof(HymnStringHead));
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

int string_compare(HymnString *a, HymnString *b) {
    return strcmp(a, b);
}

bool string_equal(HymnString *a, HymnString *b) {
    return 0 == string_compare(a, b);
}

bool string_starts_with(HymnString *s, const char *p) {
    size_t slen = string_len(s);
    size_t plen = strlen(p);
    return slen < plen ? false : memcmp(s, p, plen) == 0;
}

bool string_ends_with(HymnString *s, const char *p) {
    size_t slen = string_len(s);
    size_t plen = strlen(p);
    return slen < plen ? false : memcmp(&s[slen - plen], p, plen) == 0;
}

bool string_contains(HymnString *s, const char *p) {
    size_t slen = string_len(s);
    size_t plen = strlen(p);
    if (plen > slen) {
        return false;
    }
    size_t diff = slen - plen;
    for (size_t i = 0; i <= diff; i++) {
        if (memcmp(&s[i], p, plen) == 0) {
            return true;
        }
    }
    return false;
}

bool string_find(HymnString *this, HymnString *sub, size_t *out) {
    HymnStringHead *head = (HymnStringHead *)((char *)this - sizeof(HymnStringHead));
    HymnStringHead *head_sub = (HymnStringHead *)((char *)sub - sizeof(HymnStringHead));
    size_t len = head->length;
    size_t len_sub = head_sub->length;
    if (len_sub > len) {
        return false;
    } else if (len == 0) {
        *out = 0;
        return true;
    }
    size_t end = len - len_sub + 1;
    for (size_t i = 0; i < end; i++) {
        bool match = true;
        for (size_t k = 0; k < len_sub; k++) {
            if (sub[k] != this[i + k]) {
                match = false;
                break;
            }
        }
        if (match) {
            *out = i;
            return true;
        }
    }
    return false;
}

HymnString *string_replace(HymnString *this, const char *find, const char *replace) {
    HymnStringHead *head = (HymnStringHead *)((char *)this - sizeof(HymnStringHead));
    size_t len = head->length;
    size_t len_sub = strlen(find);
    if (len_sub > len) {
        return new_string("");
    } else if (len == 0) {
        return new_string("");
    }
    HymnString *out = new_string("");
    size_t end = len - len_sub + 1;
    size_t p = 0;
    for (size_t i = 0; i < end; i++) {
        bool match = true;
        for (size_t k = 0; k < len_sub; k++) {
            if (find[k] != this[i + k]) {
                match = false;
                break;
            }
        }
        if (match) {
            out = string_append_substring(out, this, p, i);
            out = string_append(out, replace);
            i += len_sub;
            p = i;
        }
    }
    if (p < len) {
        out = string_append_substring(out, this, p, len);
    }
    return out;
}

void string_zero(HymnString *this) {
    HymnStringHead *head = (HymnStringHead *)((char *)this - sizeof(HymnStringHead));
    head->length = 0;
    this[0] = '\0';
}

HymnString *char_to_string(char ch) {
    char *str = hymn_malloc(2);
    str[0] = ch;
    str[1] = '\0';
    HymnString *s = new_string_with_length(str, 1);
    free(str);
    return s;
}

HymnString *int64_to_string(int64_t number) {
    int len = snprintf(NULL, 0, "%" PRId64, number);
    char *str = hymn_malloc(len + 1);
    snprintf(str, len + 1, "%" PRId64, number);
    HymnString *s = new_string_with_length(str, len);
    free(str);
    return s;
}

HymnString *double_to_string(double number) {
    int len = snprintf(NULL, 0, "%g", number);
    char *str = hymn_malloc(len + 1);
    snprintf(str, len + 1, "%g", number);
    HymnString *s = new_string_with_length(str, len);
    free(str);
    return s;
}

int64_t string_to_int64(HymnString *this) {
    return (int64_t)strtoll(this, NULL, 10);
}

double string_to_double(HymnString *this, char **end) {
    return strtod(this, end);
}

char *string_to_chars(HymnString *this) {
    size_t len = string_len(this);
    char *s = hymn_malloc((len + 1) * sizeof(char));
    memcpy(s, this, len);
    s[len] = '\0';
    return s;
}

HymnString *string_format(const char *format, ...) {
    va_list args;

    va_start(args, format);
    int len = vsnprintf(NULL, 0, format, args);
    va_end(args);
    char *chars = hymn_malloc((len + 1) * sizeof(char));
    va_start(args, format);
    len = vsnprintf(chars, len + 1, format, args);
    va_end(args);
    HymnString *str = new_string_with_length(chars, len);
    free(chars);
    return str;
}

HymnString *string_append_format(HymnString *this, const char *format, ...) {
    va_list args;

    va_start(args, format);
    int len = vsnprintf(NULL, 0, format, args);
    va_end(args);
    char *chars = hymn_malloc((len + 1) * sizeof(char));
    va_start(args, format);
    len = vsnprintf(chars, len + 1, format, args);
    va_end(args);
    this = string_append(this, chars);
    free(chars);
    return this;
}

struct FilterList string_filter(HymnString **input, int count, bool (*filter)(HymnString *a, const char *b), const char *with) {
    int size = 0;
    HymnString **filtered = hymn_calloc(count, sizeof(HymnString *));
    for (int i = 0; i < count; i++) {
        if (filter(input[i], with)) {
            filtered[size++] = string_copy(input[i]);
        }
    }
    return (struct FilterList){.count = size, .filtered = filtered};
}

struct FilterList string_filter_ends_with(HymnString **input, int count, const char *with) {
    return string_filter(input, count, string_ends_with, with);
}

void delete_filter_list(struct FilterList *list) {
    for (int i = 0; i < list->count; i++) {
        string_delete(list->filtered[i]);
    }
    free(list->filtered);
}
