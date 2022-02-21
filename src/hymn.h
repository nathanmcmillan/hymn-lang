/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef HYMN_H
#define HYMN_H

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
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

#ifdef _MSC_VER
#include <direct.h>
#include <windows.h>
#define getcwd _getcwd
#define PATH_MAX FILENAME_MAX
#define PATH_SEP '\\'
#define PATH_SEP_STRING "\\"
#define PATH_SEP_OTHER '/'
#define PATH_SEP_OTHER_STRING "/"
#define UNREACHABLE() __assume(0)
#define PACK(expr) __pragma(pack(push, 1)) expr __pragma(pack(pop))
#else
#include <dirent.h>
#include <linux/limits.h>
#include <unistd.h>
#define PATH_SEP '/'
#define PATH_SEP_STRING "/"
#define PATH_SEP_OTHER '\\'
#define PATH_SEP_OTHER_STRING "\\"
#define UNREACHABLE() __builtin_unreachable()
#define PACK(expr) expr __attribute__((__packed__))
#endif

#define HYMN_UINT8_COUNT (UINT8_MAX + 1)

#define HYMN_FRAMES_MAX 64
#define HYMN_STACK_MAX (HYMN_FRAMES_MAX * HYMN_UINT8_COUNT)

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
void *hymn_calloc(size_t members, size_t member_size);
void *hymn_realloc(void *mem, size_t size);

typedef struct HymnValue HymnValue;
typedef struct HymnObject HymnObject;
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
    enum HymnValueType is;
    union {
        bool b;
        int64_t i;
        double f;
        HymnObject *o;
        void *p;
    } as;
};

struct HymnObject {
    int count;
};

struct HymnObjectString {
    HymnObject object;
    HymnString *string;
    size_t hash;
};

struct HymnArray {
    HymnObject object;
    HymnValue *items;
    int64_t length;
    int64_t capacity;
};

struct HymnTableItem {
    HymnObjectString *key;
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
    HymnObjectString *string;
    HymnSetItem *next;
};

struct HymnSet {
    unsigned int size;
    unsigned int bins;
    HymnSetItem **items;
};

struct HymnNativeFunction {
    HymnObject object;
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
    struct HymnExceptList *next;
};

struct HymnFunction {
    HymnObject object;
    HymnString *name;
    HymnString *script;
    int arity;
    HymnByteCode code;
    HymnExceptList *except;
};

struct HymnFrame {
    HymnFunction *func;
    uint8_t *ip;
    HymnValue *stack;
};

struct Hymn {
    HymnValue stack[HYMN_STACK_MAX];
    HymnValue *stack_top;
    HymnFrame frames[HYMN_FRAMES_MAX];
    int frame_count;
    HymnSet strings;
    HymnTable globals;
    HymnArray *paths;
    HymnTable *imports;
    HymnString *error;
    void (*print)(const char *format, ...);
    void (*print_error)(const char *format, ...);
};

HymnString *hymn_working_directory();
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

HymnStringHead *hymn_string_head(HymnString *string);
HymnString *hymn_string_copy(HymnString *string);
size_t hymn_string_len(HymnString *this);
void hymn_string_delete(HymnString *this);
bool hymn_string_equal(HymnString *a, HymnString *b);
void hymn_string_zero(HymnString *this);
HymnString *hymn_string_append_char(HymnString *this, const char b);
bool hymn_string_starts_with(HymnString *s, const char *using);
HymnString *hymn_string_replace(HymnString *string, const char *find, const char *replace);
HymnString *hymn_string_append(HymnString *this, const char *b);
HymnString *hymn_string_format(const char *format, ...);
HymnString *hymn_substring(const char *init, size_t start, size_t end);

HymnArray *hymn_new_array(int64_t length);

void hymn_array_push(HymnArray *this, HymnValue value);
void hymn_array_insert(HymnArray *this, int64_t index, HymnValue value);
HymnValue hymn_array_get(HymnArray *this, int64_t index);
int64_t hymn_array_index_of(HymnArray *this, HymnValue match);
HymnValue hymn_array_pop(HymnArray *this);
HymnValue hymn_array_remove_index(HymnArray *this, int64_t index);
void hymn_array_clear(Hymn *H, HymnArray *this);
void hymn_array_delete(Hymn *H, HymnArray *this);

HymnTable *hymn_new_table();

HymnValue hymn_new_undefined();
HymnValue hymn_new_none();
HymnValue hymn_new_bool(bool v);
HymnValue hymn_new_int(int64_t v);
HymnValue hymn_new_float(double v);
HymnValue hymn_new_native(HymnNativeFunction *v);
HymnValue hymn_new_pointer(void *v);
HymnValue hymn_new_string_value(HymnObjectString *v);
HymnValue hymn_new_array_value(HymnArray *v);
HymnValue hymn_new_table_value(HymnTable *v);
HymnValue hymn_new_func_value(HymnFunction *v);

HymnObjectString *hymn_new_string_object(HymnString *string);

HymnString *hymn_value_to_string(HymnValue value);

bool hymn_as_bool(HymnValue v);
int64_t hymn_as_int(HymnValue v);
double hymn_as_float(HymnValue v);
HymnNativeFunction *hymn_as_native(HymnValue v);
void *hymn_as_pointer(HymnValue v);
HymnObject *hymn_as_object(HymnValue v);
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

void hymn_reference_string(HymnObjectString *string);
void hymn_reference(HymnValue value);
void hymn_dereference_string(Hymn *H, HymnObjectString *string);
void hymn_dereference(Hymn *H, HymnValue value);

void hymn_set_property(Hymn *H, HymnTable *table, HymnObjectString *name, HymnValue value);
void hymn_set_property_const(Hymn *H, HymnTable *table, const char *name, HymnValue value);

Hymn *new_hymn();

char *hymn_debug(Hymn *H, const char *script, const char *source);
char *hymn_call(Hymn *H, const char *name, int arguments);
char *hymn_run(Hymn *H, const char *script, const char *source);
char *hymn_do(Hymn *H, const char *source);
char *hymn_read(Hymn *H, const char *script);

HymnValue hymn_get(Hymn *H, const char *name);

void hymn_add(Hymn *H, const char *name, HymnValue value);
void hymn_add_function(Hymn *H, const char *name, HymnNativeCall func);
void hymn_add_pointer(Hymn *H, const char *name, void *pointer);

void hymn_delete(Hymn *H);

#endif
