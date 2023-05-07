/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef HYMN_H
#define HYMN_H

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#define HYMN_VERSION "0.9.0"

// #define HYMN_DEBUG_STACK
// #define HYMN_DEBUG_TRACE
// #define HYMN_DEBUG_MEMORY

// #define HYMN_NO_REPL
#define HYMN_NO_DYNAMIC_LIBS
// #define HYMN_NO_OPTIMIZE
// #define HYMN_NO_MEMORY_MANAGE

#ifdef _MSC_VER
#include <Windows.h>
#include <direct.h>
#define getcwd _getcwd
#define PATH_MAX FILENAME_MAX
#define PATH_SEP '\\'
#define PATH_SEP_STRING "\\"
#define PATH_SEP_OTHER '/'
#define PATH_SEP_OTHER_STRING "/"
#define HYMN_DLIB_EXTENSION ".dll"
#define UNREACHABLE() __assume(0)
#define PACK(expr) __pragma(pack(push, 1)) expr __pragma(pack(pop))
#ifdef HYMN_NO_DYNAMIC_LIBS
#define export
#else
#define export __declspec(dllexport)
#endif
#else
#include <dirent.h>
#ifdef __APPLE__
#include <limits.h>
#else
#include <linux/limits.h>
#endif
#include <unistd.h>
#define PATH_SEP '/'
#define PATH_SEP_STRING "/"
#define PATH_SEP_OTHER '\\'
#define PATH_SEP_OTHER_STRING "\\"
#define HYMN_DLIB_EXTENSION ".so"
#define UNREACHABLE() __builtin_unreachable()
#define PACK(expr) expr __attribute__((__packed__))
#define export
#endif

#define HYMN_UINT8_COUNT (UINT8_MAX + 1)

#define HYMN_FRAMES_MAX 64
#define HYMN_STACK_MAX (HYMN_FRAMES_MAX * HYMN_UINT8_COUNT)

#define hymn_string_head(string) ((HymnStringHead *)((char *)string - sizeof(HymnStringHead)))
#define hymn_string_len(string) (hymn_string_head(string)->length)
#define hymn_string_equal(a, b) (strcmp(a, b) == 0)

typedef long long HymnInt;
typedef double HymnFloat;

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

typedef char HymnString;

typedef struct HymnStringHead HymnStringHead;

PACK(struct HymnStringHead {
    size_t length;
    size_t capacity;
    char **chars;
});

#undef PACK

void *hymn_malloc(size_t size);
void *hymn_calloc(size_t count, size_t size);
void *hymn_realloc(void *mem, size_t size);

void *hymn_malloc_int(int count, size_t size);
void *hymn_calloc_int(int count, size_t size);
void *hymn_realloc_int(void *mem, int count, size_t size);

typedef struct HymnValue HymnValue;
typedef struct HymnObjectString HymnObjectString;
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
    union {
        bool b;
        HymnInt i;
        HymnFloat f;
        void *o;
        void *p;
    } as;
    enum HymnValueType is;
    char padding[4];
};

struct HymnObjectString {
    int count;
    unsigned int hash;
    HymnString *string;
};

struct HymnArray {
    int count;
    char padding[4];
    HymnValue *items;
    HymnInt length;
    HymnInt capacity;
};

struct HymnTableItem {
    HymnObjectString *key;
    HymnValue value;
    HymnTableItem *next;
};

struct HymnTable {
    int count;
    int size;
    unsigned int bins;
    char padding[4];
    HymnTableItem **items;
};

struct HymnSetItem {
    HymnObjectString *string;
    HymnSetItem *next;
};

struct HymnSet {
    int size;
    unsigned int bins;
    HymnSetItem **items;
};

struct HymnNativeFunction {
    int count;
    char padding[4];
    HymnObjectString *name;
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
    uint8_t *instructions;
    int *lines;
    HymnValuePool constants;
};

struct HymnExceptList {
    int start;
    int end;
    int locals;
    char padding[4];
    struct HymnExceptList *next;
};

struct HymnFunction {
    int count;
    int arity;
    HymnString *name;
    HymnString *script;
    HymnString *source;
    HymnExceptList *except;
    HymnByteCode code;
};

struct HymnFrame {
    HymnFunction *func;
    uint8_t *ip;
    HymnValue *stack;
};

#ifndef HYMN_NO_DYNAMIC_LIBS
typedef struct HymnLibList HymnLibList;

struct HymnLibList {
    void *lib;
    HymnLibList *next;
};
#endif

struct Hymn {
    HymnValue stack[HYMN_STACK_MAX];
    HymnValue *stack_top;
    HymnFrame frames[HYMN_FRAMES_MAX];
    int frame_count;
    char padding[4];
    HymnSet strings;
    HymnTable globals;
    HymnArray *paths;
    HymnTable *imports;
    HymnString *error;
    HymnString *exception;
#ifndef HYMN_NO_DYNAMIC_LIBS
    HymnLibList *libraries;
#endif
    void (*print)(const char *format, ...);
    void (*print_error)(const char *format, ...);
};

HymnString *hymn_working_directory(void);
HymnString *hymn_path_convert(HymnString *path);
HymnString *hymn_path_normalize(HymnString *path);
HymnString *hymn_path_parent(HymnString *path);
HymnString *hymn_path_absolute(HymnString *path);

size_t hymn_file_size(const char *path);
HymnString *hymn_read_file(const char *path);
bool hymn_file_exists(const char *path);

HymnString *hymn_new_string_with_capacity(size_t capacity);
HymnString *hymn_new_string_with_length(const char *init, size_t length);
HymnString *hymn_new_empty_string(size_t length);
HymnString *hymn_new_string(const char *init);
HymnObjectString *hymn_intern_string(Hymn *H, HymnString *string);
HymnObjectString *hymn_new_intern_string(Hymn *H, const char *value);

HymnString *hymn_string_copy(HymnString *string);
void hymn_string_delete(HymnString *this);
void hymn_string_zero(HymnString *this);
HymnString *hymn_string_append_char(HymnString *string, const char b);
HymnString *hymn_string_append_substring(HymnString *string, const char *b, size_t start, size_t end);
bool hymn_string_starts_with(HymnString *s, const char *using);
HymnString *hymn_string_replace(HymnString *string, const char *find, const char *replace);
HymnString *hymn_string_append(HymnString *string, const char *b);
HymnString *hymn_string_format(const char *format, ...);
HymnString *hymn_substring(const char *init, size_t start, size_t end);
void hymn_string_trim(HymnString *string);
HymnString *hymn_quote_string(HymnString *string);

HymnString *hymn_int_to_string(HymnInt number);
HymnString *hymn_float_to_string(HymnFloat number);

HymnArray *hymn_new_array(HymnInt length);

void hymn_array_push(HymnArray *this, HymnValue value);
void hymn_array_insert(HymnArray *this, HymnInt index, HymnValue value);
HymnValue hymn_array_get(HymnArray *this, HymnInt index);
HymnInt hymn_array_index_of(HymnArray *this, HymnValue match);
HymnValue hymn_array_pop(HymnArray *this);
HymnValue hymn_array_remove_index(HymnArray *this, HymnInt index);
void hymn_array_clear(Hymn *H, HymnArray *this);
void hymn_array_delete(Hymn *H, HymnArray *this);

export HymnTable *hymn_new_table(void);

HymnValue hymn_table_get(HymnTable *this, const char *key);

HymnValue hymn_new_undefined(void);
HymnValue hymn_new_none(void);
HymnValue hymn_new_bool(bool v);
export HymnValue hymn_new_int(HymnInt v);
HymnValue hymn_new_float(HymnFloat v);
HymnValue hymn_new_native(HymnNativeFunction *v);
HymnValue hymn_new_pointer(void *v);
HymnValue hymn_new_string_value(HymnObjectString *v);
HymnValue hymn_new_array_value(HymnArray *v);
HymnValue hymn_new_table_value(HymnTable *v);
HymnValue hymn_new_func_value(HymnFunction *v);

HymnObjectString *hymn_new_string_object(HymnString *string);

HymnString *hymn_value_to_string(HymnValue value);

bool hymn_as_bool(HymnValue v);
HymnInt hymn_as_int(HymnValue v);
HymnFloat hymn_as_float(HymnValue v);
HymnNativeFunction *hymn_as_native(HymnValue v);
void *hymn_as_pointer(HymnValue v);
void *hymn_as_object(HymnValue v);
HymnObjectString *hymn_as_hymn_string(HymnValue v);
HymnString *hymn_as_string(HymnValue v);
HymnArray *hymn_as_array(HymnValue v);
HymnTable *hymn_as_table(HymnValue v);
HymnFunction *hymn_as_func(HymnValue v);

bool hymn_is_undefined(HymnValue v);
bool hymn_is_none(HymnValue v);
bool hymn_is_bool(HymnValue v);
bool hymn_is_int(HymnValue v);
bool hymn_is_float(HymnValue v);
bool hymn_is_native(HymnValue v);
bool hymn_is_pointer(HymnValue v);
bool hymn_is_string(HymnValue v);
bool hymn_is_array(HymnValue v);
bool hymn_is_table(HymnValue v);
bool hymn_is_func(HymnValue v);

bool hymn_value_false(HymnValue value);
bool hymn_values_equal(HymnValue a, HymnValue b);
bool hymn_match_values(HymnValue a, HymnValue b);

const char *hymn_value_type(enum HymnValueType type);

void hymn_reference_string(HymnObjectString *string);
void hymn_reference(HymnValue value);
void hymn_dereference_string(Hymn *H, HymnObjectString *string);
void hymn_dereference(Hymn *H, HymnValue value);

void hymn_set_property(Hymn *H, HymnTable *table, HymnObjectString *name, HymnValue value);
void hymn_set_property_const(Hymn *H, HymnTable *table, const char *name, HymnValue value);

HymnValue hymn_new_exception(Hymn *H, const char *error);
HymnValue hymn_arity_exception(Hymn *H, int expected, int actual);
HymnValue hymn_type_exception(Hymn *H, enum HymnValueType expected, enum HymnValueType actual);

Hymn *new_hymn(void);

char *hymn_call(Hymn *H, const char *name, int arguments);
char *hymn_debug(Hymn *H, const char *script, const char *source);
char *hymn_run(Hymn *H, const char *script, const char *source);
char *hymn_do(Hymn *H, const char *source);
char *hymn_script(Hymn *H, const char *script);

HymnValue hymn_get(Hymn *H, const char *name);

void hymn_add(Hymn *H, const char *name, HymnValue value);
void hymn_add_string(Hymn *H, const char *name, const char *string);
export void hymn_add_table(Hymn *H, const char *name, HymnTable *table);
void hymn_add_pointer(Hymn *H, const char *name, void *pointer);
void hymn_add_string_to_table(Hymn *H, HymnTable *table, const char *name, const char *string);
export void hymn_add_function_to_table(Hymn *H, HymnTable *table, const char *name, HymnNativeCall func);
void hymn_add_function(Hymn *H, const char *name, HymnNativeCall func);

void hymn_delete(Hymn *H);

#ifndef HYMN_NO_REPL
void hymn_repl(Hymn *H);
#endif

#ifndef HYMN_NO_DYNAMIC_LIBS
void hymn_add_dlib(Hymn *H, void *library);
void hymn_close_dlib(void *library);
HymnString *hymn_use_dlib(Hymn *hymn, const char *path, const char *func);
#endif

#endif
