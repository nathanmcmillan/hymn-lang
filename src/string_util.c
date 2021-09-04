/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "string_util.h"

static StringHead *string_head_init(usize length, usize capacity) {
    usize memory = sizeof(StringHead) + length + 1;
    StringHead *head = (StringHead *)safe_malloc(memory);
    memset(head, 0, memory);
    head->length = length;
    head->capacity = capacity;
    return head;
}

String *new_string_with_length(const char *init, usize length) {
    StringHead *head = string_head_init(length, length);
    char *s = (char *)(head + 1);
    memcpy(s, init, length);
    s[length] = '\0';
    return (String *)s;
}

String *new_string_from_substring(const char *init, usize start, usize end) {
    usize length = end - start;
    StringHead *head = string_head_init(length, length);
    char *s = (char *)(head + 1);
    memcpy(s, &init[start], length);
    s[length] = '\0';
    return (String *)s;
}

String *string_allocate(usize length) {
    StringHead *head = string_head_init(length, length);
    char *s = (char *)(head + 1);
    memset(s, '\0', length + 1);
    return (String *)s;
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

usize string_cap(String *this) {
    StringHead *head = (StringHead *)((char *)this - sizeof(StringHead));
    return head->capacity;
}

void string_delete(String *this) {
    if (this == NULL) {
        return;
    }
    free((char *)this - sizeof(StringHead));
}

String *string_concat(String *a, String *b) {
    usize len1 = string_len(a);
    usize len2 = string_len(b);
    usize len = len1 + len2;
    StringHead *head = string_head_init(len, len);
    char *s = (char *)(head + 1);
    memcpy(s, a, len1);
    memcpy(s + len1, b, len2 + 1);
    s[len] = '\0';
    return (String *)s;
}

String *string_concat_list(String **list, int size) {
    usize len = 0;
    for (int i = 0; i < size; i++) {
        len += string_len(list[i]);
    }
    StringHead *head = string_head_init(len, len);
    char *s = (char *)(head + 1);
    usize pos = 0;
    for (int i = 0; i < size; i++) {
        usize len_i = string_len(list[i]);
        memcpy(s + pos, list[i], len_i);
        pos += len_i;
    }
    s[len] = '\0';
    return (String *)s;
}

String *string_concat_varg(int size, ...) {
    va_list args;

    usize len = 0;
    va_start(args, size);
    for (int i = 0; i < size; i++) {
        len += string_len(va_arg(args, String *));
    }
    va_end(args);

    StringHead *head = string_head_init(len, len);
    char *s = (char *)(head + 1);

    usize pos = 0;
    va_start(args, size);
    for (int i = 0; i < size; i++) {
        String *param = va_arg(args, String *);
        usize len_i = string_len(param);
        memcpy(s + pos, param, len_i);
        pos += len_i;
    }
    va_end(args);

    s[len] = '\0';
    return (String *)s;
}

String *string_concat_const(String *a, const char *b) {
    usize len1 = string_len(a);
    usize len2 = strlen(b);
    usize len = len1 + len2;
    StringHead *head = string_head_init(len, len);
    char *s = (char *)(head + 1);
    memcpy(s, a, len1);
    memcpy(s + len1, b, len2);
    s[len] = '\0';
    return (String *)s;
}

String *substring(String *this, usize start, usize end) {
    usize len = end - start;
    StringHead *head = string_head_init(len, len);
    char *s = (char *)(head + 1);
    memcpy(s, this + start, len);
    s[len] = '\0';
    return (String *)s;
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
    usize len_a = string_len(a);
    usize len_b = string_len(b);
    if (len_a == len_b) {
        return strcmp(a, b);
    }
    return (len_a > len_b) ? (int)(len_a - len_b) : -(int)(len_b - len_a);
}

bool string_equal(String *a, String *b) {
    return 0 == string_compare(a, b);
}

bool string_starts_with(String *s, String *p) {
    usize slen = string_len(s);
    usize plen = string_len(p);
    return slen < plen ? false : memcmp(s, p, plen) == 0;
}

bool string_ends_with(String *s, String *p) {
    usize slen = string_len(s);
    usize plen = string_len(p);
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

String *int_to_string(int number) {
    int len = snprintf(NULL, 0, "%d", number);
    char *str = safe_malloc(len + 1);
    snprintf(str, len + 1, "%d", number);
    String *s = new_string_with_length(str, len);
    free(str);
    return s;
}

String *int8_to_string(i8 number) {
    int len = snprintf(NULL, 0, "%" PRId8, number);
    char *str = safe_malloc(len + 1);
    snprintf(str, len + 1, "%" PRId8, number);
    String *s = new_string_with_length(str, len);
    free(str);
    return s;
}

String *int16_to_string(i16 number) {
    int len = snprintf(NULL, 0, "%" PRId16, number);
    char *str = safe_malloc(len + 1);
    snprintf(str, len + 1, "%" PRId16, number);
    String *s = new_string_with_length(str, len);
    free(str);
    return s;
}

String *int32_to_string(i32 number) {
    int len = snprintf(NULL, 0, "%" PRId32, number);
    char *str = safe_malloc(len + 1);
    snprintf(str, len + 1, "%" PRId32, number);
    String *s = new_string_with_length(str, len);
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

String *usize_to_string(usize number) {
    int len = snprintf(NULL, 0, "%zu", number);
    char *str = safe_malloc(len + 1);
    snprintf(str, len + 1, "%zu", number);
    String *s = new_string_with_length(str, len);
    free(str);
    return s;
}

String *uint_to_string(unsigned int number) {
    int len = snprintf(NULL, 0, "%u", number);
    char *str = safe_malloc(len + 1);
    snprintf(str, len + 1, "%u", number);
    String *s = new_string_with_length(str, len);
    free(str);
    return s;
}

String *uint8_to_string(u8 number) {
    int len = snprintf(NULL, 0, "%" PRId8, number);
    char *str = safe_malloc(len + 1);
    snprintf(str, len + 1, "%" PRId8, number);
    String *s = new_string_with_length(str, len);
    free(str);
    return s;
}

String *uint16_to_string(u16 number) {
    int len = snprintf(NULL, 0, "%" PRId16, number);
    char *str = safe_malloc(len + 1);
    snprintf(str, len + 1, "%" PRId16, number);
    String *s = new_string_with_length(str, len);
    free(str);
    return s;
}

String *uint32_to_string(u32 number) {
    int len = snprintf(NULL, 0, "%" PRId32, number);
    char *str = safe_malloc(len + 1);
    snprintf(str, len + 1, "%" PRId32, number);
    String *s = new_string_with_length(str, len);
    free(str);
    return s;
}

String *uint64_to_string(u64 number) {
    int len = snprintf(NULL, 0, "%" PRId64, number);
    char *str = safe_malloc(len + 1);
    snprintf(str, len + 1, "%" PRId64, number);
    String *s = new_string_with_length(str, len);
    free(str);
    return s;
}

String *float_to_string(float number) {
    int len = snprintf(NULL, 0, "%g", number);
    char *str = safe_malloc(len + 1);
    snprintf(str, len + 1, "%g", number);
    String *s = new_string_with_length(str, len);
    free(str);
    return s;
}

String *float32_to_string(float number) {
    return float_to_string(number);
}

String *float64_to_string(double number) {
    int len = snprintf(NULL, 0, "%g", number);
    char *str = safe_malloc(len + 1);
    snprintf(str, len + 1, "%g", number);
    String *s = new_string_with_length(str, len);
    free(str);
    return s;
}

String *pointer_to_string(void *pointer) {
    int len = snprintf(NULL, 0, "%p", pointer);
    char *str = safe_malloc(len + 1);
    snprintf(str, len + 1, "%p", pointer);
    String *s = new_string_with_length(str, len);
    free(str);
    return s;
}

bool string_to_bool(String *this) {
    return strcmp(this, "true") == 0;
}

int string_to_int(String *this) {
    return (int)strtol(this, NULL, 10);
}

i8 string_to_int8(String *this) {
    return (i8)strtol(this, NULL, 10);
}

i16 string_to_int16(String *this) {
    return (i16)strtol(this, NULL, 10);
}

i32 string_to_int32(String *this) {
    return (i32)strtol(this, NULL, 10);
}

i64 string_to_int64(String *this) {
    return (i64)strtoll(this, NULL, 10);
}

usize string_to_usize(String *this) {
    return (usize)strtoll(this, NULL, 10);
}

unsigned int string_to_uint(String *this) {
    return (unsigned int)strtoul(this, NULL, 10);
}

u8 string_to_uint8(String *this) {
    return (u8)strtoul(this, NULL, 10);
}

u16 string_to_uint16(String *this) {
    return (u16)strtoul(this, NULL, 10);
}

u32 string_to_uint32(String *this) {
    return (u32)strtoul(this, NULL, 10);
}

u64 string_to_uint64(String *this) {
    return (u64)strtoull(this, NULL, 10);
}

float string_to_float(String *this) {
    return strtof(this, NULL);
}

float string_to_float32(String *this) {
    return string_to_float(this);
}

double string_to_float64(String *this) {
    return strtod(this, NULL);
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

struct FilterList string_filter(String **input, int count, bool (*filter)(String *a, String *b), String *with) {
    int size = 0;
    String **filtered = safe_calloc(count, sizeof(String *));
    for (int i = 0; i < count; i++) {
        if (filter(input[i], with)) {
            filtered[size++] = string_copy(input[i]);
        }
    }
    return (struct FilterList){.count = size, .filtered = filtered};
}

struct FilterList string_filter_ends_with(String **input, int count, String *with) {
    return string_filter(input, count, string_ends_with, with);
}

void delete_filter_list(struct FilterList *list) {
    for (int i = 0; i < list->count; i++) {
        string_delete(list->filtered[i]);
    }
    free(list->filtered);
}
