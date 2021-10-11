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

#include "hymn.h"

void *hymn_malloc(size_t size);
void *hymn_calloc(size_t members, size_t member_size);
void *hymn_realloc(void *mem, size_t size);

HymnString *new_string_with_capacity(size_t capacity);
HymnString *new_string_with_length(const char *init, size_t length);
HymnString *new_string_from_substring(const char *init, size_t start, size_t end);
HymnString *new_string(const char *init);
HymnString *string_copy(HymnString *this);

size_t string_len(HymnString *this);
void string_delete(HymnString *this);

HymnString *substring(HymnString *this, size_t start, size_t end);
HymnString *string_trim(HymnString *this);

HymnString *string_append(HymnString *this, const char *b);
HymnString *string_append_char(HymnString *this, const char b);
HymnString *string_append_substring(HymnString *this, const char *b, size_t start, size_t end);

int string_compare(HymnString *a, HymnString *b);
bool string_equal(HymnString *a, HymnString *b);
bool string_starts_with(HymnString *s, const char *p);
bool string_ends_with(HymnString *s, const char *p);
bool string_contains(HymnString *s, const char *p);

bool string_find(HymnString *this, HymnString *sub, size_t *out);

HymnString *string_replace(HymnString *this, const char *find, const char *replace);

void string_zero(HymnString *this);

HymnString *char_to_string(char ch);
HymnString *int64_to_string(int64_t number);

HymnString *double_to_string(double number);

int64_t string_to_int64(HymnString *this);

double string_to_double(HymnString *this, char **end);
char *string_to_chars(HymnString *this);

HymnString *string_format(const char *format, ...);
HymnString *string_append_format(HymnString *this, const char *format, ...);

struct FilterList {
    int count;
    HymnString **filtered;
};

struct FilterList string_filter(HymnString **input, int count, bool (*filter)(HymnString *a, const char *b), const char *with);
struct FilterList string_filter_ends_with(HymnString **input, int count, const char *with);
void delete_filter_list(struct FilterList *list);

#endif
