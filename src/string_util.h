/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef STRING_UTIL_H
#define STRING_UTIL_H

#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "mem.h"
#include "pie.h"

typedef char String;

typedef struct StringHead StringHead;

PACK(struct StringHead {
    usize length;
    usize capacity;
    char **chars;
});

String *new_string_with_length(char *init, usize length);
String *new_string_from_substring(char *init, usize start, usize end);
String *new_string(char *init);
String *string_allocate(usize length);
String *string_copy(String *this);

usize string_len(String *this);
usize string_cap(String *this);
void string_delete(String *this);

String *string_concat(String *a, String *b);
String *string_concat_list(String **list, int size);
String *string_concat_varg(int size, ...);

String *substring(String *this, usize start, usize end);

String *string_append(String *this, char *b);
String *string_append_char(String *this, char b);
String *string_append_substring(String *this, char *b, usize start, usize end);

int string_compare(String *a, String *b);
bool string_equal(String *a, String *b);

bool string_find(String *this, String *sub, usize *out);

void string_zero(String *this);

String *char_to_string(char ch);
String *int_to_string(int number);
String *int8_to_string(i8 number);
String *int16_to_string(i16 number);
String *int32_to_string(i32 number);
String *int64_to_string(i64 number);
String *usize_to_string(usize number);

String *uint_to_string(unsigned int number);
String *uint8_to_string(u8 number);
String *uint16_to_string(u16 number);
String *uint32_to_string(u32 number);
String *uint64_to_string(u64 number);

String *float_to_string(float number);
String *float32_to_string(float number);
String *float64_to_string(double number);

String *pointer_to_string(void *pointer);

bool string_to_bool(String *this);

int string_to_int(String *this);
i8 string_to_int8(String *this);
i16 string_to_int16(String *this);
i32 string_to_int32(String *this);
i64 string_to_int64(String *this);
usize string_to_usize(String *this);

unsigned int string_to_uint(String *this);
u8 string_to_uint8(String *this);
u16 string_to_uint16(String *this);
u32 string_to_uint32(String *this);
u64 string_to_uint64(String *this);

float string_to_float(String *this);
float string_to_float32(String *this);
double string_to_float64(String *this);
char *string_to_chars(String *this);

String *string_format(const char *format, ...);
String *string_append_format(String *this, const char *format, ...);

#endif
