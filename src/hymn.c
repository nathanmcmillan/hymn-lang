/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "hymn.h"

#define new_undefined() ((Value){VALUE_UNDEFINED, {.i = 0}})
#define new_none() ((Value){VALUE_NONE, {.i = 0}})
#define new_bool(v) ((Value){VALUE_BOOL, {.b = v}})
#define new_int(v) ((Value){VALUE_INTEGER, {.i = v}})
#define new_float(v) ((Value){VALUE_FLOAT, {.f = v}})
#define new_string_value(v) ((Value){VALUE_STRING, {.s = v}})
#define new_array_value(v) ((Value){VALUE_ARRAY, {.a = v}})
#define new_table_value(v) ((Value){VALUE_TABLE, {.t = v}})
#define new_func(v) ((Value){VALUE_FUNC, {.func = v}})
#define new_native(v) ((Value){VALUE_FUNC_NATIVE, {.native = v}})

#define as_bool(v) ((v).as.b)
#define as_int(v) ((v).as.i)
#define as_float(v) ((v).as.f)
#define as_string(v) ((v).as.s)
#define as_array(v) ((v).as.a)
#define as_table(v) ((v).as.t)
#define as_func(v) ((v).as.func)
#define as_native(v) ((v).as.native)

#define is_undefined(v) ((v).is == VALUE_UNDEFINED)
#define is_none(v) ((v).is == VALUE_NONE)
#define is_bool(v) ((v).is == VALUE_BOOL)
#define is_int(v) ((v).is == VALUE_INTEGER)
#define is_float(v) ((v).is == VALUE_FLOAT)
#define is_string(v) ((v).is == VALUE_STRING)
#define is_array(v) ((v).is == VALUE_ARRAY)
#define is_table(v) ((v).is == VALUE_TABLE)
#define is_func(v) ((v).is == VALUE_FUNC)
#define is_native(v) ((v).is == VALUE_FUNC_NATIVE)

#define STRING_NONE "None"
#define STRING_BOOL "Bool"
#define STRING_TRUE "True"
#define STRING_FALSE "False"
#define STRING_INTEGER "Integer"
#define STRING_FLOAT "Float"
#define STRING_STRING "String"
#define STRING_ARRAY "Array"
#define STRING_TABLE "Table"
#define STRING_FUNC "Function"
#define STRING_NATIVE "Native"

#define INTEGER_OP(_binary_)                                              \
    Value b = machine_pop(this);                                          \
    Value a = machine_pop(this);                                          \
    if (is_int(a)) {                                                      \
        if (is_int(b)) {                                                  \
            a.as.i _binary_ b.as.i;                                       \
            machine_push(this, a);                                        \
        } else {                                                          \
            machine_runtime_error(this, "Bitwise operand must integer."); \
            return;                                                       \
        }                                                                 \
    } else {                                                              \
        machine_runtime_error(this, "Bitwise operand must integer.");     \
        return;                                                           \
    }

#define ARITHMETIC_OP(_binary_)                                 \
    Value b = machine_pop(this);                                \
    Value a = machine_pop(this);                                \
    if (is_int(a)) {                                            \
        if (is_int(b)) {                                        \
            a.as.i _binary_ b.as.i;                             \
            machine_push(this, a);                              \
        } else if (is_float(b)) {                               \
            b.as.f _binary_ a.as.i;                             \
            machine_push(this, a);                              \
        } else {                                                \
            machine_runtime_error(this, "Operand non-number."); \
            return;                                             \
        }                                                       \
    } else if (is_float(a)) {                                   \
        if (is_int(b)) {                                        \
            a.as.f _binary_ b.as.i;                             \
            machine_push(this, a);                              \
        } else if (is_float(b)) {                               \
            a.as.f _binary_ b.as.f;                             \
            machine_push(this, a);                              \
        } else {                                                \
            machine_runtime_error(this, "Operand non-number."); \
            return;                                             \
        }                                                       \
    } else {                                                    \
        machine_runtime_error(this, "Operand non-number.");     \
        return;                                                 \
    }

#define COMPARE_OP(_compare_)                                                      \
    Value b = machine_pop(this);                                                   \
    Value a = machine_pop(this);                                                   \
    if (is_int(a)) {                                                               \
        if (is_int(b)) {                                                           \
            machine_push(this, new_bool(as_int(a) _compare_ as_int(b)));           \
        } else if (is_float(b)) {                                                  \
            machine_push(this, new_bool((double)as_int(a) _compare_ as_float(b))); \
        } else {                                                                   \
            machine_runtime_error(this, "Comparing non-number.");                  \
            return;                                                                \
        }                                                                          \
    } else if (is_float(a)) {                                                      \
        if (is_int(b)) {                                                           \
            machine_push(this, new_bool(as_float(a) _compare_(double) as_int(b))); \
        } else if (is_float(b)) {                                                  \
            machine_push(this, new_bool(as_float(a) _compare_ as_float(b)));       \
        } else {                                                                   \
            machine_runtime_error(this, "Comparing non-number.");                  \
            return;                                                                \
        }                                                                          \
    } else {                                                                       \
        machine_runtime_error(this, "Comparing non-number.");                      \
        return;                                                                    \
    }

static const float LOAD_FACTOR = 0.80f;

static const unsigned int INITIAL_BINS = 1 << 3;

static const unsigned int MAXIMUM_BINS = 1 << 30;

enum ValueType {
    VALUE_UNDEFINED,
    VALUE_NONE,
    VALUE_BOOL,
    VALUE_INTEGER,
    VALUE_FLOAT,
    VALUE_STRING,
    VALUE_ARRAY,
    VALUE_TABLE,
    VALUE_FUNC,
    VALUE_FUNC_NATIVE,
};

enum TokenType {
    TOKEN_ADD,
    TOKEN_AND,
    TOKEN_ASSIGN,
    TOKEN_BEGIN,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_CONST,
    TOKEN_DELETE,
    TOKEN_DIVIDE,
    TOKEN_DOT,
    TOKEN_ELIF,
    TOKEN_ELSE,
    TOKEN_END,
    TOKEN_EOF,
    TOKEN_EQUAL,
    TOKEN_ERROR,
    TOKEN_FALSE,
    TOKEN_FLOAT,
    TOKEN_FOR,
    TOKEN_FUNCTION,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_IDENT,
    TOKEN_IF,
    TOKEN_INSERT,
    TOKEN_INTEGER,
    TOKEN_LEFT_CURLY,
    TOKEN_LEFT_PAREN,
    TOKEN_LEFT_SQUARE,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_LEN,
    TOKEN_LET,
    TOKEN_LINE,
    TOKEN_MULTIPLY,
    TOKEN_NONE,
    TOKEN_NOT,
    TOKEN_NOT_EQUAL,
    TOKEN_OR,
    TOKEN_PASS,
    TOKEN_POP,
    TOKEN_PUSH,
    TOKEN_PRINT,
    TOKEN_RETURN,
    TOKEN_RIGHT_CURLY,
    TOKEN_RIGHT_PAREN,
    TOKEN_RIGHT_SQUARE,
    TOKEN_SEMICOLON,
    TOKEN_STRING,
    TOKEN_SUBTRACT,
    TOKEN_TRUE,
    TOKEN_UNDEFINED,
    TOKEN_USE,
    TOKEN_VALUE,
    TOKEN_WHILE,
    TOKEN_TYPE,
    TOKEN_TO_INTEGER,
    TOKEN_TO_FLOAT,
    TOKEN_TO_STRING,
    TOKEN_CLEAR,
    TOKEN_COPY,
    TOKEN_INDEX,
    TOKEN_KEYS,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_BIT_AND,
    TOKEN_BIT_OR,
    TOKEN_BIT_XOR,
    TOKEN_BIT_NOT,
    TOKEN_BIT_LEFT_SHIFT,
    TOKEN_BIT_RIGHT_SHIFT,
    TOKEN_TRY,
    TOKEN_EXCEPT,
    TOKEN_SWITCH,
    TOKEN_CASE,
    TOKEN_IN,
};

enum Precedence {
    PRECEDENCE_NONE,
    PRECEDENCE_ASSIGN,
    PRECEDENCE_BITS,
    PRECEDENCE_OR,
    PRECEDENCE_AND,
    PRECEDENCE_EQUALITY,
    PRECEDENCE_COMPARE,
    PRECEDENCE_TERM,
    PRECEDENCE_FACTOR,
    PRECEDENCE_UNARY,
    PRECEDENCE_CALL,
    PRECEDENCE_PRIMARY,
};

enum OpCode {
    OP_ADD,
    OP_ARRAY_INSERT,
    OP_ARRAY_POP,
    OP_ARRAY_PUSH,
    OP_BIT_AND,
    OP_BIT_LEFT_SHIFT,
    OP_BIT_NOT,
    OP_BIT_OR,
    OP_BIT_RIGHT_SHIFT,
    OP_BIT_XOR,
    OP_CALL,
    OP_CLEAR,
    OP_CONSTANT,
    OP_CONSTANT_TWO,
    OP_COPY,
    OP_DEFINE_GLOBAL,
    OP_DELETE,
    OP_DIVIDE,
    OP_EQUAL,
    OP_FALSE,
    OP_GET_DYNAMIC,
    OP_GET_GLOBAL,
    OP_GET_LOCAL,
    OP_GET_PROPERTY,
    OP_GREATER,
    OP_GREATER_EQUAL,
    OP_INDEX,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_KEYS,
    OP_LEN,
    OP_LESS,
    OP_LESS_EQUAL,
    OP_LOOP,
    OP_MULTIPLY,
    OP_NEGATE,
    OP_NONE,
    OP_NOT,
    OP_NOT_EQUAL,
    OP_POP,
    OP_PRINT,
    OP_RETURN,
    OP_SET_DYNAMIC,
    OP_SET_GLOBAL,
    OP_SET_LOCAL,
    OP_SET_PROPERTY,
    OP_SLICE,
    OP_SUBTRACT,
    OP_TO_FLOAT,
    OP_TO_INTEGER,
    OP_TO_STRING,
    OP_TRUE,
    OP_TYPE,
};

enum FunctionType {
    TYPE_FUNCTION,
    TYPE_SCRIPT,
};

typedef struct Array Array;
typedef struct Value Value;
typedef struct Token Token;
typedef struct Local Local;
typedef struct Rule Rule;
typedef struct Script Script;
typedef struct ValueMapItem ValueMapItem;
typedef struct ValueMap ValueMap;
typedef struct ValuePool ValuePool;
typedef struct ByteCode ByteCode;
typedef struct Function Function;
typedef struct NativeFunction NativeFunction;
typedef struct Frame Frame;
typedef struct Machine Machine;
typedef struct Scope Scope;
typedef struct Compiler Compiler;

static void compile_with_precedence(Compiler *this, enum Precedence precedence);
static void compile_call(Compiler *this, bool assign);
static void compile_group(Compiler *this, bool assign);
static void compile_none(Compiler *this, bool assign);
static void compile_true(Compiler *this, bool assign);
static void compile_false(Compiler *this, bool assign);
static void compile_integer(Compiler *this, bool assign);
static void compile_float(Compiler *this, bool assign);
static void compile_string(Compiler *this, bool assign);
static void compile_array(Compiler *this, bool assign);
static void compile_table(Compiler *this, bool assign);
static void compile_variable(Compiler *this, bool assign);
static void compile_unary(Compiler *this, bool assign);
static void compile_binary(Compiler *this, bool assign);
static void compile_dot(Compiler *this, bool assign);
static void compile_square(Compiler *this, bool assign);
static void compile_and(Compiler *this, bool assign);
static void compile_or(Compiler *this, bool assign);
static void array_push_expression(Compiler *this, bool assign);
static void array_insert_expression(Compiler *this, bool assign);
static void array_pop_expression(Compiler *this, bool assign);
static void delete_expression(Compiler *this, bool assign);
static void len_expression(Compiler *this, bool assign);
static void cast_integer_expression(Compiler *this, bool assign);
static void cast_float_expression(Compiler *this, bool assign);
static void cast_string_expression(Compiler *this, bool assign);
static void clear_expression(Compiler *this, bool assign);
static void copy_expression(Compiler *this, bool assign);
static void index_expression(Compiler *this, bool assign);
static void keys_expression(Compiler *this, bool assign);
static void type_expression(Compiler *this, bool assign);
static void declaration(Compiler *this);
static void statement(Compiler *this);
static void expression_statement(Compiler *this);
static void expression(Compiler *this);

struct JumpList {
    int jump;
    struct JumpList *next;
};

struct LoopList {
    int start;
    struct LoopList *next;
};

struct Token {
    enum TokenType type;
    int row;
    int column;
    usize start;
    int length;
};

struct Value {
    enum ValueType is;
    union {
        bool b;
        i64 i;
        double f;
        String *s;
        Array *a;
        ValueMap *t;
        Function *func;
        NativeFunction *native;
    } as;
};

struct Local {
    Token name;
    int depth;
    bool constant;
};

struct Array {
    Value *items;
    i64 length;
    i64 capacity;
};

struct Rule {
    void (*prefix)(Compiler *, bool);
    void (*infix)(Compiler *, bool);
    enum Precedence precedence;
};

struct Script {
    const char *name;
    Value **variables;
    usize variable_count;
};

struct ValueMapItem {
    usize hash;
    String *key;
    Value value;
    ValueMapItem *next;
};

struct ValueMap {
    unsigned int size;
    unsigned int bins;
    ValueMapItem **items;
};

struct ValuePool {
    int count;
    int capacity;
    Value *values;
};

struct ByteCode {
    int count;
    int capacity;
    u8 *instructions;
    int *rows;
    ValuePool constants;
};

struct Function {
    String *name;
    int arity;
    ByteCode code;
};

typedef Value (*NativeCall)(int count, Value *arguments);

struct NativeFunction {
    String *name;
    NativeCall func;
};

struct Frame {
    Function *func;
    usize ip;
    usize stack_top;
};

struct Machine {
    Value stack[HYMN_STACK_MAX];
    usize stack_top;
    Frame frames[HYMN_FRAMES_MAX];
    int frame_count;
    ValueMap strings;
    ValueMap globals;
    String *error;
};

struct Scope {
    struct Scope *enclosing;
    Function *func;
    enum FunctionType type;
    Local locals[UINT8_COUNT];
    int local_count;
    int depth;
};

struct Compiler {
    usize pos;
    int row;
    int column;
    char *source;
    usize size;
    Token alpha;
    Token beta;
    Token gamma;
    Machine *machine;
    Scope *scope;
    struct LoopList *loop;
    struct JumpList *jump;
    bool panic;
    String *error;
};

Rule rules[] = {
    [TOKEN_ADD] = {NULL, compile_binary, PRECEDENCE_TERM},
    [TOKEN_AND] = {NULL, compile_and, PRECEDENCE_AND},
    [TOKEN_ASSIGN] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_BEGIN] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_BIT_AND] = {NULL, compile_binary, PRECEDENCE_BITS},
    [TOKEN_BIT_LEFT_SHIFT] = {NULL, compile_binary, PRECEDENCE_BITS},
    [TOKEN_BIT_NOT] = {compile_unary, NULL, PRECEDENCE_NONE},
    [TOKEN_BIT_OR] = {NULL, compile_binary, PRECEDENCE_BITS},
    [TOKEN_BIT_RIGHT_SHIFT] = {NULL, compile_binary, PRECEDENCE_BITS},
    [TOKEN_BIT_XOR] = {NULL, compile_binary, PRECEDENCE_BITS},
    [TOKEN_BREAK] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_CASE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_CLEAR] = {clear_expression, NULL, PRECEDENCE_NONE},
    [TOKEN_COMMA] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_CONST] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_CONTINUE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_COPY] = {copy_expression, NULL, PRECEDENCE_NONE},
    [TOKEN_DELETE] = {delete_expression, NULL, PRECEDENCE_NONE},
    [TOKEN_DIVIDE] = {NULL, compile_binary, PRECEDENCE_FACTOR},
    [TOKEN_DOT] = {NULL, compile_dot, PRECEDENCE_CALL},
    [TOKEN_ELIF] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_ELSE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_END] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_EOF] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_EQUAL] = {NULL, compile_binary, PRECEDENCE_EQUALITY},
    [TOKEN_ERROR] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_EXCEPT] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_FALSE] = {compile_false, NULL, PRECEDENCE_NONE},
    [TOKEN_FLOAT] = {compile_float, NULL, PRECEDENCE_NONE},
    [TOKEN_FOR] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_FUNCTION] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_GREATER] = {NULL, compile_binary, PRECEDENCE_COMPARE},
    [TOKEN_GREATER_EQUAL] = {NULL, compile_binary, PRECEDENCE_COMPARE},
    [TOKEN_IDENT] = {compile_variable, NULL, PRECEDENCE_NONE},
    [TOKEN_IF] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_IN] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_INDEX] = {index_expression, NULL, PRECEDENCE_NONE},
    [TOKEN_INSERT] = {array_insert_expression, NULL, PRECEDENCE_NONE},
    [TOKEN_INTEGER] = {compile_integer, NULL, PRECEDENCE_NONE},
    [TOKEN_KEYS] = {keys_expression, NULL, PRECEDENCE_NONE},
    [TOKEN_LEFT_CURLY] = {compile_table, NULL, PRECEDENCE_NONE},
    [TOKEN_LEFT_PAREN] = {compile_group, compile_call, PRECEDENCE_CALL},
    [TOKEN_LEFT_SQUARE] = {compile_array, compile_square, PRECEDENCE_CALL},
    [TOKEN_LEN] = {len_expression, NULL, PRECEDENCE_NONE},
    [TOKEN_LESS] = {NULL, compile_binary, PRECEDENCE_COMPARE},
    [TOKEN_LESS_EQUAL] = {NULL, compile_binary, PRECEDENCE_COMPARE},
    [TOKEN_LET] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_LINE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_MULTIPLY] = {NULL, compile_binary, PRECEDENCE_FACTOR},
    [TOKEN_NONE] = {compile_none, NULL, PRECEDENCE_NONE},
    [TOKEN_NOT] = {compile_unary, NULL, PRECEDENCE_NONE},
    [TOKEN_NOT_EQUAL] = {NULL, compile_binary, PRECEDENCE_EQUALITY},
    [TOKEN_OR] = {NULL, compile_or, PRECEDENCE_OR},
    [TOKEN_PASS] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_POP] = {array_pop_expression, NULL, PRECEDENCE_NONE},
    [TOKEN_PRINT] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_PUSH] = {array_push_expression, NULL, PRECEDENCE_NONE},
    [TOKEN_RETURN] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_RIGHT_CURLY] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_RIGHT_PAREN] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_RIGHT_SQUARE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_STRING] = {compile_string, NULL, PRECEDENCE_NONE},
    [TOKEN_SUBTRACT] = {compile_unary, compile_binary, PRECEDENCE_TERM},
    [TOKEN_SWITCH] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_TO_FLOAT] = {cast_float_expression, NULL, PRECEDENCE_NONE},
    [TOKEN_TO_INTEGER] = {cast_integer_expression, NULL, PRECEDENCE_NONE},
    [TOKEN_TO_STRING] = {cast_string_expression, NULL, PRECEDENCE_NONE},
    [TOKEN_TRUE] = {compile_true, NULL, PRECEDENCE_NONE},
    [TOKEN_TRY] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_TYPE] = {type_expression, NULL, PRECEDENCE_NONE},
    [TOKEN_UNDEFINED] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_USE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_VALUE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_WHILE] = {NULL, NULL, PRECEDENCE_NONE},
};

static usize string_hashcode(String *key) {
    usize length = string_len(key);
    usize hash = 0;
    for (usize i = 0; i < length; i++) {
        hash = 31 * hash + (usize)key[i];
    }
    return hash;
}

static void map_init(ValueMap *this) {
    this->size = 0;
    this->bins = INITIAL_BINS;
    this->items = safe_calloc(this->bins, sizeof(ValueMapItem *));
}

static unsigned int map_get_bin(ValueMap *this, usize hash) {
    return (this->bins - 1) & hash;
}

static usize map_hash_mix(usize hash) {
    return hash ^ (hash >> 16);
}

static void map_resize(ValueMap *this) {

    unsigned int old_bins = this->bins;
    unsigned int bins = old_bins << 1;

    if (bins > MAXIMUM_BINS) {
        return;
    }

    ValueMapItem **old_items = this->items;
    ValueMapItem **items = safe_calloc(bins, sizeof(ValueMapItem *));

    for (unsigned int i = 0; i < old_bins; i++) {
        ValueMapItem *item = old_items[i];
        if (item == NULL) {
            continue;
        }
        if (item->next == NULL) {
            items[(bins - 1) & item->hash] = item;
        } else {
            ValueMapItem *low_head = NULL;
            ValueMapItem *low_tail = NULL;
            ValueMapItem *high_head = NULL;
            ValueMapItem *high_tail = NULL;
            do {
                if ((old_bins & item->hash) == 0) {
                    if (low_tail == NULL) {
                        low_head = item;
                    } else {
                        low_tail->next = item;
                    }
                    low_tail = item;
                } else {
                    if (high_tail == NULL) {
                        high_head = item;
                    } else {
                        high_tail->next = item;
                    }
                    high_tail = item;
                }
                item = item->next;
            } while (item != NULL);

            if (low_tail != NULL) {
                low_tail->next = NULL;
                items[i] = low_head;
            }

            if (high_tail != NULL) {
                high_tail->next = NULL;
                items[i + old_bins] = high_head;
            }
        }
    }

    free(old_items);

    this->bins = bins;
    this->items = items;
}

static void map_put(ValueMap *this, String *key, Value value) {
    usize hash = map_hash_mix(string_hashcode(key));
    unsigned int bin = map_get_bin(this, hash);
    ValueMapItem *item = this->items[bin];
    ValueMapItem *previous = NULL;
    while (item != NULL) {
        if (string_equal(key, item->key)) {
            item->value = value;
            return;
        }
        previous = item;
        item = item->next;
    }
    item = safe_malloc(sizeof(ValueMapItem));
    item->hash = hash;
    item->key = key;
    item->value = value;
    item->next = NULL;
    if (previous == NULL) {
        this->items[bin] = item;
    } else {
        previous->next = item;
    }
    this->size++;
    if (this->size >= this->bins * LOAD_FACTOR) {
        map_resize(this);
    }
}

static Value map_get(ValueMap *this, String *key) {
    usize hash = map_hash_mix(string_hashcode(key));
    unsigned int bin = map_get_bin(this, hash);
    ValueMapItem *item = this->items[bin];
    while (item != NULL) {
        if (string_equal(key, item->key)) {
            return item->value;
        }
        item = item->next;
    }
    return new_undefined();
}

static Value map_remove(ValueMap *this, String *key) {
    usize hash = map_hash_mix(string_hashcode(key));
    unsigned int bin = map_get_bin(this, hash);
    ValueMapItem *item = this->items[bin];
    ValueMapItem *previous = NULL;
    while (item != NULL) {
        if (string_equal(key, item->key)) {
            if (previous == NULL) {
                this->items[bin] = item->next;
            } else {
                previous->next = item->next;
            }
            this->size -= 1;
            return item->value;
        }
        previous = item;
        item = item->next;
    }
    return new_undefined();
}

static void map_clear(ValueMap *this) {
    unsigned int bins = this->bins;
    for (unsigned int i = 0; i < bins; i++) {
        ValueMapItem *item = this->items[i];
        while (item != NULL) {
            ValueMapItem *next = item->next;
            free(item);
            item = next;
        }
        this->items[i] = NULL;
    }
    this->size = 0;
}

static bool map_is_empty(ValueMap *this) {
    return this->size == 0;
}

static bool map_not_empty(ValueMap *this) {
    return this->size != 0;
}

static unsigned int map_size(ValueMap *this) {
    return this->size;
}

static void map_delete(ValueMap *this) {
    map_clear(this);
    free(this->items);
}

static const char *value_name(enum ValueType value) {
    switch (value) {
    case VALUE_UNDEFINED: return "UNDEFINED";
    case VALUE_NONE: return "NONE";
    case VALUE_BOOL: return "BOOLEAN";
    case VALUE_INTEGER: return "INTEGER";
    case VALUE_FLOAT: return "FLOAT";
    case VALUE_STRING: return "STRING";
    case VALUE_ARRAY: return "ARRAY";
    case VALUE_TABLE: return "TABLE";
    case VALUE_FUNC: return "FUNCTION";
    case VALUE_FUNC_NATIVE: return "NATIVE_FUNCTION";
    default: return "VALUE";
    }
}

static const char *token_name(enum TokenType type) {
    switch (type) {
    case TOKEN_ADD: return "ADD";
    case TOKEN_AND: return "AND";
    case TOKEN_ASSIGN: return "ASSIGN";
    case TOKEN_BEGIN: return "BEGIN";
    case TOKEN_BIT_AND: return "BTIWISE_AND";
    case TOKEN_BIT_LEFT_SHIFT: return "BTIWISE_LEFT_SHIFT";
    case TOKEN_BIT_NOT: return "BTIWISE_NOT";
    case TOKEN_BIT_OR: return "BTIWISE_OR";
    case TOKEN_BIT_RIGHT_SHIFT: return "BTIWISE_RIGHT_SHIFT";
    case TOKEN_BIT_XOR: return "BTIWISE_XOR";
    case TOKEN_BREAK: return "BREAK";
    case TOKEN_CASE: return "CASE";
    case TOKEN_CLEAR: return "CLEAR";
    case TOKEN_COLON: return "COLON";
    case TOKEN_CONST: return "CONST";
    case TOKEN_CONTINUE: return "CONTINUE";
    case TOKEN_COPY: return "COPY";
    case TOKEN_DELETE: return "DELETE";
    case TOKEN_DIVIDE: return "DIVIDE";
    case TOKEN_ELIF: return "ELIF";
    case TOKEN_ELSE: return "ELSE";
    case TOKEN_END: return "END";
    case TOKEN_EOF: return "EOF";
    case TOKEN_EQUAL: return "EQUAL";
    case TOKEN_EXCEPT: return "EXCEPT";
    case TOKEN_FALSE: return "FALSE";
    case TOKEN_FLOAT: return "FLOAT";
    case TOKEN_FOR: return "FOR";
    case TOKEN_FUNCTION: return "FUNCTION";
    case TOKEN_GREATER: return "GREATER";
    case TOKEN_GREATER_EQUAL: return "GREATER_EQUAL";
    case TOKEN_IDENT: return "IDENT";
    case TOKEN_IF: return "IF";
    case TOKEN_IN: return "IN";
    case TOKEN_INDEX: return "INDEX";
    case TOKEN_INSERT: return "INSERT";
    case TOKEN_INTEGER: return "INTEGER";
    case TOKEN_KEYS: return "KEYS";
    case TOKEN_LEFT_PAREN: return "LEFT_PAREN";
    case TOKEN_LEN: return "LEN";
    case TOKEN_LESS: return "LESS";
    case TOKEN_LESS_EQUAL: return "LESS_EQUAL";
    case TOKEN_LET: return "LET";
    case TOKEN_MULTIPLY: return "MULTIPLY";
    case TOKEN_NONE: return "NONE";
    case TOKEN_NOT: return "NOT";
    case TOKEN_NOT_EQUAL: return "NOT_EQUAL";
    case TOKEN_OR: return "OR";
    case TOKEN_PASS: return "PASS";
    case TOKEN_POP: return "POP";
    case TOKEN_PRINT: return "PRINT";
    case TOKEN_PUSH: return "PUSH";
    case TOKEN_RETURN: return "RETURN";
    case TOKEN_RIGHT_PAREN: return "RIGHT_PAREN";
    case TOKEN_SEMICOLON: return "SEMICOLON";
    case TOKEN_STRING: return "STRING";
    case TOKEN_SUBTRACT: return "SUBTRACT";
    case TOKEN_SWITCH: return "SWITCH";
    case TOKEN_TO_FLOAT: return "TO_FLOAT";
    case TOKEN_TO_INTEGER: return "TO_INTEGER";
    case TOKEN_TO_STRING: return "TO_STRING";
    case TOKEN_TRUE: return "TRUE";
    case TOKEN_TRY: return "TRY";
    case TOKEN_TYPE: return "TYPE";
    case TOKEN_WHILE: return "WHILE";
    case TOKEN_USE: return "USE";
    default: return "TOKEN";
    }
}

static void debug_value(Value value) {
    printf("%s: ", value_name(value.is));
    switch (value.is) {
    case VALUE_UNDEFINED: printf("UNDEFINED"); break;
    case VALUE_NONE: printf("NONE"); break;
    case VALUE_BOOL: printf("%s", as_bool(value) ? "TRUE" : "FALSE"); break;
    case VALUE_INTEGER: printf("%" PRId64, as_int(value)); break;
    case VALUE_FLOAT: printf("%g", as_float(value)); break;
    case VALUE_STRING: printf("\"%s\"", as_string(value)); break;
    case VALUE_ARRAY: printf("[array %p]", as_array(value)); break;
    case VALUE_TABLE: printf("[table %p]", as_table(value)); break;
    case VALUE_FUNC: printf("<%s>", as_func(value)->name); break;
    case VALUE_FUNC_NATIVE: printf("<%s>", as_native(value)->name); break;
    default: printf("?");
    }
}

static void compiler_delete(Compiler *this) {
    string_delete(this->error);
}

static inline ByteCode *current(Compiler *this) {
    return &this->scope->func->code;
}

static void compile_error(Compiler *this, Token *token, const char *format, ...) {
    if (this->panic) {
        return;
    }
    this->panic = true;

    if (this->error == NULL) {
        this->error = new_string("");
    }

    this->error = string_append_format(this->error, "[Line %d] Error", token->row);
    if (token->type == TOKEN_EOF) {
        this->error = string_append(this->error, ": ");
    } else {
        this->error = string_append_format(this->error, " at '%.*s': ", token->length, &this->source[token->start]);
    }

    va_list ap;
    va_start(ap, format);
    int len = vsnprintf(NULL, 0, format, ap);
    va_end(ap);
    char *chars = safe_malloc((len + 1) * sizeof(char));
    va_start(ap, format);
    len = vsnprintf(chars, len + 1, format, ap);
    va_end(ap);
    this->error = string_append(this->error, chars);
    free(chars);

    this->error = string_append_char(this->error, '\n');
}

static char next_char(Compiler *this) {
    usize pos = this->pos;
    if (pos == this->size) {
        return '\0';
    }
    char c = this->source[pos];
    this->pos = pos + 1;
    if (c == '\n') {
        this->row++;
        this->column = 0;
    } else {
        this->column++;
    }
    return c;
}

static char peek_char(Compiler *this) {
    if (this->pos == this->size) {
        return '\0';
    }
    return this->source[this->pos];
}

static void token(Compiler *this, enum TokenType type) {
#ifdef HYMN_DEBUG_TOKEN
    printf("TOKEN: %s\n", token_name(type));
#endif
    Token *gamma = &this->gamma;
    gamma->type = type;
    gamma->row = this->row;
    gamma->column = this->column;
}

static void value_token(Compiler *this, enum TokenType type, usize start, usize end) {
#ifdef HYMN_DEBUG_TOKEN
    printf("TOKEN: %s: %.*s\n", token_name(type), (int)(end - start), &this->source[start]);
#endif
    Token *gamma = &this->gamma;
    gamma->type = type;
    gamma->row = this->row;
    gamma->column = this->column;
    gamma->start = start;
    gamma->length = (int)(end - start);
}

static void string_token(Compiler *this, usize start, usize end) {
#ifdef HYMN_DEBUG_TOKEN
    printf("TOKEN: %s, \"%.*s\"\n", token_name(TOKEN_STRING), (int)(end - start), &this->source[start]);
#endif
    Token *gamma = &this->gamma;
    gamma->type = TOKEN_STRING;
    gamma->row = this->row;
    gamma->column = this->column;
    gamma->start = start;
    gamma->length = (int)(end - start);
}

static enum TokenType ident_trie(char *ident, int offset, const char *rest, enum TokenType type) {
    int i = 0;
    do {
        if (ident[offset + i] != rest[i]) {
            return TOKEN_UNDEFINED;
        }
        i++;
    } while (rest[i] != '\0');
    return type;
}

static enum TokenType ident_keyword(char *ident, usize size) {
    switch (ident[0]) {
    case 'o':
        if (size == 2) return ident_trie(ident, 1, "r", TOKEN_OR);
        break;
    case 'u':
        if (size == 3) return ident_trie(ident, 1, "se", TOKEN_USE);
        break;
    case 'a':
        if (size == 3) return ident_trie(ident, 1, "nd", TOKEN_AND);
        break;
    case 'n':
        if (size == 4) return ident_trie(ident, 1, "one", TOKEN_NONE);
        break;
    case 'w':
        if (size == 5) return ident_trie(ident, 1, "hile", TOKEN_WHILE);
        break;
    case 'b':
        if (size == 5) {
            if (ident[1] == 'e') return ident_trie(ident, 2, "gin", TOKEN_BEGIN);
            if (ident[1] == 'r') return ident_trie(ident, 2, "eak", TOKEN_BREAK);
        }
        break;
    case 'd':
        if (size == 6) return ident_trie(ident, 1, "elete", TOKEN_DELETE);
        break;
    case 'r':
        if (size == 6) return ident_trie(ident, 1, "eturn", TOKEN_RETURN);
        break;
    case 's':
        if (size == 6) {
            if (ident[1] == 'w') return ident_trie(ident, 2, "itch", TOKEN_SWITCH);
            if (ident[1] == 't') return ident_trie(ident, 2, "ring", TOKEN_TO_STRING);
        }
        break;
    case 'k':
        if (size == 4) return ident_trie(ident, 1, "eys", TOKEN_KEYS);
        break;
    case 'c':
        if (size == 8) return ident_trie(ident, 1, "ontinue", TOKEN_CONTINUE);
        if (size == 4) {
            if (ident[1] == 'o') return ident_trie(ident, 2, "py", TOKEN_COPY);
            if (ident[1] == 'a') return ident_trie(ident, 2, "se", TOKEN_CASE);
        } else if (size == 5) {
            if (ident[1] == 'l') return ident_trie(ident, 2, "ear", TOKEN_CLEAR);
            if (ident[1] == 'o') return ident_trie(ident, 2, "nst", TOKEN_CONST);
        }
        break;
    case 'l':
        if (size == 3 and ident[1] == 'e') {
            if (ident[2] == 't') return TOKEN_LET;
            if (ident[2] == 'n') return TOKEN_LEN;
        }
        break;
    case 't':
        if (size == 3) return ident_trie(ident, 1, "ry", TOKEN_TRY);
        if (size == 4) {
            if (ident[1] == 'r') return ident_trie(ident, 2, "ue", TOKEN_TRUE);
            if (ident[1] == 'y') return ident_trie(ident, 2, "pe", TOKEN_TYPE);
        }
        break;
    case 'i':
        if (size == 3) return ident_trie(ident, 1, "nt", TOKEN_TO_INTEGER);
        if (size == 5) return ident_trie(ident, 1, "ndex", TOKEN_INDEX);
        if (size == 6) return ident_trie(ident, 1, "nsert", TOKEN_INSERT);
        if (size == 2) {
            if (ident[1] == 'f') return TOKEN_IF;
            if (ident[1] == 'n') return TOKEN_IN;
        }
        break;
    case 'p':
        if (size == 3) return ident_trie(ident, 1, "op", TOKEN_POP);
        if (size == 5) return ident_trie(ident, 1, "rint", TOKEN_PRINT);
        if (size == 4) {
            if (ident[1] == 'u') return ident_trie(ident, 2, "sh", TOKEN_PUSH);
            if (ident[1] == 'a') return ident_trie(ident, 2, "ss", TOKEN_PASS);
        }
        break;
    case 'e':
        if (size == 3) return ident_trie(ident, 1, "nd", TOKEN_END);
        if (size == 6) return ident_trie(ident, 1, "xcept", TOKEN_EXCEPT);
        if (size == 4 and ident[1] == 'l') {
            if (ident[2] == 's') {
                if (ident[3] == 'e') {
                    return TOKEN_ELSE;
                }
            } else if (ident[2] == 'i' and ident[3] == 'f') {
                return TOKEN_ELIF;
            }
        }
        break;
    case 'f':
        if (size == 3) return ident_trie(ident, 1, "or", TOKEN_FOR);
        if (size == 8) return ident_trie(ident, 1, "unction", TOKEN_FUNCTION);
        if (size == 5) {
            if (ident[1] == 'a') return ident_trie(ident, 2, "lse", TOKEN_FALSE);
            if (ident[1] == 'l') return ident_trie(ident, 2, "oat", TOKEN_TO_FLOAT);
        }
        break;
    }
    return TOKEN_UNDEFINED;
}

static void push_ident_token(Compiler *this, usize start, usize end) {
    char *ident = &this->source[start];
    usize size = end - start;
    enum TokenType keyword = ident_keyword(ident, size);
    if (keyword != TOKEN_UNDEFINED) {
        token(this, keyword);
        return;
    }
#ifdef HYMN_DEBUG_TOKEN
    printf("TOKEN: %s, %.*s\n", token_name(TOKEN_IDENT), (int)(end - start), &this->source[start]);
#endif
    Token *gamma = &this->gamma;
    gamma->row = this->row;
    gamma->column = this->column;
    gamma->type = TOKEN_IDENT;
    gamma->start = start;
    gamma->length = (int)(end - start);
}

static bool is_digit(char c) {
    return '0' <= c and c <= '9';
}

static bool is_ident(char c) {
    return ('a' <= c and c <= 'z') or ('A' <= c and c <= 'Z') or c == '_';
}

static void advance(Compiler *this) {
    this->alpha = this->beta;
    this->beta = this->gamma;
    if (this->beta.type == TOKEN_EOF) {
        return;
    }
    while (true) {
        char c = next_char(this);
        switch (c) {
        case '#':
            c = peek_char(this);
            while (c != '\n' and c != '\0') {
                next_char(this);
                c = peek_char(this);
            }
            continue;
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            c = peek_char(this);
            while (c != '\0' and (c == ' ' or c == '\t' or c == '\r' or c == '\n')) {
                next_char(this);
                c = peek_char(this);
            }
            continue;
        case '!':
            if (peek_char(this) == '=') {
                next_char(this);
                token(this, TOKEN_NOT_EQUAL);
            } else {
                token(this, TOKEN_NOT);
            }
            return;
        case '=':
            if (peek_char(this) == '=') {
                next_char(this);
                token(this, TOKEN_EQUAL);
            } else {
                token(this, TOKEN_ASSIGN);
            }
            return;
        case '>':
            if (peek_char(this) == '=') {
                next_char(this);
                token(this, TOKEN_GREATER_EQUAL);
            } else if (peek_char(this) == '>') {
                next_char(this);
                token(this, TOKEN_BIT_RIGHT_SHIFT);
            } else {
                token(this, TOKEN_GREATER);
            }
            return;
        case '<':
            if (peek_char(this) == '=') {
                next_char(this);
                token(this, TOKEN_LESS_EQUAL);
            } else if (peek_char(this) == '<') {
                next_char(this);
                token(this, TOKEN_BIT_LEFT_SHIFT);
            } else {
                token(this, TOKEN_LESS);
            }
            return;
        case '&': token(this, TOKEN_BIT_AND); return;
        case '|': token(this, TOKEN_BIT_OR); return;
        case '^': token(this, TOKEN_BIT_XOR); return;
        case '~': token(this, TOKEN_BIT_NOT); return;
        case '+': token(this, TOKEN_ADD); return;
        case '-': token(this, TOKEN_SUBTRACT); return;
        case '*': token(this, TOKEN_MULTIPLY); return;
        case '/': token(this, TOKEN_DIVIDE); return;
        case ',': token(this, TOKEN_COMMA); return;
        case '.': token(this, TOKEN_DOT); return;
        case '(': token(this, TOKEN_LEFT_PAREN); return;
        case ')': token(this, TOKEN_RIGHT_PAREN); return;
        case '[': token(this, TOKEN_LEFT_SQUARE); return;
        case ']': token(this, TOKEN_RIGHT_SQUARE); return;
        case '{': token(this, TOKEN_LEFT_CURLY); return;
        case '}': token(this, TOKEN_RIGHT_CURLY); return;
        case ':': token(this, TOKEN_COLON); return;
        case ';': token(this, TOKEN_SEMICOLON); return;
        case '\0': token(this, TOKEN_EOF); return;
        case '"': {
            usize start = this->pos;
            while (true) {
                c = next_char(this);
                if (c == '\\') {
                    next_char(this);
                    continue;
                } else if (c == '"' or c == '\0') {
                    break;
                }
            }
            usize end = this->pos - 1;
            string_token(this, start, end);
            return;
        }
        case '\'': {
            usize start = this->pos;
            while (true) {
                c = next_char(this);
                if (c == '\\') {
                    next_char(this);
                    continue;
                } else if (c == '\'' or c == '\0') {
                    break;
                }
            }
            usize end = this->pos - 1;
            string_token(this, start, end);
            return;
        }
        default: {
            if (is_digit(c)) {
                usize start = this->pos - 1;
                while (is_digit(peek_char(this))) {
                    next_char(this);
                }
                bool discrete = true;
                if (peek_char(this) == '.') {
                    discrete = false;
                    next_char(this);
                    while (is_digit(peek_char(this))) {
                        next_char(this);
                    }
                }
                usize end = this->pos;
                if (discrete) {
                    value_token(this, TOKEN_INTEGER, start, end);
                } else {
                    value_token(this, TOKEN_FLOAT, start, end);
                }
                return;
            } else if (is_ident(c)) {
                usize start = this->pos - 1;
                while (is_ident(peek_char(this))) {
                    next_char(this);
                }
                usize end = this->pos;
                push_ident_token(this, start, end);
                return;
            } else {
                compile_error(this, &this->beta, "Unknown character '%c'", c);
            }
        }
        }
    }
}

static void value_pool_init(ValuePool *this) {
    this->count = 0;
    this->capacity = 8;
    this->values = safe_malloc(8 * sizeof(Value));
}

static void value_pool_delete(ValuePool *this) {
    free(this->values);
}

static void value_pool_add(ValuePool *this, Value value) {
    if (this->count + 1 > this->capacity) {
        this->capacity *= 2;
        this->values = safe_realloc(this->values, this->capacity * sizeof(Value));
    }
    this->values[this->count] = value;
    this->count++;
}

static void byte_code_init(ByteCode *this) {
    this->count = 0;
    this->capacity = 8;
    this->instructions = safe_malloc(8 * sizeof(u8));
    this->rows = safe_malloc(8 * sizeof(int));
    value_pool_init(&this->constants);
}

static bool match_values(Value a, Value b) {
    if (a.is != b.is) {
        return false;
    }
    switch (a.is) {
    case VALUE_NONE: return true;
    case VALUE_BOOL: return as_bool(a) == as_bool(b);
    case VALUE_INTEGER: return as_int(a) == as_int(b);
    case VALUE_FLOAT: return as_float(a) == as_float(b);
    case VALUE_STRING: return as_string(a) == as_string(b);
    case VALUE_ARRAY: return as_array(a) == as_array(b);
    case VALUE_TABLE: return as_table(a) == as_table(b);
    case VALUE_FUNC: return as_func(a) == as_func(b);
    case VALUE_FUNC_NATIVE: return as_native(a) == as_native(b);
    }
    return false;
}

static Function *new_function() {
    Function *func = safe_malloc(sizeof(Function));
    func->arity = 0;
    func->name = NULL;
    byte_code_init(&func->code);
    return func;
}

static NativeFunction *new_native_function(String *name, NativeCall func) {
    NativeFunction *native = safe_malloc(sizeof(NativeFunction));
    native->name = name;
    native->func = func;
    return native;
}

static void array_init_with_capacity(Array *this, i64 length, i64 capacity) {
    if (capacity == 0) {
        this->items = NULL;
    } else {
        this->items = safe_calloc((usize)capacity, sizeof(Value));
    }
    this->length = length;
    this->capacity = capacity;
}

static void array_init(Array *this, i64 length) {
    array_init_with_capacity(this, length, length);
}

static Array *new_array_with_capacity(i64 length, i64 capacity) {
    Array *this = safe_malloc(sizeof(Array));
    array_init_with_capacity(this, length, capacity);
    return this;
}

static Array *new_array(i64 length) {
    return new_array_with_capacity(length, length);
}

static Array *new_array_slice(Array *from, i64 start, i64 end) {
    usize length = end - start;
    usize size = length * sizeof(Value);
    Array *this = safe_malloc(sizeof(Array));
    this->items = safe_malloc(size);
    memcpy(this->items, &from->items[start], size);
    this->length = length;
    this->capacity = length;
    return this;
}

static Array *new_array_copy(Array *from) {
    return new_array_slice(from, 0, from->length);
}

static void array_update_capacity(Array *this, i64 length) {
    if (length > this->capacity) {
        if (this->capacity == 0) {
            this->capacity = length;
            this->items = safe_calloc(length, sizeof(Value));
        } else {
            this->capacity = length * 2;
            this->items = safe_realloc(this->items, this->capacity * sizeof(Value));
            memset(this->items + this->length, 0, this->capacity - this->length);
        }
    }
}

static void array_push(Array *this, Value value) {
    i64 length = this->length + 1;
    array_update_capacity(this, length);
    this->length = length;
    this->items[length - 1] = value;
}

static void array_insert(Array *this, i64 index, Value value) {
    i64 length = this->length + 1;
    array_update_capacity(this, length);
    this->length = length;
    Value *items = this->items;
    for (i64 i = length - 1; i > index; i--) {
        items[i] = items[i - 1];
    }
    items[index] = value;
}

static Value array_get(Array *this, i64 index) {
    if (index >= this->length) {
        return new_undefined();
    }
    return this->items[index];
}

static i64 array_index_of(Array *this, Value match) {
    i64 len = this->length;
    Value *items = this->items;
    for (i64 i = 0; i < len; i++) {
        if (match_values(match, items[i])) {
            return i;
        }
    }
    return -1;
}

static Value array_pop(Array *this) {
    if (this->length == 0) {
        return new_none();
    }
    return this->items[--this->length];
}

static Value array_remove_index(Array *this, i64 index) {
    i64 len = --this->length;
    Value *items = this->items;
    Value deleted = items[index];
    for (i64 i = index; i < len; i++) {
        items[i] = items[i + 1];
    }
    return deleted;
}

static void array_clear(Array *this) {
    this->length = 0;
}

static ValueMap *new_map() {
    ValueMap *this = safe_calloc(1, sizeof(ValueMap));
    map_init(this);
    return this;
}

static ValueMap *new_map_copy(ValueMap *from) {
    (void *)from;
    LOG("TODO TABLE COPY");
    ValueMap *this = safe_calloc(1, sizeof(ValueMap));
    map_init(this);
    return this;
}

static Array *map_keys(ValueMap *this) {
    Array *array = new_array_with_capacity(0, this->size);

    unsigned int bin = 0;
    ValueMapItem *item = NULL;

    unsigned int bins = this->bins;
    for (unsigned int i = 0; i < bins; i++) {
        ValueMapItem *start = this->items[i];
        if (start) {
            bin = i;
            item = start;
            break;
        }
    }

    if (item == NULL) return array;
    array_push(array, new_string_value(item->key));

    while (true) {
        item = item->next;
        if (item == NULL) {
            for (bin = bin + 1; bin < bins; bin++) {
                ValueMapItem *start = this->items[bin];
                if (start) {
                    item = start;
                    break;
                }
            }
            if (item == NULL) return array;
        }
        array_push(array, new_string_value(item->key));
    }

    return array;
}

static String *map_key_of(ValueMap *this, Value match) {

    unsigned int bin = 0;
    ValueMapItem *item = NULL;

    unsigned int bins = this->bins;
    for (unsigned int i = 0; i < bins; i++) {
        ValueMapItem *start = this->items[i];
        if (start) {
            bin = i;
            item = start;
            break;
        }
    }

    if (item == NULL) return NULL;
    if (match_values(match, item->value)) return item->key;

    while (true) {
        item = item->next;
        if (item == NULL) {
            for (bin = bin + 1; bin < bins; bin++) {
                ValueMapItem *start = this->items[bin];
                if (start) {
                    item = start;
                    break;
                }
            }
            if (item == NULL) return NULL;
        }
        if (match_values(match, item->value)) return item->key;
    }
}

static void compiler_scope_init(Compiler *this, Scope *scope, enum FunctionType type) {
    scope->enclosing = this->scope;
    this->scope = scope;

    scope->local_count = 0;
    scope->depth = 0;
    scope->func = new_function();
    scope->type = type;

    if (type != TYPE_SCRIPT) {
        scope->func->name = new_string_from_substring(this->source, this->alpha.start, this->alpha.start + this->alpha.length);
    }

    Local *local = &scope->locals[scope->local_count++];
    local->depth = 0;
    local->name.start = 0;
    local->name.length = 0;
}

static inline Compiler new_compiler(char *source, Machine *machine, Scope *scope) {
    Compiler this = {0};
    this.row = 1;
    this.column = 1;
    this.source = source;
    this.size = strlen(source);
    this.alpha.type = TOKEN_UNDEFINED;
    this.beta.type = TOKEN_UNDEFINED;
    this.gamma.type = TOKEN_UNDEFINED;
    this.machine = machine;
    compiler_scope_init(&this, scope, TYPE_SCRIPT);
    return this;
}

static void byte_code_delete(ByteCode *this) {
    free(this->instructions);
    free(this->rows);
    value_pool_delete(&this->constants);
}

static int byte_code_add_constant(ByteCode *this, Value value) {
    value_pool_add(&this->constants, value);
    return this->constants.count - 1;
}

static void write_op(ByteCode *this, u8 b, int row) {
    int count = this->count;
    if (count + 1 > this->capacity) {
        this->capacity *= 2;
        this->instructions = safe_realloc(this->instructions, this->capacity * sizeof(u8));
        this->rows = safe_realloc(this->rows, this->capacity * sizeof(int));
    }
    this->instructions[count] = b;
    this->rows[count] = row;
    this->count = count + 1;
}

static void write_two_op(ByteCode *this, u8 b, u8 n, int row) {
    int count = this->count;
    while (count + 2 > this->capacity) {
        this->capacity *= 2;
        this->instructions = safe_realloc(this->instructions, this->capacity * sizeof(u8));
        this->rows = safe_realloc(this->rows, this->capacity * sizeof(int));
    }
    this->instructions[count] = b;
    this->instructions[count + 1] = n;
    this->rows[count] = row;
    this->rows[count + 1] = row;
    this->count = count + 2;
}

static void write_constant(ByteCode *this, Value value, int row) {
    u8 constant = (u8)byte_code_add_constant(this, value);
    write_two_op(this, OP_CONSTANT, constant, row);
}

static Rule *token_rule(enum TokenType type) {
    return &rules[type];
}

static String *intern_string(ValueMap *this, String *value) {
    Value exists = map_get(this, value);
    if (is_undefined(exists)) {
        map_put(this, value, new_string_value(value));
        return NULL;
    } else {
        return as_string(exists);
    }
}

static Value machine_intern_string(Machine *this, String *value) {
    String *intern = intern_string(&this->strings, value);
    if (intern != NULL) {
        string_delete(value);
        return new_string_value(intern);
    }
    return new_string_value(value);
}

static bool check(Compiler *this, enum TokenType type) {
    return this->beta.type == type;
}

static bool match(Compiler *this, enum TokenType type) {
    if (!check(this, type)) {
        return false;
    }
    advance(this);
    return true;
}

static inline void emit(Compiler *this, u8 b) {
    write_op(current(this), b, this->alpha.row);
}

static inline void emit_two(Compiler *this, u8 b, u8 n) {
    write_two_op(current(this), b, n, this->alpha.row);
}

static void compile_with_precedence(Compiler *this, enum Precedence precedence) {
    advance(this);
    Rule *rule = token_rule(this->alpha.type);
    void (*prefix)(Compiler *, bool) = rule->prefix;
    if (prefix == NULL) {
        compile_error(this, &this->alpha, "Expected expression.");
        return;
    }
    bool assign = precedence <= PRECEDENCE_ASSIGN;
    prefix(this, assign);
    while (precedence <= token_rule(this->beta.type)->precedence) {
        advance(this);
        void (*infix)(Compiler *, bool) = token_rule(this->alpha.type)->infix;
        if (infix == NULL) {
            compile_error(this, &this->alpha, "Expected infix.");
            return;
        }
        infix(this, assign);
    }
    if (assign and match(this, TOKEN_ASSIGN)) {
        compile_error(this, &this->beta, "Invalid assignment target.");
    }
}

static void consume(Compiler *this, enum TokenType type, const char *error) {
    if (this->beta.type == type) {
        advance(this);
        return;
    }
    compile_error(this, &this->beta, error);
}

static u8 arguments(Compiler *this) {
    u8 count = 0;
    if (!check(this, TOKEN_RIGHT_PAREN)) {
        do {
            expression(this);
            if (count == 255) {
                compile_error(this, &this->alpha, "Can't have more than 255 function arguments.");
                break;
            }
            count++;
        } while (match(this, TOKEN_COMMA));
    }
    consume(this, TOKEN_RIGHT_PAREN, "Expected ')' after function arguments.");
    return count;
}

static void compile_call(Compiler *this, bool assign) {
    (void)assign;
    u8 args = arguments(this);
    emit_two(this, OP_CALL, args);
}

static void compile_group(Compiler *this, bool assign) {
    (void)assign;
    expression(this);
    consume(this, TOKEN_RIGHT_PAREN, "Expected right parenthesis.");
}

static void compile_none(Compiler *this, bool assign) {
    (void)assign;
    emit(this, OP_NONE);
}

static void compile_true(Compiler *this, bool assign) {
    (void)assign;
    emit(this, OP_TRUE);
}

static void compile_false(Compiler *this, bool assign) {
    (void)assign;
    emit(this, OP_FALSE);
}

static void compile_integer(Compiler *this, bool assign) {
    (void)assign;
    Token *alpha = &this->alpha;
    i64 number = (i64)strtoll(&this->source[alpha->start], NULL, 10);
    write_constant(current(this), new_int(number), alpha->row);
}

static void compile_float(Compiler *this, bool assign) {
    (void)assign;
    Token *alpha = &this->alpha;
    double number = strtod(&this->source[alpha->start], NULL);
    write_constant(current(this), new_float(number), alpha->row);
}

static void compile_string(Compiler *this, bool assign) {
    (void)assign;
    Token *alpha = &this->alpha;
    String *s = new_string_from_substring(this->source, alpha->start, alpha->start + alpha->length);
    write_constant(current(this), machine_intern_string(this->machine, s), alpha->row);
}

static void compile_array(Compiler *this, bool assign) {
    (void)assign;
    consume(this, TOKEN_RIGHT_SQUARE, "Expected ']' declaring array.");
    Token *alpha = &this->alpha;
    Array *array = new_array(0);
    write_constant(current(this), new_array_value(array), alpha->row);
}

static void compile_table(Compiler *this, bool assign) {
    (void)assign;
    consume(this, TOKEN_RIGHT_CURLY, "Expected '}' declaring table.");
    Token *alpha = &this->alpha;
    ValueMap *table = new_map();
    write_constant(current(this), new_table_value(table), alpha->row);
}

static void function_delete(Function *this) {
    byte_code_delete(&this->code);
    free(this);
}

static void native_function_delete(NativeFunction *this) {
    free(this);
}

static void panic_halt(Compiler *this) {
    this->panic = false;
    while (true) {
        switch (this->beta.type) {
        case TOKEN_FOR:
        case TOKEN_IF:
        case TOKEN_WHILE:
        case TOKEN_PRINT:
        case TOKEN_POP:
        case TOKEN_PUSH:
        case TOKEN_INSERT:
        case TOKEN_DELETE:
        case TOKEN_LEN:
        case TOKEN_RETURN:
        case TOKEN_BEGIN:
        case TOKEN_EOF:
            return;
        }
        advance(this);
    }
}

static void begin_scope(Compiler *this) {
    this->scope->depth++;
}

static void end_scope(Compiler *this) {
    Scope *scope = this->scope;
    scope->depth--;
    while (scope->local_count > 0 and scope->locals[scope->local_count - 1].depth > scope->depth) {
        emit(this, OP_POP);
        scope->local_count--;
    }
}

static u8 ident_constant(Compiler *this, Token *token) {
    String *s = new_string_from_substring(this->source, token->start, token->start + token->length);
    return (u8)byte_code_add_constant(current(this), machine_intern_string(this->machine, s));
}

static void push_local(Compiler *this, Token name, bool constant) {
    Scope *scope = this->scope;
    if (scope->local_count == UINT8_COUNT) {
        compile_error(this, &name, "Too many local variables in scope.");
        return;
    }
    Local *local = &scope->locals[scope->local_count++];
    local->name = name;
    local->constant = constant;
    local->depth = -1;
}

static bool ident_match(Compiler *this, Token *a, Token *b) {
    if (a->length != b->length) {
        return false;
    }
    return memcmp(&this->source[a->start], &this->source[b->start], a->length) == 0;
}

static u8 variable(Compiler *this, bool constant, const char *error) {
    consume(this, TOKEN_IDENT, error);
    Scope *scope = this->scope;
    if (scope->depth == 0) {
        return ident_constant(this, &this->alpha);
    }
    Token *name = &this->alpha;
    for (int i = scope->local_count - 1; i >= 0; i--) {
        Local *local = &scope->locals[i];
        if (local->depth != -1 and local->depth < scope->depth) {
            break;
        } else if (ident_match(this, name, &local->name)) {
            compile_error(this, name, "Variable already exists in this scope.");
        }
    }
    push_local(this, *name, constant);
    return 0;
}

static void local_initialize(Compiler *this) {
    Scope *scope = this->scope;
    if (scope->depth == 0) {
        return;
    }
    scope->locals[scope->local_count - 1].depth = scope->depth;
}

static void finalize_variable(Compiler *this, u8 global) {
    if (this->scope->depth > 0) {
        local_initialize(this);
        return;
    }
    emit_two(this, OP_DEFINE_GLOBAL, global);
}

static void define_new_variable(Compiler *this, bool constant) {
    u8 global = variable(this, constant, "Expected variable name.");
    consume(this, TOKEN_ASSIGN, "Expected '=' after variable");
    expression(this);
    finalize_variable(this, global);
}

static int resolve_local(Compiler *this, Token *name, bool *constant) {
    Scope *scope = this->scope;
    for (int i = scope->local_count - 1; i >= 0; i--) {
        Local *local = &scope->locals[i];
        if (ident_match(this, name, &local->name)) {
            if (local->depth == -1) {
                compile_error(this, name, "Can't reference local variable before initializing.");
            }
            *constant = local->constant;
            return i;
        }
    }
    return -1;
}

static void named_variable(Compiler *this, Token token, bool assign) {
    u8 get;
    u8 set;
    bool constant = false;
    int var = resolve_local(this, &token, &constant);
    if (var != -1) {
        get = OP_GET_LOCAL;
        set = OP_SET_LOCAL;
    } else {
        get = OP_GET_GLOBAL;
        set = OP_SET_GLOBAL;
        var = ident_constant(this, &token);
        // todo: const for globals
        // globals are evaluated at runtime
        // during compile they're literally just the string reference
        // this will need to change in order to store that it's const or not
    }
    if (assign and match(this, TOKEN_ASSIGN)) {
        if (constant) {
            compile_error(this, &token, "Constant variable can't be modified.");
        }
        expression(this);
        emit(this, set);
    } else {
        emit(this, get);
    }
    emit(this, (u8)var);
}

static void compile_variable(Compiler *this, bool assign) {
    named_variable(this, this->alpha, assign);
}

static void compile_unary(Compiler *this, bool assign) {
    (void)assign;
    enum TokenType type = this->alpha.type;
    compile_with_precedence(this, PRECEDENCE_UNARY);
    switch (type) {
    case TOKEN_NOT: emit(this, OP_NOT); break;
    case TOKEN_SUBTRACT: emit(this, OP_NEGATE); break;
    case TOKEN_BIT_NOT: emit(this, OP_BIT_NOT); break;
    }
}

static void compile_binary(Compiler *this, bool assign) {
    (void)assign;
    enum TokenType type = this->alpha.type;
    Rule *rule = token_rule(type);
    compile_with_precedence(this, (enum Precedence)(rule->precedence + 1));
    switch (type) {
    case TOKEN_ADD: emit(this, OP_ADD); break;
    case TOKEN_SUBTRACT: emit(this, OP_SUBTRACT); break;
    case TOKEN_MULTIPLY: emit(this, OP_MULTIPLY); break;
    case TOKEN_DIVIDE: emit(this, OP_DIVIDE); break;
    case TOKEN_EQUAL: emit(this, OP_EQUAL); break;
    case TOKEN_NOT_EQUAL: emit(this, OP_NOT_EQUAL); break;
    case TOKEN_LESS: emit(this, OP_LESS); break;
    case TOKEN_LESS_EQUAL: emit(this, OP_LESS_EQUAL); break;
    case TOKEN_GREATER: emit(this, OP_GREATER); break;
    case TOKEN_GREATER_EQUAL: emit(this, OP_GREATER_EQUAL); break;
    case TOKEN_BIT_OR: emit(this, OP_BIT_OR); break;
    case TOKEN_BIT_AND: emit(this, OP_BIT_AND); break;
    case TOKEN_BIT_XOR: emit(this, OP_BIT_XOR); break;
    case TOKEN_BIT_LEFT_SHIFT: emit(this, OP_BIT_LEFT_SHIFT); break;
    case TOKEN_BIT_RIGHT_SHIFT: emit(this, OP_BIT_RIGHT_SHIFT); break;
    }
}

static void compile_dot(Compiler *this, bool assign) {
    consume(this, TOKEN_IDENT, "Expected property name after '.'.");
    u8 name = ident_constant(this, &this->alpha);
    if (assign and match(this, TOKEN_ASSIGN)) {
        expression(this);
        emit_two(this, OP_SET_PROPERTY, name);
    } else {
        emit_two(this, OP_GET_PROPERTY, name);
    }
}

static void compile_square(Compiler *this, bool assign) {
    if (match(this, TOKEN_COLON)) {
        write_constant(current(this), new_int(0), this->alpha.row);
        if (match(this, TOKEN_RIGHT_SQUARE)) {
            write_constant(current(this), new_none(), this->alpha.row);
        } else {
            expression(this);
            consume(this, TOKEN_RIGHT_SQUARE, "Expected ']' after expression.");
        }
        emit(this, OP_SLICE);
    } else {
        expression(this);
        if (match(this, TOKEN_COLON)) {
            if (match(this, TOKEN_RIGHT_SQUARE)) {
                write_constant(current(this), new_none(), this->alpha.row);
            } else {
                expression(this);
                consume(this, TOKEN_RIGHT_SQUARE, "Expected ']' after expression.");
            }
            emit(this, OP_SLICE);
        } else {
            consume(this, TOKEN_RIGHT_SQUARE, "Expected ']' after expression.");
            if (assign and match(this, TOKEN_ASSIGN)) {
                expression(this);
                emit(this, OP_SET_DYNAMIC);
            } else {
                emit(this, OP_GET_DYNAMIC);
            }
        }
    }
}

static int emit_jump(Compiler *this, u8 instruction) {
    emit(this, instruction);
    emit_two(this, UINT8_MAX, UINT8_MAX);
    return current(this)->count - 2;
}

static void patch_jump(Compiler *this, int jump) {
    ByteCode *code = current(this);
    int offset = code->count - jump - 2;
    if (offset > UINT16_MAX) {
        compile_error(this, &this->alpha, "Jump offset too large.");
        return;
    }
    code->instructions[jump] = (offset >> 8) & UINT8_MAX;
    code->instructions[jump + 1] = offset & UINT8_MAX;
}

static void compile_and(Compiler *this, bool assign) {
    (void)assign;
    int jump = emit_jump(this, OP_JUMP_IF_FALSE);
    emit(this, OP_POP);
    compile_with_precedence(this, PRECEDENCE_AND);
    patch_jump(this, jump);
}

static void compile_or(Compiler *this, bool assign) {
    (void)assign;
    int jump_else = emit_jump(this, OP_JUMP_IF_FALSE);
    int jump = emit_jump(this, OP_JUMP);
    patch_jump(this, jump_else);
    emit(this, OP_POP);
    compile_with_precedence(this, PRECEDENCE_OR);
    patch_jump(this, jump);
}

static Function *end_function(Compiler *this) {
    emit_two(this, OP_NONE, OP_RETURN);
    Function *func = this->scope->func;
    this->scope = this->scope->enclosing;
    return func;
}

static void compile_function(Compiler *this, enum FunctionType type) {
    Scope scope = {0};
    compiler_scope_init(this, &scope, type);

    begin_scope(this);

    consume(this, TOKEN_LEFT_PAREN, "Expected '(' after function name.");

    if (!check(this, TOKEN_RIGHT_PAREN)) {
        do {
            this->scope->func->arity++;
            if (this->scope->func->arity > 255) {
                compile_error(this, &this->alpha, "Can't have more than 255 function parameters.");
            }
            u8 parameter = variable(this, false, "Expected parameter name.");
            finalize_variable(this, parameter);
        } while (match(this, TOKEN_COMMA));
    }

    consume(this, TOKEN_RIGHT_PAREN, "Expected ')' after function parameters.");

    while (!check(this, TOKEN_END) and !check(this, TOKEN_EOF)) {
        declaration(this);
    }

    end_scope(this);
    consume(this, TOKEN_END, "Expected 'end' after function body.");

    Function *func = end_function(this);

    write_constant(current(this), new_func(func), this->alpha.row);
}

static void declare_function(Compiler *this) {
    u8 global = variable(this, false, "Expected function name.");
    local_initialize(this);
    compile_function(this, TYPE_FUNCTION);
    finalize_variable(this, global);
}

static void declaration(Compiler *this) {
    if (match(this, TOKEN_LET)) {
        define_new_variable(this, false);
    } else if (match(this, TOKEN_CONST)) {
        define_new_variable(this, true);
    } else if (match(this, TOKEN_FUNCTION)) {
        declare_function(this);
    } else {
        statement(this);
    }
}

static void block(Compiler *this) {
    begin_scope(this);
    while (!check(this, TOKEN_END) and !check(this, TOKEN_EOF)) {
        declaration(this);
    }
    end_scope(this);
}

static void if_statement(Compiler *this) {
    expression(this);
    int jump = emit_jump(this, OP_JUMP_IF_FALSE);

    emit(this, OP_POP);
    begin_scope(this);
    while (!check(this, TOKEN_ELIF) and !check(this, TOKEN_ELSE) and !check(this, TOKEN_END) and !check(this, TOKEN_EOF)) {
        declaration(this);
    }
    end_scope(this);

    struct JumpList jump_end = {0};
    jump_end.jump = emit_jump(this, OP_JUMP);
    struct JumpList *tail = &jump_end;

    while (match(this, TOKEN_ELIF)) {

        patch_jump(this, jump);
        emit(this, OP_POP);

        expression(this);
        jump = emit_jump(this, OP_JUMP_IF_FALSE);

        emit(this, OP_POP);
        begin_scope(this);
        while (!check(this, TOKEN_ELIF) and !check(this, TOKEN_ELSE) and !check(this, TOKEN_END) and !check(this, TOKEN_EOF)) {
            declaration(this);
        }
        end_scope(this);

        struct JumpList *next = safe_malloc(sizeof(struct JumpList));
        next->jump = emit_jump(this, OP_JUMP);
        next->next = NULL;

        tail->next = next;
        tail = next;
    }

    patch_jump(this, jump);
    emit(this, OP_POP);

    if (match(this, TOKEN_ELSE)) {
        block(this);
    }

    patch_jump(this, jump_end.jump);
    struct JumpList *current = jump_end.next;
    while (current != NULL) {
        patch_jump(this, current->jump);
        struct JumpList *next = current->next;
        free(current);
        current = next;
    }

    consume(this, TOKEN_END, "Expected 'end' after if statement.");
}

static bool match_literal(Compiler *this) {
    switch (this->beta.type) {
    case TOKEN_NONE:
    case TOKEN_TRUE:
    case TOKEN_FALSE:
    case TOKEN_INTEGER:
    case TOKEN_FLOAT:
    case TOKEN_STRING:
        advance(this);
        return true;
    }
    return false;
}

static void switch_statement(Compiler *this) {
    expression(this);

    while (match(this, TOKEN_CASE)) {
        if (!match_literal(this)) {
            compile_error(this, &this->beta, "Expected literal for case.");
        }
        while (match(this, TOKEN_OR)) {
            if (!match_literal(this)) {
                compile_error(this, &this->beta, "Expected literal after 'or' in case.");
            }
        }
        begin_scope(this);
        while (!check(this, TOKEN_CASE) and !check(this, TOKEN_ELSE) and !check(this, TOKEN_END) and !check(this, TOKEN_EOF)) {
            declaration(this);
        }
        end_scope(this);
    }

    if (match(this, TOKEN_ELSE)) {
        begin_scope(this);
        while (!check(this, TOKEN_END) and !check(this, TOKEN_EOF)) {
            declaration(this);
        }
        end_scope(this);
    }

    consume(this, TOKEN_END, "Expected 'end' after switch statement.");
}

static void emit_loop(Compiler *this, int start) {
    emit(this, OP_LOOP);
    int offset = current(this)->count - start + 2;
    if (offset > UINT16_MAX) {
        compile_error(this, &this->alpha, "Loop is too large.");
    }
    emit_two(this, (offset >> 8) & UINT8_MAX, offset & UINT8_MAX);
}

static void for_statement(Compiler *this) {
    begin_scope(this);

    if (match(this, TOKEN_LET)) {
        define_new_variable(this, false);
    } else if (match(this, TOKEN_CONST)) {
        define_new_variable(this, true);
    } else if (!check(this, TOKEN_SEMICOLON)) {
        expression_statement(this);
    }

    consume(this, TOKEN_SEMICOLON, "Expected ';' in for.");

    int start = current(this)->count;
    int jump = -1;

    if (!check(this, TOKEN_SEMICOLON)) {
        expression(this);

        jump = emit_jump(this, OP_JUMP_IF_FALSE);
        emit(this, OP_POP);
    }

    consume(this, TOKEN_SEMICOLON, "Expected ';' in for.");

    int body = emit_jump(this, OP_JUMP);
    int increment = current(this)->count;

    expression(this);

    emit(this, OP_POP);
    emit_loop(this, start);
    start = increment;
    patch_jump(this, body);

    block(this);

    emit_loop(this, start);

    if (jump != -1) {
        patch_jump(this, jump);
        emit(this, OP_POP);
    }

    end_scope(this);
    consume(this, TOKEN_END, "Expected 'end' after for loop.");
}

static void while_statement(Compiler *this) {
    int start = current(this)->count;

    struct LoopList *loop_next = this->loop;
    struct LoopList *loop = safe_malloc(sizeof(struct LoopList));
    loop->start = start;
    loop->next = loop_next;
    this->loop = loop;

    expression(this);
    int jump = emit_jump(this, OP_JUMP_IF_FALSE);

    emit(this, OP_POP);
    block(this);
    emit_loop(this, start);

    free(loop);
    this->loop = loop_next;

    patch_jump(this, jump);
    emit(this, OP_POP);

    struct JumpList *jump_link = this->jump;
    this->jump = NULL;
    while (jump_link != NULL) {
        patch_jump(this, jump_link->jump);
        struct JumpList *next = jump_link->next;
        free(jump_link);
        jump_link = next;
    }

    consume(this, TOKEN_END, "Expected 'end' after while loop.");
}

static void return_statement(Compiler *this) {
    if (this->scope->type == TYPE_SCRIPT) {
        compile_error(this, &this->alpha, "Can't return from outside a function.");
    }
    expression(this);
    emit(this, OP_RETURN);
}

static void break_statement(Compiler *this) {
    if (this->loop == NULL) {
        compile_error(this, &this->alpha, "Can't use 'break' outside of a loop.");
    }
    struct JumpList *jump_next = this->jump;
    struct JumpList *jump = safe_malloc(sizeof(struct JumpList));
    jump->jump = emit_jump(this, OP_JUMP);
    jump->next = jump_next;
    this->jump = jump;
}

static void continue_statement(Compiler *this) {
    if (this->loop == NULL) {
        compile_error(this, &this->alpha, "Can't use 'continue' outside of a loop.");
    }
    emit_loop(this, this->loop->start);
}

static void print_statement(Compiler *this) {
    consume(this, TOKEN_LEFT_PAREN, "Expected '(' after print.");
    expression(this);
    consume(this, TOKEN_RIGHT_PAREN, "Expected ')' after print expression.");
    emit(this, OP_PRINT);
}

static void use_statement(Compiler *this) {
    consume(this, TOKEN_STRING, "Expected file path after use.");
}

static void statement(Compiler *this) {
    if (match(this, TOKEN_PRINT)) {
        print_statement(this);
    } else if (match(this, TOKEN_USE)) {
        use_statement(this);
    } else if (match(this, TOKEN_IF)) {
        if_statement(this);
    } else if (match(this, TOKEN_SWITCH)) {
        switch_statement(this);
    } else if (match(this, TOKEN_FOR)) {
        for_statement(this);
    } else if (match(this, TOKEN_WHILE)) {
        while_statement(this);
    } else if (match(this, TOKEN_RETURN)) {
        return_statement(this);
    } else if (match(this, TOKEN_BREAK)) {
        break_statement(this);
    } else if (match(this, TOKEN_CONTINUE)) {
        continue_statement(this);
    } else if (match(this, TOKEN_PASS)) {
        // do nothing
    } else if (match(this, TOKEN_BEGIN)) {
        block(this);
        consume(this, TOKEN_END, "Expected 'end' after block.");
    } else {
        expression_statement(this);
    }
    if (this->panic) {
        panic_halt(this);
    }
}

static void array_push_expression(Compiler *this, bool assign) {
    (void)assign;
    consume(this, TOKEN_LEFT_PAREN, "Expected '(' after push.");
    expression(this);
    consume(this, TOKEN_COMMA, "Expected ',' between push arguments.");
    expression(this);
    consume(this, TOKEN_RIGHT_PAREN, "Expected ')' after push expression.");
    emit(this, OP_ARRAY_PUSH);
}

static void array_insert_expression(Compiler *this, bool assign) {
    (void)assign;
    consume(this, TOKEN_LEFT_PAREN, "Expected '(' after insert.");
    expression(this);
    consume(this, TOKEN_COMMA, "Expected ',' between insert arguments.");
    expression(this);
    consume(this, TOKEN_COMMA, "Expected ',' between insert arguments.");
    expression(this);
    consume(this, TOKEN_RIGHT_PAREN, "Expected ')' after insert expression.");
    emit(this, OP_ARRAY_INSERT);
}

static void array_pop_expression(Compiler *this, bool assign) {
    (void)assign;
    consume(this, TOKEN_LEFT_PAREN, "Expected '(' after pop.");
    expression(this);
    consume(this, TOKEN_RIGHT_PAREN, "Expected ')' after pop expression.");
    emit(this, OP_ARRAY_POP);
}

static void delete_expression(Compiler *this, bool assign) {
    (void)assign;
    consume(this, TOKEN_LEFT_PAREN, "Expected '(' after delete.");
    expression(this);
    consume(this, TOKEN_COMMA, "Expected ',' between delete arguments.");
    expression(this);
    consume(this, TOKEN_RIGHT_PAREN, "Expected ')' after delete expression.");
    emit(this, OP_DELETE);
}

static void len_expression(Compiler *this, bool assign) {
    (void)assign;
    consume(this, TOKEN_LEFT_PAREN, "Expected '(' after len.");
    expression(this);
    consume(this, TOKEN_RIGHT_PAREN, "Expected ')' after len expression.");
    emit(this, OP_LEN);
}

static void cast_integer_expression(Compiler *this, bool assign) {
    (void)assign;
    consume(this, TOKEN_LEFT_PAREN, "Expected '(' after integer.");
    expression(this);
    consume(this, TOKEN_RIGHT_PAREN, "Expected ')' after integer expression.");
    emit(this, OP_TO_INTEGER);
}

static void cast_float_expression(Compiler *this, bool assign) {
    (void)assign;
    consume(this, TOKEN_LEFT_PAREN, "Expected '(' after float.");
    expression(this);
    consume(this, TOKEN_RIGHT_PAREN, "Expected ')' after float expression.");
    emit(this, OP_TO_FLOAT);
}

static void cast_string_expression(Compiler *this, bool assign) {
    (void)assign;
    consume(this, TOKEN_LEFT_PAREN, "Expected '(' after string.");
    expression(this);
    consume(this, TOKEN_RIGHT_PAREN, "Expected ')' after string expression.");
    emit(this, OP_TO_STRING);
}

static void type_expression(Compiler *this, bool assign) {
    (void)assign;
    consume(this, TOKEN_LEFT_PAREN, "Expected '(' after type.");
    expression(this);
    consume(this, TOKEN_RIGHT_PAREN, "Expected ')' after type expression.");
    emit(this, OP_TYPE);
}

static void clear_expression(Compiler *this, bool assign) {
    (void)assign;
    consume(this, TOKEN_LEFT_PAREN, "Expected '(' after clear.");
    expression(this);
    consume(this, TOKEN_RIGHT_PAREN, "Expected ')' after clear expression.");
    emit(this, OP_CLEAR);
}

static void copy_expression(Compiler *this, bool assign) {
    (void)assign;
    consume(this, TOKEN_LEFT_PAREN, "Expected '(' after copy.");
    expression(this);
    consume(this, TOKEN_RIGHT_PAREN, "Expected ')' after copy expression.");
    emit(this, OP_COPY);
}

static void keys_expression(Compiler *this, bool assign) {
    (void)assign;
    consume(this, TOKEN_LEFT_PAREN, "Expected '(' after keys.");
    expression(this);
    consume(this, TOKEN_RIGHT_PAREN, "Expected ')' after keys expression.");
    emit(this, OP_KEYS);
}

static void index_expression(Compiler *this, bool assign) {
    (void)assign;
    consume(this, TOKEN_LEFT_PAREN, "Expected '(' after index.");
    expression(this);
    consume(this, TOKEN_COMMA, "Expected ',' between index arguments.");
    expression(this);
    consume(this, TOKEN_RIGHT_PAREN, "Expected ')' after index expression.");
    emit(this, OP_INDEX);
}

static void expression_statement(Compiler *this) {
    expression(this);
    emit(this, OP_POP);
}

static void expression(Compiler *this) {
    compile_with_precedence(this, PRECEDENCE_ASSIGN);
}

static Function *compile(Machine *machine, char *source, char **error) {

    Scope scope = {0};

    Compiler c = new_compiler(source, machine, &scope);
    Compiler *compiler = &c;

    advance(compiler);
    advance(compiler);
    while (!match(compiler, TOKEN_EOF)) {
        declaration(compiler);
    }

    if (compiler->error) {
        *error = string_to_chars(compiler->error);
    }

    Function *func = end_function(compiler);

    compiler_delete(compiler);

    return func;
}

#ifdef HYMN_DEBUG_TRACE
static usize debug_constant_instruction(const char *name, ByteCode *this, usize index) {
    u8 constant = this->instructions[index + 1];
    printf("%s: [%d: ", name, constant);
    debug_value(this->constants.values[constant]);
    printf("]\n");
    return index + 2;
}

static usize debug_byte_instruction(const char *name, ByteCode *this, usize index) {
    u8 b = this->instructions[index + 1];
    printf("%s: [%d]\n", name, b);
    return index + 2;
}

static usize debug_jump_instruction(const char *name, int sign, ByteCode *this, usize index) {
    u16 jump = (u16)(this->instructions[index + 1] << 8) | (u16)this->instructions[index + 2];
    printf("%s: [%zu] -> [%zu]\n", name, index, index + 3 + sign * jump);
    return index + 3;
}

static usize debug_instruction(const char *name, usize index) {
    printf("%s\n", name);
    return index + 1;
}

static usize disassemble_instruction(ByteCode *this, usize index) {
    printf("%04zu ", index);
    if (index > 0 and this->rows[index] == this->rows[index - 1]) {
        printf("   | ");
    } else {
        printf("%4d ", this->rows[index]);
    }
    u8 op = this->instructions[index];
    switch (op) {
    case OP_RETURN: return debug_instruction("OP_RETURN", index);
    case OP_ADD: return debug_instruction("OP_ADD", index);
    case OP_SUBTRACT: return debug_instruction("OP_SUBTRACT", index);
    case OP_MULTIPLY: return debug_instruction("OP_MULTIPLY", index);
    case OP_DIVIDE: return debug_instruction("OP_DIVIDE", index);
    case OP_NEGATE: return debug_instruction("OP_NEGATE", index);
    case OP_TRUE: return debug_instruction("OP_TRUE", index);
    case OP_FALSE: return debug_instruction("OP_FALSE", index);
    case OP_NONE: return debug_instruction("OP_NONE", index);
    case OP_NOT: return debug_instruction("OP_NOT", index);
    case OP_EQUAL: return debug_instruction("OP_EQUAL", index);
    case OP_NOT_EQUAL: return debug_instruction("OP_NOT_EQUAL", index);
    case OP_GREATER: return debug_instruction("OP_GREATER", index);
    case OP_GREATER_EQUAL: return debug_instruction("OP_GREATER_EQUAL", index);
    case OP_LESS: return debug_instruction("OP_LESS", index);
    case OP_LESS_EQUAL: return debug_instruction("OP_LESS_EQUAL", index);
    case OP_PRINT: return debug_instruction("OP_PRINT", index);
    case OP_ARRAY_POP: return debug_instruction("OP_ARRAY_POP", index);
    case OP_ARRAY_PUSH: return debug_instruction("OP_ARRAY_PUSH", index);
    case OP_ARRAY_INSERT: return debug_instruction("OP_ARRAY_INSERT", index);
    case OP_DELETE: return debug_instruction("OP_DELETE", index);
    case OP_TYPE: return debug_instruction("OP_TYPE", index);
    case OP_TO_INTEGER: return debug_instruction("OP_TO_INTEGER", index);
    case OP_TO_FLOAT: return debug_instruction("OP_TO_FLOAT", index);
    case OP_TO_STRING: return debug_instruction("OP_TO_STRING", index);
    case OP_LEN: return debug_instruction("OP_LEN", index);
    case OP_POP: return debug_instruction("OP_POP", index);
    case OP_CLEAR: return debug_instruction("OP_CLEAR", index);
    case OP_COPY: return debug_instruction("OP_COPY", index);
    case OP_SLICE: return debug_instruction("OP_SLICE", index);
    case OP_INDEX: return debug_instruction("OP_INDEX", index);
    case OP_KEYS: return debug_instruction("OP_KEYS", index);
    case OP_BIT_OR: return debug_instruction("OP_BIT_OR", index);
    case OP_BIT_NOT: return debug_instruction("OP_BIT_NOT", index);
    case OP_BIT_AND: return debug_instruction("OP_BIT_AND", index);
    case OP_BIT_XOR: return debug_instruction("OP_BIT_XOR", index);
    case OP_BIT_LEFT_SHIFT: return debug_instruction("OP_BIT_LEFT_SHIFT", index);
    case OP_BIT_RIGHT_SHIFT: return debug_instruction("OP_BIT_RIGHT_SHIFT", index);
    case OP_SET_DYNAMIC: return debug_instruction("OP_SET_DYNAMIC", index);
    case OP_GET_DYNAMIC: return debug_instruction("OP_GET_DYNAMIC", index);
    case OP_LOOP: return debug_jump_instruction("OP_LOOP", -1, this, index);
    case OP_JUMP: return debug_jump_instruction("OP_JUMP", 1, this, index);
    case OP_JUMP_IF_FALSE: return debug_jump_instruction("OP_JUMP_IF_FALSE", 1, this, index);
    case OP_CONSTANT: return debug_constant_instruction("OP_CONSTANT", this, index);
    case OP_DEFINE_GLOBAL: return debug_constant_instruction("OP_DEFINE_GLOBAL", this, index);
    case OP_SET_GLOBAL: return debug_constant_instruction("OP_SET_GLOBAL", this, index);
    case OP_GET_GLOBAL: return debug_constant_instruction("OP_GET_GLOBAL", this, index);
    case OP_SET_LOCAL: return debug_byte_instruction("OP_SET_LOCAL", this, index);
    case OP_GET_LOCAL: return debug_byte_instruction("OP_GET_LOCAL", this, index);
    case OP_SET_PROPERTY: return debug_constant_instruction("OP_SET_PROPERTY", this, index);
    case OP_GET_PROPERTY: return debug_constant_instruction("OP_GET_PROPERTY", this, index);
    case OP_CALL: return debug_byte_instruction("OP_CALL", this, index);
    default: printf("UNKNOWN OPCODE %d\n", op); return index + 1;
    }
}
#endif

static void machine_reset_stack(Machine *this) {
    this->stack_top = 0;
    this->frame_count = 0;
}

static void machine_runtime_error(Machine *this, const char *format, ...) {
    if (this->error == NULL) {
        this->error = new_string("");
    }

    va_list ap;
    va_start(ap, format);
    int len = vsnprintf(NULL, 0, format, ap);
    va_end(ap);
    char *chars = safe_malloc((len + 1) * sizeof(char));
    va_start(ap, format);
    len = vsnprintf(chars, len + 1, format, ap);
    va_end(ap);
    this->error = string_append(this->error, chars);
    free(chars);

    this->error = string_append_char(this->error, '\n');

    for (int i = this->frame_count - 1; i >= 0; i--) {
        Frame *frame = &this->frames[i];
        Function *func = frame->func;
        usize ip = frame->ip - 1;
        int row = frame->func->code.rows[ip];

        this->error = string_append_format(this->error, "[Line %d] in ", row);
        if (func->name == NULL) {
            this->error = string_append_format(this->error, "script\n");
        } else {
            this->error = string_append_format(this->error, "%s()\n", func->name);
        }
    }

    machine_reset_stack(this);
}

static void machine_push(Machine *this, Value value) {
    this->stack[this->stack_top++] = value;
}

static Value machine_peek(Machine *this, int dist) {
    if (dist > this->stack_top) {
        machine_runtime_error(this, "Nothing on stack to peek");
        return new_none();
    }
    return this->stack[this->stack_top - dist];
}

static Value machine_pop(Machine *this) {
    if (this->stack_top == 0) {
        machine_runtime_error(this, "Nothing on stack to pop");
        return new_none();
    }
    return this->stack[--this->stack_top];
}

static bool machine_equal(Value a, Value b) {
    switch (a.is) {
    case VALUE_NONE: return is_none(b);
    case VALUE_BOOL: return is_bool(b) ? as_bool(a) == as_bool(b) : false;
    case VALUE_INTEGER:
        switch (b.is) {
        case VALUE_INTEGER: return as_int(a) == as_int(b);
        case VALUE_FLOAT: return (double)as_int(a) == as_float(b);
        default: return false;
        }
    case VALUE_FLOAT:
        switch (b.is) {
        case VALUE_INTEGER: return as_float(a) == (double)as_int(b);
        case VALUE_FLOAT: return as_float(a) == as_float(b);
        default: return false;
        }
    case VALUE_STRING:
        switch (b.is) {
        case VALUE_STRING: return as_string(a) == as_string(b);
        default: return false;
        }
    case VALUE_ARRAY:
        switch (b.is) {
        case VALUE_ARRAY: return as_array(a) == as_array(b);
        default: return false;
        }
    case VALUE_TABLE:
        switch (b.is) {
        case VALUE_TABLE: return as_table(a) == as_table(b);
        default: return false;
        }
    case VALUE_FUNC:
        switch (b.is) {
        case VALUE_FUNC: return as_func(a) == as_func(b);
        default: return false;
        }
    case VALUE_FUNC_NATIVE:
        switch (b.is) {
        case VALUE_FUNC_NATIVE: return as_native(a) == as_native(b);
        default: return false;
        }
    default: return false;
    }
}

static bool machine_false(Value value) {
    switch (value.is) {
    case VALUE_NONE: return true;
    case VALUE_BOOL: return !as_bool(value);
    case VALUE_INTEGER: return as_int(value) == 0;
    case VALUE_FLOAT: return as_float(value) == 0.0;
    case VALUE_STRING: return string_len(as_string(value)) == 0;
    case VALUE_ARRAY: return as_array(value) == NULL;
    case VALUE_TABLE: return as_table(value) == NULL;
    case VALUE_FUNC: return as_func(value) == NULL;
    case VALUE_FUNC_NATIVE: return as_native(value) == NULL;
    default: return false;
    }
}

static bool machine_call(Machine *this, Function *func, int count) {
    if (count != func->arity) {
        machine_runtime_error(this, "Expected %d function arguments but found %d.", func->arity, count);
        return false;
    }

    if (this->frame_count == HYMN_FRAMES_MAX) {
        machine_runtime_error(this, "Stack overflow.");
        return false;
    }

    Frame *frame = &this->frames[this->frame_count++];
    frame->func = func;
    frame->ip = 0;
    frame->stack_top = this->stack_top - count - 1;

    return true;
}

static bool machine_call_value(Machine *this, Value call, int count) {
    switch (call.is) {
    case VALUE_FUNC:
        return machine_call(this, as_func(call), count);
    case VALUE_FUNC_NATIVE: {
        NativeCall func = as_native(call)->func;
        Value result = func(count, &this->stack[this->stack_top - count]);
        this->stack_top -= count + 1;
        machine_push(this, result);
        return true;
    }
    default:
        machine_runtime_error(this, "Only functions can be called.");
        return false;
    }
}

static inline u8 read_byte(Frame *frame) {
    return frame->func->code.instructions[frame->ip++];
}

static inline u16 read_short(Frame *frame) {
    frame->ip += 2;
    return ((u16)frame->func->code.instructions[frame->ip - 2] << 8) | (u16)frame->func->code.instructions[frame->ip - 1];
}

static inline Value read_constant(Frame *frame) {
    return frame->func->code.constants.values[read_byte(frame)];
}

static void machine_run(Machine *this) {
    Frame *frame = &this->frames[this->frame_count - 1];
    while (true) {
#ifdef HYMN_DEBUG_STACK
        if (this->stack_top > 0) {
            printf("STACK ================================================== ");
            for (usize i = 0; i < this->stack_top; i++) {
                printf("[%zu: ", i);
                debug_value(this->stack[i]);
                printf("] ");
            }
            printf("\n");
        }
#endif
#ifdef HYMN_DEBUG_TRACE
        disassemble_instruction(&frame->func->code, frame->ip);
#endif
        u8 op = read_byte(frame);
        switch (op) {
        case OP_RETURN:
            Value result = machine_pop(this);
            this->frame_count--;
            if (this->frame_count == 0) {
                machine_pop(this);
                return;
            }
            this->stack_top = frame->stack_top;
            machine_push(this, result);
            frame = &this->frames[this->frame_count - 1];
            break;
        case OP_POP:
            machine_pop(this);
            break;
        case OP_TRUE:
            machine_push(this, new_bool(true));
            break;
        case OP_FALSE:
            machine_push(this, new_bool(false));
            break;
        case OP_NONE:
            machine_push(this, new_none());
            break;
        case OP_CALL: {
            int count = read_byte(frame);
            Value value = machine_peek(this, count + 1);
            if (!machine_call_value(this, value, count)) {
                return;
            }
            frame = &this->frames[this->frame_count - 1];
            break;
        }
        case OP_JUMP: {
            u16 jump = read_short(frame);
            frame->ip += jump;
            break;
        }
        case OP_JUMP_IF_FALSE: {
            u16 jump = read_short(frame);
            if (machine_false(machine_peek(this, 1))) {
                frame->ip += jump;
            }
            break;
        }
        case OP_LOOP: {
            u16 jump = read_short(frame);
            frame->ip -= jump;
            break;
        }
        case OP_EQUAL: {
            Value b = machine_pop(this);
            Value a = machine_pop(this);
            machine_push(this, new_bool(machine_equal(a, b)));
            break;
        }
        case OP_NOT_EQUAL: {
            Value b = machine_pop(this);
            Value a = machine_pop(this);
            machine_push(this, new_bool(!machine_equal(a, b)));
            break;
        }
        case OP_LESS: {
            COMPARE_OP(<);
            break;
        }
        case OP_LESS_EQUAL: {
            COMPARE_OP(<=);
            break;
        }
        case OP_GREATER: {
            COMPARE_OP(>);
            break;
        }
        case OP_GREATER_EQUAL: {
            COMPARE_OP(>=);
            break;
        }
        case OP_ADD: {
            Value b = machine_pop(this);
            Value a = machine_pop(this);
            if (is_none(a)) {
                if (is_string(b)) {
                    String *temp = new_string(STRING_NONE);
                    String *add = string_concat(temp, string_copy(as_string(b)));
                    string_delete(temp);
                    machine_push(this, machine_intern_string(this, add));
                } else {
                    machine_runtime_error(this, "Operands can't be added.");
                    return;
                }
            } else if (is_bool(a)) {
                if (is_string(b)) {
                    String *temp = new_string(as_bool(a) ? STRING_TRUE : STRING_FALSE);
                    String *add = string_concat(temp, string_copy(as_string(b)));
                    string_delete(temp);
                    machine_push(this, machine_intern_string(this, add));
                } else {
                    machine_runtime_error(this, "Operands can't be added.");
                    return;
                }
            } else if (is_int(a)) {
                if (is_int(b)) {
                    a.as.i += b.as.i;
                    machine_push(this, a);
                } else if (is_float(b)) {
                    b.as.f += a.as.i;
                    machine_push(this, a);
                } else if (is_string(b)) {
                    String *temp = int64_to_string(as_int(a));
                    String *add = string_concat(temp, as_string(b));
                    string_delete(temp);
                    machine_push(this, machine_intern_string(this, add));
                } else {
                    machine_runtime_error(this, "Operands can't be added.");
                    return;
                }
            } else if (is_float(a)) {
                if (is_int(b)) {
                    a.as.f += b.as.i;
                    machine_push(this, a);
                } else if (is_float(b)) {
                    a.as.f += b.as.f;
                    machine_push(this, a);
                } else if (is_string(b)) {
                    String *temp = float64_to_string(as_float(a));
                    String *add = string_concat(temp, as_string(b));
                    string_delete(temp);
                    machine_push(this, machine_intern_string(this, add));
                } else {
                    machine_runtime_error(this, "Operands can't be added.");
                    return;
                }
            } else if (is_string(a)) {
                String *s = as_string(a);
                String *add = NULL;
                switch (b.is) {
                case VALUE_NONE:
                    add = string_append(string_copy(s), STRING_NONE);
                    break;
                case VALUE_BOOL:
                    add = string_append(string_copy(s), as_bool(b) ? STRING_TRUE : STRING_FALSE);
                    break;
                case VALUE_INTEGER: {
                    String *temp = int64_to_string(as_int(b));
                    add = string_concat(s, temp);
                    string_delete(temp);
                    break;
                }
                case VALUE_FLOAT: {
                    String *temp = float64_to_string(as_float(b));
                    add = string_concat(s, temp);
                    string_delete(temp);
                    break;
                }
                case VALUE_STRING:
                    add = string_concat(s, as_string(b));
                    break;
                case VALUE_ARRAY: {
                    String *temp = new_string("[array ");
                    temp = string_append(temp, pointer_to_string(as_array(b)));
                    temp = string_append_char(temp, ']');
                    add = string_concat(s, temp);
                    string_delete(temp);
                    break;
                }
                case VALUE_TABLE: {
                    String *temp = new_string("[table ");
                    temp = string_append(temp, pointer_to_string(as_table(b)));
                    temp = string_append_char(temp, ']');
                    add = string_concat(s, temp);
                    string_delete(temp);
                    break;
                }
                case VALUE_FUNC:
                    add = string_concat(s, as_func(b)->name);
                    break;
                case VALUE_FUNC_NATIVE:
                    add = string_concat(s, as_native(b)->name);
                    break;
                default:
                    machine_runtime_error(this, "Operands can't be added.");
                    return;
                }
                machine_push(this, machine_intern_string(this, add));
            } else {
                machine_runtime_error(this, "Operands can't be added.");
                return;
            }
            break;
        }
        case OP_SUBTRACT: {
            ARITHMETIC_OP(-=);
            break;
        }
        case OP_MULTIPLY: {
            ARITHMETIC_OP(*=);
            break;
        }
        case OP_DIVIDE: {
            ARITHMETIC_OP(/=);
            break;
        }
        case OP_BIT_NOT: {
            Value value = machine_pop(this);
            if (is_int(value)) {
                value.as.i = ~value.as.i;
                machine_push(this, value);
            } else {
                machine_runtime_error(this, "Bitwise operand must integer.");
                return;
            }
            break;
        }
        case OP_BIT_OR: {
            INTEGER_OP(|=);
            break;
        }
        case OP_BIT_AND: {
            INTEGER_OP(&=);
            break;
        }
        case OP_BIT_XOR: {
            INTEGER_OP(^=);
            break;
        }
        case OP_BIT_LEFT_SHIFT: {
            INTEGER_OP(<<=);
            break;
        }
        case OP_BIT_RIGHT_SHIFT: {
            INTEGER_OP(>>=);
            break;
        }
        case OP_NEGATE: {
            Value value = machine_peek(this, 1);
            if (is_int(value)) {
                value.as.i = -value.as.i;
            } else if (is_float(value)) {
                value.as.f = -value.as.f;
            } else {
                machine_runtime_error(this, "Operand must be a number.");
                return;
            }
            machine_pop(this);
            machine_push(this, value);
            break;
        }

        case OP_NOT: {
            Value value = machine_peek(this, 1);
            if (is_bool(value)) {
                value.as.b = !value.as.b;
            } else {
                machine_runtime_error(this, "Operand must be a boolean.");
                return;
            }
            machine_pop(this);
            machine_push(this, value);
            break;
        }
        case OP_CONSTANT: {
            Value constant = read_constant(frame);
            machine_push(this, constant);
            break;
        }
        case OP_DEFINE_GLOBAL: {
            String *name = as_string(read_constant(frame));
            Value set = machine_peek(this, 1);
            map_put(&this->globals, name, set);
            machine_pop(this);
            break;
        }
        case OP_SET_GLOBAL: {
            String *name = as_string(read_constant(frame));
            Value set = machine_peek(this, 1);
            Value exists = map_get(&this->globals, name);
            if (is_undefined(exists)) {
                machine_runtime_error(this, "Undefined variable '%s'.", name);
                return;
            }
            map_put(&this->globals, name, set);
            break;
        }
        case OP_GET_GLOBAL: {
            String *name = as_string(read_constant(frame));
            Value get = map_get(&this->globals, name);
            if (is_undefined(get)) {
                machine_runtime_error(this, "Undefined variable '%s'.", name);
                return;
            }
            machine_push(this, get);
            break;
        }
        case OP_SET_LOCAL: {
            u8 slot = read_byte(frame);
            this->stack[slot] = machine_peek(this, 1);
            break;
        }
        case OP_GET_LOCAL: {
            u8 slot = read_byte(frame);
            machine_push(this, this->stack[slot]);
            break;
        }
        case OP_SET_PROPERTY: {
            Value value = machine_pop(this);
            Value var = machine_pop(this);
            if (!is_table(var)) {
                machine_runtime_error(this, "Only tables can set properties.");
                return;
            }
            ValueMap *table = as_table(var);
            String *name = as_string(read_constant(frame));
            map_put(table, name, value);
            machine_push(this, value);
            break;
        }
        case OP_GET_PROPERTY: {
            Value var = machine_peek(this, 1);
            if (!is_table(var)) {
                machine_runtime_error(this, "Only tables can get properties.");
                return;
            }
            ValueMap *table = as_table(var);
            String *name = as_string(read_constant(frame));
            Value value = map_get(table, name);
            if (is_undefined(value)) {
                value.is = VALUE_NONE;
            }
            machine_pop(this);
            machine_push(this, value);
            break;
        }
        case OP_SET_DYNAMIC: {
            Value value = machine_pop(this);
            Value refer = machine_pop(this);
            Value var = machine_pop(this);
            if (is_array(var)) {
                if (!is_int(refer)) {
                    machine_runtime_error(this, "Integer required to set array index.");
                    return;
                }
                Array *array = as_array(var);
                i64 size = array->length;
                i64 index = as_int(refer);
                if (index > size) {
                    machine_runtime_error(this, "Array index out of bounds %d > %d.", index, size);
                    return;
                }
                if (index < 0) {
                    index = size + index;
                    if (index < 0) {
                        machine_runtime_error(this, "Array index out of bounds %d.", index);
                        return;
                    }
                }
                if (index == size) {
                    array_push(array, value);
                } else {
                    array->items[index] = value;
                }
            } else if (is_table(var)) {
                if (!is_string(refer)) {
                    machine_runtime_error(this, "String required to set table property.");
                    return;
                }
                ValueMap *table = as_table(var);
                String *name = as_string(refer);
                map_put(table, name, value);
            } else {
                machine_runtime_error(this, "Expected array or table to set inner value.");
                return;
            }
            machine_push(this, value);
            break;
        }
        case OP_GET_DYNAMIC: {
            Value refer = machine_pop(this);
            Value var = machine_pop(this);
            switch (var.is) {
            case VALUE_STRING: {
                if (!is_int(refer)) {
                    machine_runtime_error(this, "Integer required to get string character from index.");
                    return;
                }
                String *string = as_string(var);
                i64 size = (i64)string_len(string);
                i64 index = as_int(refer);
                if (index >= size) {
                    machine_runtime_error(this, "String index out of bounds %d >= %d.", index, size);
                    return;
                }
                if (index < 0) {
                    index = size + index;
                    if (index < 0) {
                        machine_runtime_error(this, "String index out of bounds %d.", index);
                        return;
                    }
                }
                char c = string[index];
                machine_push(this, machine_intern_string(this, char_to_string(c)));
                break;
            }
            case VALUE_ARRAY: {
                if (!is_int(refer)) {
                    machine_runtime_error(this, "Integer required to get array index.");
                    return;
                }
                Array *array = as_array(var);
                i64 size = array->length;
                i64 index = as_int(refer);
                if (index >= size) {
                    machine_runtime_error(this, "Array index out of bounds %d >= %d.", index, size);
                    return;
                }
                if (index < 0) {
                    index = size + index;
                    if (index < 0) {
                        machine_runtime_error(this, "Array index out of bounds %d.", index);
                        return;
                    }
                }
                machine_push(this, array_get(array, index));
                break;
            }
            case VALUE_TABLE: {
                if (!is_string(refer)) {
                    machine_runtime_error(this, "String required to get table property.");
                    return;
                }
                ValueMap *table = as_table(var);
                String *name = as_string(refer);
                Value value = map_get(table, name);
                if (is_undefined(value)) {
                    value.is = VALUE_NONE;
                }
                machine_push(this, value);
                break;
            }
            default:
                machine_runtime_error(this, "Expected array or table to get inner value.");
                return;
            }
            break;
        }
        case OP_LEN: {
            Value var = machine_pop(this);
            switch (var.is) {
            case VALUE_STRING: {
                i64 len = (i64)string_len(as_string(var));
                machine_push(this, new_int(len));
                break;
            }
            case VALUE_ARRAY: {
                i64 len = as_array(var)->length;
                machine_push(this, new_int(len));
                break;
            }
            case VALUE_TABLE: {
                i64 len = (i64)as_table(var)->size;
                machine_push(this, new_int(len));
                break;
            }
            default:
                machine_runtime_error(this, "Expected array or table for len function.");
                return;
            }
            break;
        }
        case OP_ARRAY_POP: {
            Value var = machine_pop(this);
            if (is_array(var)) {
                Value value = array_pop(as_array(var));
                machine_push(this, value);
            } else {
                machine_runtime_error(this, "Expected array for pop function.");
                return;
            }
            break;
        }
        case OP_ARRAY_PUSH: {
            Value value = machine_pop(this);
            Value var = machine_pop(this);
            if (is_array(var)) {
                array_push(as_array(var), value);
                machine_push(this, value);
            } else {
                machine_runtime_error(this, "Expected array for push function.");
                return;
            }
            break;
        }
        case OP_ARRAY_INSERT: {
            Value value = machine_pop(this);
            Value refer = machine_pop(this);
            Value var = machine_pop(this);
            if (is_array(var)) {
                if (!is_int(refer)) {
                    machine_runtime_error(this, "Integer required to insert into array index.");
                    return;
                }
                Array *array = as_array(var);
                i64 size = array->length;
                i64 index = as_int(refer);
                if (index > size) {
                    machine_runtime_error(this, "Array index out of bounds %d > %d.", index, size);
                    return;
                }
                if (index < 0) {
                    index = size + index;
                    if (index < 0) {
                        machine_runtime_error(this, "Array index out of bounds %d.", index);
                        return;
                    }
                }
                if (index == size) {
                    array_push(array, value);
                } else {
                    array_insert(array, index, value);
                }
                machine_push(this, value);
            } else {
                machine_runtime_error(this, "Expected array for insert function.");
                return;
            }
            break;
        }
        case OP_DELETE: {
            Value refer = machine_pop(this);
            Value var = machine_pop(this);
            if (is_array(var)) {
                if (!is_int(refer)) {
                    machine_runtime_error(this, "Integer required to delete from array.");
                    return;
                }
                Array *array = as_array(var);
                i64 size = array->length;
                i64 index = as_int(refer);
                if (index >= size) {
                    machine_runtime_error(this, "Array index out of bounds %d > %d.", index, size);
                    return;
                }
                if (index < 0) {
                    index = size + index;
                    if (index < 0) {
                        machine_runtime_error(this, "Array index out of bounds %d.", index);
                        return;
                    }
                }
                Value value = array_remove_index(array, index);
                machine_push(this, value);
            } else if (is_table(var)) {
                if (!is_string(refer)) {
                    machine_runtime_error(this, "String required to delete from table.");
                    return;
                }
                String *key = as_string(refer);
                ValueMap *table = as_table(var);
                Value value = map_remove(table, key);
                if (is_undefined(value)) {
                    value.is = VALUE_NONE;
                }
                machine_push(this, value);
            } else {
                machine_runtime_error(this, "Expected array or table for function.");
                return;
            }
            break;
        }
        case OP_COPY: {
            Value value = machine_pop(this);
            switch (value.is) {
            case VALUE_NONE:
            case VALUE_BOOL:
            case VALUE_INTEGER:
            case VALUE_FLOAT:
            case VALUE_STRING:
            case VALUE_FUNC:
            case VALUE_FUNC_NATIVE:
                machine_push(this, value);
                break;
            case VALUE_ARRAY: {
                Array *copy = new_array_copy(as_array(value));
                machine_push(this, new_array_value(copy));
                break;
            }
            case VALUE_TABLE: {
                ValueMap *copy = new_map_copy(as_table(value));
                machine_push(this, new_table_value(copy));
                break;
            }
            default:
                machine_push(this, new_none());
            }
            break;
        }
        case OP_SLICE: {
            Value two = machine_pop(this);
            Value one = machine_pop(this);
            if (!is_int(one)) {
                machine_runtime_error(this, "Integer required for slice expression.");
                return;
            }
            i64 left = as_int(one);
            Value value = machine_pop(this);
            if (is_string(value)) {
                String *original = as_string(value);
                i64 size = (i64)string_len(original);
                i64 right;
                if (is_int(two)) {
                    right = as_int(two);
                } else if (is_none(two)) {
                    right = size;
                } else {
                    machine_runtime_error(this, "Integer required for slice expression.");
                    return;
                }
                if (right > size) {
                    machine_runtime_error(this, "String index out of bounds %d > %d.", right, size);
                    return;
                }
                if (right < 0) {
                    right = size + right;
                    if (right < 0) {
                        machine_runtime_error(this, "String index out of bounds %d.", right);
                        return;
                    }
                }
                if (left >= right) {
                    machine_runtime_error(this, "String start index %d > right index %d.", left, right);
                    return;
                }
                String *sub = new_string_from_substring(original, left, right);
                machine_push(this, machine_intern_string(this, sub));
            } else if (is_array(value)) {
                Array *array = as_array(value);
                i64 size = array->length;
                i64 right;
                if (is_int(two)) {
                    right = as_int(two);
                } else if (is_none(two)) {
                    right = size;
                } else {
                    machine_runtime_error(this, "Integer required for slice expression.");
                    return;
                }
                if (right > size) {
                    machine_runtime_error(this, "Array index out of bounds %d > %d.", right, size);
                    return;
                }
                if (right < 0) {
                    right = size + right;
                    if (right < 0) {
                        machine_runtime_error(this, "Array index out of bounds %d.", right);
                        return;
                    }
                }
                if (left >= right) {
                    machine_runtime_error(this, "Array start index %d >= right index %d.", left, right);
                    return;
                }
                Array *copy = new_array_slice(array, left, right);
                machine_push(this, new_array_value(copy));
                break;
            } else {
                machine_runtime_error(this, "Expected string or array for slice expression.");
                return;
            }
            break;
        }
        case OP_CLEAR: {
            Value value = machine_pop(this);
            switch (value.is) {
            case VALUE_BOOL:
                machine_push(this, new_bool(false));
                break;
            case VALUE_INTEGER:
                machine_push(this, new_int(0));
                break;
            case VALUE_FLOAT:
                machine_push(this, new_float(0.0f));
                break;
            case VALUE_STRING:
                machine_push(this, machine_intern_string(this, new_string("")));
                break;
            case VALUE_ARRAY: {
                Array *array = as_array(value);
                array_clear(array);
                machine_push(this, value);
                break;
            }
            case VALUE_TABLE: {
                ValueMap *table = as_table(value);
                map_clear(table);
                machine_push(this, value);
                break;
            }
            case VALUE_NONE:
            case VALUE_FUNC:
            case VALUE_FUNC_NATIVE:
                machine_push(this, new_none());
                break;
            }
            break;
        }
        case OP_KEYS: {
            Value value = machine_pop(this);
            if (is_table(value)) {
                ValueMap *table = as_table(value);
                Array *array = map_keys(table);
                machine_push(this, new_array_value(array));
            } else {
                machine_runtime_error(this, "Expected table for keys function.");
                return;
            }
            break;
        }
        case OP_INDEX: {
            Value find = machine_pop(this);
            Value var = machine_pop(this);
            switch (var.is) {
            case VALUE_STRING: {
                if (!is_string(find)) {
                    machine_runtime_error(this, "Index function requires a string.");
                    return;
                }
                usize index = 0;
                bool found = string_find(as_string(var), as_string(find), &index);
                if (found) {
                    machine_push(this, new_int((i64)index));
                } else {
                    machine_push(this, new_int(-1));
                }
                break;
            }
            case VALUE_ARRAY:
                machine_push(this, new_int(array_index_of(as_array(var), find)));
                break;
            case VALUE_TABLE:
                String *key = map_key_of(as_table(var), find);
                if (key == NULL) {
                    machine_push(this, new_none());
                } else {
                    machine_push(this, new_string_value(key));
                }
                break;
            default:
                machine_runtime_error(this, "Expected string, array, or table for index function.");
                return;
            }
            break;
        }
        case OP_TYPE: {
            Value value = machine_pop(this);
            switch (value.is) {
            case VALUE_NONE:
                machine_push(this, machine_intern_string(this, new_string(STRING_NONE)));
                break;
            case VALUE_BOOL:
                machine_push(this, machine_intern_string(this, new_string(STRING_BOOL)));
                break;
            case VALUE_INTEGER:
                machine_push(this, machine_intern_string(this, new_string(STRING_INTEGER)));
                break;
            case VALUE_FLOAT:
                machine_push(this, machine_intern_string(this, new_string(STRING_FLOAT)));
                break;
            case VALUE_STRING:
                machine_push(this, machine_intern_string(this, new_string(STRING_STRING)));
                break;
            case VALUE_ARRAY:
                machine_push(this, machine_intern_string(this, new_string(STRING_ARRAY)));
                break;
            case VALUE_TABLE:
                machine_push(this, machine_intern_string(this, new_string(STRING_TABLE)));
                break;
            case VALUE_FUNC:
                machine_push(this, machine_intern_string(this, new_string(STRING_FUNC)));
                break;
            case VALUE_FUNC_NATIVE:
                machine_push(this, machine_intern_string(this, new_string(STRING_NATIVE)));
                break;
            default:
                machine_push(this, new_none());
            }
            break;
        }
        case OP_TO_INTEGER: {
            Value value = machine_pop(this);
            if (is_int(value)) {
                machine_push(this, value);
            } else if (is_float(value)) {
                i64 push = (i64)as_float(value);
                machine_push(this, new_int(push));
            } else {
                machine_runtime_error(this, "Only integers and floats can be casted to an integer.");
                return;
            }
            break;
        }
        case OP_TO_FLOAT: {
            Value value = machine_pop(this);
            if (is_int(value)) {
                float push = (float)as_int(value);
                machine_push(this, new_float(push));
            } else if (is_float(value)) {
                machine_push(this, value);
            } else {
                machine_runtime_error(this, "Only integers and floats can be casted to a float.");
                return;
            }
            break;
        }
        case OP_TO_STRING: {
            Value value = machine_pop(this);
            switch (value.is) {
            case VALUE_NONE:
                machine_push(this, machine_intern_string(this, new_string(STRING_NONE)));
                break;
            case VALUE_BOOL:
                machine_push(this, machine_intern_string(this, as_bool(value) ? new_string(STRING_TRUE) : new_string(STRING_FALSE)));
                break;
            case VALUE_INTEGER:
                machine_push(this, machine_intern_string(this, int64_to_string(as_int(value))));
                break;
            case VALUE_FLOAT:
                machine_push(this, machine_intern_string(this, float64_to_string(as_float(value))));
                break;
            case VALUE_STRING:
                machine_push(this, value);
                break;
            case VALUE_ARRAY:
                machine_push(this, machine_intern_string(this, string_format("[array %p]", as_array(value))));
                break;
            case VALUE_TABLE:
                machine_push(this, machine_intern_string(this, string_format("[table %p]", as_table(value))));
                break;
            case VALUE_FUNC:
                machine_push(this, machine_intern_string(this, as_func(value)->name));
                break;
            case VALUE_FUNC_NATIVE:
                machine_push(this, machine_intern_string(this, as_native(value)->name));
                break;
            default:
                machine_push(this, value);
            }
            break;
        }
        case OP_PRINT: {
            Value value = machine_pop(this);
            switch (value.is) {
            case VALUE_NONE:
                printf("%s\n", STRING_NONE);
                break;
            case VALUE_BOOL:
                printf("%s\n", as_bool(value) ? STRING_TRUE : STRING_FALSE);
                break;
            case VALUE_INTEGER:
                printf("%" PRId64 "\n", as_int(value));
                break;
            case VALUE_FLOAT:
                printf("%g\n", as_float(value));
                break;
            case VALUE_STRING:
                printf("%s\n", as_string(value));
                break;
            case VALUE_ARRAY:
                printf("[array %p]\n", as_array(value));
                break;
            case VALUE_TABLE:
                printf("[table %p]\n", as_table(value));
                break;
            case VALUE_FUNC:
                printf("%s\n", as_func(value)->name);
                break;
            case VALUE_FUNC_NATIVE:
                printf("%s\n", as_native(value)->name);
                break;
            default:
                printf("%p\n", &value);
            }
            break;
        }
        default:
            machine_runtime_error(this, "Unknown instruction.");
            return;
        }
    }
}

static char *machine_interpret(Machine *this) {
    machine_run(this);

    char *error = NULL;
    if (this->error) {
        error = string_copy(this->error);
    }
    return error;
}

static void add_func(Machine *this, char *name, NativeCall func) {
    Value intern = machine_intern_string(this, new_string(name));
    String *key = as_string(intern);
    String *copy = string_copy(key);
    NativeFunction *value = new_native_function(copy, func);
    map_put(&this->globals, key, new_native(value));
}

static inline Machine new_machine() {
    Machine this = {0};
    machine_reset_stack(&this);
    map_init(&this.strings);
    map_init(&this.globals);
    return this;
}

static void machine_delete(Machine *this) {
    map_delete(&this->strings);
    map_delete(&this->globals);
    string_delete(this->error);
}

Hymn *new_hymn() {
    Hymn *this = safe_calloc(1, sizeof(Hymn));
    return this;
}

static Value temp_native_test(int count, Value *arguments) {
    if (count == 0) {
        return new_none();
    }
    i64 i = as_int(arguments[0]) + 1;
    return new_int(i);
}

char *hymn_eval(Hymn *this, char *source) {
    (void *)this;

    Machine m = new_machine();
    Machine *machine = &m;

    add_func(machine, "inc", temp_native_test);

    char *error = NULL;

    Function *func = compile(machine, source, &error);
    if (error) {
        return error;
    }

    machine_push(machine, new_func(func));
    machine_call(machine, func, 0);

    error = machine_interpret(machine);
    if (error) {
        return error;
    }

    machine_reset_stack(machine);

    machine_delete(machine);
    return error;
}

char *hymn_read(Hymn *this, char *file) {
    String *source = cat(file);
    char *error = hymn_eval(this, source);
    string_delete(source);
    return error;
}

char *hymn_repl(Hymn *this) {
    (void *)this;
    printf("Welcome to Hymn\n");

    char input[1024];
    char *error = NULL;

    Machine m = new_machine();
    Machine *machine = &m;

    while (true) {
        printf("> ");
        if (!fgets(input, sizeof(input), stdin)) {
            printf("\n");
            break;
        }

        Function *func = compile(machine, input, &error);
        if (error) {
            break;
        }

        machine_push(machine, new_func(func));
        machine_call(machine, func, 0);

        error = machine_interpret(machine);
        if (error) {
            return error;
        }

        machine_reset_stack(machine);
    }

    machine_delete(machine);
    return error;
}

void hymn_add_func(Hymn *this, char *name, char *(*func)(Hymn *)) {
    (void *)this;
    (void *)name;
    (void *)func;
}

void hymn_add_pointer(Hymn *this, char *name, void *pointer) {
    (void *)this;
    (void *)name;
    (void *)pointer;
}

char *hymn_call(Hymn *this, char *name) {
    (void *)this;
    (void *)name;
    return NULL;
}

void *hymn_pointer(Hymn *this, i32 index) {
    (void *)this;
    (i32) index;
    return NULL;
}

i32 hymn_i32(Hymn *this, i32 index) {
    (void *)this;
    (i32) index;
    return 0;
}

u32 hymn_u32(Hymn *this, i32 index) {
    (void *)this;
    (i32) index;
    return 0;
}

i64 hymn_i64(Hymn *this, i32 index) {
    (void *)this;
    (i32) index;
    return 0;
}

u64 hymn_u64(Hymn *this, i32 index) {
    (void *)this;
    (i32) index;
    return 0;
}

f32 hymn_f32(Hymn *this, i32 index) {
    (void *)this;
    (i32) index;
    return 0.0f;
}

f64 hymn_f64(Hymn *this, i32 index) {
    (void *)this;
    (i32) index;
    return 0.0;
}

void hymn_delete(Hymn *this) {
    free(this);
}
