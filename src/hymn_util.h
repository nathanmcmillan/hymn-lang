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

HymnChar *new_string_with_capacity(size_t capacity);
HymnChar *new_string_with_length(const char *init, size_t length);
HymnChar *new_string_from_substring(const char *init, size_t start, size_t end);
HymnChar *new_string(const char *init);
HymnChar *string_copy(HymnChar *this);

size_t string_len(HymnChar *this);
void string_delete(HymnChar *this);

HymnChar *substring(HymnChar *this, size_t start, size_t end);
HymnChar *string_trim(HymnChar *this);

HymnChar *string_append(HymnChar *this, const char *b);
HymnChar *string_append_char(HymnChar *this, const char b);
HymnChar *string_append_substring(HymnChar *this, const char *b, size_t start, size_t end);

int string_compare(HymnChar *a, HymnChar *b);
bool string_equal(HymnChar *a, HymnChar *b);
bool string_starts_with(HymnChar *s, const char *p);
bool string_ends_with(HymnChar *s, const char *p);
bool string_contains(HymnChar *s, const char *p);

bool string_find(HymnChar *this, HymnChar *sub, size_t *out);

HymnChar *string_replace(HymnChar *this, const char *find, const char *replace);

void string_zero(HymnChar *this);

HymnChar *char_to_string(char ch);
HymnChar *int64_to_string(int64_t number);

HymnChar *double_to_string(double number);

int64_t string_to_int64(HymnChar *this);

double string_to_double(HymnChar *this, char **end);
char *string_to_chars(HymnChar *this);

HymnChar *string_format(const char *format, ...);
HymnChar *string_append_format(HymnChar *this, const char *format, ...);

struct FilterList {
    int count;
    HymnChar **filtered;
};

struct FilterList string_filter(HymnChar **input, int count, bool (*filter)(HymnChar *a, const char *b), const char *with);
struct FilterList string_filter_ends_with(HymnChar **input, int count, const char *with);
void delete_filter_list(struct FilterList *list);

#endif
