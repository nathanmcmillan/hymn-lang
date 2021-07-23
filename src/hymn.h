/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef HYMN_H
#define HYMN_H

// #define NDEBUG

#include <assert.h>

#include "file_io.h"
#include "log.h"
#include "mem.h"
#include "pie.h"
#include "string_util.h"

// #define HYMN_DEBUG_NONE

#ifndef HYMN_DEBUG_NONE
#define HYMN_DEBUG_TRACE
// #define HYMN_DEBUG_STACK
#define HYMN_DEBUG_REFERENCE
// #define HYMN_DEBUG_CODE
// #define HYMN_DEBUG_TOKEN
#endif

#define HYMN_UINT8_COUNT (UINT8_MAX + 1)

#define HYMN_FRAMES_MAX 64
#define HYMN_STACK_MAX (HYMN_FRAMES_MAX * HYMN_UINT8_COUNT)

#define hymn_new_undefined() ((HymnValue){.is = HYMN_VALUE_UNDEFINED, .as = {.i = 0}})
#define hymn_new_none() ((HymnValue){.is = HYMN_VALUE_NONE, .as = {.i = 0}})
#define hymn_new_bool(v) ((HymnValue){.is = HYMN_VALUE_BOOL, .as = {.b = v}})
#define hymn_new_int(v) ((HymnValue){.is = HYMN_VALUE_INTEGER, .as = {.i = v}})
#define hymn_new_float(v) ((HymnValue){.is = HYMN_VALUE_FLOAT, .as = {.f = v}})
#define hymn_new_native(v) ((HymnValue){.is = HYMN_VALUE_FUNC_NATIVE, .as = {.n = v}})
#define hymn_new_string_value(v) ((HymnValue){.is = HYMN_VALUE_STRING, .as = {.o = (HymnObject *)v}})
#define hymn_new_array_value(v) ((HymnValue){.is = HYMN_VALUE_ARRAY, .as = {.o = (HymnObject *)v}})
#define hymn_new_table_value(v) ((HymnValue){.is = HYMN_VALUE_TABLE, .as = {.o = (HymnObject *)v}})
#define hymn_new_func_value(v) ((HymnValue){.is = HYMN_VALUE_FUNC, .as = {.o = (HymnObject *)v}})

#define hymn_as_bool(v) ((v).as.b)
#define hymn_as_int(v) ((v).as.i)
#define hymn_as_float(v) ((v).as.f)
#define hymn_as_native(v) ((v).as.n)
#define hymn_as_object(v) ((HymnObject *)(v).as.o)
#define hymn_as_string(v) ((HymnString *)(v).as.o)
#define hymn_as_array(v) ((HymnArray *)(v).as.o)
#define hymn_as_table(v) ((HymnValueMap *)(v).as.o)
#define hymn_as_func(v) ((HymnFunction *)(v).as.o)

enum HymnValueType {
    HYMN_VALUE_UNDEFINED,
    HYMN_VALUE_NONE,
    HYMN_VALUE_BOOL,
    HYMN_VALUE_INTEGER,
    HYMN_VALUE_FLOAT,
    HYMN_VALUE_STRING,
    HYMN_VALUE_ARRAY,
    HYMN_VALUE_TABLE,
    HYMN_VALUE_FUNC,
    HYMN_VALUE_FUNC_NATIVE,
};

typedef struct HymnValue HymnValue;
typedef struct HymnObject HymnObject;
typedef struct HymnString HymnString;
typedef struct HymnArray HymnArray;
typedef struct HymnValueMap HymnValueMap;
typedef struct HymnValueMapItem HymnValueMapItem;
typedef struct HymnSet HymnSet;
typedef struct HymnSetItem HymnSetItem;
typedef struct HymnFunction HymnFunction;
typedef struct HymnNativeFunction HymnNativeFunction;
typedef struct HymnFrame HymnFrame;
typedef struct HymnValuePool HymnValuePool;
typedef struct HymnByteCode HymnByteCode;
typedef struct Hymn Hymn;

typedef struct HymnValue (*HymnNativeCall)(Hymn *this, int count, HymnValue *arguments);

struct HymnValue {
    enum HymnValueType is;
    union {
        bool b;
        i64 i;
        double f;
        HymnObject *o;
        HymnNativeFunction *n;
    } as;
};

struct HymnObject {
    int count;
};

struct HymnString {
    HymnObject object;
    String *string;
};

struct HymnArray {
    HymnObject object;
    HymnValue *items;
    i64 length;
    i64 capacity;
};

struct HymnValueMapItem {
    usize hash;
    HymnString *key;
    HymnValue value;
    HymnValueMapItem *next;
};

struct HymnValueMap {
    HymnObject object;
    unsigned int size;
    unsigned int bins;
    HymnValueMapItem **items;
};

struct HymnSetItem {
    usize hash;
    HymnString *string;
    HymnSetItem *next;
};

struct HymnSet {
    unsigned int size;
    unsigned int bins;
    HymnSetItem **items;
};

struct HymnNativeFunction {
    String *name;
    HymnNativeCall func;
};

struct HymnValuePool {
    int count;
    int capacity;
    HymnValue *values;
};

struct HymnByteCode {
    int count;
    int capacity;
    u8 *instructions;
    int *rows;
    HymnValuePool constants;
};

struct HymnFunction {
    HymnObject object;
    String *name;
    int arity;
    HymnByteCode code;
};

struct HymnFrame {
    HymnFunction *func;
    usize ip;
    usize stack;
};

struct Hymn {
    HymnValue stack[HYMN_STACK_MAX];
    usize stack_top;
    HymnFrame frames[HYMN_FRAMES_MAX];
    int frame_count;
    HymnValueMap globals;
    HymnSet strings;
    HymnSet imports;
    String *error;
};

HymnString *new_hymn_string(String *string);

Hymn *new_hymn();

char *hymn_eval(Hymn *this, char *content);
char *hymn_read(Hymn *this, char *file);
char *hymn_call(Hymn *this, char *function);
char *hymn_repl(Hymn *this);

void hymn_add_function(Hymn *this, const char *name, HymnNativeCall func);
void hymn_add_pointer(Hymn *this, char *name, void *pointer);

void *hymn_pointer(Hymn *this, i32 index);
i32 hymn_i32(Hymn *this, i32 index);
u32 hymn_u32(Hymn *this, i32 index);
i64 hymn_i64(Hymn *this, i32 index);
u64 hymn_u64(Hymn *this, i32 index);
f32 hymn_f32(Hymn *this, i32 index);
f64 hymn_f64(Hymn *this, i32 index);

void hymn_delete(Hymn *this);

#endif
