/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef HYMN_UTIL_H
#define HYMN_UTIL_H

#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define and &&
#define or ||

typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint64_t u64;

typedef size_t usize;

#ifdef __GNUC__
#define PACK(expr) expr __attribute__((__packed__))
#elif _MSC_VER
#define PACK(expr) __pragma(pack(push, 1)) expr __pragma(pack(pop))
#endif

typedef char String;

typedef struct StringHead StringHead;

PACK(struct StringHead {
    usize length;
    usize capacity;
    char **chars;
});

void *safe_malloc(usize size);
void *safe_calloc(usize members, usize member_size);
void *safe_realloc(void *mem, usize size);

String *new_string_with_capacity(usize capacity);
String *new_string_with_length(const char *init, usize length);
String *new_string_from_substring(const char *init, usize start, usize end);
String *new_string(const char *init);
String *string_copy(String *this);

usize string_len(String *this);
void string_delete(String *this);

String *substring(String *this, usize start, usize end);
String *string_trim(String *this);

String *string_append(String *this, const char *b);
String *string_append_char(String *this, const char b);
String *string_append_substring(String *this, const char *b, usize start, usize end);

int string_compare(String *a, String *b);
bool string_equal(String *a, String *b);
bool string_starts_with(String *s, const char *p);
bool string_ends_with(String *s, const char *p);
bool string_contains(String *s, const char *p);

bool string_find(String *this, String *sub, usize *out);

String *string_replace(String *this, const char *find, const char *replace);

void string_zero(String *this);

String *char_to_string(char ch);
String *int64_to_string(i64 number);

String *double_to_string(double number);

i64 string_to_int64(String *this);

double string_to_double(String *this, char **end);
char *string_to_chars(String *this);

String *string_format(const char *format, ...);
String *string_append_format(String *this, const char *format, ...);

struct FilterList {
    int count;
    String **filtered;
};

struct FilterList string_filter(String **input, int count, bool (*filter)(String *a, const char *b), const char *with);
struct FilterList string_filter_ends_with(String **input, int count, const char *with);
void delete_filter_list(struct FilterList *list);

#endif
