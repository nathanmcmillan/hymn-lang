/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "hymn.h"

#define new_undefined() hymn_new_undefined()
#define new_none() hymn_new_none()
#define new_bool(v) hymn_new_bool(v)
#define new_int(v) hymn_new_int(v)
#define new_float(v) hymn_new_float(v)
#define new_native(v) hymn_new_native(v)
#define new_pointer(v) hymn_new_pointer(v)
#define new_string_value(v) hymn_new_string_value(v)
#define new_array_value(v) hymn_new_array_value(v)
#define new_table_value(v) hymn_new_table_value(v)
#define new_func_value(v) hymn_new_func_value(v)

#define as_bool(v) hymn_as_bool(v)
#define as_int(v) hymn_as_int(v)
#define as_float(v) hymn_as_float(v)
#define as_native(v) hymn_as_native(v)
#define as_pointer(v) hymn_as_pointer(v)
#define as_object(v) hymn_as_object(v)
#define as_hymn_string(v) hymn_as_string(v)
#define as_string(v) (hymn_as_string(v)->string)
#define as_array(v) hymn_as_array(v)
#define as_table(v) hymn_as_table(v)
#define as_func(v) hymn_as_func(v)

#define is_undefined(v) ((v).is == HYMN_VALUE_UNDEFINED)
#define is_none(v) ((v).is == HYMN_VALUE_NONE)
#define is_bool(v) ((v).is == HYMN_VALUE_BOOL)
#define is_int(v) ((v).is == HYMN_VALUE_INTEGER)
#define is_float(v) ((v).is == HYMN_VALUE_FLOAT)
#define is_native(v) ((v).is == HYMN_VALUE_FUNC_NATIVE)
#define is_pointer(v) ((v).is == HYMN_VALUE_POINTER)
#define is_string(v) ((v).is == HYMN_VALUE_STRING)
#define is_array(v) ((v).is == HYMN_VALUE_ARRAY)
#define is_table(v) ((v).is == HYMN_VALUE_TABLE)
#define is_func(v) ((v).is == HYMN_VALUE_FUNC)

#define STRING_UNDEFINED "Undefined"
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
#define STRING_POINTER "Pointer"

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define POP(v) Value v = machine_pop(this);

#define PUSH(v) machine_push(this, v);

#define THROW(...)                                    \
    frame = machine_throw_error(this, ##__VA_ARGS__); \
    if (frame == NULL) {                              \
        return;                                       \
    } else {                                          \
        break;                                        \
    }

#define DEREF(x) dereference(this, x);

#define DEREF_TWO(x, y)   \
    dereference(this, x); \
    dereference(this, y);

#define DEREF_THREE(x, y, z) \
    dereference(this, x);    \
    dereference(this, y);    \
    dereference(this, z);

#define INTEGER_OP(_do_)                                                \
    POP(b)                                                              \
    POP(a)                                                              \
    if (is_int(a)) {                                                    \
        if (is_int(b)) {                                                \
            a.as.i _do_ b.as.i;                                         \
            PUSH(a)                                                     \
        } else {                                                        \
            DEREF_TWO(a, b)                                             \
            THROW("Operation Error: 2nd value must be `Integer`.")      \
        }                                                               \
    } else {                                                            \
        DEREF_TWO(a, b)                                                 \
        THROW("Operation Error: 1st and 2nd values must be `Integer`.") \
    }

#define ARITHMETIC_OP(_binary_)                                                        \
    POP(b)                                                                             \
    POP(a)                                                                             \
    if (is_int(a)) {                                                                   \
        if (is_int(b)) {                                                               \
            a.as.i _binary_ b.as.i;                                                    \
            PUSH(a)                                                                    \
        } else if (is_float(b)) {                                                      \
            b.as.f _binary_ a.as.i;                                                    \
            PUSH(a)                                                                    \
        } else {                                                                       \
            DEREF_TWO(a, b)                                                            \
            THROW("Operation Error: 2nd value must be `Integer` or `Float`.")          \
        }                                                                              \
    } else if (is_float(a)) {                                                          \
        if (is_int(b)) {                                                               \
            a.as.f _binary_ b.as.i;                                                    \
            PUSH(a)                                                                    \
        } else if (is_float(b)) {                                                      \
            a.as.f _binary_ b.as.f;                                                    \
            PUSH(a)                                                                    \
        } else {                                                                       \
            DEREF_TWO(a, b)                                                            \
            THROW("Operation Error: 1st and 2nd values must be `Integer` or `Float`.") \
        }                                                                              \
    } else {                                                                           \
        DEREF_TWO(a, b)                                                                \
        THROW("Operation Error: 1st and 2nd values must be `Integer` or `Float`.")     \
    }

#define COMPARE_OP(_compare_)                                       \
    POP(b)                                                          \
    POP(a)                                                          \
    if (is_int(a)) {                                                \
        if (is_int(b)) {                                            \
            PUSH(new_bool(as_int(a) _compare_ as_int(b)))           \
        } else if (is_float(b)) {                                   \
            PUSH(new_bool((double)as_int(a) _compare_ as_float(b))) \
        } else {                                                    \
            DEREF_TWO(a, b)                                         \
            THROW("Operands must be numbers.")                      \
        }                                                           \
    } else if (is_float(a)) {                                       \
        if (is_int(b)) {                                            \
            PUSH(new_bool(as_float(a) _compare_(double) as_int(b))) \
        } else if (is_float(b)) {                                   \
            PUSH(new_bool(as_float(a) _compare_ as_float(b)))       \
        } else {                                                    \
            DEREF_TWO(a, b)                                         \
            THROW("Operands must be numbers.")                      \
        }                                                           \
    } else {                                                        \
        DEREF_TWO(a, b)                                             \
        THROW("Operands must be numbers.")                          \
    }

typedef HymnValue Value;
typedef HymnObject Object;
typedef HymnArray Array;
typedef HymnTable Table;
typedef HymnTableItem TableItem;
typedef HymnSet Set;
typedef HymnSetItem SetItem;
typedef HymnNativeCall NativeCall;
typedef HymnExceptList ExceptList;
typedef HymnFunction Function;
typedef HymnNativeFunction NativeFunction;
typedef HymnFrame Frame;
typedef HymnValuePool ValuePool;
typedef HymnByteCode ByteCode;

typedef struct Token Token;
typedef struct Local Local;
typedef struct Rule Rule;
typedef struct Scope Scope;
typedef struct Compiler Compiler;
typedef struct Hymn Machine;

static const float LOAD_FACTOR = 0.80f;

static const unsigned int INITIAL_BINS = 1 << 3;

static const unsigned int MAXIMUM_BINS = 1 << 30;

enum TokenType {
    TOKEN_ADD,
    TOKEN_AND,
    TOKEN_ASSIGN,
    TOKEN_BEGIN,
    TOKEN_BIT_AND,
    TOKEN_BIT_LEFT_SHIFT,
    TOKEN_BIT_NOT,
    TOKEN_BIT_OR,
    TOKEN_BIT_RIGHT_SHIFT,
    TOKEN_BIT_XOR,
    TOKEN_BREAK,
    TOKEN_CASE,
    TOKEN_CLEAR,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_CONST,
    TOKEN_CONTINUE,
    TOKEN_COPY,
    TOKEN_DELETE,
    TOKEN_DIVIDE,
    TOKEN_DO,
    TOKEN_DOT,
    TOKEN_ELIF,
    TOKEN_ELSE,
    TOKEN_END,
    TOKEN_EOF,
    TOKEN_EQUAL,
    TOKEN_ERROR,
    TOKEN_EXCEPT,
    TOKEN_FALSE,
    TOKEN_FLOAT,
    TOKEN_FOR,
    TOKEN_FUNCTION,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_IDENT,
    TOKEN_IF,
    TOKEN_IN,
    TOKEN_INDEX,
    TOKEN_INSERT,
    TOKEN_INTEGER,
    TOKEN_ITERATE,
    TOKEN_KEYS,
    TOKEN_LEFT_CURLY,
    TOKEN_LEFT_PAREN,
    TOKEN_LEFT_SQUARE,
    TOKEN_LEN,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_LET,
    TOKEN_LINE,
    TOKEN_MODULO,
    TOKEN_MULTIPLY,
    TOKEN_NONE,
    TOKEN_NOT,
    TOKEN_NOT_EQUAL,
    TOKEN_OR,
    TOKEN_PASS,
    TOKEN_POP,
    TOKEN_PRINT,
    TOKEN_PUSH,
    TOKEN_RETURN,
    TOKEN_RIGHT_CURLY,
    TOKEN_RIGHT_PAREN,
    TOKEN_RIGHT_SQUARE,
    TOKEN_SEMICOLON,
    TOKEN_STRING,
    TOKEN_SUBTRACT,
    TOKEN_SWITCH,
    TOKEN_THROW,
    TOKEN_TO_FLOAT,
    TOKEN_TO_INTEGER,
    TOKEN_TO_STRING,
    TOKEN_TRUE,
    TOKEN_TRY,
    TOKEN_TYPE,
    TOKEN_UNDEFINED,
    TOKEN_USE,
    TOKEN_VALUE,
    TOKEN_WHILE,
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
    OP_DO,
    OP_DUPLICATE,
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
    OP_JUMP_IF_TRUE,
    OP_KEYS,
    OP_LEN,
    OP_LESS,
    OP_LESS_EQUAL,
    OP_LOOP,
    OP_MODULO,
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
    OP_THROW,
    OP_TO_FLOAT,
    OP_TO_INTEGER,
    OP_TO_STRING,
    OP_TRUE,
    OP_TYPE,
    OP_USE,
};

enum FunctionType {
    TYPE_FUNCTION,
    TYPE_SCRIPT,
};

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

static inline void reference_string(HymnString *string);
static inline void reference(Value value);
static inline void dereference_string(Machine *this, HymnString *string);
static inline void dereference(Machine *this, Value value);

static char *machine_interpret(Machine *this);

struct JumpList {
    int jump;
    int depth;
    struct JumpList *next;
};

struct LoopList {
    int start;
    int depth;
    struct LoopList *next;
};

struct Token {
    enum TokenType type;
    int row;
    int column;
    usize start;
    int length;
};

struct Local {
    Token name;
    int depth;
    bool constant;
};

struct Rule {
    void (*prefix)(Compiler *, bool);
    void (*infix)(Compiler *, bool);
    enum Precedence precedence;
};

struct Scope {
    struct Scope *enclosing;
    Function *func;
    enum FunctionType type;
    Local locals[HYMN_UINT8_COUNT];
    int local_count;
    int depth;
};

struct Compiler {
    usize pos;
    int row;
    int column;
    const char *script;
    const char *source;
    usize size;
    Token previous;
    Token current;
    Machine *machine;
    Scope *scope;
    struct LoopList *loop;
    struct JumpList *jump;
    struct JumpList *iterator_jump;
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
    [TOKEN_DO] = {NULL, NULL, PRECEDENCE_NONE},
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
    [TOKEN_ITERATE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_KEYS] = {keys_expression, NULL, PRECEDENCE_NONE},
    [TOKEN_LEFT_CURLY] = {compile_table, NULL, PRECEDENCE_NONE},
    [TOKEN_LEFT_PAREN] = {compile_group, compile_call, PRECEDENCE_CALL},
    [TOKEN_LEFT_SQUARE] = {compile_array, compile_square, PRECEDENCE_CALL},
    [TOKEN_LEN] = {len_expression, NULL, PRECEDENCE_NONE},
    [TOKEN_LESS] = {NULL, compile_binary, PRECEDENCE_COMPARE},
    [TOKEN_LESS_EQUAL] = {NULL, compile_binary, PRECEDENCE_COMPARE},
    [TOKEN_LET] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_LINE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_MODULO] = {NULL, compile_binary, PRECEDENCE_FACTOR},
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
    [TOKEN_THROW] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_TYPE] = {type_expression, NULL, PRECEDENCE_NONE},
    [TOKEN_UNDEFINED] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_USE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_VALUE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_WHILE] = {NULL, NULL, PRECEDENCE_NONE},
};

static const char *value_name(enum HymnValueType type) {
    switch (type) {
    case HYMN_VALUE_UNDEFINED: return STRING_UNDEFINED;
    case HYMN_VALUE_NONE: return STRING_NONE;
    case HYMN_VALUE_BOOL: return STRING_BOOL;
    case HYMN_VALUE_INTEGER: return STRING_INTEGER;
    case HYMN_VALUE_FLOAT: return STRING_FLOAT;
    case HYMN_VALUE_STRING: return STRING_STRING;
    case HYMN_VALUE_ARRAY: return STRING_ARRAY;
    case HYMN_VALUE_TABLE: return STRING_TABLE;
    case HYMN_VALUE_FUNC: return STRING_FUNC;
    case HYMN_VALUE_FUNC_NATIVE: return STRING_NATIVE;
    default: return "Value";
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
    case TOKEN_DO: return "DO";
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
    case TOKEN_IDENT: return "IDENTITY";
    case TOKEN_IF: return "IF";
    case TOKEN_IN: return "IN";
    case TOKEN_INDEX: return "INDEX";
    case TOKEN_INSERT: return "INSERT";
    case TOKEN_INTEGER: return "INTEGER";
    case TOKEN_ITERATE: return "ITERATE";
    case TOKEN_KEYS: return "KEYS";
    case TOKEN_LEFT_PAREN: return "LEFT_PAREN";
    case TOKEN_LEN: return "LEN";
    case TOKEN_LESS: return "LESS";
    case TOKEN_LESS_EQUAL: return "LESS_EQUAL";
    case TOKEN_LET: return "LET";
    case TOKEN_MODULO: return "MODULO";
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
    case TOKEN_TO_FLOAT: return "FLOAT";
    case TOKEN_TO_INTEGER: return "INT";
    case TOKEN_TO_STRING: return "STRING";
    case TOKEN_TRUE: return "TRUE";
    case TOKEN_TRY: return "TRY";
    case TOKEN_THROW: return "THROW";
    case TOKEN_TYPE: return "TYPE";
    case TOKEN_WHILE: return "WHILE";
    case TOKEN_USE: return "USE";
    case TOKEN_LEFT_CURLY: return "LEFT_CURLY";
    case TOKEN_RIGHT_CURLY: return "RIGHT_CURLY";
    case TOKEN_LEFT_SQUARE: return "LEFT_SQUARE";
    case TOKEN_RIGHT_SQUARE: return "RIGHT_SQUARE";
    default: return "token";
    }
}

static String *debug_value_to_string(Value value) {
    String *string = new_string(value_name(value.is));
    string = string_append(string, ": ");
    switch (value.is) {
    case HYMN_VALUE_UNDEFINED: return string_append(string, "UNDEFINED");
    case HYMN_VALUE_NONE: return string_append(string, "NONE");
    case HYMN_VALUE_BOOL: return string_append(string, as_bool(value) ? "TRUE" : "FALSE");
    case HYMN_VALUE_INTEGER: return string_append_format(string, "%" PRId64, as_int(value));
    case HYMN_VALUE_FLOAT: return string_append_format(string, "%g", as_float(value));
    case HYMN_VALUE_STRING: return string_append_format(string, "\"%s\"", as_string(value));
    case HYMN_VALUE_ARRAY: return string_append_format(string, "[%p]", as_array(value));
    case HYMN_VALUE_TABLE: return string_append_format(string, "[%p]", as_table(value));
    case HYMN_VALUE_FUNC: {
        Function *func = as_func(value);
        if (func->name) return string_append_format(string, "<%s>", func->name);
        return string_append_format(string, "<%s>", func->script);
    }
    case HYMN_VALUE_FUNC_NATIVE: return string_append_format(string, "<%s>", as_native(value)->name);
    default: return string_append_char(string, '?');
    }
}

static void debug_value(Value value) {
    String *string = debug_value_to_string(value);
    printf("%s", string);
    string_delete(string);
}

static void debug_value_message(const char *prefix, Value value) {
    printf("%s", prefix);
    debug_value(value);
    printf("\n");
}

static usize string_hashcode(String *key) {
    usize length = string_len(key);
    usize hash = 0;
    for (usize i = 0; i < length; i++) {
        hash = 31 * hash + (usize)key[i];
    }
    return hash;
}

static usize mix_hash(usize hash) {
    return hash ^ (hash >> 16);
}

static HymnString *new_hymn_string_with_hash(String *string, usize hash) {
    HymnString *object = safe_calloc(1, sizeof(HymnString));
    object->string = string;
    object->hash = hash;
    return object;
}

HymnString *new_hymn_string(String *string) {
    return new_hymn_string_with_hash(string, mix_hash(string_hashcode(string)));
}

static void table_init(Table *this) {
    this->size = 0;
    this->bins = INITIAL_BINS;
    this->items = safe_calloc(this->bins, sizeof(TableItem *));
}

static unsigned int table_get_bin(Table *this, usize hash) {
    return (this->bins - 1) & hash;
}

static void table_resize(Table *this) {

    unsigned int old_bins = this->bins;
    unsigned int bins = old_bins << 1;

    if (bins > MAXIMUM_BINS) {
        return;
    }

    TableItem **old_items = this->items;
    TableItem **items = safe_calloc(bins, sizeof(TableItem *));

    for (unsigned int i = 0; i < old_bins; i++) {
        TableItem *item = old_items[i];
        if (item == NULL) {
            continue;
        }
        if (item->next == NULL) {
            items[(bins - 1) & item->key->hash] = item;
        } else {
            TableItem *low_head = NULL;
            TableItem *low_tail = NULL;
            TableItem *high_head = NULL;
            TableItem *high_tail = NULL;
            do {
                if ((old_bins & item->key->hash) == 0) {
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

static void table_put(Table *this, HymnString *key, Value value) {
    unsigned int bin = table_get_bin(this, key->hash);
    TableItem *item = this->items[bin];
    TableItem *previous = NULL;
    while (item != NULL) {
        if (key == item->key) {
            item->value = value;
            return;
        }
        previous = item;
        item = item->next;
    }
    item = safe_malloc(sizeof(TableItem));
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
        table_resize(this);
    }
}

static Value table_get(Table *this, HymnString *key) {
    unsigned int bin = table_get_bin(this, key->hash);
    TableItem *item = this->items[bin];
    while (item != NULL) {
        if (key == item->key) {
            return item->value;
        }
        item = item->next;
    }
    return new_undefined();
}

static Value table_remove(Table *this, HymnString *key) {
    unsigned int bin = table_get_bin(this, key->hash);
    TableItem *item = this->items[bin];
    TableItem *previous = NULL;
    while (item != NULL) {
        if (key == item->key) {
            if (previous == NULL) {
                this->items[bin] = item->next;
            } else {
                previous->next = item->next;
            }
            Value value = item->value;
            free(item);
            this->size -= 1;
            return value;
        }
        previous = item;
        item = item->next;
    }
    return new_undefined();
}

static void table_clear(Machine *machine, Table *this) {
    unsigned int bins = this->bins;
    for (unsigned int i = 0; i < bins; i++) {
        TableItem *item = this->items[i];
        while (item != NULL) {
            TableItem *next = item->next;
            dereference(machine, item->value);
            free(item);
            item = next;
        }
        this->items[i] = NULL;
    }
    this->size = 0;
}

static void table_release(Machine *machine, Table *this) {
    table_clear(machine, this);
    free(this->items);
}

static void table_delete(Machine *machine, Table *this) {
    table_release(machine, this);
    free(this);
}

static void set_init(Set *this) {
    this->size = 0;
    this->bins = INITIAL_BINS;
    this->items = safe_calloc(this->bins, sizeof(SetItem *));
}

static unsigned int set_get_bin(Set *this, usize hash) {
    return (this->bins - 1) & hash;
}

static void set_resize(Set *this) {

    unsigned int old_bins = this->bins;
    unsigned int bins = old_bins << 1;

    if (bins > MAXIMUM_BINS) {
        return;
    }

    SetItem **old_items = this->items;
    SetItem **items = safe_calloc(bins, sizeof(SetItem *));

    for (unsigned int i = 0; i < old_bins; i++) {
        SetItem *item = old_items[i];
        if (item == NULL) {
            continue;
        }
        if (item->next == NULL) {
            items[(bins - 1) & item->string->hash] = item;
        } else {
            SetItem *low_head = NULL;
            SetItem *low_tail = NULL;
            SetItem *high_head = NULL;
            SetItem *high_tail = NULL;
            do {
                if ((old_bins & item->string->hash) == 0) {
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

static HymnString *set_add_or_get(Set *this, String *add) {
    usize hash = mix_hash(string_hashcode(add));
    unsigned int bin = set_get_bin(this, hash);
    SetItem *item = this->items[bin];
    SetItem *previous = NULL;
    while (item != NULL) {
        if (string_equal(add, item->string->string)) {
            return item->string;
        }
        previous = item;
        item = item->next;
    }
    HymnString *new = new_hymn_string_with_hash(add, hash);
    item = safe_malloc(sizeof(SetItem));
    item->string = new;
    item->next = NULL;
    if (previous == NULL) {
        this->items[bin] = item;
    } else {
        previous->next = item;
    }
    this->size++;
    if (this->size >= this->bins * LOAD_FACTOR) {
        set_resize(this);
    }
    return new;
}

static HymnString *set_remove(Set *this, String *remove) {
    usize hash = mix_hash(string_hashcode(remove));
    unsigned int bin = set_get_bin(this, hash);
    SetItem *item = this->items[bin];
    SetItem *previous = NULL;
    while (item != NULL) {
        if (string_equal(remove, item->string->string)) {
            if (previous == NULL) {
                this->items[bin] = item->next;
            } else {
                previous->next = item->next;
            }
            HymnString *string = item->string;
            free(item);
            this->size -= 1;
            return string;
        }
        previous = item;
        item = item->next;
    }
    return NULL;
}

static void set_clear(Machine *machine, Set *this) {
    unsigned int bins = this->bins;
    for (unsigned int i = 0; i < bins; i++) {
        SetItem *item = this->items[i];
        while (item != NULL) {
            SetItem *next = item->next;
            dereference_string(machine, item->string);
            free(item);
            item = next;
        }
        this->items[i] = NULL;
    }
    this->size = 0;
}

static void set_release(Machine *machine, Set *this) {
    set_clear(machine, this);
    free(this->items);
}

static inline Function *current_func(Compiler *this) {
    return this->scope->func;
}

static inline ByteCode *current(Compiler *this) {
    return &current_func(this)->code;
}

static usize beginning_of_line(const char *source, usize i) {
    while (true) {
        if (i == 0) return 0;
        if (source[i] == '\n') return i + 1;
        i--;
    }
}

static usize end_of_line(const char *source, usize size, usize i) {
    while (true) {
        if (i + 1 >= size) return i + 1;
        if (source[i] == '\n') return i;
        i++;
    }
}

static String *string_append_previous_line(const char *source, String *string, usize i) {
    if (i < 2) {
        return string;
    }
    i--;
    usize begin = beginning_of_line(source, i - 1);
    if (i - begin < 2) {
        return string;
    }
    return string_append_format(string, "%.*s\n", i - begin, &source[begin]);
}

static String *string_append_second_previous_line(const char *source, String *string, usize i) {
    if (i < 2) {
        return string;
    }
    i--;
    usize begin = beginning_of_line(source, i - 1);
    return string_append_previous_line(source, string, begin);
}

static void compile_error(Compiler *this, Token *token, const char *format, ...) {

    if (this->error != NULL) return;

    String *error = new_string("");

    va_list ap;
    va_start(ap, format);
    int len = vsnprintf(NULL, 0, format, ap);
    va_end(ap);
    char *chars = safe_malloc((len + 1) * sizeof(char));
    va_start(ap, format);
    len = vsnprintf(chars, len + 1, format, ap);
    va_end(ap);
    error = string_append(error, chars);
    free(chars);
    error = string_append(error, "\n\n");

    usize begin = beginning_of_line(this->source, token->start);
    usize end = end_of_line(this->source, this->size, token->start);

    error = string_append_second_previous_line(this->source, error, begin);
    error = string_append_previous_line(this->source, error, begin);
    error = string_append_format(error, "%.*s\n", end - begin, &this->source[begin]);
    for (int i = 0; i < (int)(token->start - begin); i++) {
        error = string_append_char(error, ' ');
    }
    error = string_append(error, ANSI_COLOR_RED);
    for (int i = 0; i < token->length; i++) {
        error = string_append_char(error, '^');
    }
    error = string_append(error, ANSI_COLOR_RESET);

    error = string_append_format(error, "\nat %s:%d\n", this->script, token->row);

    this->error = error;

    this->previous.type = TOKEN_EOF;
    this->current.type = TOKEN_EOF;
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
    Token *current = &this->current;
    current->type = type;
    current->row = this->row;
    current->column = this->column;
    if (this->pos == 0) {
        current->start = 0;
    } else {
        current->start = this->pos - 1;
    }
    current->length = 1;
#ifdef HYMN_DEBUG_TOKEN
    printf("TOKEN: %s: %.*s\n", token_name(type), current->length, &this->source[current->start]);
#endif
}

static void token_special(Compiler *this, enum TokenType type, usize offset, usize length) {
    Token *current = &this->current;
    current->type = type;
    current->row = this->row;
    current->column = this->column;
    if (this->pos < offset) {
        current->start = 0;
    } else {
        current->start = this->pos - offset;
    }
    current->length = length;
#ifdef HYMN_DEBUG_TOKEN
    printf("TOKEN: %s: %.*s\n", token_name(type), current->length, &this->source[current->start]);
#endif
}

static void value_token(Compiler *this, enum TokenType type, usize start, usize end) {
    Token *current = &this->current;
    current->type = type;
    current->row = this->row;
    current->column = this->column;
    current->start = start;
    current->length = (int)(end - start);
#ifdef HYMN_DEBUG_TOKEN
    printf("TOKEN: %s: %.*s\n", token_name(type), current->length, &this->source[start]);
#endif
}

static enum TokenType ident_trie(const char *ident, int offset, const char *rest, enum TokenType type) {
    int i = 0;
    do {
        if (ident[offset + i] != rest[i]) {
            return TOKEN_UNDEFINED;
        }
        i++;
    } while (rest[i] != '\0');
    return type;
}

static enum TokenType ident_keyword(const char *ident, usize size) {
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
        if (size == 2 and ident[1] == 'o') return TOKEN_DO;
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
        if (size == 5) return ident_trie(ident, 1, "hrow", TOKEN_THROW);
        if (size == 4) {
            if (ident[1] == 'r') return ident_trie(ident, 2, "ue", TOKEN_TRUE);
            if (ident[1] == 'y') return ident_trie(ident, 2, "pe", TOKEN_TYPE);
        }
        break;
    case 'i':
        if (size == 3) return ident_trie(ident, 1, "nt", TOKEN_TO_INTEGER);
        if (size == 5) return ident_trie(ident, 1, "ndex", TOKEN_INDEX);
        if (size == 6) return ident_trie(ident, 1, "nsert", TOKEN_INSERT);
        if (size == 7) return ident_trie(ident, 1, "terate", TOKEN_ITERATE);
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
    const char *ident = &this->source[start];
    usize size = end - start;
    enum TokenType keyword = ident_keyword(ident, size);
    if (keyword != TOKEN_UNDEFINED) {
        value_token(this, keyword, start, end);
        return;
    }
    value_token(this, TOKEN_IDENT, start, end);
}

static bool is_digit(char c) {
    return '0' <= c and c <= '9';
}

static bool is_ident(char c) {
    return ('a' <= c and c <= 'z') or ('A' <= c and c <= 'Z') or c == '_';
}

static void advance(Compiler *this) {
    this->previous = this->current;
    if (this->previous.type == TOKEN_EOF) {
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
                token_special(this, TOKEN_NOT_EQUAL, 2, 2);
            } else {
                token(this, TOKEN_NOT);
            }
            return;
        case '=':
            if (peek_char(this) == '=') {
                next_char(this);
                token_special(this, TOKEN_EQUAL, 2, 2);
            } else {
                token(this, TOKEN_ASSIGN);
            }
            return;
        case '>':
            if (peek_char(this) == '=') {
                next_char(this);
                token_special(this, TOKEN_GREATER_EQUAL, 2, 2);
            } else if (peek_char(this) == '>') {
                next_char(this);
                token_special(this, TOKEN_BIT_RIGHT_SHIFT, 2, 2);
            } else {
                token(this, TOKEN_GREATER);
            }
            return;
        case '<':
            if (peek_char(this) == '=') {
                next_char(this);
                token_special(this, TOKEN_LESS_EQUAL, 2, 2);
            } else if (peek_char(this) == '<') {
                next_char(this);
                token_special(this, TOKEN_BIT_LEFT_SHIFT, 2, 2);
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
        case '%': token(this, TOKEN_MODULO); return;
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
            value_token(this, TOKEN_STRING, start, end);
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
            value_token(this, TOKEN_STRING, start, end);
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
                compile_error(this, &this->current, "Unknown Character: `%c`", c);
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

static void value_pool_add(ValuePool *this, Value value) {
    int count = this->count;
    if (count + 1 > this->capacity) {
        this->capacity *= 2;
        this->values = safe_realloc(this->values, this->capacity * sizeof(Value));
    }
    this->values[count] = value;
    this->count = count + 1;
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
    case HYMN_VALUE_UNDEFINED:
    case HYMN_VALUE_NONE: return true;
    case HYMN_VALUE_BOOL: return as_bool(a) == as_bool(b);
    case HYMN_VALUE_INTEGER: return as_int(a) == as_int(b);
    case HYMN_VALUE_FLOAT: return as_float(a) == as_float(b);
    case HYMN_VALUE_STRING:
    case HYMN_VALUE_ARRAY:
    case HYMN_VALUE_TABLE:
    case HYMN_VALUE_FUNC:
        return as_object(a) == as_object(b);
    case HYMN_VALUE_FUNC_NATIVE: return as_native(a) == as_native(b);
    case HYMN_VALUE_POINTER: return as_pointer(a) == as_pointer(b);
    }
    return false;
}

static Function *new_function(const char *script) {
    Function *func = safe_calloc(1, sizeof(Function));
    byte_code_init(&func->code);
    if (script) func->script = new_string(script);
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
    Array *this = safe_calloc(1, sizeof(Array));
    array_init_with_capacity(this, length, capacity);
    return this;
}

static Array *new_array(i64 length) {
    return new_array_with_capacity(length, length);
}

static Array *new_array_slice(Array *from, i64 start, i64 end) {
    usize length = end - start;
    usize size = length * sizeof(Value);
    Array *this = safe_calloc(1, sizeof(Array));
    this->items = safe_malloc(size);
    memcpy(this->items, &from->items[start], size);
    this->length = length;
    this->capacity = length;
    for (usize i = 0; i < length; i++) {
        reference(this->items[i]);
    }
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

static void array_clear(Machine *machine, Array *this) {
    i64 len = this->length;
    Value *items = this->items;
    for (i64 i = 0; i < len; i++) {
        dereference(machine, items[i]);
    }
    this->length = 0;
}

static void array_delete(Machine *machine, Array *this) {
    array_clear(machine, this);
    free(this->items);
    free(this);
}

static Table *new_table() {
    Table *this = safe_calloc(1, sizeof(Table));
    table_init(this);
    return this;
}

static Table *new_table_copy(Table *from) {
    Table *this = new_table();
    unsigned int bins = from->bins;
    for (unsigned int i = 0; i < bins; i++) {
        TableItem *item = from->items[i];
        while (item != NULL) {
            table_put(this, item->key, item->value);
            reference(item->value);
            item = item->next;
        }
    }
    return this;
}

static Array *table_keys(Table *this) {
    Array *array = new_array_with_capacity(0, this->size);

    unsigned int bin = 0;
    TableItem *item = NULL;

    unsigned int bins = this->bins;
    for (unsigned int i = 0; i < bins; i++) {
        TableItem *start = this->items[i];
        if (start) {
            bin = i;
            item = start;
            break;
        }
    }

    if (item == NULL) return array;

    {
        Value value = new_string_value(item->key);
        reference(value);
        array_push(array, value);
    }

    while (true) {
        item = item->next;
        if (item == NULL) {
            for (bin = bin + 1; bin < bins; bin++) {
                TableItem *start = this->items[bin];
                if (start) {
                    item = start;
                    break;
                }
            }
            if (item == NULL) return array;
        }

        Value value = new_string_value(item->key);
        reference(value);
        array_push(array, value);
    }

    return array;
}

static HymnString *table_key_of(Table *this, Value match) {

    unsigned int bin = 0;
    TableItem *item = NULL;

    unsigned int bins = this->bins;
    for (unsigned int i = 0; i < bins; i++) {
        TableItem *start = this->items[i];
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
                TableItem *start = this->items[bin];
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

static void value_pool_delete(ValuePool *this) {
    free(this->values);
}

static void scope_init(Compiler *this, Scope *scope, enum FunctionType type) {
    scope->enclosing = this->scope;
    this->scope = scope;

    scope->local_count = 0;
    scope->depth = 0;
    scope->func = new_function(this->script);
    scope->type = type;

    if (type != TYPE_SCRIPT) {
        scope->func->name = new_string_from_substring(this->source, this->previous.start, this->previous.start + this->previous.length);
    }

    Local *local = &scope->locals[scope->local_count++];
    local->depth = 0;
    local->name.start = 0;
    local->name.length = 0;
}

static inline Compiler new_compiler(const char *script, const char *source, Machine *machine, Scope *scope) {
    Compiler this = {0};
    this.row = 1;
    this.column = 1;
    this.script = script;
    this.source = source;
    this.size = strlen(source);
    this.previous.type = TOKEN_UNDEFINED;
    this.current.type = TOKEN_UNDEFINED;
    this.machine = machine;
    scope_init(&this, scope, TYPE_SCRIPT);
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

static u8 write_constant(Compiler *this, Value value, int row) {
    int constant = byte_code_add_constant(current(this), value);
    if (constant > UINT8_MAX) {
        compile_error(this, &this->previous, "Too many constants.");
        constant = 0;
    }
    write_two_op(current(this), OP_CONSTANT, (u8)constant, row);
    return (u8)constant;
}

static Rule *token_rule(enum TokenType type) {
    return &rules[type];
}

static HymnString *machine_intern_string(Machine *this, String *string) {
    HymnString *object = set_add_or_get(&this->strings, string);
    if (object->string != string) {
        string_delete(string);
    }
    return object;
}

static Value compile_intern_string(Machine *this, String *string) {
    HymnString *object = set_add_or_get(&this->strings, string);
    if (object->string == string) {
        reference_string(object);
    } else {
        string_delete(string);
    }
    return new_string_value(object);
}

static void machine_set_global(Machine *this, const char *name, Value value) {
    HymnString *intern = machine_intern_string(this, new_string(name));
    reference_string(intern);
    table_put(&this->globals, intern, value);
}

static bool check(Compiler *this, enum TokenType type) {
    return this->current.type == type;
}

static bool match(Compiler *this, enum TokenType type) {
    if (!check(this, type)) {
        return false;
    }
    advance(this);
    return true;
}

static inline void emit(Compiler *this, u8 b) {
    write_op(current(this), b, this->previous.row);
}

static inline void emit_two(Compiler *this, u8 b, u8 n) {
    write_two_op(current(this), b, n, this->previous.row);
}

static void compile_with_precedence(Compiler *this, enum Precedence precedence) {
    advance(this);
    Rule *rule = token_rule(this->previous.type);
    void (*prefix)(Compiler *, bool) = rule->prefix;
    if (prefix == NULL) {
        compile_error(this, &this->previous, "Syntax Error: Expected expression following `%.*s`.", this->previous.length, &this->source[this->previous.start]);
        return;
    }
    bool assign = precedence <= PRECEDENCE_ASSIGN;
    prefix(this, assign);
    while (precedence <= token_rule(this->current.type)->precedence) {
        advance(this);
        void (*infix)(Compiler *, bool) = token_rule(this->previous.type)->infix;
        if (infix == NULL) {
            compile_error(this, &this->previous, "Expected infix.");
            return;
        }
        infix(this, assign);
    }
    if (assign and match(this, TOKEN_ASSIGN)) {
        compile_error(this, &this->current, "Invalid assignment target.");
    }
}

static void consume(Compiler *this, enum TokenType type, const char *error) {
    if (this->current.type == type) {
        advance(this);
        return;
    }
    compile_error(this, &this->current, error);
}

static u8 push_hidden_local(Compiler *this) {
    Scope *scope = this->scope;
    if (scope->local_count == HYMN_UINT8_COUNT) {
        compile_error(this, &this->previous, "Too many local variables in scope.");
        return 0;
    }
    u8 index = (u8)scope->local_count++;
    Local *local = &scope->locals[index];
    local->name = (Token){0};
    local->constant = true;
    local->depth = scope->depth;
    return index;
}

static u8 arguments(Compiler *this) {
    u8 count = 0;
    if (!check(this, TOKEN_RIGHT_PAREN)) {
        do {
            expression(this);
            if (count == 255) {
                compile_error(this, &this->previous, "Can't have more than 255 function arguments.");
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
    Token *alpha = &this->previous;
    i64 number = (i64)strtoll(&this->source[alpha->start], NULL, 10);
    write_constant(this, new_int(number), alpha->row);
}

static void compile_float(Compiler *this, bool assign) {
    (void)assign;
    Token *alpha = &this->previous;
    double number = strtod(&this->source[alpha->start], NULL);
    write_constant(this, new_float(number), alpha->row);
}

static void compile_string(Compiler *this, bool assign) {
    (void)assign;
    Token *alpha = &this->previous;
    String *s = new_string_from_substring(this->source, alpha->start, alpha->start + alpha->length);
    write_constant(this, compile_intern_string(this->machine, s), alpha->row);
}

static u8 ident_constant(Compiler *this, Token *token) {
    String *s = new_string_from_substring(this->source, token->start, token->start + token->length);
    return (u8)byte_code_add_constant(current(this), compile_intern_string(this->machine, s));
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

static void compile_array(Compiler *this, bool assign) {
    (void)assign;
    write_constant(this, new_array_value(NULL), this->previous.row);
    if (match(this, TOKEN_RIGHT_SQUARE)) {
        return;
    }
    while (!check(this, TOKEN_RIGHT_SQUARE) and !check(this, TOKEN_EOF)) {
        emit(this, OP_DUPLICATE);
        expression(this);
        emit_two(this, OP_ARRAY_PUSH, OP_POP);
        if (!check(this, TOKEN_RIGHT_SQUARE)) {
            consume(this, TOKEN_COMMA, "Expected ','.");
        }
    }
    consume(this, TOKEN_RIGHT_SQUARE, "Expected ']' declaring array.");
}

static void compile_table(Compiler *this, bool assign) {
    (void)assign;
    write_constant(this, new_table_value(NULL), this->previous.row);
    if (match(this, TOKEN_RIGHT_CURLY)) {
        return;
    }
    while (!check(this, TOKEN_RIGHT_CURLY) and !check(this, TOKEN_EOF)) {
        emit(this, OP_DUPLICATE);
        consume(this, TOKEN_IDENT, "Expected property name");
        u8 name = ident_constant(this, &this->previous);
        consume(this, TOKEN_COLON, "Expected ':'.");
        expression(this);
        emit_two(this, OP_SET_PROPERTY, name);
        emit(this, OP_POP);
        if (!check(this, TOKEN_RIGHT_CURLY)) {
            consume(this, TOKEN_COMMA, "Expected ','.");
        }
    }
    consume(this, TOKEN_RIGHT_CURLY, "Expected '}' declaring table.");
}

static void function_delete(Function *this) {
    byte_code_delete(&this->code);
    string_delete(this->name);
    string_delete(this->script);
    ExceptList *except = this->except;
    while (except != NULL) {
        ExceptList *next = except->next;
        free(except);
        except = next;
    }
    free(this);
}

static void native_function_delete(NativeFunction *this) {
    string_delete(this->name);
    free(this);
}

static void push_local(Compiler *this, Token name, bool is_constant) {
    Scope *scope = this->scope;
    if (scope->local_count == HYMN_UINT8_COUNT) {
        compile_error(this, &name, "Too many local variables in scope.");
        return;
    }
    Local *local = &scope->locals[scope->local_count++];
    local->name = name;
    local->constant = is_constant;
    local->depth = -1;
}

static bool ident_match(Compiler *this, Token *a, Token *b) {
    if (a->length != b->length) {
        return false;
    }
    return memcmp(&this->source[a->start], &this->source[b->start], a->length) == 0;
}

static u8 variable(Compiler *this, bool is_constant, const char *error) {
    consume(this, TOKEN_IDENT, error);
    Scope *scope = this->scope;
    if (scope->depth == 0) {
        return ident_constant(this, &this->previous);
    }
    Token *name = &this->previous;
    for (int i = scope->local_count - 1; i >= 0; i--) {
        Local *local = &scope->locals[i];
        if (local->depth != -1 and local->depth < scope->depth) {
            break;
        } else if (ident_match(this, name, &local->name)) {
            compile_error(this, name, "Scope Error: Variable `%.*s` already exists in this scope.", name->length, &this->source[name->start]);
        }
    }
    push_local(this, *name, is_constant);
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

static void define_new_variable(Compiler *this, bool is_constant) {
    u8 v = variable(this, is_constant, "Syntax Error: Expected variable name.");
    consume(this, TOKEN_ASSIGN, "Assignment Error: Expected '=' after variable.");
    expression(this);
    finalize_variable(this, v);
}

static int resolve_local(Compiler *this, Token *name, bool *is_constant) {
    Scope *scope = this->scope;
    for (int i = scope->local_count - 1; i >= 0; i--) {
        Local *local = &scope->locals[i];
        if (ident_match(this, name, &local->name)) {
            if (local->depth == -1) {
                compile_error(this, name, "Reference Error: Local variable `%.*s` referenced before assignment.", name->length, &this->source[name->start]);
            }
            *is_constant = local->constant;
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
        // TODO: CONST FOR GLOBALS
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
    named_variable(this, this->previous, assign);
}

static void compile_unary(Compiler *this, bool assign) {
    (void)assign;
    enum TokenType type = this->previous.type;
    compile_with_precedence(this, PRECEDENCE_UNARY);
    switch (type) {
    case TOKEN_NOT: emit(this, OP_NOT); break;
    case TOKEN_SUBTRACT: emit(this, OP_NEGATE); break;
    case TOKEN_BIT_NOT: emit(this, OP_BIT_NOT); break;
    default: return;
    }
}

static void compile_binary(Compiler *this, bool assign) {
    (void)assign;
    enum TokenType type = this->previous.type;
    Rule *rule = token_rule(type);
    compile_with_precedence(this, (enum Precedence)(rule->precedence + 1));
    switch (type) {
    case TOKEN_ADD: emit(this, OP_ADD); break;
    case TOKEN_SUBTRACT: emit(this, OP_SUBTRACT); break;
    case TOKEN_MODULO: emit(this, OP_MODULO); break;
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
    default: return;
    }
}

static void compile_dot(Compiler *this, bool assign) {
    consume(this, TOKEN_IDENT, "Expected property name after '.'.");
    u8 name = ident_constant(this, &this->previous);
    if (assign and match(this, TOKEN_ASSIGN)) {
        expression(this);
        emit_two(this, OP_SET_PROPERTY, name);
    } else {
        emit_two(this, OP_GET_PROPERTY, name);
    }
}

static void compile_square(Compiler *this, bool assign) {
    if (match(this, TOKEN_COLON)) {
        write_constant(this, new_int(0), this->previous.row);
        if (match(this, TOKEN_RIGHT_SQUARE)) {
            write_constant(this, new_none(), this->previous.row);
        } else {
            expression(this);
            consume(this, TOKEN_RIGHT_SQUARE, "Expected ']' after expression.");
        }
        emit(this, OP_SLICE);
    } else {
        expression(this);
        if (match(this, TOKEN_COLON)) {
            if (match(this, TOKEN_RIGHT_SQUARE)) {
                write_constant(this, new_none(), this->previous.row);
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
        compile_error(this, &this->previous, "Jump offset too large.");
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
    scope_init(this, &scope, type);

    begin_scope(this);

    consume(this, TOKEN_LEFT_PAREN, "Expected '(' after function name.");

    if (!check(this, TOKEN_RIGHT_PAREN)) {
        do {
            this->scope->func->arity++;
            if (this->scope->func->arity > 255) {
                compile_error(this, &this->previous, "Can't have more than 255 function parameters.");
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
    write_constant(this, new_func_value(func), this->previous.row);
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

        struct JumpList *next = safe_calloc(1, sizeof(struct JumpList));
        next->jump = emit_jump(this, OP_JUMP);

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

static bool compile_literal(Compiler *this) {
    advance(this);
    switch (this->previous.type) {
    case TOKEN_NONE:
        compile_none(this, false);
        return true;
    case TOKEN_TRUE:
        compile_true(this, false);
        return true;
    case TOKEN_FALSE:
        compile_false(this, false);
        return true;
    case TOKEN_INTEGER:
        compile_integer(this, false);
        return true;
    case TOKEN_FLOAT:
        compile_float(this, false);
        return true;
    case TOKEN_STRING:
        compile_string(this, false);
        return true;
    default:
        return false;
    }
}

static void switch_statement(Compiler *this) {

    begin_scope(this);

    u8 local = push_hidden_local(this);
    expression(this);

    if (!check(this, TOKEN_CASE)) {
        compile_error(this, &this->current, "Expected case.");
        return;
    }

    int jump = -1;

    struct JumpList *head = NULL;
    struct JumpList *tail = NULL;

    while (match(this, TOKEN_CASE)) {

        if (jump != -1) {
            patch_jump(this, jump);
            emit(this, OP_POP);
        }

        if (!compile_literal(this)) {
            compile_error(this, &this->current, "Expected literal for case.");
        }
        emit_two(this, OP_GET_LOCAL, local);
        emit(this, OP_EQUAL);

        struct JumpList *body = NULL;

        if (match(this, TOKEN_OR)) {
            body = safe_calloc(1, sizeof(struct JumpList));
            struct JumpList *link = body;
            body->jump = emit_jump(this, OP_JUMP_IF_TRUE);
            emit(this, OP_POP);

            while (true) {
                if (!compile_literal(this)) {
                    compile_error(this, &this->current, "Expected literal after 'or' in case.");
                }
                emit_two(this, OP_GET_LOCAL, local);
                emit(this, OP_EQUAL);

                if (match(this, TOKEN_OR)) {
                    struct JumpList *next = safe_calloc(1, sizeof(struct JumpList));
                    next->jump = emit_jump(this, OP_JUMP_IF_TRUE);
                    emit(this, OP_POP);

                    link->next = next;
                    link = next;
                } else {
                    break;
                }
            }
        }

        jump = emit_jump(this, OP_JUMP_IF_FALSE);

        while (body != NULL) {
            patch_jump(this, body->jump);
            struct JumpList *next = body->next;
            free(body);
            body = next;
        }

        emit(this, OP_POP);

        begin_scope(this);
        while (!check(this, TOKEN_CASE) and !check(this, TOKEN_ELSE) and !check(this, TOKEN_END) and !check(this, TOKEN_EOF)) {
            declaration(this);
        }
        end_scope(this);

        struct JumpList *next = safe_calloc(1, sizeof(struct JumpList));
        next->jump = emit_jump(this, OP_JUMP);

        if (head == NULL) {
            head = next;
            tail = next;
        } else {
            tail->next = next;
            tail = next;
        }
    }

    if (jump != -1) {
        patch_jump(this, jump);
        emit(this, OP_POP);
    }

    if (match(this, TOKEN_ELSE)) {
        block(this);
    }

    while (head != NULL) {
        patch_jump(this, head->jump);
        struct JumpList *next = head->next;
        free(head);
        head = next;
    }

    end_scope(this);

    consume(this, TOKEN_END, "Expected 'end' after switch statement.");
}

static void emit_loop(Compiler *this, int start) {
    emit(this, OP_LOOP);
    int offset = current(this)->count - start + 2;
    if (offset > UINT16_MAX) {
        compile_error(this, &this->previous, "Loop is too large.");
    }
    emit_two(this, (offset >> 8) & UINT8_MAX, offset & UINT8_MAX);
}

static void patch_jump_list(Compiler *this) {
    while (this->jump != NULL) {
        int depth = 1;
        if (this->loop != NULL) {
            depth = this->loop->depth + 1;
        }
        if (this->jump->depth < depth) {
            break;
        }
        patch_jump(this, this->jump->jump);
        struct JumpList *next = this->jump->next;
        free(this->jump);
        this->jump = next;
    }
}

static void patch_iterator_jump_list(Compiler *this) {
    while (this->iterator_jump != NULL) {
        int depth = 1;
        if (this->loop != NULL) {
            depth = this->loop->depth + 1;
        }
        if (this->iterator_jump->depth < depth) {
            break;
        }
        patch_jump(this, this->iterator_jump->jump);
        struct JumpList *next = this->iterator_jump->next;
        free(this->iterator_jump);
        this->iterator_jump = next;
    }
}

static void iterate_statement(Compiler *this) {

    begin_scope(this);

    // parameters

    u8 index;

    u8 value = (u8)this->scope->local_count;
    variable(this, true, "Expected parameter name.");
    local_initialize(this);

    if (match(this, TOKEN_COMMA)) {
        index = value;
        write_constant(this, new_int(0), this->previous.row);

        value = (u8)this->scope->local_count;
        variable(this, true, "Expected second parameter name.");
        local_initialize(this);
        emit(this, OP_NONE);
    } else {
        emit(this, OP_NONE);

        index = push_hidden_local(this);
        write_constant(this, new_int(0), this->previous.row);
    }

    consume(this, TOKEN_IN, "Expected 'in' after iterate parameters.");

    // setup

    u8 local = push_hidden_local(this);
    expression(this);

    u8 length = push_hidden_local(this);
    emit_two(this, OP_GET_LOCAL, local);
    emit(this, OP_LEN);

    // compare

    int compare = current(this)->count;

    struct LoopList loop = {.start = -1, .depth = this->scope->depth + 1, .next = this->loop};
    this->loop = &loop;

    emit_two(this, OP_GET_LOCAL, index);
    emit_two(this, OP_GET_LOCAL, length);
    emit(this, OP_LESS);

    int jump = emit_jump(this, OP_JUMP_IF_FALSE);
    emit(this, OP_POP);

    // update

    emit_two(this, OP_GET_LOCAL, local);
    emit_two(this, OP_GET_LOCAL, index);
    emit(this, OP_GET_DYNAMIC);

    emit_two(this, OP_SET_LOCAL, value);
    emit(this, OP_POP);

    // inside

    block(this);

    // increment

    patch_iterator_jump_list(this);

    emit_two(this, OP_GET_LOCAL, index);
    write_constant(this, new_int(1), this->previous.row);
    emit(this, OP_ADD);
    emit_two(this, OP_SET_LOCAL, index);
    emit(this, OP_POP);

    // next

    emit_loop(this, compare);

    // end

    this->loop = loop.next;

    patch_jump(this, jump);
    emit(this, OP_POP);

    patch_jump_list(this);
    end_scope(this);

    consume(this, TOKEN_END, "Expected 'end'.");
}

static void for_statement(Compiler *this) {
    begin_scope(this);

    // assign

    if (match(this, TOKEN_LET)) {
        define_new_variable(this, false);
    } else if (match(this, TOKEN_CONST)) {
        define_new_variable(this, true);
    } else if (!check(this, TOKEN_SEMICOLON)) {
        expression_statement(this);
    }

    consume(this, TOKEN_SEMICOLON, "Expected ';' in for.");

    // compare

    int compare = current(this)->count;

    expression(this);

    int jump = emit_jump(this, OP_JUMP_IF_FALSE);
    emit(this, OP_POP);

    consume(this, TOKEN_SEMICOLON, "Expected ';' in for.");

    // increment

    int body = emit_jump(this, OP_JUMP);
    int increment = current(this)->count;

    struct LoopList loop = {.start = increment, .depth = this->scope->depth + 1, .next = this->loop};
    this->loop = &loop;

    expression(this);

    emit(this, OP_POP);
    emit_loop(this, compare);

    // body

    patch_jump(this, body);

    block(this);
    emit_loop(this, increment);

    // end

    this->loop = loop.next;

    patch_jump(this, jump);
    emit(this, OP_POP);

    patch_jump_list(this);
    end_scope(this);

    consume(this, TOKEN_END, "Expected 'end' after for loop.");
}

static void while_statement(Compiler *this) {
    int start = current(this)->count;

    struct LoopList loop = {.start = start, .depth = this->scope->depth + 1, .next = this->loop};
    this->loop = &loop;

    expression(this);
    int jump = emit_jump(this, OP_JUMP_IF_FALSE);

    emit(this, OP_POP);
    block(this);
    emit_loop(this, start);

    this->loop = loop.next;

    patch_jump(this, jump);
    emit(this, OP_POP);

    patch_jump_list(this);

    consume(this, TOKEN_END, "Expected 'end' after while loop.");
}

static void return_statement(Compiler *this) {
    if (this->scope->type == TYPE_SCRIPT) {
        compile_error(this, &this->previous, "Can't return from outside a function.");
    }
    expression(this);
    emit(this, OP_RETURN);
}

static void pop_stack_loop(Compiler *this) {
    int depth = this->loop->depth;
    Scope *scope = this->scope;
    for (int i = scope->local_count; i > 0; i--) {
        if (scope->locals[i - 1].depth < depth) {
            return;
        }
        emit(this, OP_POP);
    }
}

static void break_statement(Compiler *this) {
    if (this->loop == NULL) {
        compile_error(this, &this->previous, "Can't use 'break' outside of a loop.");
    }
    pop_stack_loop(this);
    struct JumpList *jump_next = this->jump;
    struct JumpList *jump = safe_malloc(sizeof(struct JumpList));
    jump->jump = emit_jump(this, OP_JUMP);
    jump->depth = this->loop->depth;
    jump->next = jump_next;
    this->jump = jump;
}

static void continue_statement(Compiler *this) {
    if (this->loop == NULL) {
        compile_error(this, &this->previous, "Can't use 'continue' outside of a loop.");
    }
    pop_stack_loop(this);
    if (this->loop->start == -1) {
        struct JumpList *jump_next = this->iterator_jump;
        struct JumpList *jump = safe_malloc(sizeof(struct JumpList));
        jump->jump = emit_jump(this, OP_JUMP);
        jump->depth = this->loop->depth;
        jump->next = jump_next;
        this->iterator_jump = jump;
    } else {
        emit_loop(this, this->loop->start);
    }
}

static void try_statement(Compiler *this) {

    ExceptList *except = safe_calloc(1, sizeof(ExceptList));
    except->stack = this->scope->local_count;
    except->start = (usize)current(this)->count;

    Function *func = current_func(this);
    except->next = func->except;
    func->except = except;

    begin_scope(this);
    while (!check(this, TOKEN_EXCEPT) and !check(this, TOKEN_EOF)) {
        declaration(this);
    }
    end_scope(this);

    int jump = emit_jump(this, OP_JUMP);

    consume(this, TOKEN_EXCEPT, "Expected 'except' after 'try'.");

    except->end = (usize)current(this)->count;

    begin_scope(this);
    u8 message = variable(this, false, "Expected variable after 'except'.");
    finalize_variable(this, message);
    while (!check(this, TOKEN_END) and !check(this, TOKEN_EOF)) {
        declaration(this);
    }
    end_scope(this);

    consume(this, TOKEN_END, "Expected 'end' after 'except'.");

    patch_jump(this, jump);
}

static void print_statement(Compiler *this) {
    expression(this);
    emit(this, OP_PRINT);
}

static void use_statement(Compiler *this) {
    expression(this);
    emit(this, OP_USE);
}

static void do_statement(Compiler *this) {
    expression(this);
    emit(this, OP_DO);
}

static void throw_statement(Compiler *this) {
    expression(this);
    emit(this, OP_THROW);
}

static void statement(Compiler *this) {
    if (match(this, TOKEN_PRINT)) {
        print_statement(this);
    } else if (match(this, TOKEN_DO)) {
        do_statement(this);
    } else if (match(this, TOKEN_USE)) {
        use_statement(this);
    } else if (match(this, TOKEN_IF)) {
        if_statement(this);
    } else if (match(this, TOKEN_SWITCH)) {
        switch_statement(this);
    } else if (match(this, TOKEN_ITERATE)) {
        iterate_statement(this);
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
    } else if (match(this, TOKEN_TRY)) {
        try_statement(this);
    } else if (match(this, TOKEN_THROW)) {
        throw_statement(this);
    } else if (match(this, TOKEN_PASS)) {
        // do nothing
    } else if (match(this, TOKEN_BEGIN)) {
        block(this);
        consume(this, TOKEN_END, "Expected 'end' after block.");
    } else {
        expression_statement(this);
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
    consume(this, TOKEN_LEFT_PAREN, "Missing '(' for paramters in `index` function.");
    expression(this);
    consume(this, TOKEN_COMMA, "Expected 2 arguments for `index` function.");
    expression(this);
    consume(this, TOKEN_RIGHT_PAREN, "Missing ')' after parameters in `index` function.");
    emit(this, OP_INDEX);
}

static void expression_statement(Compiler *this) {
    expression(this);
    emit(this, OP_POP);
}

static void expression(Compiler *this) {
    compile_with_precedence(this, PRECEDENCE_ASSIGN);
}

static inline Frame *parent_frame(Machine *this, int offset) {
    int frame_count = this->frame_count;
    if (offset > frame_count) {
        return NULL;
    }
    return &this->frames[frame_count - offset];
}

static inline Frame *current_frame(Machine *this) {
    return &this->frames[this->frame_count - 1];
}

struct CompileReturn {
    Function *func;
    char *error;
};

static struct CompileReturn compile(Machine *machine, const char *script, const char *source) {

    Scope scope = {0};

    Compiler c = new_compiler(script, source, machine, &scope);
    Compiler *compiler = &c;

    advance(compiler);
    while (!match(compiler, TOKEN_EOF)) {
        declaration(compiler);
    }

    Function *func = end_function(compiler);
    char *error = NULL;

    if (compiler->error) {
        error = string_to_chars(compiler->error);
        string_delete(compiler->error);
    }

    return (struct CompileReturn){.func = func, .error = error};
}

#if defined HYMN_DEBUG_TRACE || defined HYMN_DEBUG_CODE
static usize debug_constant_instruction(String **debug, const char *name, ByteCode *this, usize index) {
    u8 constant = this->instructions[index + 1];
    *debug = string_append_format(*debug, "%s: [", name);
    String *value = debug_value_to_string(this->constants.values[constant]);
    *debug = string_append(*debug, value);
    string_delete(value);
    *debug = string_append(*debug, "]");
    return index + 2;
}

static usize debug_byte_instruction(String **debug, const char *name, ByteCode *this, usize index) {
    u8 b = this->instructions[index + 1];
    *debug = string_append_format(*debug, "%s: [%d]", name, b);
    return index + 2;
}

static usize debug_jump_instruction(String **debug, const char *name, int sign, ByteCode *this, usize index) {
    u16 jump = (u16)(this->instructions[index + 1] << 8) | (u16)this->instructions[index + 2];
    *debug = string_append_format(*debug, "%s: [%zu] -> [%zu]", name, index, index + 3 + sign * jump);
    return index + 3;
}

static usize debug_instruction(String **debug, const char *name, usize index) {
    *debug = string_append_format(*debug, "%s", name);
    return index + 1;
}

static usize disassemble_instruction(String **debug, ByteCode *this, usize index) {
    *debug = string_append_format(*debug, "%04zu ", index);
    if (index > 0 and this->rows[index] == this->rows[index - 1]) {
        *debug = string_append(*debug, "   | ");
    } else {
        *debug = string_append_format(*debug, "%4d ", this->rows[index]);
    }
    u8 op = this->instructions[index];
    switch (op) {
    case OP_ADD: return debug_instruction(debug, "OP_ADD", index);
    case OP_ARRAY_INSERT: return debug_instruction(debug, "OP_ARRAY_INSERT", index);
    case OP_ARRAY_POP: return debug_instruction(debug, "OP_ARRAY_POP", index);
    case OP_ARRAY_PUSH: return debug_instruction(debug, "OP_ARRAY_PUSH", index);
    case OP_BIT_AND: return debug_instruction(debug, "OP_BIT_AND", index);
    case OP_BIT_LEFT_SHIFT: return debug_instruction(debug, "OP_BIT_LEFT_SHIFT", index);
    case OP_BIT_NOT: return debug_instruction(debug, "OP_BIT_NOT", index);
    case OP_BIT_OR: return debug_instruction(debug, "OP_BIT_OR", index);
    case OP_BIT_RIGHT_SHIFT: return debug_instruction(debug, "OP_BIT_RIGHT_SHIFT", index);
    case OP_BIT_XOR: return debug_instruction(debug, "OP_BIT_XOR", index);
    case OP_CALL: return debug_byte_instruction(debug, "OP_CALL", this, index);
    case OP_CLEAR: return debug_instruction(debug, "OP_CLEAR", index);
    case OP_CONSTANT: return debug_constant_instruction(debug, "OP_CONSTANT", this, index);
    case OP_COPY: return debug_instruction(debug, "OP_COPY", index);
    case OP_DO: return debug_instruction(debug, "OP_DO", index);
    case OP_DUPLICATE: return debug_instruction(debug, "OP_DUPLICATE", index);
    case OP_DEFINE_GLOBAL: return debug_constant_instruction(debug, "OP_DEFINE_GLOBAL", this, index);
    case OP_DELETE: return debug_instruction(debug, "OP_DELETE", index);
    case OP_DIVIDE: return debug_instruction(debug, "OP_DIVIDE", index);
    case OP_EQUAL: return debug_instruction(debug, "OP_EQUAL", index);
    case OP_FALSE: return debug_instruction(debug, "OP_FALSE", index);
    case OP_GET_DYNAMIC: return debug_instruction(debug, "OP_GET_DYNAMIC", index);
    case OP_GET_GLOBAL: return debug_constant_instruction(debug, "OP_GET_GLOBAL", this, index);
    case OP_GET_LOCAL: return debug_byte_instruction(debug, "OP_GET_LOCAL", this, index);
    case OP_GET_PROPERTY: return debug_constant_instruction(debug, "OP_GET_PROPERTY", this, index);
    case OP_GREATER: return debug_instruction(debug, "OP_GREATER", index);
    case OP_GREATER_EQUAL: return debug_instruction(debug, "OP_GREATER_EQUAL", index);
    case OP_INDEX: return debug_instruction(debug, "OP_INDEX", index);
    case OP_JUMP: return debug_jump_instruction(debug, "OP_JUMP", 1, this, index);
    case OP_JUMP_IF_FALSE: return debug_jump_instruction(debug, "OP_JUMP_IF_FALSE", 1, this, index);
    case OP_JUMP_IF_TRUE: return debug_jump_instruction(debug, "OP_JUMP_IF_TRUE", 1, this, index);
    case OP_KEYS: return debug_instruction(debug, "OP_KEYS", index);
    case OP_LEN: return debug_instruction(debug, "OP_LEN", index);
    case OP_LESS: return debug_instruction(debug, "OP_LESS", index);
    case OP_LESS_EQUAL: return debug_instruction(debug, "OP_LESS_EQUAL", index);
    case OP_LOOP: return debug_jump_instruction(debug, "OP_LOOP", -1, this, index);
    case OP_MODULO: return debug_instruction(debug, "OP_MODULO", index);
    case OP_MULTIPLY: return debug_instruction(debug, "OP_MULTIPLY", index);
    case OP_NEGATE: return debug_instruction(debug, "OP_NEGATE", index);
    case OP_NONE: return debug_instruction(debug, "OP_NONE", index);
    case OP_NOT: return debug_instruction(debug, "OP_NOT", index);
    case OP_NOT_EQUAL: return debug_instruction(debug, "OP_NOT_EQUAL", index);
    case OP_POP: return debug_instruction(debug, "OP_POP", index);
    case OP_PRINT: return debug_instruction(debug, "OP_PRINT", index);
    case OP_THROW: return debug_instruction(debug, "OP_THROW", index);
    case OP_SET_DYNAMIC: return debug_instruction(debug, "OP_SET_DYNAMIC", index);
    case OP_SET_GLOBAL: return debug_constant_instruction(debug, "OP_SET_GLOBAL", this, index);
    case OP_SET_LOCAL: return debug_byte_instruction(debug, "OP_SET_LOCAL", this, index);
    case OP_SET_PROPERTY: return debug_constant_instruction(debug, "OP_SET_PROPERTY", this, index);
    case OP_SLICE: return debug_instruction(debug, "OP_SLICE", index);
    case OP_SUBTRACT: return debug_instruction(debug, "OP_SUBTRACT", index);
    case OP_TO_FLOAT: return debug_instruction(debug, "OP_TO_FLOAT", index);
    case OP_TO_INTEGER: return debug_instruction(debug, "OP_TO_INTEGER", index);
    case OP_TO_STRING: return debug_instruction(debug, "OP_TO_STRING", index);
    case OP_TRUE: return debug_instruction(debug, "OP_TRUE", index);
    case OP_TYPE: return debug_instruction(debug, "OP_TYPE", index);
    case OP_USE: return debug_instruction(debug, "OP_USE", index);
    case OP_RETURN: return debug_instruction(debug, "OP_RETURN", index);
    default: *debug = string_append_format(*debug, "UNKNOWN OPCODE %d\n", op); return index + 1;
    }
}
#endif

#ifdef HYMN_DEBUG_CODE
void disassemble_byte_code(ByteCode *this, const char *name) {
    printf("== %s ==\n", name);
    String *debug = new_string("");
    usize offset = 0;
    while (offset < this->count) {
        offset = disassemble_instruction(&debug, this, offset);
        printf("%s\n", debug);
        string_zero(debug);
    }
    string_delete(debug);
}
#endif

static void machine_reset_stack(Machine *this) {
    this->stack_top = 0;
    this->frame_count = 0;
}

static inline bool is_object(Value value) {
    switch (value.is) {
    case HYMN_VALUE_STRING:
    case HYMN_VALUE_ARRAY:
    case HYMN_VALUE_TABLE:
    case HYMN_VALUE_FUNC:
        return true;
    default:
        return false;
    }
}

#ifdef HYMN_DEBUG_REFERENCE
static void debug_reference(Value value) {
    if (is_object(value)) {
        int count = as_object(value)->count;
        printf("REF: [%p]: %d, [", (void *)as_object(value), count);
        debug_value(value);
        printf("]\n");
        assert(count >= 0);
    }
}

static void debug_dereference(Value value) {
    if (is_object(value)) {
        Object *object = as_object(value);
        int count = object->count - 1;
        if (count == 0) {
            printf("FREE: [");
        } else if (count < 0) {
            printf("BAD: [");
        } else {
            printf("DEREF: %d: [", count);
        }
        debug_value(value);
        printf("]\n");
    }
}
#endif

static inline void reference_string(HymnString *string) {
    string->object.count++;
#ifdef HYMN_DEBUG_REFERENCE
    debug_reference(new_string_value(string));
#endif
}

static void reference(Value value) {
    if (is_object(value)) {
        as_object(value)->count++;
#ifdef HYMN_DEBUG_REFERENCE
        debug_reference(value);
#endif
    }
}

static inline void dereference_string(Machine *this, HymnString *string) {
#ifdef HYMN_DEBUG_REFERENCE
    debug_dereference(new_string_value(string));
#endif
    int count = --(string->object.count);
    assert(count >= 0);
    if (count == 0) {
        set_remove(&this->strings, string->string);
        string_delete(string->string);
        free(string);
    }
}

static void dereference(Machine *this, Value value) {
    switch (value.is) {
    case HYMN_VALUE_STRING: {
        HymnString *string = as_hymn_string(value);
        dereference_string(this, string);
        break;
    }
    case HYMN_VALUE_ARRAY: {
#ifdef HYMN_DEBUG_REFERENCE
        debug_dereference(value);
#endif
        Array *array = as_array(value);
        int count = --(array->object.count);
        assert(count >= 0);
        if (count == 0) {
            array_delete(this, array);
        }
        break;
    }
    case HYMN_VALUE_TABLE: {
#ifdef HYMN_DEBUG_REFERENCE
        debug_dereference(value);
#endif
        Table *table = as_table(value);
        int count = --(table->object.count);
        assert(count >= 0);
        if (count == 0) {
            table_delete(this, table);
        }
        break;
    }
    case HYMN_VALUE_FUNC: {
#ifdef HYMN_DEBUG_REFERENCE
        debug_dereference(value);
#endif
        Function *func = as_func(value);
        int count = --(func->object.count);
        assert(count >= 0);
        if (count == 0) {
            function_delete(func);
        }
        break;
    }
    default:
        return;
    }
}

static void machine_push(Machine *this, Value value) {
    this->stack[this->stack_top++] = value;
}

static Value machine_peek(Machine *this, usize dist) {
    if (dist > this->stack_top) {
        fprintf(stderr, "Nothing on stack to peek");
        return new_none();
    }
    return this->stack[this->stack_top - dist];
}

static Value machine_pop(Machine *this) {
    if (this->stack_top == 0) {
        fprintf(stderr, "Nothing on stack to pop");
        return new_none();
    }
    return this->stack[--this->stack_top];
}

static void machine_push_intern_string(Machine *this, String *string) {
    HymnString *intern = machine_intern_string(this, string);
    reference_string(intern);
    PUSH(new_string_value(intern))
}

static String *value_to_new_string(Value value) {
    switch (value.is) {
    case HYMN_VALUE_UNDEFINED:
    case HYMN_VALUE_NONE:
        return new_string(STRING_NONE);
    case HYMN_VALUE_BOOL:
        return as_bool(value) ? new_string(STRING_TRUE) : new_string(STRING_FALSE);
    case HYMN_VALUE_INTEGER:
        return int64_to_string(as_int(value));
    case HYMN_VALUE_FLOAT:
        return float64_to_string(as_float(value));
    case HYMN_VALUE_STRING:
        return string_copy(as_string(value));
    case HYMN_VALUE_ARRAY:
        return string_format("[array %p]", as_array(value));
    case HYMN_VALUE_TABLE:
        return string_format("[table %p]", as_table(value));
    case HYMN_VALUE_FUNC:
        return string_format("%s", as_func(value)->name);
    case HYMN_VALUE_FUNC_NATIVE:
        return string_format("%s", as_native(value)->name);
    case HYMN_VALUE_POINTER:
        return string_format("[pointer %p]", as_pointer(value));
    }
    return NULL;
}

static Frame *machine_exception(Machine *this) {
    Frame *frame = current_frame(this);
    while (true) {
        ExceptList *except = NULL;
        ExceptList *range = frame->func->except;
        while (range != NULL) {
            if (frame->ip >= range->start and frame->ip <= range->end) {
                except = range;
                break;
            }
            range = range->next;
        }
        POP(result)
        if (except != NULL) {
            while (this->stack_top > frame->stack + except->stack) {
                DEREF(this->stack[--this->stack_top])
            }
            frame->ip = except->end;
            PUSH(result)
            return frame;
        }
        while (this->stack_top != frame->stack) {
            DEREF(this->stack[--this->stack_top])
        }
        this->frame_count--;
        if (this->frame_count == 0 or frame->func->name == NULL) {
            assert(this->error == NULL);
            this->error = value_to_new_string(result);
            DEREF(result)
            return NULL;
        }
        PUSH(result)
        frame = current_frame(this);
    }
}

static String *machine_stacktrace(Machine *this) {
    String *trace = new_string("");

    for (int i = this->frame_count - 1; i >= 0; i--) {
        Frame *frame = &this->frames[i];
        Function *func = frame->func;
        usize ip = frame->ip - 1;
        int row = func->code.rows[ip];

        trace = string_append(trace, "at");

        if (func->name != NULL) {
            trace = string_append_format(trace, " %s", func->name);
        }

        if (func->script == NULL) {
            trace = string_append_format(trace, " script:");
        } else {
            trace = string_append_format(trace, " %s:", func->script);
        }

        trace = string_append_format(trace, "%d\n", row);
    }

    return trace;
}

static Frame *machine_push_error(Machine *this, String *error) {
    HymnString *message = machine_intern_string(this, error);
    reference_string(message);
    PUSH(new_string_value(message))
    return machine_exception(this);
}

static Frame *machine_throw_existing_error(Machine *this, char *error) {
    String *message = new_string(error);
    free(error);
    return machine_push_error(this, message);
}

static Frame *machine_throw_error(Machine *this, const char *format, ...) {

    String *error = new_string("");

    va_list ap;
    va_start(ap, format);
    int len = vsnprintf(NULL, 0, format, ap);
    va_end(ap);
    char *chars = safe_malloc((len + 1) * sizeof(char));
    va_start(ap, format);
    len = vsnprintf(chars, len + 1, format, ap);
    va_end(ap);
    error = string_append(error, chars);
    free(chars);

#ifdef HYMN_INCLUDE_STACKTRACE
    String *trace = machine_stacktrace(this);
    error = string_append(error, "\n\n");
    error = string_append(error, trace);
    string_delete(trace);
#endif

    return machine_push_error(this, error);
}

static bool machine_equal(Value a, Value b) {
    switch (a.is) {
    case HYMN_VALUE_NONE: return is_none(b);
    case HYMN_VALUE_BOOL: return is_bool(b) && as_bool(a) == as_bool(b);
    case HYMN_VALUE_INTEGER:
        switch (b.is) {
        case HYMN_VALUE_INTEGER: return as_int(a) == as_int(b);
        case HYMN_VALUE_FLOAT: return (double)as_int(a) == as_float(b);
        default: return false;
        }
    case HYMN_VALUE_FLOAT:
        switch (b.is) {
        case HYMN_VALUE_INTEGER: return as_float(a) == (double)as_int(b);
        case HYMN_VALUE_FLOAT: return as_float(a) == as_float(b);
        default: return false;
        }
    case HYMN_VALUE_STRING:
    case HYMN_VALUE_ARRAY:
    case HYMN_VALUE_TABLE:
    case HYMN_VALUE_FUNC:
        return b.is == a.is && as_object(a) == as_object(b);
    case HYMN_VALUE_FUNC_NATIVE:
        return is_native(b) && as_native(a) == as_native(b);
    default: return false;
    }
}

static bool machine_false(Value value) {
    switch (value.is) {
    case HYMN_VALUE_NONE: return true;
    case HYMN_VALUE_BOOL: return !as_bool(value);
    case HYMN_VALUE_INTEGER: return as_int(value) == 0;
    case HYMN_VALUE_FLOAT: return as_float(value) == 0.0;
    case HYMN_VALUE_STRING: return string_len(as_string(value)) == 0;
    case HYMN_VALUE_ARRAY: return as_array(value) == NULL;
    case HYMN_VALUE_TABLE: return as_table(value) == NULL;
    case HYMN_VALUE_FUNC: return as_func(value) == NULL;
    case HYMN_VALUE_FUNC_NATIVE: return as_native(value) == NULL;
    default: return false;
    }
}

static Frame *machine_call(Machine *this, Function *func, int count) {
    if (count != func->arity) {
        return machine_throw_error(this, "Expected %d function arguments but found %d.", func->arity, count);
    } else if (this->frame_count == HYMN_FRAMES_MAX) {
        return machine_throw_error(this, "Stack overflow.");
    }

    Frame *frame = &this->frames[this->frame_count++];
    frame->func = func;
    frame->ip = 0;
    frame->stack = this->stack_top - count - 1;

    return frame;
}

static Frame *machine_call_value(Machine *this, Value call, int count) {
    switch (call.is) {
    case HYMN_VALUE_FUNC:
        return machine_call(this, as_func(call), count);
    case HYMN_VALUE_FUNC_NATIVE: {
        NativeCall func = as_native(call)->func;
        Value result = func(this, count, &this->stack[this->stack_top - count]);
        reference(result);
        usize top = this->stack_top - (count + 1);
        while (this->stack_top != top) {
            DEREF(this->stack[--this->stack_top])
        }
        PUSH(result)
        return current_frame(this);
    }
    default:
        return machine_throw_error(this, "Only functions can be called.");
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

static Frame *machine_do(Machine *this, HymnString *source) {

    struct CompileReturn result = compile(this, NULL, source->string);

    Function *func = result.func;
    char *error = result.error;

    if (error) {
        function_delete(func);
        return machine_throw_existing_error(this, error);
    }

    Value function = new_func_value(func);
    reference(function);

    PUSH(function)
    machine_call(this, func, 0);

    error = machine_interpret(this);
    if (error) {
        return machine_throw_existing_error(this, error);
    }

    return current_frame(this);
}

static Frame *machine_import(Machine *this, HymnString *file) {

    Table *imports = this->imports;

    String *script = NULL;
    int p = 1;
    while (true) {
        Frame *frame = parent_frame(this, p);
        if (frame == NULL) break;
        script = frame->func->script;
        if (script) break;
        p++;
    }
    String *parent = script ? path_parent(script) : NULL;

    HymnString *module = NULL;

    Array *paths = this->paths;
    usize size = paths->length;
    for (usize i = 0; i < size; i++) {
        Value value = paths->items[i];
        if (!is_string(value)) {
            continue;
        }
        String *question = as_string(value);

        String *replace = string_replace(question, "<path>", file->string);
        String *path = parent ? string_replace(replace, "<parent>", parent) : string_copy(replace);

        HymnString *use = machine_intern_string(this, path_absolute(path));
        reference_string(use);

        string_delete(path);
        string_delete(replace);

        if (!is_undefined(table_get(imports, use))) {
            dereference_string(this, use);
            if (parent) string_delete(parent);
            return current_frame(this);
        }

        if (file_exists(use->string)) {
            module = use;
            break;
        }

        dereference_string(this, use);
    }

    if (parent) string_delete(parent);

    if (module == NULL) {
        return machine_throw_error(this, "Import not found: %s", file->string);
    }

    table_put(imports, module, new_bool(true));

    String *source = cat(module->string);

    struct CompileReturn result = compile(this, module->string, source);

    Function *func = result.func;
    char *error = result.error;

    string_delete(source);

    if (error) {
        function_delete(func);
        return machine_throw_existing_error(this, error);
    }

    Value function = new_func_value(func);
    reference(function);

    PUSH(function)
    machine_call(this, func, 0);

    error = machine_interpret(this);
    if (error) {
        return machine_throw_existing_error(this, error);
    }

    return current_frame(this);
}

static void machine_run(Machine *this) {
    Frame *frame = current_frame(this);
    while (true) {
#if defined HYMN_DEBUG_TRACE || defined HYMN_DEBUG_STACK
        {
            String *debug = new_string("");
#ifdef HYMN_DEBUG_TRACE
            disassemble_instruction(&debug, &frame->func->code, frame->ip);
#endif

#ifdef HYMN_DEBUG_STACK
            if (this->stack_top > 0) {
                if (string_len(debug) > 0) {
                    while (string_len(debug) < 70) {
                        debug = string_append_char(debug, ' ');
                    }
                }
                for (usize i = 0; i < this->stack_top; i++) {
                    debug = string_append_char(debug, '[');
                    String *stack_debug = debug_value_to_string(this->stack[i]);
                    debug = string_append(debug, stack_debug);
                    string_delete(stack_debug);
                    debug = string_append(debug, "] ");
                }
            }
#endif
            printf("%s\n", debug);
            string_delete(debug);
        }
#endif
        u8 op = read_byte(frame);
        switch (op) {
        case OP_RETURN: {
            POP(result)
            this->frame_count--;
            if (this->frame_count == 0 or frame->func->name == NULL) {
                DEREF(machine_pop(this))
                return;
            }
            while (this->stack_top != frame->stack) {
                DEREF(this->stack[--this->stack_top])
            }
            PUSH(result)
            frame = current_frame(this);
            break;
        }
        case OP_POP:
            DEREF(machine_pop(this))
            break;
        case OP_TRUE:
            PUSH(new_bool(true))
            break;
        case OP_FALSE:
            PUSH(new_bool(false))
            break;
        case OP_NONE:
            PUSH(new_none())
            break;
        case OP_CALL: {
            int count = read_byte(frame);
            Value call = machine_peek(this, count + 1);
            frame = machine_call_value(this, call, count);
            if (frame == NULL) return;
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
        case OP_JUMP_IF_TRUE: {
            u16 jump = read_short(frame);
            if (!machine_false(machine_peek(this, 1))) {
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
            POP(b)
            POP(a)
            PUSH(new_bool(machine_equal(a, b)))
            DEREF_TWO(a, b)
            break;
        }
        case OP_NOT_EQUAL: {
            POP(b)
            POP(a)
            PUSH(new_bool(!machine_equal(a, b)))
            DEREF_TWO(a, b)
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
            POP(b)
            POP(a)
            if (is_none(a)) {
                if (is_string(b)) {
                    String *temp = new_string(STRING_NONE);
                    String *add = string_concat(temp, string_copy(as_string(b)));
                    string_delete(temp);
                    machine_push_intern_string(this, add);
                } else {
                    DEREF_TWO(a, b)
                    THROW("Operation Error: 1st and 2nd values can't be added.")
                }
            } else if (is_bool(a)) {
                if (is_string(b)) {
                    String *temp = new_string(as_bool(a) ? STRING_TRUE : STRING_FALSE);
                    String *add = string_concat(temp, string_copy(as_string(b)));
                    string_delete(temp);
                    machine_push_intern_string(this, add);
                } else {
                    DEREF_TWO(a, b)
                    THROW("Operation Error: 1st and 2nd values can't be added.")
                }
            } else if (is_int(a)) {
                if (is_int(b)) {
                    a.as.i += b.as.i;
                    PUSH(a)
                } else if (is_float(b)) {
                    b.as.f += a.as.i;
                    PUSH(a)
                } else if (is_string(b)) {
                    String *temp = int64_to_string(as_int(a));
                    String *add = string_concat(temp, as_string(b));
                    string_delete(temp);
                    machine_push_intern_string(this, add);
                } else {
                    DEREF_TWO(a, b)
                    THROW("Operation Error: 1st and 2nd values can't be added.")
                }
            } else if (is_float(a)) {
                if (is_int(b)) {
                    a.as.f += b.as.i;
                    PUSH(a)
                } else if (is_float(b)) {
                    a.as.f += b.as.f;
                    PUSH(a)
                } else if (is_string(b)) {
                    String *temp = float64_to_string(as_float(a));
                    String *add = string_concat(temp, as_string(b));
                    string_delete(temp);
                    machine_push_intern_string(this, add);
                } else {
                    DEREF_TWO(a, b)
                    THROW("Operation Error: 1st and 2nd values can't be added.")
                }
            } else if (is_string(a)) {
                String *s = as_string(a);
                String *add = NULL;
                switch (b.is) {
                case HYMN_VALUE_NONE:
                    add = string_append(string_copy(s), STRING_NONE);
                    break;
                case HYMN_VALUE_BOOL:
                    add = string_append(string_copy(s), as_bool(b) ? STRING_TRUE : STRING_FALSE);
                    break;
                case HYMN_VALUE_INTEGER: {
                    String *temp = int64_to_string(as_int(b));
                    add = string_concat(s, temp);
                    string_delete(temp);
                    break;
                }
                case HYMN_VALUE_FLOAT: {
                    String *temp = float64_to_string(as_float(b));
                    add = string_concat(s, temp);
                    string_delete(temp);
                    break;
                }
                case HYMN_VALUE_STRING:
                    add = string_concat(s, as_string(b));
                    break;
                case HYMN_VALUE_ARRAY: {
                    String *temp = new_string("[");
                    temp = string_append(temp, pointer_to_string(as_array(b)));
                    temp = string_append_char(temp, ']');
                    add = string_concat(s, temp);
                    string_delete(temp);
                    break;
                }
                case HYMN_VALUE_TABLE: {
                    String *temp = new_string("[");
                    temp = string_append(temp, pointer_to_string(as_table(b)));
                    temp = string_append_char(temp, ']');
                    add = string_concat(s, temp);
                    string_delete(temp);
                    break;
                }
                case HYMN_VALUE_FUNC:
                    add = string_concat(s, as_func(b)->name);
                    break;
                case HYMN_VALUE_FUNC_NATIVE:
                    add = string_concat(s, as_native(b)->name);
                    break;
                default:
                    THROW("Operands can't be added.")
                }
                machine_push_intern_string(this, add);
            } else {
                THROW("Operands can't be added.")
            }
            DEREF_TWO(a, b)
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
        case OP_MODULO: {
            INTEGER_OP(%=);
            break;
        }
        case OP_BIT_NOT: {
            POP(value)
            if (is_int(value)) {
                value.as.i = ~value.as.i;
                PUSH(value)
            } else {
                DEREF(value)
                THROW("Bitwise operand must integer.")
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
            POP(value)
            if (is_int(value)) {
                value.as.i = -value.as.i;
            } else if (is_float(value)) {
                value.as.f = -value.as.f;
            } else {
                DEREF(value)
                THROW("Operand must be a number.")
            }
            PUSH(value)
            break;
        }
        case OP_NOT: {
            POP(value)
            if (is_bool(value)) {
                value.as.b = !value.as.b;
            } else {
                DEREF(value)
                THROW("Operand must be a boolean.")
            }
            PUSH(value)
            break;
        }
        case OP_CONSTANT: {
            Value constant = read_constant(frame);
            switch (constant.is) {
            case HYMN_VALUE_ARRAY: {
                constant = new_array_value(new_array(0));
                break;
            }
            case HYMN_VALUE_TABLE: {
                constant = new_table_value(new_table());
                break;
            }
            default:
                break;
            }
            reference(constant);
            PUSH(constant)
            break;
        }
        case OP_DEFINE_GLOBAL: {
            HymnString *name = as_hymn_string(read_constant(frame));
            POP(value)
            table_put(&this->globals, name, value);
            break;
        }
        case OP_SET_GLOBAL: {
            HymnString *name = as_hymn_string(read_constant(frame));
            Value value = machine_peek(this, 1);
            Value exists = table_get(&this->globals, name);
            if (is_undefined(exists)) {
                THROW("Undefined variable '%s'.", name->string)
            }
            // TODO: PERFORMANCE: DEREF OLD AND REF NEW SAME TIME
            Value previous = table_get(&this->globals, name);
            if (!is_undefined(previous)) {
                DEREF(previous)
            }
            table_put(&this->globals, name, value);
            reference(value);
            break;
        }
        case OP_GET_GLOBAL: {
            HymnString *name = as_hymn_string(read_constant(frame));
            Value get = table_get(&this->globals, name);
            if (is_undefined(get)) {
                THROW("Undefined variable `%s`.", name->string)
            }
            reference(get);
            PUSH(get)
            break;
        }
        case OP_SET_LOCAL: {
            u8 slot = read_byte(frame);
            Value value = machine_peek(this, 1);
            reference(value);
            DEREF(this->stack[frame->stack + slot])
            this->stack[frame->stack + slot] = value;
            break;
        }
        case OP_GET_LOCAL: {
            u8 slot = read_byte(frame);
            Value value = this->stack[frame->stack + slot];
            reference(value);
            PUSH(value)
            break;
        }
        case OP_SET_PROPERTY: {
            POP(p)
            POP(v)
            if (!is_table(v)) {
                DEREF_TWO(p, v)
                THROW("Only tables can set properties.")
            }
            Table *table = as_table(v);
            HymnString *name = as_hymn_string(read_constant(frame));
            // TODO: PERFORMANCE: DEREF OLD AND REF NEW SAME TIME
            Value previous = table_get(table, name);
            if (!is_undefined(previous)) {
                DEREF(previous)
            }
            table_put(table, name, p);
            PUSH(p)
            reference(p);
            DEREF(v)
            break;
        }
        case OP_GET_PROPERTY: {
            POP(v)
            if (!is_table(v)) {
                DEREF(v)
                THROW("Only tables can get properties.")
            }
            Table *table = as_table(v);
            HymnString *name = as_hymn_string(read_constant(frame));
            Value g = table_get(table, name);
            if (is_undefined(g)) {
                g.is = HYMN_VALUE_NONE;
            } else {
                reference(g);
            }
            DEREF(v)
            PUSH(g)
            break;
        }
        case OP_SET_DYNAMIC: {
            POP(s)
            POP(i)
            POP(v)
            if (is_array(v)) {
                if (!is_int(i)) {
                    DEREF_THREE(s, i, v)
                    THROW("Integer required to set array index.")
                }
                Array *array = as_array(v);
                i64 size = array->length;
                i64 index = as_int(i);
                if (index > size) {
                    DEREF_THREE(s, i, v)
                    THROW("Array index out of bounds %d > %d.", index, size)
                }
                if (index < 0) {
                    index = size + index;
                    if (index < 0) {
                        DEREF_THREE(s, i, v)
                        THROW("Array index out of bounds %d.", index)
                    }
                }
                if (index == size) {
                    array_push(array, s);
                } else {
                    DEREF(array->items[index])
                    array->items[index] = s;
                }
            } else if (is_table(v)) {
                if (!is_string(i)) {
                    DEREF_THREE(s, i, v)
                    THROW("String required to set table property.")
                }
                Table *table = as_table(v);
                HymnString *name = as_hymn_string(i);
                table_put(table, name, s);
                DEREF(i)
            } else {
                const char *is = value_name(v.is);
                DEREF_THREE(s, i, v)
                THROW("Dynamic Set: 1st argument requires `Array` or `Table`, but was `%s`.", is)
            }
            PUSH(s)
            DEREF(v)
            reference(s);
            break;
        }
        case OP_GET_DYNAMIC: {
            POP(i)
            POP(v)
            switch (v.is) {
            case HYMN_VALUE_STRING: {
                if (!is_int(i)) {
                    DEREF_TWO(i, v)
                    THROW("Integer required to get string character from index.")
                }
                String *string = as_string(v);
                i64 size = (i64)string_len(string);
                i64 index = as_int(i);
                if (index >= size) {
                    DEREF_TWO(i, v)
                    THROW("String index out of bounds %d >= %d.", index, size)
                }
                if (index < 0) {
                    index = size + index;
                    if (index < 0) {
                        DEREF_TWO(i, v)
                        THROW("String index out of bounds %d.", index)
                    }
                }
                char c = string[index];
                machine_push_intern_string(this, char_to_string(c));
                DEREF(v)
                break;
            }
            case HYMN_VALUE_ARRAY: {
                if (!is_int(i)) {
                    DEREF_TWO(i, v)
                    THROW("Integer required to get array index.")
                }
                Array *array = as_array(v);
                i64 size = array->length;
                i64 index = as_int(i);
                if (index >= size) {
                    DEREF_TWO(i, v)
                    THROW("Array index out of bounds %d >= %d.", index, size)
                }
                if (index < 0) {
                    index = size + index;
                    if (index < 0) {
                        DEREF_TWO(i, v)
                        THROW("Array index out of bounds %d.", index)
                    }
                }
                Value g = array_get(array, index);
                reference(g);
                PUSH(g)
                DEREF(v)
                break;
            }
            case HYMN_VALUE_TABLE: {
                if (!is_string(i)) {
                    const char *is = value_name(i.is);
                    DEREF_TWO(i, v)
                    THROW("Dynamic Get: Expected 2nd argument to be `String`, but was `%s`.", is)
                }
                Table *table = as_table(v);
                HymnString *name = as_hymn_string(i);
                Value g = table_get(table, name);
                if (is_undefined(g)) {
                    g.is = HYMN_VALUE_NONE;
                } else {
                    reference(g);
                }
                PUSH(g)
                DEREF_TWO(i, v)
                break;
            }
            default: {
                const char *is = value_name(v.is);
                DEREF_TWO(i, v)
                THROW("Dynamic Get: 1st argument requires `Array` or `Table`, but was `%s`.", is)
            }
            }
            break;
        }
        case OP_LEN: {
            POP(value)
            switch (value.is) {
            case HYMN_VALUE_STRING: {
                i64 len = (i64)string_len(as_string(value));
                PUSH(new_int(len))
                break;
            }
            case HYMN_VALUE_ARRAY: {
                i64 len = as_array(value)->length;
                PUSH(new_int(len))
                break;
            }
            case HYMN_VALUE_TABLE: {
                i64 len = (i64)as_table(value)->size;
                PUSH(new_int(len))
                break;
            }
            default:
                DEREF(value)
                THROW("Expected array or table for `len` function.")
            }
            DEREF(value)
            break;
        }
        case OP_ARRAY_POP: {
            POP(a)
            if (!is_array(a)) {
                const char *is = value_name(a.is);
                DEREF(a)
                THROW("Pop Function: Expected `Array` for 1st argument, but was `%s`.", is)
            } else {
                Value value = array_pop(as_array(a));
                PUSH(value)
                DEREF(a)
            }
            break;
        }
        case OP_ARRAY_PUSH: {
            POP(v)
            POP(a)
            if (!is_array(a)) {
                const char *is = value_name(v.is);
                DEREF_TWO(a, v)
                THROW("Push Function: Expected `Array` for 1st argument, but was `%s`.", is)
            } else {
                array_push(as_array(a), v);
                PUSH(v)
                reference(v);
                DEREF(a)
            }
            break;
        }
        case OP_ARRAY_INSERT: {
            POP(p)
            POP(i)
            POP(v)
            if (is_array(v)) {
                if (!is_int(i)) {
                    const char *is = value_name(i.is);
                    DEREF_THREE(p, i, v)
                    THROW("Insert Function: Expected `Integer` for 2nd argument, but was `%s`.", is)
                }
                Array *array = as_array(v);
                i64 size = array->length;
                i64 index = as_int(i);
                if (index > size) {
                    DEREF_THREE(p, i, v)
                    THROW("Insert Function: Array index out of bounds: %d > %d", index, size)
                }
                if (index < 0) {
                    index = size + index;
                    if (index < 0) {
                        DEREF_THREE(p, i, v)
                        THROW("Insert Function: Array index less than zero: %d", index)
                    }
                }
                if (index == size) {
                    array_push(array, p);
                } else {
                    array_insert(array, index, p);
                }
                PUSH(p)
                reference(v);
                DEREF(v)
            } else {
                const char *is = value_name(v.is);
                DEREF_THREE(p, i, v)
                THROW("Insert Function: Expected `Array` for 1st argument, but was `%s`.", is)
            }
            break;
        }
        case OP_DELETE: {
            POP(i)
            POP(v)
            if (is_array(v)) {
                if (!is_int(i)) {
                    DEREF_TWO(i, v)
                    THROW("Integer required to delete from array.")
                }
                Array *array = as_array(v);
                i64 size = array->length;
                i64 index = as_int(i);
                if (index >= size) {
                    DEREF_TWO(i, v)
                    THROW("Array index out of bounds %d > %d.", index, size)
                }
                if (index < 0) {
                    index = size + index;
                    if (index < 0) {
                        DEREF_TWO(i, v)
                        THROW("Array index out of bounds %d.", index)
                    }
                }
                Value value = array_remove_index(array, index);
                PUSH(value)
                DEREF(v)
            } else if (is_table(v)) {
                if (!is_string(i)) {
                    DEREF_TWO(i, v)
                    THROW("String required to delete from table.")
                }
                Table *table = as_table(v);
                HymnString *name = as_hymn_string(i);
                Value value = table_remove(table, name);
                if (is_undefined(value)) {
                    value.is = HYMN_VALUE_NONE;
                }
                PUSH(value)
                DEREF_TWO(i, v)
            } else {
                DEREF_TWO(i, v)
                THROW("Expected array or table for `delete` function.")
            }
            break;
        }
        case OP_COPY: {
            POP(value)
            switch (value.is) {
            case HYMN_VALUE_NONE:
            case HYMN_VALUE_BOOL:
            case HYMN_VALUE_INTEGER:
            case HYMN_VALUE_FLOAT:
            case HYMN_VALUE_STRING:
            case HYMN_VALUE_FUNC:
            case HYMN_VALUE_FUNC_NATIVE:
                PUSH(value)
                break;
            case HYMN_VALUE_ARRAY: {
                Array *copy = new_array_copy(as_array(value));
                Value new = new_array_value(copy);
                PUSH(new)
                reference(new);
                DEREF(value)
                break;
            }
            case HYMN_VALUE_TABLE: {
                Table *copy = new_table_copy(as_table(value));
                Value new = new_table_value(copy);
                PUSH(new)
                reference(new);
                DEREF(value)
                break;
            }
            default:
                PUSH(new_none())
            }
            break;
        }
        case OP_SLICE: {
            POP(b)
            POP(a)
            POP(v)
            if (!is_int(a)) {
                DEREF_THREE(a, b, v)
                THROW("Integer required for slice expression.")
            }
            i64 left = as_int(a);
            if (is_string(v)) {
                String *original = as_string(v);
                i64 size = (i64)string_len(original);
                i64 right;
                if (is_int(b)) {
                    right = as_int(b);
                } else if (is_none(b)) {
                    right = size;
                } else {
                    DEREF_THREE(a, b, v)
                    THROW("Integer required for slice expression.")
                }
                if (right > size) {
                    DEREF_THREE(a, b, v)
                    THROW("String index out of bounds %d > %d.", right, size)
                }
                if (right < 0) {
                    right = size + right;
                    if (right < 0) {
                        DEREF_THREE(a, b, v)
                        THROW("String index out of bounds %d.", right)
                    }
                }
                if (left >= right) {
                    DEREF_THREE(a, b, v)
                    THROW("String start index %d > right index %d.", left, right)
                }
                String *sub = new_string_from_substring(original, left, right);
                machine_push_intern_string(this, sub);
            } else if (is_array(v)) {
                Array *array = as_array(v);
                i64 size = array->length;
                i64 right;
                if (is_int(b)) {
                    right = as_int(b);
                } else if (is_none(b)) {
                    right = size;
                } else {
                    DEREF_THREE(a, b, v)
                    THROW("Integer required for slice expression.")
                }
                if (right > size) {
                    DEREF_THREE(a, b, v)
                    THROW("Array index out of bounds %d > %d.", right, size)
                }
                if (right < 0) {
                    right = size + right;
                    if (right < 0) {
                        DEREF_THREE(a, b, v)
                        THROW("Array index out of bounds %d.", right)
                    }
                }
                if (left >= right) {
                    DEREF_THREE(a, b, v)
                    THROW("Array start index %d >= right index %d.", left, right)
                }
                Array *copy = new_array_slice(array, left, right);
                Value new = new_array_value(copy);
                PUSH(new)
                reference(new);
                break;
            } else {
                DEREF_THREE(a, b, v)
                THROW("Expected string or array for `slice` function.")
            }
            DEREF(v)
            break;
        }
        case OP_CLEAR: {
            POP(value)
            switch (value.is) {
            case HYMN_VALUE_BOOL:
                PUSH(new_bool(false))
                break;
            case HYMN_VALUE_INTEGER:
                PUSH(new_int(0))
                break;
            case HYMN_VALUE_FLOAT:
                PUSH(new_float(0.0f))
                break;
            case HYMN_VALUE_STRING:
                machine_push_intern_string(this, new_string(""));
                break;
            case HYMN_VALUE_ARRAY: {
                Array *array = as_array(value);
                array_clear(this, array);
                PUSH(value)
                break;
            }
            case HYMN_VALUE_TABLE: {
                Table *table = as_table(value);
                table_clear(this, table);
                PUSH(value)
                break;
            }
            case HYMN_VALUE_UNDEFINED:
            case HYMN_VALUE_NONE:
            case HYMN_VALUE_FUNC:
            case HYMN_VALUE_FUNC_NATIVE:
            case HYMN_VALUE_POINTER:
                PUSH(new_none())
                break;
            }
            break;
        }
        case OP_KEYS: {
            POP(value)
            if (!is_table(value)) {
                DEREF(value)
                THROW("Expected table for `keys` function.")
            } else {
                Table *table = as_table(value);
                Array *array = table_keys(table);
                Value keys = new_array_value(array);
                reference(keys);
                PUSH(keys)
                DEREF(value)
            }
            break;
        }
        case OP_INDEX: {
            POP(b)
            POP(a)
            switch (a.is) {
            case HYMN_VALUE_STRING: {
                if (!is_string(b)) {
                    DEREF_TWO(a, b)
                    THROW("Expected substring for 2nd argument of `index` function.")
                }
                usize index = 0;
                bool found = string_find(as_string(a), as_string(b), &index);
                if (found) {
                    PUSH(new_int((i64)index))
                } else {
                    PUSH(new_int(-1))
                }
                DEREF_TWO(a, b)
                break;
            }
            case HYMN_VALUE_ARRAY:
                PUSH(new_int(array_index_of(as_array(a), b)))
                DEREF_TWO(a, b)
                break;
            case HYMN_VALUE_TABLE: {
                HymnString *key = table_key_of(as_table(a), b);
                if (key == NULL) {
                    PUSH(new_none())
                } else {
                    PUSH(new_string_value(key))
                }
                DEREF_TWO(a, b)
                break;
            }
            default:
                DEREF_TWO(a, b)
                THROW("Expected string, array, or table for `index` function.")
            }
            break;
        }
        case OP_TYPE: {
            POP(value)
            switch (value.is) {
            case HYMN_VALUE_UNDEFINED:
            case HYMN_VALUE_NONE:
                machine_push_intern_string(this, new_string(STRING_NONE));
                break;
            case HYMN_VALUE_BOOL:
                machine_push_intern_string(this, new_string(STRING_BOOL));
                break;
            case HYMN_VALUE_INTEGER:
                machine_push_intern_string(this, new_string(STRING_INTEGER));
                break;
            case HYMN_VALUE_FLOAT:
                machine_push_intern_string(this, new_string(STRING_FLOAT));
                break;
            case HYMN_VALUE_STRING:
                machine_push_intern_string(this, new_string(STRING_STRING));
                DEREF(value)
                break;
            case HYMN_VALUE_ARRAY:
                machine_push_intern_string(this, new_string(STRING_ARRAY));
                DEREF(value)
                break;
            case HYMN_VALUE_TABLE:
                machine_push_intern_string(this, new_string(STRING_TABLE));
                DEREF(value)
                break;
            case HYMN_VALUE_FUNC:
                machine_push_intern_string(this, new_string(STRING_FUNC));
                DEREF(value)
                break;
            case HYMN_VALUE_FUNC_NATIVE:
                machine_push_intern_string(this, new_string(STRING_NATIVE));
                break;
            case HYMN_VALUE_POINTER:
                machine_push_intern_string(this, new_string(STRING_POINTER));
                break;
            }
            break;
        }
        case OP_TO_INTEGER: {
            POP(value)
            if (is_int(value)) {
                PUSH(value)
            } else if (is_float(value)) {
                i64 push = (i64)as_float(value);
                PUSH(new_int(push))
            } else if (is_string(value)) {
                String *s = as_string(value);
                if (string_len(s) == 1) {
                    i64 c = (i64)s[0];
                    PUSH(new_int(c))
                } else {
                    PUSH(new_int(-1))
                }
                DEREF(value)
            } else {
                DEREF(value)
                THROW("Can't cast to an integer.")
            }
            break;
        }
        case OP_TO_FLOAT: {
            POP(value)
            if (is_int(value)) {
                float push = (float)as_int(value);
                PUSH(new_float(push))
            } else if (is_float(value)) {
                PUSH(value)
            } else if (is_string(value)) {
                String *s = as_string(value);
                if (string_len(s) == 1) {
                    float c = (float)((i64)s[0]);
                    PUSH(new_float(c))
                } else {
                    PUSH(new_float(-1))
                }
                DEREF(value)
            } else {
                DEREF(value)
                THROW("Can't cast to a float.")
            }
            break;
        }
        case OP_TO_STRING: {
            POP(value)
            switch (value.is) {
            case HYMN_VALUE_UNDEFINED:
            case HYMN_VALUE_NONE:
                machine_push_intern_string(this, new_string(STRING_NONE));
                break;
            case HYMN_VALUE_BOOL:
                machine_push_intern_string(this, as_bool(value) ? new_string(STRING_TRUE) : new_string(STRING_FALSE));
                break;
            case HYMN_VALUE_INTEGER:
                machine_push_intern_string(this, int64_to_string(as_int(value)));
                break;
            case HYMN_VALUE_FLOAT:
                machine_push_intern_string(this, float64_to_string(as_float(value)));
                break;
            case HYMN_VALUE_STRING:
                PUSH(value)
                break;
            case HYMN_VALUE_ARRAY:
                machine_push_intern_string(this, string_format("[array %p]", as_array(value)));
                DEREF(value)
                break;
            case HYMN_VALUE_TABLE:
                machine_push_intern_string(this, string_format("[table %p]", as_table(value)));
                DEREF(value)
                break;
            case HYMN_VALUE_FUNC:
                machine_push_intern_string(this, as_func(value)->name);
                DEREF(value)
                break;
            case HYMN_VALUE_FUNC_NATIVE:
                machine_push_intern_string(this, as_native(value)->name);
                break;
            case HYMN_VALUE_POINTER:
                machine_push_intern_string(this, string_format("[pointer %p]", as_pointer(value)));
                DEREF(value)
            }
            break;
        }
        case OP_PRINT: {
            POP(value)
            switch (value.is) {
            case HYMN_VALUE_UNDEFINED:
            case HYMN_VALUE_NONE:
                printf("%s\n", STRING_NONE);
                break;
            case HYMN_VALUE_BOOL:
                printf("%s\n", as_bool(value) ? STRING_TRUE : STRING_FALSE);
                break;
            case HYMN_VALUE_INTEGER:
                printf("%" PRId64 "\n", as_int(value));
                break;
            case HYMN_VALUE_FLOAT:
                printf("%g\n", as_float(value));
                break;
            case HYMN_VALUE_STRING:
                printf("%s\n", as_string(value));
                DEREF(value)
                break;
            case HYMN_VALUE_ARRAY:
                printf("[array %p]\n", (void *)as_array(value));
                DEREF(value)
                break;
            case HYMN_VALUE_TABLE:
                printf("[table %p]\n", (void *)as_table(value));
                DEREF(value)
                break;
            case HYMN_VALUE_FUNC:
                printf("%s\n", as_func(value)->name);
                DEREF(value)
                break;
            case HYMN_VALUE_FUNC_NATIVE:
                printf("%s\n", as_native(value)->name);
                break;
            case HYMN_VALUE_POINTER:
                printf("[pointer %p]\n", as_pointer(value));
                DEREF(value)
                break;
            }
            break;
        }
        case OP_THROW: {
            frame = machine_exception(this);
            if (frame == NULL) return;
            break;
        }
        case OP_DUPLICATE: {
            Value top = machine_peek(this, 1);
            PUSH(top);
            reference(top);
            break;
        }
        case OP_DO: {
            POP(code)
            if (is_string(code)) {
                frame = machine_do(this, as_hymn_string(code));
                DEREF(code)
                if (frame == NULL) return;
            } else {
                DEREF(code)
                THROW("Expected string for 'do' command.")
            }
            break;
        }
        case OP_USE: {
            POP(file)
            if (is_string(file)) {
                frame = machine_import(this, as_hymn_string(file));
                DEREF(file)
                if (frame == NULL) return;
            } else {
                DEREF(file)
                THROW("Expected string for 'use' command.")
            }
            break;
        }
        default:
            fprintf(stderr, "Unknown instruction: %" PRId8, op);
            return;
        }
    }
}

static char *machine_interpret(Machine *this) {
    machine_run(this);
    char *error = NULL;
    if (this->error) {
        error = string_to_chars(this->error);
        string_delete(this->error);
        this->error = NULL;
    }
    return error;
}

Hymn *new_hymn() {
    Hymn *this = safe_calloc(1, sizeof(Hymn));
    machine_reset_stack(this);

    set_init(&this->strings);
    table_init(&this->globals);

    HymnString *question_this = machine_intern_string(this, new_string("<parent>/<path>.hm"));
    HymnString *question_relative = machine_intern_string(this, new_string("./<path>.hm"));
    HymnString *question_modules = machine_intern_string(this, new_string("./modules/<path>.hm"));
    reference_string(question_this);
    reference_string(question_relative);
    reference_string(question_modules);
    HymnString *paths = machine_intern_string(this, new_string("__paths"));
    reference_string(paths);
    this->paths = new_array(3);
    this->paths->items[0] = new_string_value(question_this);
    this->paths->items[1] = new_string_value(question_relative);
    this->paths->items[2] = new_string_value(question_modules);
    Value paths_value = new_array_value(this->paths);
    table_put(&this->globals, paths, paths_value);
    reference(paths_value);

    HymnString *imports = machine_intern_string(this, new_string("__imports"));
    reference_string(imports);
    this->imports = new_table();
    Value imports_value = new_table_value(this->imports);
    table_put(&this->globals, imports, imports_value);
    reference(imports_value);

    return this;
}

void hymn_delete(Hymn *this) {
    {
        Table *globals = &this->globals;
        unsigned int bins = globals->bins;
        for (unsigned int i = 0; i < bins; i++) {
            TableItem *item = globals->items[i];
            TableItem *previous = NULL;
            while (item != NULL) {
                TableItem *next = item->next;
                if (is_native(item->value)) {
                    if (previous == NULL) {
                        globals->items[i] = next;
                    } else {
                        previous->next = next;
                    }
                    native_function_delete(as_native(item->value));
                    free(item);
                    globals->size -= 1;
                } else {
                    previous = item;
                }
                item = next;
            }
        }
#ifndef NDEBUG
        for (unsigned int i = 0; i < bins; i++) {
            TableItem *item = globals->items[i];
            while (item != NULL) {
                Value value = item->value;
                if (is_object(value)) {
                    if (as_object(value)->count != 1) {
                        if (is_string(value) && as_object(value)->count == 2) {
                            // pass
                        } else {
                            printf("BAD GLOBAL: [%d]: ", as_object(value)->count);
                            debug_value(item->value);
                            printf("\n");
                        }
                    }
                }
                item = item->next;
            }
        }
#endif
        table_release(this, &this->globals);
        assert(this->globals.size == 0);
    }

    {
        Set *strings = &this->strings;
        unsigned int bins = strings->bins;
        for (unsigned int i = 0; i < bins; i++) {
            SetItem *item = strings->items[i];
            while (item != NULL) {
                SetItem *next = item->next;
                dereference_string(this, item->string);
                item = next;
            }
        }
#ifndef NDEBUG
        if (strings->size != 0) {
            for (unsigned int i = 0; i < bins; i++) {
                SetItem *item = strings->items[i];
                while (item != NULL) {
                    printf("BAD STRING: ");
                    debug_value(new_string_value(item->string));
                    printf("\n");
                    item = item->next;
                }
            }
        }
        assert(strings->size == 0);
#endif
        free(strings->items);
    }

    string_delete(this->error);

    free(this);
}

void hymn_add_function(Hymn *this, const char *name, NativeCall func) {
    HymnString *intern = machine_intern_string(this, new_string(name));
    reference_string(intern);
    String *copy = string_copy(intern->string);
    NativeFunction *value = new_native_function(copy, func);
    table_put(&this->globals, intern, new_native(value));
}

void hymn_add_pointer(Hymn *this, const char *name, void *pointer) {
    machine_set_global(this, name, new_pointer(pointer));
}

static char *hymn_exec(Hymn *this, const char *script, const char *source) {

    struct CompileReturn result = compile(this, script, source);

    Function *func = result.func;

    char *error = result.error;
    if (error) {
        function_delete(func);
        return error;
    }

#ifdef HYMN_DEBUG_CODE
    disassemble_byte_code(&func->code, "<script>");
#endif

    Value function = new_func_value(func);
    reference(function);

    PUSH(function)
    machine_call(this, func, 0);

    error = machine_interpret(this);
    if (error) {
        return error;
    }

    assert(this->stack_top == 0);
    machine_reset_stack(this);

    return NULL;
}

char *hymn_do(Hymn *this, const char *source) {
    return hymn_exec(this, NULL, source);
}

char *hymn_read(Hymn *this, const char *script) {
    String *source = cat(script);
    char *error = hymn_exec(this, script, source);
    string_delete(source);
    return error;
}

char *hymn_repl(Hymn *this) {
    (void)this;
    printf("Welcome to Hymn\n");

    char input[1024];

    while (true) {
        printf("> ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            continue;
        }

        if (strcmp(input, ".exit\n") == 0) {
            break;
        } else if (strcmp(input, ".help\n") == 0) {
            printf("Type '.exit' to exit the REPL\n");
            continue;
        }

        struct CompileReturn result = compile(this, NULL, input);

        Function *func = result.func;

        char *error = result.error;
        if (error) {
            function_delete(func);
            return error;
        }

        Value function = new_func_value(func);
        reference(function);

        PUSH(function)
        machine_call(this, func, 0);

        error = machine_interpret(this);
        if (error) return error;

        assert(this->stack_top == 0);
        machine_reset_stack(this);
    }

    return NULL;
}
