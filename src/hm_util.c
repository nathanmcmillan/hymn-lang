/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "hm_util.h"

void *safe_malloc(usize size) {
    void *mem = malloc(size);
    if (mem) {
        return mem;
    }
    fprintf(stderr, "malloc failed.\n");
    exit(1);
}

void *safe_calloc(usize members, usize member_size) {
    void *mem = calloc(members, member_size);
    if (mem) {
        return mem;
    }
    fprintf(stderr, "calloc failed.\n");
    exit(1);
}

void *safe_realloc(void *mem, usize size) {
    mem = realloc(mem, size);
    if (mem) {
        return mem;
    }
    fprintf(stderr, "realloc failed.\n");
    exit(1);
}

static StringHead *string_head_init(usize length, usize capacity) {
    usize memory = sizeof(StringHead) + capacity + 1;
    StringHead *head = (StringHead *)safe_malloc(memory);
    memset(head, 0, memory);
    head->length = length;
    head->capacity = capacity;
    return head;
}

String *new_string_with_capacity(usize capacity) {
    StringHead *head = string_head_init(0, capacity);
    return (String *)(head + 1);
}

String *new_string_with_length(const char *init, usize length) {
    StringHead *head = string_head_init(length, length);
    char *string = (char *)(head + 1);
    memcpy(string, init, length);
    string[length] = '\0';
    return (String *)string;
}

String *new_string_from_substring(const char *init, usize start, usize end) {
    usize length = end - start;
    StringHead *head = string_head_init(length, length);
    char *string = (char *)(head + 1);
    memcpy(string, &init[start], length);
    string[length] = '\0';
    return (String *)string;
}

String *new_string(const char *init) {
    usize len = strlen(init);
    return new_string_with_length(init, len);
}

String *string_copy(String *this) {
    StringHead *head = (StringHead *)((char *)this - sizeof(StringHead));
    return new_string_with_length(this, head->length);
}

usize string_len(String *this) {
    StringHead *head = (StringHead *)((char *)this - sizeof(StringHead));
    return head->length;
}

void string_delete(String *this) {
    if (this == NULL) {
        return;
    }
    free((char *)this - sizeof(StringHead));
}

String *substring(String *this, usize start, usize end) {
    usize len = end - start;
    StringHead *head = string_head_init(len, len);
    char *s = (char *)(head + 1);
    memcpy(s, this + start, len);
    s[len] = '\0';
    return (String *)s;
}

String *string_trim(String *this) {
    usize len = string_len(this);
    usize start = 0;
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
        usize end = len - 1;
        while (end > start) {
            char c = this[end];
            if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
                break;
            }
            end--;
        }
        end++;
        usize offset = start;
        usize size = end - start;
        for (usize i = 0; i < size; i++) {
            this[i] = this[offset++];
        }
        StringHead *head = (StringHead *)((char *)this - sizeof(StringHead));
        head->length = size;
        this[end] = '\0';
    }
    return this;
}

static StringHead *string_resize(StringHead *head, usize capacity) {
    usize memory = sizeof(StringHead) + capacity + 1;
    StringHead *new = safe_realloc(head, memory);
    new->capacity = capacity;
    return new;
}

String *string_append(String *this, const char *b) {
    StringHead *head = (StringHead *)((char *)this - sizeof(StringHead));
    usize len_a = head->length;
    usize len_b = strlen(b);
    usize len = len_a + len_b;
    if (len > head->capacity) {
        head = string_resize(head, len * 2);
    }
    head->length = len;
    char *s = (char *)(head + 1);
    memcpy(s + len_a, b, len_b + 1);
    s[len] = '\0';
    return (String *)s;
}

String *string_append_char(String *this, const char b) {
    StringHead *head = (StringHead *)((char *)this - sizeof(StringHead));
    usize len = head->length + 1;
    if (len > head->capacity) {
        head = string_resize(head, len * 2);
    }
    head->length = len;
    char *s = (char *)(head + 1);
    s[len - 1] = b;
    s[len] = '\0';
    return (String *)s;
}

String *string_append_substring(String *this, const char *b, usize start, usize end) {
    StringHead *head = (StringHead *)((char *)this - sizeof(StringHead));
    usize len_a = head->length;
    usize len_b = end - start;
    usize len = len_a + len_b;
    if (len > head->capacity) {
        head = string_resize(head, len * 2);
    }
    head->length = len;
    char *s = (char *)(head + 1);
    memcpy(s + len_a, &b[start], len_b);
    s[len] = '\0';
    return (String *)s;
}

int string_compare(String *a, String *b) {
    return strcmp(a, b);
}

bool string_equal(String *a, String *b) {
    return 0 == string_compare(a, b);
}

bool string_starts_with(String *s, const char *p) {
    usize slen = string_len(s);
    usize plen = strlen(p);
    return slen < plen ? false : memcmp(s, p, plen) == 0;
}

bool string_ends_with(String *s, const char *p) {
    usize slen = string_len(s);
    usize plen = strlen(p);
    return slen < plen ? false : memcmp(&s[slen - plen], p, plen) == 0;
}

bool string_contains(String *s, const char *p) {
    usize slen = string_len(s);
    usize plen = strlen(p);
    if (plen > slen) {
        return false;
    }
    usize diff = slen - plen;
    for (usize i = 0; i <= diff; i++) {
        if (memcmp(&s[i], p, plen) == 0) {
            return true;
        }
    }
    return false;
}

bool string_find(String *this, String *sub, usize *out) {
    StringHead *head = (StringHead *)((char *)this - sizeof(StringHead));
    StringHead *head_sub = (StringHead *)((char *)sub - sizeof(StringHead));
    usize len = head->length;
    usize len_sub = head_sub->length;
    if (len_sub > len) {
        return false;
    } else if (len == 0) {
        *out = 0;
        return true;
    }
    usize end = len - len_sub + 1;
    for (usize i = 0; i < end; i++) {
        bool match = true;
        for (usize k = 0; k < len_sub; k++) {
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

String *string_replace(String *this, const char *find, const char *replace) {
    StringHead *head = (StringHead *)((char *)this - sizeof(StringHead));
    usize len = head->length;
    usize len_sub = strlen(find);
    if (len_sub > len) {
        return new_string("");
    } else if (len == 0) {
        return new_string("");
    }
    String *out = new_string("");
    usize end = len - len_sub + 1;
    usize p = 0;
    for (usize i = 0; i < end; i++) {
        bool match = true;
        for (usize k = 0; k < len_sub; k++) {
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

void string_zero(String *this) {
    StringHead *head = (StringHead *)((char *)this - sizeof(StringHead));
    head->length = 0;
    this[0] = '\0';
}

String *char_to_string(char ch) {
    char *str = safe_malloc(2);
    str[0] = ch;
    str[1] = '\0';
    String *s = new_string_with_length(str, 1);
    free(str);
    return s;
}

String *int64_to_string(i64 number) {
    int len = snprintf(NULL, 0, "%" PRId64, number);
    char *str = safe_malloc(len + 1);
    snprintf(str, len + 1, "%" PRId64, number);
    String *s = new_string_with_length(str, len);
    free(str);
    return s;
}

String *double_to_string(double number) {
    int len = snprintf(NULL, 0, "%g", number);
    char *str = safe_malloc(len + 1);
    snprintf(str, len + 1, "%g", number);
    String *s = new_string_with_length(str, len);
    free(str);
    return s;
}

i64 string_to_int64(String *this) {
    return (i64)strtoll(this, NULL, 10);
}

double string_to_double(String *this, char **end) {
    return strtod(this, end);
}

char *string_to_chars(String *this) {
    usize len = string_len(this);
    char *s = safe_malloc((len + 1) * sizeof(char));
    memcpy(s, this, len);
    s[len] = '\0';
    return s;
}

String *string_format(const char *format, ...) {
    va_list args;

    va_start(args, format);
    int len = vsnprintf(NULL, 0, format, args);
    va_end(args);
    char *chars = safe_malloc((len + 1) * sizeof(char));
    va_start(args, format);
    len = vsnprintf(chars, len + 1, format, args);
    va_end(args);
    String *str = new_string_with_length(chars, len);
    free(chars);
    return str;
}

String *string_append_format(String *this, const char *format, ...) {
    va_list args;

    va_start(args, format);
    int len = vsnprintf(NULL, 0, format, args);
    va_end(args);
    char *chars = safe_malloc((len + 1) * sizeof(char));
    va_start(args, format);
    len = vsnprintf(chars, len + 1, format, args);
    va_end(args);
    this = string_append(this, chars);
    free(chars);
    return this;
}

struct FilterList string_filter(String **input, int count, bool (*filter)(String *a, const char *b), const char *with) {
    int size = 0;
    String **filtered = safe_calloc(count, sizeof(String *));
    for (int i = 0; i < count; i++) {
        if (filter(input[i], with)) {
            filtered[size++] = string_copy(input[i]);
        }
    }
    return (struct FilterList){.count = size, .filtered = filtered};
}

struct FilterList string_filter_ends_with(String **input, int count, const char *with) {
    return string_filter(input, count, string_ends_with, with);
}

void delete_filter_list(struct FilterList *list) {
    for (int i = 0; i < list->count; i++) {
        string_delete(list->filtered[i]);
    }
    free(list->filtered);
}
