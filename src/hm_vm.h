/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef HYMN_VM_H
#define HYMN_VM_H

// #define NDEBUG

#include <assert.h>
#include <errno.h>

#include "hm_util.h"

// #define HYMN_DEBUG_TOKEN
// #define HYMN_DEBUG_CODE
// #define HYMN_DEBUG_TRACE
// #define HYMN_DEBUG_STACK
// #define HYMN_DEBUG_REFERENCE

#define HYMN_INCLUDE_STACKTRACE

#define HYMN_UINT8_COUNT (UINT8_MAX + 1)

#define HYMN_FRAMES_MAX 64
#define HYMN_STACK_MAX (HYMN_FRAMES_MAX * HYMN_UINT8_COUNT)

#define hymn_new_undefined() ((HymnValue){.is = HYMN_VALUE_UNDEFINED, .as = {.i = 0}})
#define hymn_new_none() ((HymnValue){.is = HYMN_VALUE_NONE, .as = {.i = 0}})
#define hymn_new_bool(v) ((HymnValue){.is = HYMN_VALUE_BOOL, .as = {.b = v}})
#define hymn_new_int(v) ((HymnValue){.is = HYMN_VALUE_INTEGER, .as = {.i = v}})
#define hymn_new_float(v) ((HymnValue){.is = HYMN_VALUE_FLOAT, .as = {.f = v}})
#define hymn_new_native(v) ((HymnValue){.is = HYMN_VALUE_FUNC_NATIVE, .as = {.n = v}})
#define hymn_new_pointer(v) ((HymnValue){.is = HYMN_VALUE_POINTER, .as = {.p = v}})
#define hymn_new_string_value(v) ((HymnValue){.is = HYMN_VALUE_STRING, .as = {.o = (HymnObject *)v}})
#define hymn_new_array_value(v) ((HymnValue){.is = HYMN_VALUE_ARRAY, .as = {.o = (HymnObject *)v}})
#define hymn_new_table_value(v) ((HymnValue){.is = HYMN_VALUE_TABLE, .as = {.o = (HymnObject *)v}})
#define hymn_new_func_value(v) ((HymnValue){.is = HYMN_VALUE_FUNC, .as = {.o = (HymnObject *)v}})

#define hymn_as_bool(v) ((v).as.b)
#define hymn_as_int(v) ((v).as.i)
#define hymn_as_float(v) ((v).as.f)
#define hymn_as_native(v) ((v).as.n)
#define hymn_as_pointer(v) ((v).as.p)
#define hymn_as_object(v) ((HymnObject *)(v).as.o)
#define hymn_as_string(v) ((HymnString *)(v).as.o)
#define hymn_as_array(v) ((HymnArray *)(v).as.o)
#define hymn_as_table(v) ((HymnTable *)(v).as.o)
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
    HYMN_VALUE_POINTER,
};

typedef struct HymnValue HymnValue;
typedef struct HymnObject HymnObject;
typedef struct HymnString HymnString;
typedef struct HymnArray HymnArray;
typedef struct HymnTable HymnTable;
typedef struct HymnTableItem HymnTableItem;
typedef struct HymnSet HymnSet;
typedef struct HymnSetItem HymnSetItem;
typedef struct HymnExceptList HymnExceptList;
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
        void *p;
    } as;
};

struct HymnObject {
    int count;
};

struct HymnString {
    HymnObject object;
    String *string;
    usize hash;
};

struct HymnArray {
    HymnObject object;
    HymnValue *items;
    i64 length;
    i64 capacity;
};

struct HymnTableItem {
    HymnString *key;
    HymnValue value;
    HymnTableItem *next;
};

struct HymnTable {
    HymnObject object;
    unsigned int size;
    unsigned int bins;
    HymnTableItem **items;
};

struct HymnSetItem {
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

struct HymnExceptList {
    usize start;
    usize end;
    usize stack;
    struct HymnExceptList *next;
};

struct HymnFunction {
    HymnObject object;
    String *name;
    String *script;
    int arity;
    HymnByteCode code;
    HymnExceptList *except;
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
    HymnSet strings;
    HymnTable globals;
    HymnArray *paths;
    HymnTable *imports;
    String *error;
    void (*print)(const char *format, ...);
};

HymnString *new_hymn_string(String *string);

Hymn *new_hymn();

char *hymn_do_script(Hymn *this, const char *script, const char *source);
char *hymn_do(Hymn *this, const char *source);
char *hymn_read(Hymn *this, const char *script);

void hymn_add_function(Hymn *this, const char *name, HymnNativeCall func);
void hymn_add_pointer(Hymn *this, const char *name, void *pointer);

void hymn_delete(Hymn *this);

#endif
