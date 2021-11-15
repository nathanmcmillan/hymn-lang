/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef HYMN_H
#define HYMN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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

#ifdef __GNUC__
#define PACK(expr) expr __attribute__((__packed__))
#elif _MSC_VER
#define PACK(expr) __pragma(pack(push, 1)) expr __pragma(pack(pop))
#endif

PACK(struct HymnStringHead {
    size_t length;
    size_t capacity;
    char **chars;
});

#undef PACK

void *hymn_malloc(size_t size);
void *hymn_calloc(size_t members, size_t member_size);
void *hymn_realloc(void *mem, size_t size);

struct HymnFilterList {
    int count;
    HymnString **filtered;
};

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
        HymnNativeFunction *n;
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
    HymnString *name;
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
};

bool hymn_file_exists(const char *path);
HymnString *hymn_read_file(const char *path);

HymnString *hymn_new_string(const char *init);

size_t hymn_string_len(HymnString *this);
void hymn_string_delete(HymnString *this);
bool hymn_string_equal(HymnString *a, HymnString *b);
void hymn_string_zero(HymnString *this);
HymnString *hymn_string_append_char(HymnString *this, const char b);
HymnString *hymn_string_trim(HymnString *this);
bool hymn_string_contains(HymnString *s, const char *p);
bool hymn_string_starts_with(HymnString *s, const char *p);
HymnString *hymn_string_append(HymnString *this, const char *b);
HymnString *hymn_string_format(const char *format, ...);
HymnString *hymn_substring(const char *init, size_t start, size_t end);

struct HymnFilterList hymn_string_filter_ends_with(HymnString **input, int count, const char *with);
void hymn_delete_filter_list(struct HymnFilterList *list);

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

Hymn *new_hymn();

char *hymn_debug(Hymn *H, const char *script);
char *hymn_call(Hymn *H, const char *name, int arguments);
char *hymn_run(Hymn *H, const char *script, const char *source);
char *hymn_do(Hymn *H, const char *source);
char *hymn_read(Hymn *H, const char *script);

void hymn_add_function(Hymn *H, const char *name, HymnNativeCall func);
void hymn_add_pointer(Hymn *H, const char *name, void *pointer);

void hymn_delete(Hymn *H);

#endif
