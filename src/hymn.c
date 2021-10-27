/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef __GNUC__
#include <dirent.h>
#include <linux/limits.h>
#include <unistd.h>
#define PATH_SEP '/'
#define PATH_SEP_STRING "/"
#define UNREACHABLE() __builtin_unreachable()
#elif _MSC_VER
#include <direct.h>
#include <windows.h>
#define getcwd _getcwd
#define PATH_MAX FILENAME_MAX
#define PATH_SEP '\\'
#define PATH_SEP_STRING "\\"
#define UNREACHABLE() __assume(0)
#endif

#include "hymn.h"

// #define HYMN_DEBUG_TRACE
// #define HYMN_DEBUG_STACK
// #define HYMN_DEBUG_REFERENCE

// #define HYMN_NO_MEMORY_MANAGE

void *hymn_malloc(size_t size) {
    void *mem = malloc(size);
    if (mem) {
        return mem;
    }
    fprintf(stderr, "malloc failed.\n");
    exit(1);
}

void *hymn_calloc(size_t members, size_t member_size) {
    void *mem = calloc(members, member_size);
    if (mem) {
        return mem;
    }
    fprintf(stderr, "calloc failed.\n");
    exit(1);
}

void *hymn_realloc(void *mem, size_t size) {
    mem = realloc(mem, size);
    if (mem) {
        return mem;
    }
    fprintf(stderr, "realloc failed.\n");
    exit(1);
}

static HymnStringHead *string_head_init(size_t length, size_t capacity) {
    size_t memory = sizeof(HymnStringHead) + capacity + 1;
    HymnStringHead *head = (HymnStringHead *)hymn_malloc(memory);
    memset(head, 0, memory);
    head->length = length;
    head->capacity = capacity;
    return head;
}

static HymnString *new_string_with_capacity(size_t capacity) {
    HymnStringHead *head = string_head_init(0, capacity);
    return (HymnString *)(head + 1);
}

static HymnString *new_string_with_length(const char *init, size_t length) {
    HymnStringHead *head = string_head_init(length, length);
    char *string = (char *)(head + 1);
    memcpy(string, init, length);
    string[length] = '\0';
    return (HymnString *)string;
}

HymnString *hymn_substring(const char *init, size_t start, size_t end) {
    size_t length = end - start;
    HymnStringHead *head = string_head_init(length, length);
    char *string = (char *)(head + 1);
    memcpy(string, &init[start], length);
    string[length] = '\0';
    return (HymnString *)string;
}

HymnString *hymn_new_string(const char *init) {
    size_t len = strlen(init);
    return new_string_with_length(init, len);
}

static HymnString *string_copy(HymnString *this) {
    HymnStringHead *head = (HymnStringHead *)((char *)this - sizeof(HymnStringHead));
    return new_string_with_length(this, head->length);
}

size_t hymn_string_len(HymnString *this) {
    HymnStringHead *head = (HymnStringHead *)((char *)this - sizeof(HymnStringHead));
    return head->length;
}

void hymn_string_delete(HymnString *this) {
    if (this == NULL) {
        return;
    }
    free((char *)this - sizeof(HymnStringHead));
}

static HymnString *substring(HymnString *this, size_t start, size_t end) {
    size_t len = end - start;
    HymnStringHead *head = string_head_init(len, len);
    char *s = (char *)(head + 1);
    memcpy(s, this + start, len);
    s[len] = '\0';
    return (HymnString *)s;
}

void hymn_string_zero(HymnString *this) {
    HymnStringHead *head = (HymnStringHead *)((char *)this - sizeof(HymnStringHead));
    head->length = 0;
    this[0] = '\0';
}

HymnString *hymn_string_trim(HymnString *this) {
    size_t len = hymn_string_len(this);
    size_t start = 0;
    while (start < len) {
        char c = this[start];
        if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
            break;
        }
        start++;
    }
    if (start == len) {
        hymn_string_zero(this);
    } else {
        size_t end = len - 1;
        while (end > start) {
            char c = this[end];
            if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
                break;
            }
            end--;
        }
        end++;
        size_t offset = start;
        size_t size = end - start;
        for (size_t i = 0; i < size; i++) {
            this[i] = this[offset++];
        }
        HymnStringHead *head = (HymnStringHead *)((char *)this - sizeof(HymnStringHead));
        head->length = size;
        this[end] = '\0';
    }
    return this;
}

static HymnStringHead *string_resize(HymnStringHead *head, size_t capacity) {
    size_t memory = sizeof(HymnStringHead) + capacity + 1;
    HymnStringHead *new = hymn_realloc(head, memory);
    new->capacity = capacity;
    return new;
}

HymnString *hymn_string_append(HymnString *this, const char *b) {
    HymnStringHead *head = (HymnStringHead *)((char *)this - sizeof(HymnStringHead));
    size_t len_a = head->length;
    size_t len_b = strlen(b);
    size_t len = len_a + len_b;
    if (len > head->capacity) {
        head = string_resize(head, len * 2);
    }
    head->length = len;
    char *s = (char *)(head + 1);
    memcpy(s + len_a, b, len_b + 1);
    s[len] = '\0';
    return (HymnString *)s;
}

HymnString *hymn_string_append_char(HymnString *this, const char b) {
    HymnStringHead *head = (HymnStringHead *)((char *)this - sizeof(HymnStringHead));
    size_t len = head->length + 1;
    if (len > head->capacity) {
        head = string_resize(head, len * 2);
    }
    head->length = len;
    char *s = (char *)(head + 1);
    s[len - 1] = b;
    s[len] = '\0';
    return (HymnString *)s;
}

static HymnString *string_append_substring(HymnString *this, const char *b, size_t start, size_t end) {
    HymnStringHead *head = (HymnStringHead *)((char *)this - sizeof(HymnStringHead));
    size_t len_a = head->length;
    size_t len_b = end - start;
    size_t len = len_a + len_b;
    if (len > head->capacity) {
        head = string_resize(head, len * 2);
    }
    head->length = len;
    char *s = (char *)(head + 1);
    memcpy(s + len_a, &b[start], len_b);
    s[len] = '\0';
    return (HymnString *)s;
}

static int string_compare(HymnString *a, HymnString *b) {
    return strcmp(a, b);
}

bool hymn_string_equal(HymnString *a, HymnString *b) {
    return 0 == string_compare(a, b);
}

bool hymn_string_starts_with(HymnString *s, const char *p) {
    size_t slen = hymn_string_len(s);
    size_t plen = strlen(p);
    return slen < plen ? false : memcmp(s, p, plen) == 0;
}

static bool string_ends_with(HymnString *s, const char *p) {
    size_t slen = hymn_string_len(s);
    size_t plen = strlen(p);
    return slen < plen ? false : memcmp(&s[slen - plen], p, plen) == 0;
}

bool hymn_string_contains(HymnString *s, const char *p) {
    size_t slen = hymn_string_len(s);
    size_t plen = strlen(p);
    if (plen > slen) {
        return false;
    }
    size_t diff = slen - plen;
    for (size_t i = 0; i <= diff; i++) {
        if (memcmp(&s[i], p, plen) == 0) {
            return true;
        }
    }
    return false;
}

static bool string_find(HymnString *this, HymnString *sub, size_t *out) {
    HymnStringHead *head = (HymnStringHead *)((char *)this - sizeof(HymnStringHead));
    HymnStringHead *head_sub = (HymnStringHead *)((char *)sub - sizeof(HymnStringHead));
    size_t len = head->length;
    size_t len_sub = head_sub->length;
    if (len_sub > len) {
        return false;
    } else if (len == 0) {
        *out = 0;
        return true;
    }
    size_t end = len - len_sub + 1;
    for (size_t i = 0; i < end; i++) {
        bool match = true;
        for (size_t k = 0; k < len_sub; k++) {
            if (sub[k] != this[i + k]) {
                match = false;
                break;
            }
        }
        if (match) {
            *out = i;
            return true;
        }
    }
    return false;
}

static HymnString *string_replace(HymnString *this, const char *find, const char *replace) {
    HymnStringHead *head = (HymnStringHead *)((char *)this - sizeof(HymnStringHead));
    size_t len = head->length;
    size_t len_sub = strlen(find);
    if (len_sub > len) {
        return hymn_new_string("");
    } else if (len == 0) {
        return hymn_new_string("");
    }
    HymnString *out = hymn_new_string("");
    size_t end = len - len_sub + 1;
    size_t p = 0;
    for (size_t i = 0; i < end; i++) {
        bool match = true;
        for (size_t k = 0; k < len_sub; k++) {
            if (find[k] != this[i + k]) {
                match = false;
                break;
            }
        }
        if (match) {
            out = string_append_substring(out, this, p, i);
            out = hymn_string_append(out, replace);
            i += len_sub;
            p = i;
        }
    }
    if (p < len) {
        out = string_append_substring(out, this, p, len);
    }
    return out;
}

static HymnString *char_to_string(char ch) {
    char *str = hymn_malloc(2);
    str[0] = ch;
    str[1] = '\0';
    HymnString *s = new_string_with_length(str, 1);
    free(str);
    return s;
}

static HymnString *int64_to_string(int64_t number) {
    int len = snprintf(NULL, 0, "%" PRId64, number);
    char *str = hymn_malloc(len + 1);
    snprintf(str, len + 1, "%" PRId64, number);
    HymnString *s = new_string_with_length(str, len);
    free(str);
    return s;
}

static HymnString *double_to_string(double number) {
    int len = snprintf(NULL, 0, "%g", number);
    char *str = hymn_malloc(len + 1);
    snprintf(str, len + 1, "%g", number);
    HymnString *s = new_string_with_length(str, len);
    free(str);
    return s;
}

static int64_t string_to_int64(HymnString *this) {
    return (int64_t)strtoll(this, NULL, 10);
}

static double string_to_double(HymnString *this, char **end) {
    return strtod(this, end);
}

static char *string_to_chars(HymnString *this) {
    size_t len = hymn_string_len(this);
    char *s = hymn_malloc((len + 1) * sizeof(char));
    memcpy(s, this, len);
    s[len] = '\0';
    return s;
}

HymnString *hymn_string_format(const char *format, ...) {
    va_list args;

    va_start(args, format);
    int len = vsnprintf(NULL, 0, format, args);
    va_end(args);
    char *chars = hymn_malloc((len + 1) * sizeof(char));
    va_start(args, format);
    len = vsnprintf(chars, len + 1, format, args);
    va_end(args);
    HymnString *str = new_string_with_length(chars, len);
    free(chars);
    return str;
}

static HymnString *string_append_format(HymnString *this, const char *format, ...) {
    va_list args;

    va_start(args, format);
    int len = vsnprintf(NULL, 0, format, args);
    va_end(args);
    char *chars = hymn_malloc((len + 1) * sizeof(char));
    va_start(args, format);
    len = vsnprintf(chars, len + 1, format, args);
    va_end(args);
    this = hymn_string_append(this, chars);
    free(chars);
    return this;
}

static struct HymnFilterList string_filter(HymnString **input, int count, bool (*filter)(HymnString *a, const char *b), const char *with) {
    int size = 0;
    HymnString **filtered = hymn_calloc(count, sizeof(HymnString *));
    for (int i = 0; i < count; i++) {
        if (filter(input[i], with)) {
            filtered[size++] = string_copy(input[i]);
        }
    }
    return (struct HymnFilterList){.count = size, .filtered = filtered};
}

struct HymnFilterList hymn_string_filter_ends_with(HymnString **input, int count, const char *with) {
    return string_filter(input, count, string_ends_with, with);
}

void hymn_delete_filter_list(struct HymnFilterList *list) {
    for (int i = 0; i < list->count; i++) {
        hymn_string_delete(list->filtered[i]);
    }
    free(list->filtered);
}

// IO

static HymnString *working_directory() {
    char path[PATH_MAX];
    if (getcwd(path, sizeof(path)) != NULL) {
        return hymn_new_string(path);
    }
    return NULL;
}

static HymnString *path_normalize(HymnString *path) {
    size_t i = 0;
    size_t size = hymn_string_len(path);
    if (size > 1 && path[0] == '.') {
        if (path[1] == '.') {
            if (size > 2 && path[2] == PATH_SEP) {
                i = 3;
            }
        } else if (path[1] == PATH_SEP) {
            i = 2;
        }
    }

    size_t n = 0;
    char normal[PATH_MAX];

    while (i < size) {
        if (path[i] == PATH_SEP) {
            if (i + 2 < size) {
                if (path[i + 1] == '.' && path[i + 2] == PATH_SEP) {
                    i += 2;
                    continue;
                } else if (path[i + 2] == '.' && i + 3 < size && path[i + 3] == PATH_SEP) {
                    if (n > 0) {
                        n--;
                        while (n > 0) {
                            if (normal[n] == PATH_SEP) {
                                break;
                            }
                            n--;
                        }
                    }
                    i += 3;
                    continue;
                }
            }
        }

        normal[n] = path[i];
        n++;
        i++;
    }

    normal[n] = '\0';
    return hymn_new_string(normal);
}

static HymnString *path_parent(HymnString *path) {
    size_t size = hymn_string_len(path);
    if (size < 2) {
        return string_copy(path);
    }
    size_t i = size - 2;
    while (true) {
        if (i == 0) break;
        if (path[i] == PATH_SEP) break;
        i--;
    }
    return hymn_substring(path, 0, i);
}

static HymnString *path_join(HymnString *path, HymnString *child) {
    HymnString *new = string_copy(path);
    new = hymn_string_append_char(new, PATH_SEP);
    return hymn_string_append(new, child);
}

static HymnString *path_absolute(HymnString *path) {
    HymnString *working = working_directory();
    if (hymn_string_starts_with(path, working)) {
        hymn_string_delete(working);
        return path_normalize(path);
    }
    working = hymn_string_append_char(working, PATH_SEP);
    working = hymn_string_append(working, path);
    HymnString *normal = path_normalize(working);
    hymn_string_delete(working);
    return normal;
}

static size_t file_size(const char *path) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        return 0;
    }
    size_t num = 0;
    int ch;
    while ((ch = fgetc(fp)) != EOF) {
        num++;
    }
    fclose(fp);
    return num;
}

bool hymn_file_exists(const char *path) {
    struct stat b;
    return stat(path, &b) == 0;
}

HymnString *hymn_read_file(const char *path) {
    size_t size = file_size(path);
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file: %s\n", path);
        return hymn_new_string("");
    }
    char *content = hymn_malloc((size + 1) * sizeof(char));
    for (size_t i = 0; i < size; i++) {
        content[i] = (char)fgetc(fp);
    }
    fclose(fp);
    HymnString *s = new_string_with_length(content, size);
    free(content);
    return s;
}

// END IO

// VM

#define STRING_UNDEFINED "Undefined"
#define STRING_NONE_TYPE "None"
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

typedef struct Token Token;
typedef struct Local Local;
typedef struct Rule Rule;
typedef struct Scope Scope;
typedef struct Compiler Compiler;

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
    TOKEN_CONTINUE,
    TOKEN_COPY,
    TOKEN_DELETE,
    TOKEN_DIVIDE,
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
    TOKEN_TYPE_FUNC,
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
    OP_ADD_TWO_LOCAL,
    OP_INCREMENT,
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
    OP_TAIL_CALL,
    OP_CLEAR,
    OP_CONSTANT,
    OP_COPY,
    OP_DEFINE_GLOBAL,
    OP_DELETE,
    OP_DIVIDE,
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
    OP_JUMP_IF_EQUAL,
    OP_JUMP_IF_NOT_EQUAL,
    OP_JUMP_IF_LESS,
    OP_JUMP_IF_GREATER,
    OP_JUMP_IF_LESS_EQUAL,
    OP_JUMP_IF_GREATER_EQUAL,
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
    OP_POP_TWO,
    OP_POP_N,
    OP_PRINT,
    OP_RETURN,
    OP_SET_DYNAMIC,
    OP_SET_GLOBAL,
    OP_SET_LOCAL,
    OP_SET_PROPERTY,
    OP_INCREMENT_LOCAL,
    OP_INCREMENT_LOCAL_AND_SET,
    OP_SLICE,
    OP_SUBTRACT,
    OP_THROW,
    OP_TO_FLOAT,
    OP_TO_INTEGER,
    OP_TO_STRING,
    OP_TRUE,
    OP_TYPE,
    OP_USE
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

static void reference_string(HymnObjectString *string);
static void reference(HymnValue value);
static void dereference_string(Hymn *this, HymnObjectString *string);
static void dereference(Hymn *this, HymnValue value);

static char *machine_interpret(Hymn *this);

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
    size_t start;
    int length;
};

struct Local {
    Token name;
    int depth;
};

struct Rule {
    void (*prefix)(Compiler *, bool);
    void (*infix)(Compiler *, bool);
    enum Precedence precedence;
};

struct Scope {
    struct Scope *enclosing;
    HymnFunction *func;
    enum FunctionType type;
    Local locals[HYMN_UINT8_COUNT];
    int local_count;
    int depth;
};

struct Compiler {
    size_t pos;
    int row;
    int column;
    const char *script;
    const char *source;
    size_t size;
    Token previous;
    Token current;
    Hymn *H;
    Scope *scope;
    struct LoopList *loop;
    struct JumpList *jump;
    struct JumpList *jump_or;
    struct JumpList *jump_and;
    HymnString *error;
};

HymnValue hymn_new_undefined() {
    return (HymnValue){.is = HYMN_VALUE_UNDEFINED, .as = {.i = 0}};
}

HymnValue hymn_new_none() {
    return (HymnValue){.is = HYMN_VALUE_NONE, .as = {.i = 0}};
}

HymnValue hymn_new_bool(bool v) {
    return (HymnValue){.is = HYMN_VALUE_BOOL, .as = {.b = v}};
}

HymnValue hymn_new_int(int64_t v) {
    return (HymnValue){.is = HYMN_VALUE_INTEGER, .as = {.i = v}};
}

HymnValue hymn_new_float(double v) {
    return (HymnValue){.is = HYMN_VALUE_FLOAT, .as = {.f = v}};
}

HymnValue hymn_new_native(HymnNativeFunction *v) {
    return (HymnValue){.is = HYMN_VALUE_FUNC_NATIVE, .as = {.n = v}};
}

HymnValue hymn_new_pointer(void *v) {
    return (HymnValue){.is = HYMN_VALUE_POINTER, .as = {.p = v}};
}

HymnValue hymn_new_string_value(HymnObjectString *v) {
    return (HymnValue){.is = HYMN_VALUE_STRING, .as = {.o = (HymnObject *)v}};
}

HymnValue hymn_new_array_value(HymnArray *v) {
    return (HymnValue){.is = HYMN_VALUE_ARRAY, .as = {.o = (HymnObject *)v}};
}

HymnValue hymn_new_table_value(HymnTable *v) {
    return (HymnValue){.is = HYMN_VALUE_TABLE, .as = {.o = (HymnObject *)v}};
}

HymnValue hymn_new_func_value(HymnFunction *v) {
    return (HymnValue){.is = HYMN_VALUE_FUNC, .as = {.o = (HymnObject *)v}};
}

bool hymn_as_bool(HymnValue v) {
    return (v).as.b;
}

int64_t hymn_as_int(HymnValue v) {
    return (v).as.i;
}

double hymn_as_float(HymnValue v) {
    return (v).as.f;
}

HymnNativeFunction *hymn_as_native(HymnValue v) {
    return (v).as.n;
}

void *hymn_as_pointer(HymnValue v) {
    return (v).as.p;
}

HymnObject *hymn_as_object(HymnValue v) {
    return (HymnObject *)(v).as.o;
}

HymnObjectString *hymn_as_hymn_string(HymnValue v) {
    return (HymnObjectString *)(v).as.o;
}

HymnString *hymn_as_string(HymnValue v) {
    return hymn_as_hymn_string(v)->string;
}

HymnArray *hymn_as_array(HymnValue v) {
    return (HymnArray *)(v).as.o;
}

HymnTable *hymn_as_table(HymnValue v) {
    return (HymnTable *)(v).as.o;
}

HymnFunction *hymn_as_func(HymnValue v) {
    return (HymnFunction *)(v).as.o;
}

bool hymn_is_undefined(HymnValue v) {
    return (v).is == HYMN_VALUE_UNDEFINED;
}

bool hymn_is_none(HymnValue v) {
    return (v).is == HYMN_VALUE_NONE;
}

bool hymn_is_bool(HymnValue v) {
    return (v).is == HYMN_VALUE_BOOL;
}

bool hymn_is_int(HymnValue v) {
    return (v).is == HYMN_VALUE_INTEGER;
}

bool hymn_is_float(HymnValue v) {
    return (v).is == HYMN_VALUE_FLOAT;
}

bool hymn_is_native(HymnValue v) {
    return (v).is == HYMN_VALUE_FUNC_NATIVE;
}

bool hymn_is_pointer(HymnValue v) {
    return (v).is == HYMN_VALUE_POINTER;
}

bool hymn_is_string(HymnValue v) {
    return (v).is == HYMN_VALUE_STRING;
}

bool hymn_is_array(HymnValue v) {
    return (v).is == HYMN_VALUE_ARRAY;
}

bool hymn_is_table(HymnValue v) {
    return (v).is == HYMN_VALUE_TABLE;
}

bool hymn_is_func(HymnValue v) {
    return (v).is == HYMN_VALUE_FUNC;
}

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
    [TOKEN_COLON] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_CLEAR] = {clear_expression, NULL, PRECEDENCE_NONE},
    [TOKEN_COMMA] = {NULL, NULL, PRECEDENCE_NONE},
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
    [TOKEN_TYPE_FUNC] = {type_expression, NULL, PRECEDENCE_NONE},
    [TOKEN_UNDEFINED] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_USE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_VALUE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_WHILE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_SEMICOLON] = {NULL, NULL, PRECEDENCE_NONE},
};

static const char *value_name(enum HymnValueType type) {
    switch (type) {
    case HYMN_VALUE_UNDEFINED: return STRING_UNDEFINED;
    case HYMN_VALUE_NONE: return STRING_NONE_TYPE;
    case HYMN_VALUE_BOOL: return STRING_BOOL;
    case HYMN_VALUE_INTEGER: return STRING_INTEGER;
    case HYMN_VALUE_FLOAT: return STRING_FLOAT;
    case HYMN_VALUE_STRING: return STRING_STRING;
    case HYMN_VALUE_ARRAY: return STRING_ARRAY;
    case HYMN_VALUE_TABLE: return STRING_TABLE;
    case HYMN_VALUE_FUNC: return STRING_FUNC;
    case HYMN_VALUE_FUNC_NATIVE: return STRING_NATIVE;
    default: return "HymnValue";
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
    case TOKEN_CONTINUE: return "CONTINUE";
    case TOKEN_COPY: return "COPY";
    case TOKEN_DOT: return "DOT";
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
    case TOKEN_TYPE_FUNC: return "TYPE";
    case TOKEN_WHILE: return "WHILE";
    case TOKEN_USE: return "USE";
    case TOKEN_LEFT_CURLY: return "LEFT_CURLY";
    case TOKEN_RIGHT_CURLY: return "RIGHT_CURLY";
    case TOKEN_LEFT_SQUARE: return "LEFT_SQUARE";
    case TOKEN_RIGHT_SQUARE: return "RIGHT_SQUARE";
    default: return "token";
    }
}

static size_t string_hashcode(HymnString *key) {
    size_t length = hymn_string_len(key);
    size_t hash = 0;
    for (size_t i = 0; i < length; i++) {
        hash = 31 * hash + (size_t)key[i];
    }
    return hash;
}

static size_t mix_hash(size_t hash) {
    return hash ^ (hash >> 16);
}

static HymnObjectString *new_hymn_string_with_hash(HymnString *string, size_t hash) {
    HymnObjectString *object = hymn_calloc(1, sizeof(HymnObjectString));
    object->string = string;
    object->hash = hash;
    return object;
}

HymnObjectString *hymn_new_string_object(HymnString *string) {
    return new_hymn_string_with_hash(string, mix_hash(string_hashcode(string)));
}

static void table_init(HymnTable *this) {
    this->size = 0;
    this->bins = INITIAL_BINS;
    this->items = hymn_calloc(this->bins, sizeof(HymnTableItem *));
}

static unsigned int table_get_bin(HymnTable *this, size_t hash) {
    return (this->bins - 1) & hash;
}

static void table_resize(HymnTable *this) {
    unsigned int old_bins = this->bins;
    unsigned int bins = old_bins << 1;

    if (bins > MAXIMUM_BINS) {
        return;
    }

    HymnTableItem **old_items = this->items;
    HymnTableItem **items = hymn_calloc(bins, sizeof(HymnTableItem *));

    for (unsigned int i = 0; i < old_bins; i++) {
        HymnTableItem *item = old_items[i];
        if (item == NULL) {
            continue;
        }
        if (item->next == NULL) {
            items[(bins - 1) & item->key->hash] = item;
        } else {
            HymnTableItem *low_head = NULL;
            HymnTableItem *low_tail = NULL;
            HymnTableItem *high_head = NULL;
            HymnTableItem *high_tail = NULL;
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

static HymnValue table_put(HymnTable *this, HymnObjectString *key, HymnValue value) {
    unsigned int bin = table_get_bin(this, key->hash);
    HymnTableItem *item = this->items[bin];
    HymnTableItem *previous = NULL;
    while (item != NULL) {
        if (key == item->key) {
            HymnValue get = item->value;
            item->value = value;
            return get;
        }
        previous = item;
        item = item->next;
    }
    item = hymn_malloc(sizeof(HymnTableItem));
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
    return hymn_new_undefined();
}

static HymnValue table_get(HymnTable *this, HymnObjectString *key) {
    unsigned int bin = table_get_bin(this, key->hash);
    HymnTableItem *item = this->items[bin];
    while (item != NULL) {
        if (key == item->key) {
            return item->value;
        }
        item = item->next;
    }
    return hymn_new_undefined();
}

static HymnValue table_remove(HymnTable *this, HymnObjectString *key) {
    unsigned int bin = table_get_bin(this, key->hash);
    HymnTableItem *item = this->items[bin];
    HymnTableItem *previous = NULL;
    while (item != NULL) {
        if (key == item->key) {
            if (previous == NULL) {
                this->items[bin] = item->next;
            } else {
                previous->next = item->next;
            }
            HymnValue value = item->value;
            free(item);
            this->size -= 1;
            return value;
        }
        previous = item;
        item = item->next;
    }
    return hymn_new_undefined();
}

static void table_clear(Hymn *H, HymnTable *this) {
    unsigned int bins = this->bins;
    for (unsigned int i = 0; i < bins; i++) {
        HymnTableItem *item = this->items[i];
        while (item != NULL) {
            HymnTableItem *next = item->next;
            dereference_string(H, item->key);
            dereference(H, item->value);
            free(item);
            item = next;
        }
        this->items[i] = NULL;
    }
    this->size = 0;
}

static void table_release(Hymn *H, HymnTable *this) {
    table_clear(H, this);
    free(this->items);
}

static void table_delete(Hymn *H, HymnTable *this) {
    table_release(H, this);
    free(this);
}

static void set_init(HymnSet *this) {
    this->size = 0;
    this->bins = INITIAL_BINS;
    this->items = hymn_calloc(this->bins, sizeof(HymnSetItem *));
}

static unsigned int set_get_bin(HymnSet *this, size_t hash) {
    return (this->bins - 1) & hash;
}

static void set_resize(HymnSet *this) {
    unsigned int old_bins = this->bins;
    unsigned int bins = old_bins << 1;

    if (bins > MAXIMUM_BINS) {
        return;
    }

    HymnSetItem **old_items = this->items;
    HymnSetItem **items = hymn_calloc(bins, sizeof(HymnSetItem *));

    for (unsigned int i = 0; i < old_bins; i++) {
        HymnSetItem *item = old_items[i];
        if (item == NULL) {
            continue;
        }
        if (item->next == NULL) {
            items[(bins - 1) & item->string->hash] = item;
        } else {
            HymnSetItem *low_head = NULL;
            HymnSetItem *low_tail = NULL;
            HymnSetItem *high_head = NULL;
            HymnSetItem *high_tail = NULL;
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

static HymnObjectString *set_add_or_get(HymnSet *this, HymnString *add) {
    size_t hash = mix_hash(string_hashcode(add));
    unsigned int bin = set_get_bin(this, hash);
    HymnSetItem *item = this->items[bin];
    HymnSetItem *previous = NULL;
    while (item != NULL) {
        if (hymn_string_equal(add, item->string->string)) {
            return item->string;
        }
        previous = item;
        item = item->next;
    }
    HymnObjectString *new = new_hymn_string_with_hash(add, hash);
    item = hymn_malloc(sizeof(HymnSetItem));
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

static HymnObjectString *set_remove(HymnSet *this, HymnString *remove) {
    size_t hash = mix_hash(string_hashcode(remove));
    unsigned int bin = set_get_bin(this, hash);
    HymnSetItem *item = this->items[bin];
    HymnSetItem *previous = NULL;
    while (item != NULL) {
        if (hymn_string_equal(remove, item->string->string)) {
            if (previous == NULL) {
                this->items[bin] = item->next;
            } else {
                previous->next = item->next;
            }
            HymnObjectString *string = item->string;
            free(item);
            this->size -= 1;
            return string;
        }
        previous = item;
        item = item->next;
    }
    return NULL;
}

static void set_clear(Hymn *H, HymnSet *this) {
    unsigned int bins = this->bins;
    for (unsigned int i = 0; i < bins; i++) {
        HymnSetItem *item = this->items[i];
        while (item != NULL) {
            HymnSetItem *next = item->next;
            dereference_string(H, item->string);
            free(item);
            item = next;
        }
        this->items[i] = NULL;
    }
    this->size = 0;
}

static void set_release(Hymn *H, HymnSet *this) {
    set_clear(H, this);
    free(this->items);
}

static HymnFunction *current_func(Compiler *this) {
    return this->scope->func;
}

static HymnByteCode *current(Compiler *this) {
    return &current_func(this)->code;
}

static size_t beginning_of_line(const char *source, size_t i) {
    while (true) {
        if (i == 0) return 0;
        if (source[i] == '\n') return i + 1;
        i--;
    }
}

static size_t end_of_line(const char *source, size_t size, size_t i) {
    while (true) {
        if (i + 1 >= size) return i + 1;
        if (source[i] == '\n') return i;
        i++;
    }
}

static HymnString *string_append_previous_line(const char *source, HymnString *string, size_t i) {
    if (i < 2) {
        return string;
    }
    i--;
    size_t begin = beginning_of_line(source, i - 1);
    if (i - begin < 2) {
        return string;
    }
    return string_append_format(string, "%.*s\n", i - begin, &source[begin]);
}

static HymnString *string_append_second_previous_line(const char *source, HymnString *string, size_t i) {
    if (i < 2) {
        return string;
    }
    i--;
    size_t begin = beginning_of_line(source, i - 1);
    return string_append_previous_line(source, string, begin);
}

static void compile_error(Compiler *this, Token *token, const char *format, ...) {
    if (this->error != NULL) return;

    va_list ap;
    va_start(ap, format);
    int len = vsnprintf(NULL, 0, format, ap);
    va_end(ap);
    char *chars = hymn_malloc((len + 1) * sizeof(char));
    va_start(ap, format);
    len = vsnprintf(chars, len + 1, format, ap);
    va_end(ap);
    HymnString *error = hymn_new_string(chars);
    free(chars);
    error = hymn_string_append(error, "\n\n");

    size_t begin = beginning_of_line(this->source, token->start);
    size_t end = end_of_line(this->source, this->size, token->start);

    error = string_append_second_previous_line(this->source, error, begin);
    error = string_append_previous_line(this->source, error, begin);
    error = string_append_format(error, "%.*s\n", end - begin, &this->source[begin]);
    for (int i = 0; i < (int)(token->start - begin); i++) {
        error = hymn_string_append_char(error, ' ');
    }
    error = hymn_string_append(error, ANSI_COLOR_RED);
    for (int i = 0; i < token->length; i++) {
        error = hymn_string_append_char(error, '^');
    }
    error = hymn_string_append(error, ANSI_COLOR_RESET);

    error = string_append_format(error, "\nat %s:%d\n", this->script, token->row);

    this->error = error;

    this->previous.type = TOKEN_EOF;
    this->current.type = TOKEN_EOF;
}

static char next_char(Compiler *this) {
    size_t pos = this->pos;
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
}

static void token_special(Compiler *this, enum TokenType type, size_t offset, size_t length) {
    Token *current = &this->current;
    current->type = type;
    current->row = this->row;
    current->column = this->column;
    if (this->pos < offset) {
        current->start = 0;
    } else {
        current->start = this->pos - offset;
    }
    current->length = (int)length;
}

static void value_token(Compiler *this, enum TokenType type, size_t start, size_t end) {
    Token *current = &this->current;
    current->type = type;
    current->row = this->row;
    current->column = this->column;
    current->start = start;
    current->length = (int)(end - start);
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

static enum TokenType ident_keyword(const char *ident, size_t size) {
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
        if (size == 5) return ident_trie(ident, 1, "lear", TOKEN_CLEAR);
        if (size == 8) return ident_trie(ident, 1, "ontinue", TOKEN_CONTINUE);
        if (size == 4) {
            if (ident[1] == 'o') return ident_trie(ident, 2, "py", TOKEN_COPY);
            if (ident[1] == 'a') return ident_trie(ident, 2, "se", TOKEN_CASE);
        }
        break;
    case 'l':
        if (size == 3 && ident[1] == 'e') {
            if (ident[2] == 't') return TOKEN_LET;
            if (ident[2] == 'n') return TOKEN_LEN;
        }
        break;
    case 't':
        if (size == 3) return ident_trie(ident, 1, "ry", TOKEN_TRY);
        if (size == 5) return ident_trie(ident, 1, "hrow", TOKEN_THROW);
        if (size == 4) {
            if (ident[1] == 'r') return ident_trie(ident, 2, "ue", TOKEN_TRUE);
            if (ident[1] == 'y') return ident_trie(ident, 2, "pe", TOKEN_TYPE_FUNC);
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
        if (size == 4) return ident_trie(ident, 1, "ush", TOKEN_PUSH);
        break;
    case 'e':
        if (size == 3) return ident_trie(ident, 1, "nd", TOKEN_END);
        if (size == 6) return ident_trie(ident, 1, "xcept", TOKEN_EXCEPT);
        if (size == 4 && ident[1] == 'l') {
            if (ident[2] == 's') {
                if (ident[3] == 'e') {
                    return TOKEN_ELSE;
                }
            } else if (ident[2] == 'i' && ident[3] == 'f') {
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

static void push_ident_token(Compiler *this, size_t start, size_t end) {
    const char *ident = &this->source[start];
    size_t size = end - start;
    enum TokenType keyword = ident_keyword(ident, size);
    if (keyword != TOKEN_UNDEFINED) {
        value_token(this, keyword, start, end);
    } else {
        value_token(this, TOKEN_IDENT, start, end);
    }
}

static bool is_digit(char c) {
    return '0' <= c && c <= '9';
}

static bool is_ident(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

static void advance(Compiler *this) {
    this->previous = this->current;
    if (this->previous.type == TOKEN_EOF) {
        return;
    }
    while (true) {
        char c = next_char(this);
        switch (c) {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            c = peek_char(this);
            while (c != '\0' && (c == ' ' || c == '\t' || c == '\r' || c == '\n')) {
                next_char(this);
                c = peek_char(this);
            }
            continue;
        case '-': {
            if (peek_char(this) == '-') {
                next_char(this);
                c = peek_char(this);
                while (c != '\n' && c != '\0') {
                    next_char(this);
                    c = peek_char(this);
                }
                continue;
            } else {
                token(this, TOKEN_SUBTRACT);
                return;
            }
        }
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
            size_t start = this->pos;
            while (true) {
                c = next_char(this);
                if (c == '\\') {
                    next_char(this);
                    continue;
                } else if (c == '"' || c == '\0') {
                    break;
                }
            }
            size_t end = this->pos - 1;
            value_token(this, TOKEN_STRING, start, end);
            return;
        }
        case '\'': {
            size_t start = this->pos;
            while (true) {
                c = next_char(this);
                if (c == '\\') {
                    next_char(this);
                    continue;
                } else if (c == '\'' || c == '\0') {
                    break;
                }
            }
            size_t end = this->pos - 1;
            value_token(this, TOKEN_STRING, start, end);
            return;
        }
        default: {
            if (is_digit(c)) {
                size_t start = this->pos - 1;
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
                size_t end = this->pos;
                if (discrete) {
                    value_token(this, TOKEN_INTEGER, start, end);
                } else {
                    value_token(this, TOKEN_FLOAT, start, end);
                }
                return;
            } else if (is_ident(c)) {
                size_t start = this->pos - 1;
                while (is_ident(peek_char(this))) {
                    next_char(this);
                }
                size_t end = this->pos;
                push_ident_token(this, start, end);
                return;
            } else {
                compile_error(this, &this->current, "Unknown character: `%c`", c);
            }
        }
        }
    }
}

static void value_pool_init(HymnValuePool *this) {
    this->count = 0;
    this->capacity = 8;
    this->values = hymn_malloc(8 * sizeof(HymnValue));
}

static void value_pool_add(HymnValuePool *this, HymnValue value) {
    int count = this->count;
    if (count + 1 > this->capacity) {
        this->capacity *= 2;
        this->values = hymn_realloc(this->values, this->capacity * sizeof(HymnValue));
    }
    this->values[count] = value;
    this->count = count + 1;
}

static void byte_code_init(HymnByteCode *this) {
    this->count = 0;
    this->capacity = 8;
    this->instructions = hymn_malloc(8 * sizeof(uint8_t));
    this->previous = UINT8_MAX;
    this->lines = hymn_malloc(8 * sizeof(int));
    value_pool_init(&this->constants);
}

static bool match_values(HymnValue a, HymnValue b) {
    if (a.is != b.is) {
        return false;
    }
    switch (a.is) {
    case HYMN_VALUE_UNDEFINED:
    case HYMN_VALUE_NONE: return true;
    case HYMN_VALUE_BOOL: return hymn_as_bool(a) == hymn_as_bool(b);
    case HYMN_VALUE_INTEGER: return hymn_as_int(a) == hymn_as_int(b);
    case HYMN_VALUE_FLOAT: return hymn_as_float(a) == hymn_as_float(b);
    case HYMN_VALUE_STRING:
    case HYMN_VALUE_ARRAY:
    case HYMN_VALUE_TABLE:
    case HYMN_VALUE_FUNC:
        return hymn_as_object(a) == hymn_as_object(b);
    case HYMN_VALUE_FUNC_NATIVE: return hymn_as_native(a) == hymn_as_native(b);
    case HYMN_VALUE_POINTER: return hymn_as_pointer(a) == hymn_as_pointer(b);
    }
    return false;
}

static HymnFunction *new_function(const char *script) {
    HymnFunction *func = hymn_calloc(1, sizeof(HymnFunction));
    byte_code_init(&func->code);
    if (script) func->script = hymn_new_string(script);
    return func;
}

static HymnNativeFunction *new_native_function(HymnString *name, HymnNativeCall func) {
    HymnNativeFunction *native = hymn_malloc(sizeof(HymnNativeFunction));
    native->name = name;
    native->func = func;
    return native;
}

static void array_init_with_capacity(HymnArray *this, int64_t length, int64_t capacity) {
    if (capacity == 0) {
        this->items = NULL;
    } else {
        this->items = hymn_calloc((size_t)capacity, sizeof(HymnValue));
    }
    this->length = length;
    this->capacity = capacity;
}

static void array_init(HymnArray *this, int64_t length) {
    array_init_with_capacity(this, length, length);
}

static HymnArray *new_array_with_capacity(int64_t length, int64_t capacity) {
    HymnArray *this = hymn_calloc(1, sizeof(HymnArray));
    array_init_with_capacity(this, length, capacity);
    return this;
}

static HymnArray *new_array(int64_t length) {
    return new_array_with_capacity(length, length);
}

static HymnArray *new_array_slice(HymnArray *from, int64_t start, int64_t end) {
    int64_t length = end - start;
    size_t size = (size_t)length * sizeof(HymnValue);
    HymnArray *this = hymn_calloc(1, sizeof(HymnArray));
    this->items = hymn_malloc(size);
    memcpy(this->items, &from->items[start], size);
    this->length = length;
    this->capacity = length;
    for (int64_t i = 0; i < length; i++) {
        reference(this->items[i]);
    }
    return this;
}

static HymnArray *new_array_copy(HymnArray *from) {
    return new_array_slice(from, 0, from->length);
}

static void array_update_capacity(HymnArray *this, int64_t length) {
    if (length > this->capacity) {
        if (this->capacity == 0) {
            this->capacity = length;
            this->items = hymn_calloc((size_t)length, sizeof(HymnValue));
        } else {
            this->capacity = length * 2;
            this->items = hymn_realloc(this->items, (size_t)this->capacity * sizeof(HymnValue));
            memset(this->items + (size_t)this->length, 0, (size_t)(this->capacity - this->length));
        }
    }
}

static void array_push(HymnArray *this, HymnValue value) {
    int64_t length = this->length + 1;
    array_update_capacity(this, length);
    this->length = length;
    this->items[length - 1] = value;
}

static void array_insert(HymnArray *this, int64_t index, HymnValue value) {
    int64_t length = this->length + 1;
    array_update_capacity(this, length);
    this->length = length;
    HymnValue *items = this->items;
    for (int64_t i = length - 1; i > index; i--) {
        items[i] = items[i - 1];
    }
    items[index] = value;
}

static HymnValue array_get(HymnArray *this, int64_t index) {
    if (index >= this->length) {
        return hymn_new_undefined();
    }
    return this->items[index];
}

static int64_t array_index_of(HymnArray *this, HymnValue match) {
    int64_t len = this->length;
    HymnValue *items = this->items;
    for (int64_t i = 0; i < len; i++) {
        if (match_values(match, items[i])) {
            return i;
        }
    }
    return -1;
}

static HymnValue array_pop(HymnArray *this) {
    if (this->length == 0) {
        return hymn_new_none();
    }
    return this->items[--this->length];
}

static HymnValue array_remove_index(HymnArray *this, int64_t index) {
    int64_t len = --this->length;
    HymnValue *items = this->items;
    HymnValue deleted = items[index];
    for (int64_t i = index; i < len; i++) {
        items[i] = items[i + 1];
    }
    return deleted;
}

static void array_clear(Hymn *H, HymnArray *this) {
    int64_t len = this->length;
    HymnValue *items = this->items;
    for (int64_t i = 0; i < len; i++) {
        dereference(H, items[i]);
    }
    this->length = 0;
}

static void array_delete(Hymn *H, HymnArray *this) {
    array_clear(H, this);
    free(this->items);
    free(this);
}

static HymnTable *new_table() {
    HymnTable *this = hymn_calloc(1, sizeof(HymnTable));
    table_init(this);
    return this;
}

static HymnTable *new_table_copy(HymnTable *from) {
    HymnTable *this = new_table();
    unsigned int bins = from->bins;
    for (unsigned int i = 0; i < bins; i++) {
        HymnTableItem *item = from->items[i];
        while (item != NULL) {
            table_put(this, item->key, item->value);
            reference_string(item->key);
            reference(item->value);
            item = item->next;
        }
    }
    return this;
}

static HymnArray *table_keys(HymnTable *this) {
    unsigned int size = this->size;
    HymnArray *array = new_array_with_capacity(size, size);
    if (size == 0) {
        return array;
    }
    HymnValue *keys = array->items;
    unsigned int total = 0;
    unsigned int bins = this->bins;
    for (unsigned int i = 0; i < bins; i++) {
        HymnTableItem *item = this->items[i];
        while (item != NULL) {
            HymnString *string = item->key->string;
            unsigned int insert = 0;
            while (insert != total) {
                if (string_compare(string, hymn_as_string(keys[insert])) < 0) {
                    for (unsigned int swap = total; swap > insert; swap--) {
                        keys[swap] = keys[swap - 1];
                    }
                    break;
                }
                insert++;
            }
            HymnValue value = hymn_new_string_value(item->key);
            reference(value);
            keys[insert] = value;
            total++;
            item = item->next;
        }
    }
    return array;
}

static HymnObjectString *table_key_of(HymnTable *this, HymnValue match) {
    unsigned int bin = 0;
    HymnTableItem *item = NULL;

    unsigned int bins = this->bins;
    for (unsigned int i = 0; i < bins; i++) {
        HymnTableItem *start = this->items[i];
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
                HymnTableItem *start = this->items[bin];
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

static void value_pool_delete(HymnValuePool *this) {
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
        scope->func->name = hymn_substring(this->source, this->previous.start, this->previous.start + this->previous.length);
    }

    Local *local = &scope->locals[scope->local_count++];
    local->depth = 0;
    local->name.start = 0;
    local->name.length = 0;
}

static inline Compiler new_compiler(const char *script, const char *source, Hymn *H, Scope *scope) {
    Compiler this = {0};
    this.row = 1;
    this.column = 1;
    this.script = script;
    this.source = source;
    this.size = strlen(source);
    this.previous.type = TOKEN_UNDEFINED;
    this.current.type = TOKEN_UNDEFINED;
    this.H = H;
    scope_init(&this, scope, TYPE_SCRIPT);
    return this;
}

static void byte_code_delete(HymnByteCode *this) {
    free(this->instructions);
    free(this->lines);
    value_pool_delete(&this->constants);
}

static uint8_t byte_code_new_constant(Compiler *this, HymnValue value) {
    HymnByteCode *code = current(this);
    value_pool_add(&code->constants, value);
    int constant = code->constants.count - 1;
    if (constant > UINT8_MAX) {
        compile_error(this, &this->previous, "Too many constants.");
        constant = 0;
    }
    return (uint8_t)constant;
}

static void boundary(HymnByteCode *code) {
    code->behind = UINT8_MAX;
    code->previous = UINT8_MAX;
}

#define REWRITE_BYTE(B)                \
    code->instructions[count - 1] = B; \
    return

#define REWRITE_CODE(OP)                \
    code->instructions[count - 1] = OP; \
    code->behind = p;                   \
    code->previous = OP;                \
    return

static void optimize_byte(HymnByteCode *code) {
    int count = code->count;
    if (count == 0) {
        return;
    }
    uint8_t previous = code->previous;
    if (previous == OP_SET_LOCAL && code->behind == OP_INCREMENT_LOCAL && count - 5 >= 0) {
        if (code->instructions[count - 1] == code->instructions[count - 4]) {
            code->instructions[count - 5] = OP_INCREMENT_LOCAL_AND_SET;
            code->behind = UINT8_MAX;
            code->previous = OP_INCREMENT_LOCAL_AND_SET;
            code->count -= 2;
        }
    }
}

static void write_byte(HymnByteCode *code, uint8_t b, int row) {
    int count = code->count;
    if (count + 1 > code->capacity) {
        code->capacity *= 2;
        code->instructions = hymn_realloc(code->instructions, code->capacity * sizeof(uint8_t));
        code->lines = hymn_realloc(code->lines, code->capacity * sizeof(int));
    }
    code->instructions[count] = b;
    code->lines[count] = row;
    code->count = count + 1;
}

static void write_instruction(Compiler *this, uint8_t i, int row) {
    HymnByteCode *code = current(this);
    int count = code->count;
    if (count != 0) {
        uint8_t p = code->previous;

        // OP_CONSTANT: [Integer: -1]
        // OP_GET_DYNAMIC

        // OP_GET_GLOBAL: [String: N]
        // OP_INCREMENT: [1]
        // OP_SET_GLOBAL: [String: N]

        // OP_GET_GLOBAL
        // OP_GET_LOCAL
        // OP_ARRAY_PUSH

        if (i == OP_RETURN && p == OP_CALL) {
            code->instructions[count - 2] = OP_TAIL_CALL;

        } else if (i == OP_POP) {
            if (p == OP_POP) {
                REWRITE_CODE(OP_POP_TWO);

            } else if (p == OP_POP_TWO) {
                code->instructions[count - 1] = OP_POP_N;
                code->behind = p;
                code->previous = OP_POP_N;
                write_byte(code, 3, row);
                return;

            } else if (p == OP_POP_N) {
                uint8_t pop = code->instructions[count - 1];
                if (pop < UINT8_MAX - 1) {
                    REWRITE_BYTE(pop + 1);
                }
            } else if (p == OP_INCREMENT_LOCAL_AND_SET) {
                return;
            } else if (p == OP_SET_LOCAL) {
                return;
            }
        } else if (i == OP_NEGATE && p == OP_CONSTANT) {
            HymnValue value = code->constants.values[code->instructions[count - 1]];
            if (hymn_is_int(value)) {
                value.as.i = -value.as.i;
            } else if (hymn_is_float(value)) {
                value.as.f = -value.as.f;
            }
            REWRITE_BYTE(byte_code_new_constant(this, value));

        } else if (i == OP_ADD) {
            if (p == OP_CONSTANT) {
                HymnValue value = code->constants.values[code->instructions[count - 1]];
                if (hymn_is_int(value)) {
                    int64_t add = hymn_as_int(value);
                    if (add > 0 && add < UINT8_MAX) {
                        if (code->behind == OP_GET_LOCAL) {
                            code->instructions[count - 4] = OP_INCREMENT_LOCAL;
                            code->instructions[count - 2] = (uint8_t)add;
                            code->behind = UINT8_MAX;
                            code->previous = OP_INCREMENT_LOCAL;
                            code->count--;
                            return;

                        } else {
                            code->instructions[count - 2] = OP_INCREMENT;
                            code->behind = p;
                            code->previous = OP_INCREMENT;
                            REWRITE_BYTE((uint8_t)add);
                        }
                    }
                }
            } else if (p == OP_GET_LOCAL && code->behind == OP_GET_LOCAL) {
                code->instructions[count - 4] = OP_ADD_TWO_LOCAL;
                code->instructions[count - 2] = code->instructions[count - 1];
                code->behind = UINT8_MAX;
                code->previous = OP_ADD_TWO_LOCAL;
                code->count--;
                return;
            }
        } else if (i == OP_JUMP_IF_TRUE) {
            switch (p) {
            case OP_EQUAL: REWRITE_CODE(OP_JUMP_IF_EQUAL);
            case OP_NOT_EQUAL: REWRITE_CODE(OP_JUMP_IF_NOT_EQUAL);
            case OP_LESS: REWRITE_CODE(OP_JUMP_IF_LESS);
            case OP_GREATER: REWRITE_CODE(OP_JUMP_IF_GREATER);
            case OP_LESS_EQUAL: REWRITE_CODE(OP_JUMP_IF_LESS_EQUAL);
            case OP_GREATER_EQUAL: REWRITE_CODE(OP_JUMP_IF_GREATER_EQUAL);
            }
        } else if (i == OP_JUMP_IF_FALSE) {
            switch (p) {
            case OP_EQUAL: REWRITE_CODE(OP_JUMP_IF_NOT_EQUAL);
            case OP_NOT_EQUAL: REWRITE_CODE(OP_JUMP_IF_EQUAL);
            case OP_LESS: REWRITE_CODE(OP_JUMP_IF_GREATER_EQUAL);
            case OP_GREATER: REWRITE_CODE(OP_JUMP_IF_LESS_EQUAL);
            case OP_LESS_EQUAL: REWRITE_CODE(OP_JUMP_IF_GREATER);
            case OP_GREATER_EQUAL: REWRITE_CODE(OP_JUMP_IF_LESS);
            }
        }
    }
    code->behind = code->previous;
    code->previous = i;
    write_byte(code, i, row);
}

static void write_short_instruction(Compiler *this, uint8_t i, uint8_t b) {
    int row = this->previous.row;
    write_instruction(this, i, row);
    write_byte(current(this), b, row);
    optimize_byte(current(this));
}

static void write_word_instruction(Compiler *this, uint8_t i, uint8_t b, uint8_t n) {
    int row = this->previous.row;
    write_instruction(this, i, row);
    HymnByteCode *code = current(this);
    write_byte(code, b, row);
    write_byte(code, n, row);
}

static void emit(Compiler *this, uint8_t i) {
    write_instruction(this, i, this->previous.row);
}

static void emit_byte(Compiler *this, uint8_t i) {
    write_byte(current(this), i, this->previous.row);
}

static uint8_t write_constant(Compiler *this, HymnValue value) {
    uint8_t constant = byte_code_new_constant(this, value);
    write_short_instruction(this, OP_CONSTANT, constant);
    return constant;
}

static Rule *token_rule(enum TokenType type) {
    return &rules[type];
}

static HymnObjectString *machine_intern_string(Hymn *this, HymnString *string) {
    HymnObjectString *object = set_add_or_get(&this->strings, string);
    if (object->string != string) {
        hymn_string_delete(string);
    }
    return object;
}

static HymnValue compile_intern_string(Hymn *this, HymnString *string) {
    HymnObjectString *object = set_add_or_get(&this->strings, string);
    if (object->string == string) {
        reference_string(object);
    } else {
        hymn_string_delete(string);
    }
    return hymn_new_string_value(object);
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
    if (assign && match(this, TOKEN_ASSIGN)) {
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

static uint8_t push_hidden_local(Compiler *this) {
    Scope *scope = this->scope;
    if (scope->local_count == HYMN_UINT8_COUNT) {
        compile_error(this, &this->previous, "Too many local variables in scope.");
        return 0;
    }
    uint8_t index = (uint8_t)scope->local_count++;
    Local *local = &scope->locals[index];
    local->name = (Token){0};
    local->depth = scope->depth;
    return index;
}

static uint8_t arguments(Compiler *this) {
    uint8_t count = 0;
    if (!check(this, TOKEN_RIGHT_PAREN)) {
        do {
            expression(this);
            if (count == UINT8_MAX) {
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
    uint8_t count = arguments(this);
    write_short_instruction(this, OP_CALL, count);
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
    Token *previous = &this->previous;
    int64_t number = (int64_t)strtoll(&this->source[previous->start], NULL, 10);
    write_constant(this, hymn_new_int(number));
}

static void compile_float(Compiler *this, bool assign) {
    (void)assign;
    Token *previous = &this->previous;
    double number = strtod(&this->source[previous->start], NULL);
    write_constant(this, hymn_new_float(number));
}

char escape_sequence(const char c) {
    switch (c) {
    case 'a':
        return '\a';
    case 'b':
        return '\b';
    case 'f':
        return '\f';
    case 'n':
        return '\n';
    case 'r':
        return '\r';
    case 't':
        return '\t';
    case 'v':
        return '\v';
    case '\\':
        return '\\';
    case '\'':
        return '\'';
    case '"':
        return '"';
    case '?':
        return '\?';
    default:
        return '\0';
    }
}

HymnString *parse_string_literal(const char *string, size_t start, size_t len) {
    const size_t end = start + len;
    HymnString *literal = new_string_with_capacity(len);
    for (size_t i = start; i < end; i++) {
        const char c = string[i];
        if (c == '\\' && i + 1 < end) {
            const char e = escape_sequence(string[i + 1]);
            if (e != '\0') {
                literal = hymn_string_append_char(literal, e);
                i++;
                continue;
            }
        }
        literal = hymn_string_append_char(literal, c);
    }
    return literal;
}

static void compile_string(Compiler *this, bool assign) {
    (void)assign;
    Token *previous = &this->previous;
    HymnString *s = parse_string_literal(this->source, previous->start, previous->length);
    write_constant(this, compile_intern_string(this->H, s));
}

static uint8_t ident_constant(Compiler *this, Token *token) {
    HymnString *string = hymn_substring(this->source, token->start, token->start + token->length);
    return byte_code_new_constant(this, compile_intern_string(this->H, string));
}

static void begin_scope(Compiler *this) {
    this->scope->depth++;
}

static void end_scope(Compiler *this) {
    Scope *scope = this->scope;
    scope->depth--;
    while (scope->local_count > 0 && scope->locals[scope->local_count - 1].depth > scope->depth) {
        emit(this, OP_POP);
        scope->local_count--;
    }
}

static void compile_array(Compiler *this, bool assign) {
    (void)assign;
    write_constant(this, hymn_new_array_value(NULL));
    if (match(this, TOKEN_RIGHT_SQUARE)) {
        return;
    }
    while (!check(this, TOKEN_RIGHT_SQUARE) && !check(this, TOKEN_EOF)) {
        emit(this, OP_DUPLICATE);
        expression(this);
        emit(this, OP_ARRAY_PUSH);
        emit(this, OP_POP);
        if (!check(this, TOKEN_RIGHT_SQUARE)) {
            consume(this, TOKEN_COMMA, "Expected ','.");
        }
    }
    consume(this, TOKEN_RIGHT_SQUARE, "Expected ']' declaring array.");
}

static void compile_table(Compiler *this, bool assign) {
    (void)assign;
    write_constant(this, hymn_new_table_value(NULL));
    if (match(this, TOKEN_RIGHT_CURLY)) {
        return;
    }
    while (!check(this, TOKEN_RIGHT_CURLY) && !check(this, TOKEN_EOF)) {
        emit(this, OP_DUPLICATE);
        consume(this, TOKEN_IDENT, "Expected property name");
        uint8_t name = ident_constant(this, &this->previous);
        consume(this, TOKEN_COLON, "Expected ':'.");
        expression(this);
        write_short_instruction(this, OP_SET_PROPERTY, name);
        emit(this, OP_POP);
        if (!check(this, TOKEN_RIGHT_CURLY)) {
            consume(this, TOKEN_COMMA, "Expected ','.");
        }
    }
    consume(this, TOKEN_RIGHT_CURLY, "Expected '}' declaring table.");
}

static void function_delete(HymnFunction *this) {
    byte_code_delete(&this->code);
    hymn_string_delete(this->name);
    hymn_string_delete(this->script);
    HymnExceptList *except = this->except;
    while (except != NULL) {
        HymnExceptList *next = except->next;
        free(except);
        except = next;
    }
    free(this);
}

static void native_function_delete(HymnNativeFunction *this) {
    hymn_string_delete(this->name);
    free(this);
}

static void push_local(Compiler *this, Token name) {
    Scope *scope = this->scope;
    if (scope->local_count == HYMN_UINT8_COUNT) {
        compile_error(this, &name, "Too many local variables in scope.");
        return;
    }
    Local *local = &scope->locals[scope->local_count++];
    local->name = name;
    local->depth = -1;
}

static bool ident_match(Compiler *this, Token *a, Token *b) {
    if (a->length != b->length) {
        return false;
    }
    return memcmp(&this->source[a->start], &this->source[b->start], a->length) == 0;
}

static uint8_t variable(Compiler *this, const char *error) {
    consume(this, TOKEN_IDENT, error);
    Scope *scope = this->scope;
    if (scope->depth == 0) {
        return ident_constant(this, &this->previous);
    }
    Token *name = &this->previous;
    for (int i = scope->local_count - 1; i >= 0; i--) {
        Local *local = &scope->locals[i];
        if (local->depth != -1 && local->depth < scope->depth) {
            break;
        } else if (ident_match(this, name, &local->name)) {
            compile_error(this, name, "Scope Error: Variable `%.*s` already exists in this scope.", name->length, &this->source[name->start]);
        }
    }
    push_local(this, *name);
    return 0;
}

static void local_initialize(Compiler *this) {
    Scope *scope = this->scope;
    if (scope->depth == 0) {
        return;
    }
    scope->locals[scope->local_count - 1].depth = scope->depth;
}

static void finalize_variable(Compiler *this, uint8_t global) {
    if (this->scope->depth > 0) {
        local_initialize(this);
        return;
    }
    write_short_instruction(this, OP_DEFINE_GLOBAL, global);
}

static void define_new_variable(Compiler *this) {
    uint8_t v = variable(this, "Syntax Error: Expected variable name.");
    consume(this, TOKEN_ASSIGN, "Assignment Error: Expected '=' after variable.");
    expression(this);
    finalize_variable(this, v);
}

static int resolve_local(Compiler *this, Token *name) {
    Scope *scope = this->scope;
    for (int i = scope->local_count - 1; i >= 0; i--) {
        Local *local = &scope->locals[i];
        if (ident_match(this, name, &local->name)) {
            if (local->depth == -1) {
                compile_error(this, name, "Reference Error: Local variable `%.*s` referenced before assignment.", name->length, &this->source[name->start]);
            }
            return i;
        }
    }
    return -1;
}

static void named_variable(Compiler *this, Token token, bool assign) {
    uint8_t get;
    uint8_t set;
    int var = resolve_local(this, &token);
    if (var != -1) {
        get = OP_GET_LOCAL;
        set = OP_SET_LOCAL;
    } else {
        get = OP_GET_GLOBAL;
        set = OP_SET_GLOBAL;
        var = ident_constant(this, &token);
    }
    if (assign && match(this, TOKEN_ASSIGN)) {
        expression(this);
        write_short_instruction(this, set, (uint8_t)var);
    } else {
        write_short_instruction(this, get, (uint8_t)var);
    }
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
    uint8_t name = ident_constant(this, &this->previous);
    if (assign && match(this, TOKEN_ASSIGN)) {
        expression(this);
        write_short_instruction(this, OP_SET_PROPERTY, name);
    } else {
        write_short_instruction(this, OP_GET_PROPERTY, name);
    }
}

static void compile_square(Compiler *this, bool assign) {
    if (match(this, TOKEN_COLON)) {
        write_constant(this, hymn_new_int(0));
        if (match(this, TOKEN_RIGHT_SQUARE)) {
            write_constant(this, hymn_new_none());
        } else {
            expression(this);
            consume(this, TOKEN_RIGHT_SQUARE, "Expected ']' after expression.");
        }
        emit(this, OP_SLICE);
    } else {
        expression(this);
        if (match(this, TOKEN_COLON)) {
            if (match(this, TOKEN_RIGHT_SQUARE)) {
                write_constant(this, hymn_new_none());
            } else {
                expression(this);
                consume(this, TOKEN_RIGHT_SQUARE, "Expected ']' after expression.");
            }
            emit(this, OP_SLICE);
        } else {
            consume(this, TOKEN_RIGHT_SQUARE, "Expected ']' after expression.");
            if (assign && match(this, TOKEN_ASSIGN)) {
                expression(this);
                emit(this, OP_SET_DYNAMIC);
            } else {
                emit(this, OP_GET_DYNAMIC);
            }
        }
    }
}

static int emit_jump(Compiler *this, uint8_t instruction) {
    emit(this, instruction);
    emit_byte(this, UINT8_MAX);
    emit_byte(this, UINT8_MAX);
    // boundary(current(this));
    return current(this)->count - 2;
}

static void patch_jump(Compiler *this, int jump) {
    HymnByteCode *code = current(this);
    int offset = code->count - jump - 2;
    if (offset > UINT16_MAX) {
        compile_error(this, &this->previous, "Jump offset too large.");
        return;
    }
    code->instructions[jump] = (offset >> 8) & UINT8_MAX;
    code->instructions[jump + 1] = offset & UINT8_MAX;
    // boundary(current(this));
}

static struct JumpList *add_jump(Compiler *C, struct JumpList *list, enum OpCode code) {
    struct JumpList *jump = hymn_calloc(1, sizeof(struct JumpList));
    jump->jump = emit_jump(C, code);
    jump->next = list;
    return jump;
}

static void free_jumps(Compiler *C, struct JumpList *list) {
    while (list != NULL) {
        patch_jump(C, list->jump);
        struct JumpList *next = list->next;
        free(list);
        list = next;
    }
}

static void compile_and(Compiler *C, bool assign) {
    (void)assign;
    C->jump_and = add_jump(C, C->jump_and, OP_JUMP_IF_FALSE);
    compile_with_precedence(C, PRECEDENCE_AND);
}

static void compile_or(Compiler *C, bool assign) {
    (void)assign;
    C->jump_or = add_jump(C, C->jump_or, OP_JUMP_IF_TRUE);
    free_jumps(C, C->jump_and);
    C->jump_and = NULL;
    compile_with_precedence(C, PRECEDENCE_OR);
}

static HymnFunction *end_function(Compiler *this) {
    emit(this, OP_NONE);
    emit(this, OP_RETURN);
    HymnFunction *func = this->scope->func;
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
            if (this->scope->func->arity > UINT8_MAX) {
                compile_error(this, &this->previous, "Can't have more than 255 function parameters.");
            }
            uint8_t parameter = variable(this, "Expected parameter name.");
            finalize_variable(this, parameter);
        } while (match(this, TOKEN_COMMA));
    }

    consume(this, TOKEN_RIGHT_PAREN, "Expected ')' after function parameters.");

    while (!check(this, TOKEN_END) && !check(this, TOKEN_EOF)) {
        declaration(this);
    }

    end_scope(this);
    consume(this, TOKEN_END, "Expected 'end' after function body.");

    HymnFunction *func = end_function(this);
    write_constant(this, hymn_new_func_value(func));
}

static void declare_function(Compiler *this) {
    uint8_t global = variable(this, "Expected function name.");
    local_initialize(this);
    compile_function(this, TYPE_FUNCTION);
    finalize_variable(this, global);
}

static void declaration(Compiler *this) {
    if (match(this, TOKEN_LET)) {
        define_new_variable(this);
    } else if (match(this, TOKEN_FUNCTION)) {
        declare_function(this);
    } else {
        statement(this);
    }
}

static void block(Compiler *this) {
    begin_scope(this);
    while (!check(this, TOKEN_END) && !check(this, TOKEN_EOF)) {
        declaration(this);
    }
    end_scope(this);
}

static void if_statement(Compiler *C) {
    expression(C);
    int jump = emit_jump(C, OP_JUMP_IF_FALSE);

    // FIXME NESTED IF STATEMENTS PROBABLY BROKEN

    free_jumps(C, C->jump_or);
    C->jump_or = NULL;

    begin_scope(C);
    while (!check(C, TOKEN_ELIF) && !check(C, TOKEN_ELSE) && !check(C, TOKEN_END) && !check(C, TOKEN_EOF)) {
        declaration(C);
    }
    end_scope(C);

    struct JumpList jump_end = {0};
    jump_end.jump = emit_jump(C, OP_JUMP);
    struct JumpList *tail = &jump_end;

    while (match(C, TOKEN_ELIF)) {
        patch_jump(C, jump);

        free_jumps(C, C->jump_and);
        C->jump_and = NULL;

        expression(C);
        jump = emit_jump(C, OP_JUMP_IF_FALSE);

        free_jumps(C, C->jump_or);
        C->jump_or = NULL;

        begin_scope(C);
        while (!check(C, TOKEN_ELIF) && !check(C, TOKEN_ELSE) && !check(C, TOKEN_END) && !check(C, TOKEN_EOF)) {
            declaration(C);
        }
        end_scope(C);

        struct JumpList *next = hymn_calloc(1, sizeof(struct JumpList));
        next->jump = emit_jump(C, OP_JUMP);

        tail->next = next;
        tail = next;
    }

    patch_jump(C, jump);

    free_jumps(C, C->jump_and);
    C->jump_and = NULL;

    if (match(C, TOKEN_ELSE)) {
        block(C);
    }

    patch_jump(C, jump_end.jump);
    free_jumps(C, jump_end.next);

    consume(C, TOKEN_END, "If: Missing 'end'.");
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

    uint8_t local = push_hidden_local(this);
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
        write_short_instruction(this, OP_GET_LOCAL, local);
        emit(this, OP_EQUAL);

        struct JumpList *body = NULL;

        if (match(this, TOKEN_OR)) {
            body = hymn_calloc(1, sizeof(struct JumpList));
            struct JumpList *link = body;
            body->jump = emit_jump(this, OP_JUMP_IF_TRUE);
            emit(this, OP_POP);

            while (true) {
                if (!compile_literal(this)) {
                    compile_error(this, &this->current, "Expected literal after 'or' in case.");
                }
                write_short_instruction(this, OP_GET_LOCAL, local);
                emit(this, OP_EQUAL);

                if (match(this, TOKEN_OR)) {
                    struct JumpList *next = hymn_calloc(1, sizeof(struct JumpList));
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
        while (!check(this, TOKEN_CASE) && !check(this, TOKEN_ELSE) && !check(this, TOKEN_END) && !check(this, TOKEN_EOF)) {
            declaration(this);
        }
        end_scope(this);

        struct JumpList *next = hymn_calloc(1, sizeof(struct JumpList));
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
    emit_byte(this, (offset >> 8) & UINT8_MAX);
    emit_byte(this, offset & UINT8_MAX);
    boundary(current(this));
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

static void iterate_statement(Compiler *this) {
    begin_scope(this);

    // parameters

    uint8_t id;

    uint8_t value = (uint8_t)this->scope->local_count;
    variable(this, "Iterator: Missing parameter.");
    local_initialize(this);

    if (match(this, TOKEN_COMMA)) {
        id = value;
        emit(this, OP_NONE);

        value = (uint8_t)this->scope->local_count;
        variable(this, "Iterator: Missing second parameter.");
        local_initialize(this);
        emit(this, OP_NONE);
    } else {
        emit(this, OP_NONE);

        id = push_hidden_local(this);
        emit(this, OP_NONE);
    }

    consume(this, TOKEN_IN, "Iterator: Missing 'in' after parameters.");

    // setup

    uint8_t object = push_hidden_local(this);
    expression(this);

    uint8_t keys = push_hidden_local(this);
    emit(this, OP_NONE);

    uint8_t length = push_hidden_local(this);
    emit(this, OP_NONE);

    uint8_t index = push_hidden_local(this);
    write_constant(this, hymn_new_int(0));

    // type check

    uint8_t type = push_hidden_local(this);
    write_short_instruction(this, OP_GET_LOCAL, object);
    emit(this, OP_TYPE);

    write_short_instruction(this, OP_GET_LOCAL, type);
    write_constant(this, compile_intern_string(this->H, hymn_new_string(STRING_TABLE)));
    emit(this, OP_EQUAL);

    int jump_not_table = emit_jump(this, OP_JUMP_IF_FALSE);

    // type is table

    emit(this, OP_POP);

    write_short_instruction(this, OP_GET_LOCAL, object);
    emit(this, OP_KEYS);
    write_short_instruction(this, OP_SET_LOCAL, keys);
    emit(this, OP_LEN);
    write_short_instruction(this, OP_SET_LOCAL, length);
    emit(this, OP_POP);

    int jump_table_end = emit_jump(this, OP_JUMP);

    patch_jump(this, jump_not_table);

    emit(this, OP_POP);

    write_short_instruction(this, OP_GET_LOCAL, type);
    write_constant(this, compile_intern_string(this->H, hymn_new_string(STRING_ARRAY)));
    emit(this, OP_EQUAL);

    int jump_not_array = emit_jump(this, OP_JUMP_IF_FALSE);

    // type is array

    emit(this, OP_POP);
    write_short_instruction(this, OP_GET_LOCAL, object);
    emit(this, OP_LEN);
    write_short_instruction(this, OP_SET_LOCAL, length);
    emit(this, OP_POP);

    int jump_array_end = emit_jump(this, OP_JUMP);

    // unexpected type

    patch_jump(this, jump_not_array);

    emit(this, OP_POP);
    write_constant(this, compile_intern_string(this->H, hymn_new_string("Iterator: Expected `Array` or `Table`")));
    emit(this, OP_THROW);

    patch_jump(this, jump_table_end);
    patch_jump(this, jump_array_end);

    // compare

    int compare = current(this)->count;

    write_short_instruction(this, OP_GET_LOCAL, index);
    write_short_instruction(this, OP_GET_LOCAL, length);
    emit(this, OP_LESS);

    int jump = emit_jump(this, OP_JUMP_IF_FALSE);
    emit(this, OP_POP);

    // increment

    int body = emit_jump(this, OP_JUMP);
    int increment = current(this)->count;

    struct LoopList loop = {.start = increment, .depth = this->scope->depth + 1, .next = this->loop};
    this->loop = &loop;

    write_short_instruction(this, OP_GET_LOCAL, index);
    write_constant(this, hymn_new_int(1));
    emit(this, OP_ADD);
    write_short_instruction(this, OP_SET_LOCAL, index);

    emit(this, OP_POP);
    emit_loop(this, compare);

    // body

    patch_jump(this, body);

    write_short_instruction(this, OP_GET_LOCAL, object);

    write_short_instruction(this, OP_GET_LOCAL, keys);
    emit(this, OP_NONE);
    emit(this, OP_EQUAL);

    int jump_no_keys = emit_jump(this, OP_JUMP_IF_FALSE);

    emit(this, OP_POP);
    write_short_instruction(this, OP_GET_LOCAL, index);

    int jump_no_keys_end = emit_jump(this, OP_JUMP);

    patch_jump(this, jump_no_keys);

    emit(this, OP_POP);
    write_short_instruction(this, OP_GET_LOCAL, keys);
    write_short_instruction(this, OP_GET_LOCAL, index);
    emit(this, OP_GET_DYNAMIC);

    patch_jump(this, jump_no_keys_end);

    write_short_instruction(this, OP_SET_LOCAL, id);
    emit(this, OP_GET_DYNAMIC);

    write_short_instruction(this, OP_SET_LOCAL, value);
    emit(this, OP_POP);

    block(this);
    emit_loop(this, increment);

    // end

    this->loop = loop.next;

    patch_jump(this, jump);
    emit(this, OP_POP);

    patch_jump_list(this);
    end_scope(this);

    consume(this, TOKEN_END, "Iterator: Missing 'end'.");
}

static void for_statement(Compiler *this) {
    begin_scope(this);

    // assign

    define_new_variable(this);
    uint8_t index = (uint8_t)(this->scope->local_count - 1);

    consume(this, TOKEN_COMMA, "For: Missing ','.");

    // compare

    int compare = current(this)->count;

    expression(this);

    int jump = emit_jump(this, OP_JUMP_IF_FALSE);

    // increment

    int body = emit_jump(this, OP_JUMP);
    int increment = current(this)->count;

    struct LoopList loop = {.start = increment, .depth = this->scope->depth + 1, .next = this->loop};
    this->loop = &loop;

    if (match(this, TOKEN_COMMA)) {
        expression(this);
    } else {
        write_word_instruction(this, OP_INCREMENT_LOCAL_AND_SET, index, 1);
    }

    emit_loop(this, compare);

    // body

    patch_jump(this, body);

    block(this);
    emit_loop(this, increment);

    // end

    this->loop = loop.next;

    patch_jump(this, jump);

    patch_jump_list(this);
    end_scope(this);

    consume(this, TOKEN_END, "For: Missing 'end'.");
}

static void while_statement(Compiler *this) {
    int start = current(this)->count;

    struct LoopList loop = {.start = start, .depth = this->scope->depth + 1, .next = this->loop};
    this->loop = &loop;

    expression(this);

    int jump = emit_jump(this, OP_JUMP_IF_FALSE);

    block(this);
    emit_loop(this, start);

    this->loop = loop.next;

    patch_jump(this, jump);

    patch_jump_list(this);

    consume(this, TOKEN_END, "While: Missing 'end'.");
}

static void return_statement(Compiler *this) {
    if (this->scope->type == TYPE_SCRIPT) {
        compile_error(this, &this->previous, "Return: Outside of function.");
    }
    if (check(this, TOKEN_END)) {
        emit(this, OP_NONE);
    } else {
        expression(this);
    }
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
        compile_error(this, &this->previous, "Break Error: Outside of loop.");
    }
    pop_stack_loop(this);
    struct JumpList *jump_next = this->jump;
    struct JumpList *jump = hymn_malloc(sizeof(struct JumpList));
    jump->jump = emit_jump(this, OP_JUMP);
    jump->depth = this->loop->depth;
    jump->next = jump_next;
    this->jump = jump;
}

static void continue_statement(Compiler *this) {
    if (this->loop == NULL) {
        compile_error(this, &this->previous, "Continue Error: Outside of loop.");
    }
    pop_stack_loop(this);
    emit_loop(this, this->loop->start);
}

static void try_statement(Compiler *this) {
    HymnByteCode *code = current(this);

    HymnExceptList *except = hymn_calloc(1, sizeof(HymnExceptList));
    except->locals = this->scope->local_count;
    except->start = code->count;

    HymnFunction *func = current_func(this);
    except->next = func->except;
    func->except = except;

    begin_scope(this);
    while (!check(this, TOKEN_EXCEPT) && !check(this, TOKEN_EOF)) {
        declaration(this);
    }
    end_scope(this);

    int jump = emit_jump(this, OP_JUMP);

    consume(this, TOKEN_EXCEPT, "Try: Missing 'except'.");

    boundary(code);
    except->end = code->count;

    begin_scope(this);
    uint8_t message = variable(this, "Try: Missing variable after 'except'.");
    finalize_variable(this, message);
    while (!check(this, TOKEN_END) && !check(this, TOKEN_EOF)) {
        declaration(this);
    }
    end_scope(this);

    consume(this, TOKEN_END, "Try: Missing 'end'.");

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

static void throw_statement(Compiler *this) {
    expression(this);
    emit(this, OP_THROW);
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

static HymnFrame *parent_frame(Hymn *this, int offset) {
    int frame_count = this->frame_count;
    if (offset > frame_count) return NULL;
    return &this->frames[frame_count - offset];
}

static HymnFrame *current_frame(Hymn *this) {
    return &this->frames[this->frame_count - 1];
}

struct CompileReturn {
    HymnFunction *func;
    char *error;
};

static struct CompileReturn compile(Hymn *H, const char *script, const char *source) {
    Scope scope = {0};

    Compiler c = new_compiler(script, source, H, &scope);
    Compiler *compiler = &c;

    advance(compiler);
    while (!match(compiler, TOKEN_EOF)) {
        declaration(compiler);
    }

    HymnFunction *func = end_function(compiler);
    char *error = NULL;

    if (compiler->error) {
        error = string_to_chars(compiler->error);
        hymn_string_delete(compiler->error);
    }

    return (struct CompileReturn){.func = func, .error = error};
}

struct PointerSet {
    int count;
    int capacity;
    void **items;
};

bool pointer_set_has(struct PointerSet *set, void *pointer) {
    void **items = set->items;
    if (items) {
        int count = set->count;
        for (int i = 0; i < count; i++) {
            if (pointer == items[i]) {
                return true;
            }
        }
    }
    return false;
}

void pointer_set_add(struct PointerSet *set, void *pointer) {
    if (set->items) {
        int count = set->count;
        if (count + 1 > set->capacity) {
            set->capacity *= 2;
            set->items = hymn_realloc(set->items, set->capacity * sizeof(void *));
        }
        set->items[count] = pointer;
        set->count = count + 1;
    } else {
        set->count = 1;
        set->capacity = 1;
        set->items = hymn_calloc(1, sizeof(void *));
        set->items[0] = pointer;
    }
}

static HymnString *value_to_string_recusive(HymnValue value, struct PointerSet *set, bool quote) {
    switch (value.is) {
    case HYMN_VALUE_UNDEFINED: return hymn_new_string(STRING_UNDEFINED);
    case HYMN_VALUE_NONE: return hymn_new_string(STRING_NONE_TYPE);
    case HYMN_VALUE_BOOL: return hymn_as_bool(value) ? hymn_new_string(STRING_TRUE) : hymn_new_string(STRING_FALSE);
    case HYMN_VALUE_INTEGER: return int64_to_string(hymn_as_int(value));
    case HYMN_VALUE_FLOAT: return double_to_string(hymn_as_float(value));
    case HYMN_VALUE_STRING: {
        if (quote) return hymn_string_format("\"%s\"", hymn_as_string(value));
        return string_copy(hymn_as_string(value));
    }
    case HYMN_VALUE_ARRAY: {
        HymnArray *array = hymn_as_array(value);
        if (array == NULL || array->length == 0) {
            return hymn_new_string("[]");
        }
        if (pointer_set_has(set, array)) {
            return hymn_new_string("[..]");
        } else {
            pointer_set_add(set, array);
        }
        HymnString *string = hymn_new_string("[");
        for (int64_t i = 0; i < array->length; i++) {
            if (i != 0) {
                string = hymn_string_append(string, ", ");
            }
            HymnString *add = value_to_string_recusive(array->items[i], set, true);
            string = hymn_string_append(string, add);
            hymn_string_delete(add);
        }
        string = hymn_string_append_char(string, ']');
        return string;
    }
    case HYMN_VALUE_TABLE: {
        HymnTable *table = hymn_as_table(value);
        if (table == NULL || table->size == 0) {
            return hymn_new_string("{}");
        }
        if (pointer_set_has(set, table)) {
            return hymn_new_string("{ .. }");
        } else {
            pointer_set_add(set, table);
        }
        HymnObjectString **keys = hymn_malloc(table->size * sizeof(HymnObjectString *));
        unsigned int total = 0;
        unsigned int bins = table->bins;
        for (unsigned int i = 0; i < bins; i++) {
            HymnTableItem *item = table->items[i];
            while (item != NULL) {
                HymnString *string = item->key->string;
                unsigned int insert = 0;
                while (insert != total) {
                    if (string_compare(string, keys[insert]->string) < 0) {
                        for (unsigned int swap = total; swap > insert; swap--) {
                            keys[swap] = keys[swap - 1];
                        }
                        break;
                    }
                    insert++;
                }
                keys[insert] = item->key;
                total++;
                item = item->next;
            }
        }
        HymnString *string = hymn_new_string("{ ");
        for (unsigned int i = 0; i < table->size; i++) {
            if (i != 0) {
                string = hymn_string_append(string, ", ");
            }
            HymnValue item = table_get(table, keys[i]);
            HymnString *add = value_to_string_recusive(item, set, true);
            string = string_append_format(string, "%s: %s", keys[i]->string, add);
            hymn_string_delete(add);
        }
        string = hymn_string_append(string, " }");
        free(keys);
        return string;
    }
    case HYMN_VALUE_FUNC: {
        HymnFunction *func = hymn_as_func(value);
        if (func->name) return string_copy(func->name);
        if (func->script) return string_copy(func->script);
        return hymn_new_string("Script");
    }
    case HYMN_VALUE_FUNC_NATIVE: return string_copy(hymn_as_native(value)->name);
    case HYMN_VALUE_POINTER: return hymn_string_format("%p", hymn_as_pointer(value));
    }
    return hymn_new_string("?");
}

static HymnString *value_to_string(HymnValue value) {
    struct PointerSet set = {.count = 0, .capacity = 0, .items = NULL};
    HymnString *string = value_to_string_recusive(value, &set, false);
    free(set.items);
    return string;
}

static HymnString *value_concat(HymnValue a, HymnValue b) {
    HymnString *string = value_to_string(a);
    HymnString *second = value_to_string(b);
    string = hymn_string_append(string, second);
    hymn_string_delete(second);
    return string;
}

static HymnString *debug_value_to_string(HymnValue value) {
    HymnString *string = value_to_string(value);
    HymnString *format = hymn_string_format("%s: %s", value_name(value.is), string);
    hymn_string_delete(string);
    return format;
}

static void debug_value(HymnValue value) {
    HymnString *string = debug_value_to_string(value);
    printf("%s", string);
    hymn_string_delete(string);
}

static void debug_value_message(const char *prefix, HymnValue value) {
    printf("%s", prefix);
    debug_value(value);
    printf("\n");
}

static void reset_stack(Hymn *this) {
    this->stack_top = this->stack;
    this->frame_count = 0;
}

static bool is_object(HymnValue value) {
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
static void debug_reference(HymnValue value) {
    if (is_object(value)) {
        HymnObject *object = hymn_as_object(value);
        int count = object->count;
        printf("REF     | [%p] [", (void *)object);
        debug_value(value);
        printf("] [%d]\n", count);
        assert(count >= 0);
    }
}

static void debug_dereference(HymnValue value) {
    if (is_object(value)) {
        HymnObject *object = hymn_as_object(value);
        int count = object->count - 1;
        if (count == 0) {
            printf("FREE    | [%p] [", (void *)object);
            debug_value(value);
            printf("]\n");
        } else if (count < 0) {
            printf("BAD     | [%p] [", (void *)object);
            debug_value(value);
            printf("]\n");
        } else {
            printf("DEREF   | [%p] [", (void *)object);
            debug_value(value);
            printf("] [%d]\n", count);
        }
        assert(count >= 0);
    }
}
#endif

#ifdef HYMN_NO_MEMORY_MANAGE
static void reference_string(HymnObjectString *string) {
    (void)string;
}
#else
static void reference_string(HymnObjectString *string) {
    string->object.count++;
#ifdef HYMN_DEBUG_REFERENCE
    debug_reference(hymn_new_string_value(string));
#endif
}
#endif

#ifdef HYMN_NO_MEMORY_MANAGE
static void reference(HymnValue value) {
    (void)value;
}
#else
static void reference(HymnValue value) {
    if (is_object(value)) {
        hymn_as_object(value)->count++;
#ifdef HYMN_DEBUG_REFERENCE
        debug_reference(value);
#endif
    }
}
#endif

#ifdef HYMN_NO_MEMORY_MANAGE
static void dereference_string(Hymn *this, HymnObjectString *string) {
    (void)this;
    (void)string;
}
#else
static void dereference_string(Hymn *this, HymnObjectString *string) {
#ifdef HYMN_DEBUG_REFERENCE
    debug_dereference(hymn_new_string_value(string));
#endif
    int count = --(string->object.count);
    assert(count >= 0);
    if (count == 0) {
        set_remove(&this->strings, string->string);
        hymn_string_delete(string->string);
        free(string);
    }
}
#endif

#ifdef HYMN_NO_MEMORY_MANAGE
static void dereference(Hymn *this, HymnValue value) {
    (void)this;
    (void)value;
}
#else
static void dereference(Hymn *this, HymnValue value) {
    switch (value.is) {
    case HYMN_VALUE_STRING: {
        HymnObjectString *string = hymn_as_hymn_string(value);
        dereference_string(this, string);
        break;
    }
    case HYMN_VALUE_ARRAY: {
#ifdef HYMN_DEBUG_REFERENCE
        debug_dereference(value);
#endif
        HymnArray *array = hymn_as_array(value);
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
        HymnTable *table = hymn_as_table(value);
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
        HymnFunction *func = hymn_as_func(value);
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
#endif

static void push(Hymn *this, HymnValue value) {
    *this->stack_top = value;
    this->stack_top++;
}

static HymnValue peek(Hymn *this, int dist) {
    assert(&this->stack_top[-dist] >= this->stack);
    return this->stack_top[-dist];
}

static HymnValue pop(Hymn *this) {
    assert(&this->stack_top[-1] >= this->stack);
    this->stack_top--;
    return *this->stack_top;
}

static void machine_push_intern_string(Hymn *this, HymnString *string) {
    HymnObjectString *intern = machine_intern_string(this, string);
    reference_string(intern);
    push(this, hymn_new_string_value(intern));
}

static HymnFrame *machine_exception(Hymn *H) {
    HymnFrame *frame = current_frame(H);
    while (true) {
        HymnFunction *func = frame->func;
        uint8_t *instructions = func->code.instructions;
        HymnExceptList *except = NULL;
        HymnExceptList *range = func->except;
        while (range != NULL) {
            if (frame->ip >= &instructions[range->start] && frame->ip <= &instructions[range->end]) {
                except = range;
                break;
            }
            range = range->next;
        }
        HymnValue result = pop(H);
        if (except != NULL) {
            while (H->stack_top != &frame->stack[except->locals]) {
                dereference(H, pop(H));
            }
            frame->ip = &instructions[except->end];
            push(H, result);
            return frame;
        }
        while (H->stack_top != frame->stack) {
            dereference(H, pop(H));
        }
        H->frame_count--;
        if (H->frame_count == 0 || func->name == NULL) {
            assert(H->error == NULL);
            H->error = value_to_string(result);
            dereference(H, result);
            return NULL;
        }
        push(H, result);
        frame = current_frame(H);
    }
}

static HymnString *machine_stacktrace(Hymn *this) {
    HymnString *trace = hymn_new_string("");

    for (int i = this->frame_count - 1; i >= 0; i--) {
        HymnFrame *frame = &this->frames[i];
        HymnFunction *func = frame->func;
        int row = func->code.lines[frame->ip - func->code.instructions - 1];

        trace = hymn_string_append(trace, "at");

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

static HymnFrame *machine_push_error(Hymn *this, HymnString *error) {
    HymnObjectString *message = machine_intern_string(this, error);
    reference_string(message);
    push(this, hymn_new_string_value(message));
    return machine_exception(this);
}

static HymnFrame *machine_throw_existing_error(Hymn *this, char *error) {
    HymnString *message = hymn_new_string(error);
    free(error);
    return machine_push_error(this, message);
}

static HymnFrame *machine_throw_error(Hymn *this, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int len = vsnprintf(NULL, 0, format, ap);
    va_end(ap);
    char *chars = hymn_malloc((len + 1) * sizeof(char));
    va_start(ap, format);
    len = vsnprintf(chars, len + 1, format, ap);
    va_end(ap);
    HymnString *error = hymn_new_string(chars);
    free(chars);

    HymnString *trace = machine_stacktrace(this);
    error = hymn_string_append(error, "\n\n");
    error = hymn_string_append(error, trace);
    hymn_string_delete(trace);

    return machine_push_error(this, error);
}

static HymnFrame *machine_throw_error_string(Hymn *this, HymnString *string) {
    HymnFrame *frame = machine_throw_error(this, string);
    hymn_string_delete(string);
    return frame;
}

static bool machine_equal(HymnValue a, HymnValue b) {
    switch (a.is) {
    case HYMN_VALUE_NONE: return hymn_is_none(b);
    case HYMN_VALUE_BOOL: return hymn_is_bool(b) && hymn_as_bool(a) == hymn_as_bool(b);
    case HYMN_VALUE_INTEGER:
        switch (b.is) {
        case HYMN_VALUE_INTEGER: return hymn_as_int(a) == hymn_as_int(b);
        case HYMN_VALUE_FLOAT: return (double)hymn_as_int(a) == hymn_as_float(b);
        default: return false;
        }
    case HYMN_VALUE_FLOAT:
        switch (b.is) {
        case HYMN_VALUE_INTEGER: return hymn_as_float(a) == (double)hymn_as_int(b);
        case HYMN_VALUE_FLOAT: return hymn_as_float(a) == hymn_as_float(b);
        default: return false;
        }
    case HYMN_VALUE_STRING:
    case HYMN_VALUE_ARRAY:
    case HYMN_VALUE_TABLE:
    case HYMN_VALUE_FUNC:
        return b.is == a.is && hymn_as_object(a) == hymn_as_object(b);
    case HYMN_VALUE_FUNC_NATIVE:
        return hymn_is_native(b) && hymn_as_native(a) == hymn_as_native(b);
    default: return false;
    }
}

static bool machine_false(HymnValue value) {
    switch (value.is) {
    case HYMN_VALUE_NONE: return true;
    case HYMN_VALUE_BOOL: return !hymn_as_bool(value);
    case HYMN_VALUE_INTEGER: return hymn_as_int(value) == 0;
    case HYMN_VALUE_FLOAT: return hymn_as_float(value) == 0.0;
    case HYMN_VALUE_STRING: return hymn_string_len(hymn_as_string(value)) == 0;
    case HYMN_VALUE_ARRAY: return hymn_as_array(value)->length == 0;
    case HYMN_VALUE_TABLE: return hymn_as_table(value)->size == 0;
    case HYMN_VALUE_FUNC: return hymn_as_func(value) == NULL;
    case HYMN_VALUE_FUNC_NATIVE: return hymn_as_native(value) == NULL;
    default: return false;
    }
}

static HymnFrame *call(Hymn *this, HymnFunction *func, int count) {
    if (count != func->arity) {
        return machine_throw_error(this, "Expected %d function arguments but found %d.", func->arity, count);
    } else if (this->frame_count == HYMN_FRAMES_MAX) {
        return machine_throw_error(this, "Stack overflow.");
    }

    HymnFrame *frame = &this->frames[this->frame_count++];
    frame->func = func;
    frame->ip = func->code.instructions;
    frame->stack = this->stack_top - count - 1;

    return frame;
}

static HymnFrame *call_value(Hymn *this, HymnValue value, int count) {
    switch (value.is) {
    case HYMN_VALUE_FUNC:
        return call(this, hymn_as_func(value), count);
    case HYMN_VALUE_FUNC_NATIVE: {
        HymnNativeCall func = hymn_as_native(value)->func;
        HymnValue result = func(this, count, this->stack_top - count);
        reference(result);
        HymnValue *top = this->stack_top - (count + 1);
        while (this->stack_top != top) {
            dereference(this, pop(this));
        }
        push(this, result);
        return current_frame(this);
    }
    default: {
        const char *is = value_name(value.is);
        return machine_throw_error(this, "Call: Requires `Function`, but was `%s`.", is);
    }
    }
}

static HymnFrame *machine_import(Hymn *this, HymnObjectString *file) {
    HymnTable *imports = this->imports;

    HymnString *script = NULL;
    int p = 1;
    while (true) {
        HymnFrame *frame = parent_frame(this, p);
        if (frame == NULL) break;
        script = frame->func->script;
        if (script) break;
        p++;
    }
    HymnString *parent = script ? path_parent(script) : NULL;

    HymnObjectString *module = NULL;

    HymnArray *paths = this->paths;
    int64_t size = paths->length;
    for (int64_t i = 0; i < size; i++) {
        HymnValue value = paths->items[i];
        if (!hymn_is_string(value)) {
            continue;
        }
        HymnString *question = hymn_as_string(value);

        HymnString *replace = string_replace(question, "<path>", file->string);
        HymnString *path = parent ? string_replace(replace, "<parent>", parent) : string_copy(replace);

        HymnObjectString *use = machine_intern_string(this, path_absolute(path));
        reference_string(use);

        hymn_string_delete(path);
        hymn_string_delete(replace);

        if (!hymn_is_undefined(table_get(imports, use))) {
            dereference_string(this, use);
            if (parent) hymn_string_delete(parent);
            return current_frame(this);
        }

        if (hymn_file_exists(use->string)) {
            module = use;
            break;
        }

        dereference_string(this, use);
    }

    if (module == NULL) {
        HymnString *missing = hymn_string_format("Import not found: %s\n", file->string);

        for (int64_t i = 0; i < size; i++) {
            HymnValue value = paths->items[i];
            if (!hymn_is_string(value)) {
                continue;
            }
            HymnString *question = hymn_as_string(value);

            HymnString *replace = string_replace(question, "<path>", file->string);
            HymnString *path = parent ? string_replace(replace, "<parent>", parent) : string_copy(replace);
            HymnString *use = path_absolute(path);

            missing = string_append_format(missing, "\nno file %s", use);

            hymn_string_delete(path);
            hymn_string_delete(replace);
            hymn_string_delete(use);
        }

        if (parent) {
            hymn_string_delete(parent);
        }

        return machine_throw_error_string(this, missing);
    }

    if (parent) {
        hymn_string_delete(parent);
    }

    table_put(imports, module, hymn_new_bool(true));

    HymnString *source = hymn_read_file(module->string);

    struct CompileReturn result = compile(this, module->string, source);

    HymnFunction *func = result.func;
    char *error = result.error;

    hymn_string_delete(source);

    if (error) {
        function_delete(func);
        return machine_throw_existing_error(this, error);
    }

    HymnValue function = hymn_new_func_value(func);
    reference(function);

    push(this, function);
    call(this, func, 0);

    error = machine_interpret(this);
    if (error) {
        return machine_throw_existing_error(this, error);
    }

    return current_frame(this);
}

static size_t debug_constant_instruction(HymnString **debug, const char *name, HymnByteCode *this, size_t index) {
    uint8_t constant = this->instructions[index + 1];
    *debug = string_append_format(*debug, "%s: [", name);
    HymnString *value = debug_value_to_string(this->constants.values[constant]);
    *debug = hymn_string_append(*debug, value);
    hymn_string_delete(value);
    *debug = hymn_string_append(*debug, "]");
    return index + 2;
}

static size_t debug_byte_instruction(HymnString **debug, const char *name, HymnByteCode *this, size_t index) {
    uint8_t b = this->instructions[index + 1];
    *debug = string_append_format(*debug, "%s: [%d]", name, b);
    return index + 2;
}

static size_t debug_jump_instruction(HymnString **debug, const char *name, int sign, HymnByteCode *this, size_t index) {
    uint16_t jump = (uint16_t)(this->instructions[index + 1] << 8) | (uint16_t)this->instructions[index + 2];
    *debug = string_append_format(*debug, "%s: [%zu] -> [%zu]", name, index, index + 3 + sign * jump);
    return index + 3;
}

static size_t debug_three_byte_instruction(HymnString **debug, const char *name, HymnByteCode *this, size_t index) {
    uint8_t b = this->instructions[index + 1];
    uint8_t n = this->instructions[index + 2];
    *debug = string_append_format(*debug, "%s: [%d] [%d]", name, b, n);
    return index + 3;
}

static size_t debug_instruction(HymnString **debug, const char *name, size_t index) {
    *debug = string_append_format(*debug, "%s", name);
    return index + 1;
}

static size_t disassemble_instruction(HymnString **debug, HymnByteCode *this, size_t index) {
    *debug = string_append_format(*debug, "%04zu ", index);
    if (index > 0 && this->lines[index] == this->lines[index - 1]) {
        *debug = hymn_string_append(*debug, "   | ");
    } else {
        *debug = string_append_format(*debug, "%4d ", this->lines[index]);
    }
    uint8_t instruction = this->instructions[index];
    switch (instruction) {
    case OP_ADD: return debug_instruction(debug, "OP_ADD", index);
    case OP_ADD_TWO_LOCAL: return debug_three_byte_instruction(debug, "OP_ADD_TWO_LOCAL", this, index);
    case OP_INCREMENT: return debug_byte_instruction(debug, "OP_INCREMENT", this, index);
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
    case OP_TAIL_CALL: return debug_byte_instruction(debug, "OP_TAIL_CALL", this, index);
    case OP_CLEAR: return debug_instruction(debug, "OP_CLEAR", index);
    case OP_CONSTANT: return debug_constant_instruction(debug, "OP_CONSTANT", this, index);
    case OP_COPY: return debug_instruction(debug, "OP_COPY", index);
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
    case OP_JUMP_IF_EQUAL: return debug_jump_instruction(debug, "OP_JUMP_IF_EQUAL", 1, this, index);
    case OP_JUMP_IF_NOT_EQUAL: return debug_jump_instruction(debug, "OP_JUMP_IF_NOT_EQUAL", 1, this, index);
    case OP_JUMP_IF_LESS: return debug_jump_instruction(debug, "OP_JUMP_IF_LESS", 1, this, index);
    case OP_JUMP_IF_GREATER: return debug_jump_instruction(debug, "OP_JUMP_IF_GREATER", 1, this, index);
    case OP_JUMP_IF_LESS_EQUAL: return debug_jump_instruction(debug, "OP_JUMP_IF_LESS_EQUAL", 1, this, index);
    case OP_JUMP_IF_GREATER_EQUAL: return debug_jump_instruction(debug, "OP_JUMP_IF_GREATER_EQUAL", 1, this, index);
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
    case OP_POP_TWO: return debug_instruction(debug, "OP_POP_TWO", index);
    case OP_POP_N: return debug_byte_instruction(debug, "OP_POP_N", this, index);
    case OP_PRINT: return debug_instruction(debug, "OP_PRINT", index);
    case OP_THROW: return debug_instruction(debug, "OP_THROW", index);
    case OP_SET_DYNAMIC: return debug_instruction(debug, "OP_SET_DYNAMIC", index);
    case OP_SET_GLOBAL: return debug_constant_instruction(debug, "OP_SET_GLOBAL", this, index);
    case OP_SET_LOCAL: return debug_byte_instruction(debug, "OP_SET_LOCAL", this, index);
    case OP_SET_PROPERTY: return debug_constant_instruction(debug, "OP_SET_PROPERTY", this, index);
    case OP_INCREMENT_LOCAL_AND_SET: return debug_three_byte_instruction(debug, "OP_INCREMENT_LOCAL_AND_SET", this, index);
    case OP_INCREMENT_LOCAL: return debug_three_byte_instruction(debug, "OP_INCREMENT_LOCAL", this, index);
    case OP_SLICE: return debug_instruction(debug, "OP_SLICE", index);
    case OP_SUBTRACT: return debug_instruction(debug, "OP_SUBTRACT", index);
    case OP_TO_FLOAT: return debug_instruction(debug, "OP_TO_FLOAT", index);
    case OP_TO_INTEGER: return debug_instruction(debug, "OP_TO_INTEGER", index);
    case OP_TO_STRING: return debug_instruction(debug, "OP_TO_STRING", index);
    case OP_TRUE: return debug_instruction(debug, "OP_TRUE", index);
    case OP_TYPE: return debug_instruction(debug, "OP_TYPE", index);
    case OP_USE: return debug_instruction(debug, "OP_USE", index);
    case OP_RETURN: return debug_instruction(debug, "OP_RETURN", index);
    default: *debug = string_append_format(*debug, "UNKNOWN OPCODE %d\n", instruction); return index + 1;
    }
}

void disassemble_byte_code(HymnByteCode *this, const char *name) {
    printf("\n-- %s --\n", name);
    HymnString *debug = hymn_new_string("");
    size_t offset = 0;
    while (offset < (size_t)this->count) {
        offset = disassemble_instruction(&debug, this, offset);
        printf("%s\n", debug);
        hymn_string_zero(debug);
    }
    hymn_string_delete(debug);
}

#define READ_BYTE(frame) (*frame->ip++)

#define READ_SHORT(frame) (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))

#define READ_CONSTANT(frame) (frame->func->code.constants.values[READ_BYTE(frame)])

#define THROW(...)                                    \
    frame = machine_throw_error(this, ##__VA_ARGS__); \
    if (frame == NULL) return;                        \
    break;

#define ARITHMETIC_OP(_arithmetic_)                                                    \
    HymnValue b = pop(this);                                                           \
    HymnValue a = pop(this);                                                           \
    if (hymn_is_int(a)) {                                                              \
        if (hymn_is_int(b)) {                                                          \
            a.as.i _arithmetic_ b.as.i;                                                \
            push(this, a);                                                             \
        } else if (hymn_is_float(b)) {                                                 \
            HymnValue new = hymn_new_float((double)a.as.i);                            \
            new.as.f _arithmetic_ b.as.f;                                              \
            push(this, new);                                                           \
        } else {                                                                       \
            dereference(this, a);                                                      \
            dereference(this, b);                                                      \
            THROW("Operation Error: 2nd value must be `Integer` or `Float`.")          \
        }                                                                              \
    } else if (hymn_is_float(a)) {                                                     \
        if (hymn_is_int(b)) {                                                          \
            a.as.f _arithmetic_(double) b.as.i;                                        \
            push(this, a);                                                             \
        } else if (hymn_is_float(b)) {                                                 \
            a.as.f _arithmetic_ b.as.f;                                                \
            push(this, a);                                                             \
        } else {                                                                       \
            dereference(this, a);                                                      \
            dereference(this, b);                                                      \
            THROW("Operation Error: 1st and 2nd values must be `Integer` or `Float`.") \
        }                                                                              \
    } else {                                                                           \
        dereference(this, a);                                                          \
        dereference(this, b);                                                          \
        THROW("Operation Error: 1st and 2nd values must be `Integer` or `Float`.")     \
    }

#define COMPARE_OP(compare)                                                             \
    HymnValue b = pop(this);                                                            \
    HymnValue a = pop(this);                                                            \
    if (hymn_is_int(a)) {                                                               \
        if (hymn_is_int(b)) {                                                           \
            push(this, hymn_new_bool(hymn_as_int(a) compare hymn_as_int(b)));           \
        } else if (hymn_is_float(b)) {                                                  \
            push(this, hymn_new_bool((double)hymn_as_int(a) compare hymn_as_float(b))); \
        } else {                                                                        \
            dereference(this, a);                                                       \
            dereference(this, b);                                                       \
            THROW("Operands must be numbers.")                                          \
        }                                                                               \
    } else if (hymn_is_float(a)) {                                                      \
        if (hymn_is_int(b)) {                                                           \
            push(this, hymn_new_bool(hymn_as_float(a) compare(double) hymn_as_int(b))); \
        } else if (hymn_is_float(b)) {                                                  \
            push(this, hymn_new_bool(hymn_as_float(a) compare hymn_as_float(b)));       \
        } else {                                                                        \
            dereference(this, a);                                                       \
            dereference(this, b);                                                       \
            THROW("Operands must be numbers.")                                          \
        }                                                                               \
    } else {                                                                            \
        dereference(this, a);                                                           \
        dereference(this, b);                                                           \
        THROW("Operands must be numbers.")                                              \
    }

#define JUMP_COMPARE_OP(compare)                                      \
    HymnValue b = pop(this);                                          \
    HymnValue a = pop(this);                                          \
    bool answer;                                                      \
    if (hymn_is_int(a)) {                                             \
        if (hymn_is_int(b)) {                                         \
            answer = hymn_as_int(a) compare hymn_as_int(b);           \
        } else if (hymn_is_float(b)) {                                \
            answer = (double)hymn_as_int(a) compare hymn_as_float(b); \
        } else {                                                      \
            dereference(this, a);                                     \
            dereference(this, b);                                     \
            THROW("Comparison: Operands must be numbers.")            \
        }                                                             \
    } else if (hymn_is_float(a)) {                                    \
        if (hymn_is_int(b)) {                                         \
            answer = hymn_as_float(a) compare(double) hymn_as_int(b); \
        } else if (hymn_is_float(b)) {                                \
            answer = hymn_as_float(a) compare hymn_as_float(b);       \
        } else {                                                      \
            dereference(this, a);                                     \
            dereference(this, b);                                     \
            THROW("Comparison: Operands must be numbers.")            \
        }                                                             \
    } else {                                                          \
        dereference(this, a);                                         \
        dereference(this, b);                                         \
        THROW("Comparison: Operands must be numbers.")                \
    }                                                                 \
    uint16_t jump = READ_SHORT(frame);                                \
    if (answer) {                                                     \
        frame->ip += jump;                                            \
    }

static void machine_run(Hymn *this) {
    HymnFrame *frame = current_frame(this);

    while (true) {
#ifdef HYMN_DEBUG_TRACE
        {
            HymnString *debug = hymn_new_string("");
            disassemble_instruction(&debug, &frame->func->code, (int)(frame->ip - frame->func->code.instructions));
            printf("%s\n", debug);
            hymn_string_delete(debug);
        }
#endif
#ifdef HYMN_DEBUG_STACK
        if (this->stack_top != this->stack) {
            HymnString *debug = hymn_new_string("");
            for (HymnValue *i = this->stack; i != this->stack_top; i++) {
                debug = hymn_string_append_char(debug, '[');
                HymnString *stack_debug = debug_value_to_string(*i);
                debug = hymn_string_append(debug, stack_debug);
                hymn_string_delete(stack_debug);
                debug = hymn_string_append(debug, "] ");
            }
            printf("STACK   | %s\n", debug);
            hymn_string_delete(debug);
        }
#endif
        switch (READ_BYTE(frame)) {
        case OP_RETURN: {
            HymnValue result = pop(this);
            this->frame_count--;
            if (this->frame_count == 0 || frame->func->name == NULL) {
                dereference(this, pop(this));
                return;
            }
            while (this->stack_top != frame->stack) {
                dereference(this, pop(this));
            }
            push(this, result);
            frame = current_frame(this);
            break;
        }
        case OP_POP:
            dereference(this, pop(this));
            break;
        case OP_POP_TWO:
            dereference(this, pop(this));
            dereference(this, pop(this));
            break;
        case OP_POP_N: {
            int count = READ_BYTE(frame);
            while (count--) {
                dereference(this, pop(this));
            }
            break;
        }
        case OP_TRUE:
            push(this, hymn_new_bool(true));
            break;
        case OP_FALSE:
            push(this, hymn_new_bool(false));
            break;
        case OP_NONE:
            push(this, hymn_new_none());
            break;
        case OP_CALL: {
            int count = READ_BYTE(frame);
            HymnValue value = peek(this, count + 1);
            frame = call_value(this, value, count);
            if (frame == NULL) return;
            break;
        }
        case OP_TAIL_CALL: {
            // TODO
            int count = READ_BYTE(frame);
            HymnValue value = peek(this, count + 1);
            frame = call_value(this, value, count);
            if (frame == NULL) return;
            break;
        }
        case OP_JUMP: {
            uint16_t jump = READ_SHORT(frame);
            frame->ip += jump;
            break;
        }
        case OP_JUMP_IF_FALSE: {
            HymnValue a = pop(this);
            uint16_t jump = READ_SHORT(frame);
            if (machine_false(a)) {
                frame->ip += jump;
            }
            dereference(this, a);
            break;
        }
        case OP_JUMP_IF_TRUE: {
            HymnValue a = pop(this);
            uint16_t jump = READ_SHORT(frame);
            if (!machine_false(a)) {
                frame->ip += jump;
            }
            dereference(this, a);
            break;
        }
        case OP_JUMP_IF_EQUAL: {
            HymnValue b = pop(this);
            HymnValue a = pop(this);
            uint16_t jump = READ_SHORT(frame);
            if (machine_equal(a, b)) {
                frame->ip += jump;
            }
            dereference(this, a);
            dereference(this, b);
            break;
        }
        case OP_JUMP_IF_NOT_EQUAL: {
            HymnValue b = pop(this);
            HymnValue a = pop(this);
            uint16_t jump = READ_SHORT(frame);
            if (!machine_equal(a, b)) {
                frame->ip += jump;
            }
            dereference(this, a);
            dereference(this, b);
            break;
        }
        case OP_JUMP_IF_LESS: {
            JUMP_COMPARE_OP(<);
            break;
        }
        case OP_JUMP_IF_LESS_EQUAL: {
            JUMP_COMPARE_OP(<=);
            break;
        }
        case OP_JUMP_IF_GREATER: {
            JUMP_COMPARE_OP(>);
            break;
        }
        case OP_JUMP_IF_GREATER_EQUAL: {
            JUMP_COMPARE_OP(>=);
            break;
        }
        case OP_LOOP: {
            uint16_t jump = READ_SHORT(frame);
            frame->ip -= jump;
            break;
        }
        case OP_EQUAL: {
            HymnValue b = pop(this);
            HymnValue a = pop(this);
            push(this, hymn_new_bool(machine_equal(a, b)));
            dereference(this, a);
            dereference(this, b);
            break;
        }
        case OP_NOT_EQUAL: {
            HymnValue b = pop(this);
            HymnValue a = pop(this);
            push(this, hymn_new_bool(!machine_equal(a, b)));
            dereference(this, a);
            dereference(this, b);
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
            HymnValue b = pop(this);
            HymnValue a = pop(this);
            if (hymn_is_none(a)) {
                if (hymn_is_string(b)) {
                    machine_push_intern_string(this, value_concat(a, b));
                } else {
                    dereference(this, a);
                    dereference(this, b);
                    THROW("Add: 1st and 2nd values can't be added.")
                }
            } else if (hymn_is_bool(a)) {
                if (hymn_is_string(b)) {
                    machine_push_intern_string(this, value_concat(a, b));
                } else {
                    dereference(this, a);
                    dereference(this, b);
                    THROW("Add: 1st and 2nd values can't be added.")
                }
            } else if (hymn_is_int(a)) {
                if (hymn_is_int(b)) {
                    a.as.i += b.as.i;
                    push(this, a);
                } else if (hymn_is_float(b)) {
                    b.as.f += a.as.i;
                    push(this, a);
                } else if (hymn_is_string(b)) {
                    machine_push_intern_string(this, value_concat(a, b));
                } else {
                    dereference(this, a);
                    dereference(this, b);
                    THROW("Add: 1st and 2nd values can't be added.")
                }
            } else if (hymn_is_float(a)) {
                if (hymn_is_int(b)) {
                    a.as.f += b.as.i;
                    push(this, a);
                } else if (hymn_is_float(b)) {
                    a.as.f += b.as.f;
                    push(this, a);
                } else if (hymn_is_string(b)) {
                    machine_push_intern_string(this, value_concat(a, b));
                } else {
                    dereference(this, a);
                    dereference(this, b);
                    THROW("Add: 1st and 2nd values can't be added.")
                }
            } else if (hymn_is_string(a)) {
                machine_push_intern_string(this, value_concat(a, b));
            } else {
                THROW("Add: 1st and 2nd values can't be added.")
            }
            dereference(this, a);
            dereference(this, b);
            break;
        }
        case OP_ADD_TWO_LOCAL: {
            uint8_t slot_a = READ_BYTE(frame);
            uint8_t slot_b = READ_BYTE(frame);
            HymnValue b = frame->stack[slot_a];
            HymnValue a = frame->stack[slot_b];
            if (hymn_is_none(a)) {
                if (hymn_is_string(b)) {
                    machine_push_intern_string(this, value_concat(a, b));
                } else {
                    dereference(this, a);
                    dereference(this, b);
                    THROW("Add: 1st and 2nd values can't be added.")
                }
            } else if (hymn_is_bool(a)) {
                if (hymn_is_string(b)) {
                    machine_push_intern_string(this, value_concat(a, b));
                } else {
                    dereference(this, a);
                    dereference(this, b);
                    THROW("Add: 1st and 2nd values can't be added.")
                }
            } else if (hymn_is_int(a)) {
                if (hymn_is_int(b)) {
                    a.as.i += b.as.i;
                    push(this, a);
                } else if (hymn_is_float(b)) {
                    b.as.f += a.as.i;
                    push(this, a);
                } else if (hymn_is_string(b)) {
                    machine_push_intern_string(this, value_concat(a, b));
                } else {
                    dereference(this, a);
                    dereference(this, b);
                    THROW("Add: 1st and 2nd values can't be added.")
                }
            } else if (hymn_is_float(a)) {
                if (hymn_is_int(b)) {
                    a.as.f += b.as.i;
                    push(this, a);
                } else if (hymn_is_float(b)) {
                    a.as.f += b.as.f;
                    push(this, a);
                } else if (hymn_is_string(b)) {
                    machine_push_intern_string(this, value_concat(a, b));
                } else {
                    dereference(this, a);
                    dereference(this, b);
                    THROW("Add: 1st and 2nd values can't be added.")
                }
            } else if (hymn_is_string(a)) {
                machine_push_intern_string(this, value_concat(a, b));
            } else {
                THROW("Add: 1st and 2nd values can't be added.")
            }
            break;
        }
        case OP_INCREMENT: {
            HymnValue a = pop(this);
            uint8_t increment = READ_BYTE(frame);
            if (hymn_is_none(a)) {
                dereference(this, a);
                THROW("Increment: 1st and 2nd values can't be added.")
            } else if (hymn_is_bool(a)) {
                dereference(this, a);
                THROW("Increment: 1st and 2nd values can't be added.")
            } else if (hymn_is_int(a)) {
                a.as.i += (int64_t)increment;
                push(this, a);
            } else if (hymn_is_float(a)) {
                a.as.f += (double)increment;
                push(this, a);
            } else if (hymn_is_string(a)) {
                machine_push_intern_string(this, value_concat(a, hymn_new_int((int64_t)increment)));
            } else {
                THROW("Increment: 1st and 2nd values can't be added.")
            }
            dereference(this, a);
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
            HymnValue b = pop(this);
            HymnValue a = pop(this);
            if (hymn_is_int(a)) {
                if (hymn_is_int(b)) {
                    a.as.i %= b.as.i;
                    push(this, a);
                } else {
                    dereference(this, a);
                    dereference(this, b);
                    THROW("Modulo Error: 2nd value must be `Integer`.")
                }
            } else {
                dereference(this, a);
                dereference(this, b);
                THROW("Modulo Error: 1st and 2nd values must be `Integer`.")
            }
            break;
        }
        case OP_BIT_NOT: {
            HymnValue value = pop(this);
            if (hymn_is_int(value)) {
                value.as.i = ~value.as.i;
                push(this, value);
            } else {
                dereference(this, value);
                THROW("Bitwise operand must integer.")
            }
            break;
        }
        case OP_BIT_OR: {
            HymnValue b = pop(this);
            HymnValue a = pop(this);
            if (hymn_is_int(a)) {
                if (hymn_is_int(b)) {
                    a.as.i |= b.as.i;
                    push(this, a);
                } else {
                    dereference(this, a);
                    dereference(this, b);
                    THROW("Bit Or Error: 2nd value must be `Integer`.")
                }
            } else {
                dereference(this, a);
                dereference(this, b);
                THROW("Bit Or Error: 1st and 2nd values must be `Integer`.")
            }
            break;
        }
        case OP_BIT_AND: {
            HymnValue b = pop(this);
            HymnValue a = pop(this);
            if (hymn_is_int(a)) {
                if (hymn_is_int(b)) {
                    a.as.i &= b.as.i;
                    push(this, a);
                } else {
                    dereference(this, a);
                    dereference(this, b);
                    THROW("Bit And Error: 2nd value must be `Integer`.")
                }
            } else {
                dereference(this, a);
                dereference(this, b);
                THROW("Bit And Error: 1st and 2nd values must be `Integer`.")
            }
            break;
        }
        case OP_BIT_XOR: {
            HymnValue b = pop(this);
            HymnValue a = pop(this);
            if (hymn_is_int(a)) {
                if (hymn_is_int(b)) {
                    a.as.i ^= b.as.i;
                    push(this, a);
                } else {
                    dereference(this, a);
                    dereference(this, b);
                    THROW("Bit Xor Error: 2nd value must be `Integer`.")
                }
            } else {
                dereference(this, a);
                dereference(this, b);
                THROW("Bit Xor Error: 1st and 2nd values must be `Integer`.")
            }
            break;
        }
        case OP_BIT_LEFT_SHIFT: {
            HymnValue b = pop(this);
            HymnValue a = pop(this);
            if (hymn_is_int(a)) {
                if (hymn_is_int(b)) {
                    a.as.i <<= b.as.i;
                    push(this, a);
                } else {
                    dereference(this, a);
                    dereference(this, b);
                    THROW("Bit Left Shift Error: 2nd value must be `Integer`.")
                }
            } else {
                dereference(this, a);
                dereference(this, b);
                THROW("Bit Left Shift Error: 1st and 2nd values must be `Integer`.")
            }
            break;
        }
        case OP_BIT_RIGHT_SHIFT: {
            HymnValue b = pop(this);
            HymnValue a = pop(this);
            if (hymn_is_int(a)) {
                if (hymn_is_int(b)) {
                    a.as.i >>= b.as.i;
                    push(this, a);
                } else {
                    dereference(this, a);
                    dereference(this, b);
                    THROW("Bit Right Shift Error: 2nd value must be `Integer`.")
                }
            } else {
                dereference(this, a);
                dereference(this, b);
                THROW("Bit Right Shift Error: 1st and 2nd values must be `Integer`.")
            }
            break;
        }
        case OP_NEGATE: {
            HymnValue value = pop(this);
            if (hymn_is_int(value)) {
                value.as.i = -value.as.i;
            } else if (hymn_is_float(value)) {
                value.as.f = -value.as.f;
            } else {
                dereference(this, value);
                THROW("Negate: Must be a number.")
            }
            push(this, value);
            break;
        }
        case OP_NOT: {
            HymnValue value = pop(this);
            if (hymn_is_bool(value)) {
                value.as.b = !value.as.b;
            } else {
                dereference(this, value);
                THROW("Not: Operand must be a boolean.")
            }
            push(this, value);
            break;
        }
        case OP_CONSTANT: {
            HymnValue constant = READ_CONSTANT(frame);
            switch (constant.is) {
            case HYMN_VALUE_ARRAY: {
                constant = hymn_new_array_value(new_array(0));
                break;
            }
            case HYMN_VALUE_TABLE: {
                constant = hymn_new_table_value(new_table());
                break;
            }
            default:
                break;
            }
            reference(constant);
            push(this, constant);
            break;
        }
        case OP_DEFINE_GLOBAL: {
            HymnObjectString *name = hymn_as_hymn_string(READ_CONSTANT(frame));
            HymnValue value = pop(this);
            table_put(&this->globals, name, value);
            reference_string(name);
            break;
        }
        case OP_SET_GLOBAL: {
            HymnObjectString *name = hymn_as_hymn_string(READ_CONSTANT(frame));
            HymnValue value = peek(this, 1);
            HymnValue exists = table_get(&this->globals, name);
            if (hymn_is_undefined(exists)) {
                THROW("Undefined variable '%s'.", name->string)
            }
            HymnValue previous = table_put(&this->globals, name, value);
            if (hymn_is_undefined(previous)) {
                reference_string(name);
            } else {
                dereference(this, previous);
            }
            reference(value);
            break;
        }
        case OP_GET_GLOBAL: {
            HymnObjectString *name = hymn_as_hymn_string(READ_CONSTANT(frame));
            HymnValue get = table_get(&this->globals, name);
            if (hymn_is_undefined(get)) {
                THROW("Undefined variable `%s`.", name->string)
            }
            reference(get);
            push(this, get);
            break;
        }
        case OP_SET_LOCAL: {
            uint8_t slot = READ_BYTE(frame);
            HymnValue value = pop(this);
            dereference(this, frame->stack[slot]);
            frame->stack[slot] = value;
            break;
        }
        case OP_GET_LOCAL: {
            uint8_t slot = READ_BYTE(frame);
            HymnValue value = frame->stack[slot];
            reference(value);
            push(this, value);
            break;
        }
        case OP_INCREMENT_LOCAL: {
            uint8_t slot = READ_BYTE(frame);
            uint8_t increment = READ_BYTE(frame);
            HymnValue value = frame->stack[slot];
            if (hymn_is_int(value)) {
                value.as.i += (uint64_t)increment;
            } else if (hymn_is_float(value)) {
                value.as.f += (double)increment;
            } else {
                const char *is = value_name(value.is);
                THROW("Increment Local: Expected `Number` but was `%s`", is)
            }
            push(this, value);
            break;
        }
        case OP_INCREMENT_LOCAL_AND_SET: {
            uint8_t slot = READ_BYTE(frame);
            uint8_t increment = READ_BYTE(frame);
            HymnValue value = frame->stack[slot];
            if (hymn_is_int(value)) {
                value.as.i += (uint64_t)increment;
            } else if (hymn_is_float(value)) {
                value.as.f += (double)increment;
            } else {
                const char *is = value_name(value.is);
                THROW("Get and Set Local: Expected `Number` but was `%s`", is)
            }
            frame->stack[slot] = value;
            break;
        }
        case OP_SET_PROPERTY: {
            HymnValue value = pop(this);
            HymnValue tableValue = pop(this);
            if (!hymn_is_table(tableValue)) {
                const char *is = value_name(tableValue.is);
                dereference(this, value);
                dereference(this, tableValue);
                THROW("Set Property: Expected `Table` but was `%s`", is)
            }
            HymnTable *table = hymn_as_table(tableValue);
            HymnObjectString *name = hymn_as_hymn_string(READ_CONSTANT(frame));
            HymnValue previous = table_put(table, name, value);
            if (hymn_is_undefined(previous)) {
                reference_string(name);
            } else {
                dereference(this, previous);
            }
            push(this, value);
            reference(value);
            dereference(this, tableValue);
            break;
        }
        case OP_GET_PROPERTY: {
            HymnValue v = pop(this);
            if (!hymn_is_table(v)) {
                dereference(this, v);
                THROW("Only tables can get properties.")
            }
            HymnTable *table = hymn_as_table(v);
            HymnObjectString *name = hymn_as_hymn_string(READ_CONSTANT(frame));
            HymnValue g = table_get(table, name);
            if (hymn_is_undefined(g)) {
                g.is = HYMN_VALUE_NONE;
            } else {
                reference(g);
            }
            dereference(this, v);
            push(this, g);
            break;
        }
        case OP_SET_DYNAMIC: {
            HymnValue value = pop(this);
            HymnValue property = pop(this);
            HymnValue object = pop(this);
            if (hymn_is_array(object)) {
                if (!hymn_is_int(property)) {
                    dereference(this, value);
                    dereference(this, property);
                    dereference(this, object);
                    THROW("Dynamic Set: `Integer` required to set `Array` index.")
                }
                HymnArray *array = hymn_as_array(object);
                int64_t size = array->length;
                int64_t index = hymn_as_int(property);
                if (index > size) {
                    dereference(this, value);
                    dereference(this, property);
                    dereference(this, object);
                    THROW("Dynamic Set: Array index out of bounds %d > %d.", index, size)
                }
                if (index < 0) {
                    index = size + index;
                    if (index < 0) {
                        dereference(this, value);
                        dereference(this, property);
                        dereference(this, object);
                        THROW("Dynamic Set: Array index out of bounds %d.", index)
                    }
                }
                if (index == size) {
                    array_push(array, value);
                } else {
                    dereference(this, array->items[index]);
                    array->items[index] = value;
                }
            } else if (hymn_is_table(object)) {
                if (!hymn_is_string(property)) {
                    dereference(this, value);
                    dereference(this, property);
                    dereference(this, object);
                    THROW("Dynamic Set: `String` required to set `Table` property.")
                }
                HymnTable *table = hymn_as_table(object);
                HymnObjectString *name = hymn_as_hymn_string(property);
                table_put(table, name, value);
            } else {
                const char *is = value_name(object.is);
                dereference(this, value);
                dereference(this, property);
                dereference(this, object);
                THROW("Dynamic Set: 1st argument requires `Array` or `Table`, but was `%s`.", is)
            }
            push(this, value);
            dereference(this, object);
            reference(value);
            break;
        }
        case OP_GET_DYNAMIC: {
            HymnValue i = pop(this);
            HymnValue v = pop(this);
            switch (v.is) {
            case HYMN_VALUE_STRING: {
                if (!hymn_is_int(i)) {
                    dereference(this, i);
                    dereference(this, v);
                    THROW("Integer required to get string character from index.")
                }
                HymnString *string = hymn_as_string(v);
                int64_t size = (int64_t)hymn_string_len(string);
                int64_t index = hymn_as_int(i);
                if (index >= size) {
                    dereference(this, i);
                    dereference(this, v);
                    THROW("String index out of bounds %d >= %d.", index, size)
                }
                if (index < 0) {
                    index = size + index;
                    if (index < 0) {
                        dereference(this, i);
                        dereference(this, v);
                        THROW("String index out of bounds %d.", index)
                    }
                }
                char c = string[index];
                machine_push_intern_string(this, char_to_string(c));
                dereference(this, v);
                break;
            }
            case HYMN_VALUE_ARRAY: {
                if (!hymn_is_int(i)) {
                    dereference(this, i);
                    dereference(this, v);
                    THROW("Integer required to get array index.")
                }
                HymnArray *array = hymn_as_array(v);
                int64_t size = array->length;
                int64_t index = hymn_as_int(i);
                if (index >= size) {
                    dereference(this, i);
                    dereference(this, v);
                    THROW("Array index out of bounds %d >= %d.", index, size)
                }
                if (index < 0) {
                    index = size + index;
                    if (index < 0) {
                        dereference(this, i);
                        dereference(this, v);
                        THROW("Array index out of bounds %d.", index)
                    }
                }
                HymnValue g = array_get(array, index);
                reference(g);
                push(this, g);
                dereference(this, v);
                break;
            }
            case HYMN_VALUE_TABLE: {
                if (!hymn_is_string(i)) {
                    const char *is = value_name(i.is);
                    dereference(this, i);
                    dereference(this, v);
                    THROW("Dynamic Get: Expected 2nd argument to be `String`, but was `%s`.", is)
                }
                HymnTable *table = hymn_as_table(v);
                HymnObjectString *name = hymn_as_hymn_string(i);
                HymnValue g = table_get(table, name);
                if (hymn_is_undefined(g)) {
                    g.is = HYMN_VALUE_NONE;
                } else {
                    reference(g);
                }
                push(this, g);
                dereference(this, i);
                dereference(this, v);
                break;
            }
            default: {
                const char *is = value_name(v.is);
                dereference(this, i);
                dereference(this, v);
                THROW("Dynamic Get: 1st argument requires `Array` or `Table`, but was `%s`.", is)
            }
            }
            break;
        }
        case OP_LEN: {
            HymnValue value = pop(this);
            switch (value.is) {
            case HYMN_VALUE_STRING: {
                int64_t len = (int64_t)hymn_string_len(hymn_as_string(value));
                push(this, hymn_new_int(len));
                break;
            }
            case HYMN_VALUE_ARRAY: {
                int64_t len = hymn_as_array(value)->length;
                push(this, hymn_new_int(len));
                break;
            }
            case HYMN_VALUE_TABLE: {
                int64_t len = (int64_t)hymn_as_table(value)->size;
                push(this, hymn_new_int(len));
                break;
            }
            default:
                dereference(this, value);
                THROW("Len: Expected `Array` or `Table`.")
            }
            dereference(this, value);
            break;
        }
        case OP_ARRAY_POP: {
            HymnValue a = pop(this);
            if (!hymn_is_array(a)) {
                const char *is = value_name(a.is);
                dereference(this, a);
                THROW("Pop: Expected `Array` for 1st argument, but was `%s`.", is)
            } else {
                HymnValue value = array_pop(hymn_as_array(a));
                push(this, value);
                dereference(this, a);
            }
            break;
        }
        case OP_ARRAY_PUSH: {
            HymnValue value = pop(this);
            HymnValue array = pop(this);
            if (!hymn_is_array(array)) {
                const char *is = value_name(value.is);
                dereference(this, array);
                dereference(this, value);
                THROW("Push: Expected `Array` for 1st argument, but was `%s`.", is)
            } else {
                array_push(hymn_as_array(array), value);
                push(this, value);
                reference(value);
                dereference(this, array);
            }
            break;
        }
        case OP_ARRAY_INSERT: {
            HymnValue p = pop(this);
            HymnValue i = pop(this);
            HymnValue v = pop(this);
            if (hymn_is_array(v)) {
                if (!hymn_is_int(i)) {
                    const char *is = value_name(i.is);
                    dereference(this, p);
                    dereference(this, i);
                    dereference(this, v);
                    THROW("Insert Function: Expected `Integer` for 2nd argument, but was `%s`.", is)
                }
                HymnArray *array = hymn_as_array(v);
                int64_t size = array->length;
                int64_t index = hymn_as_int(i);
                if (index > size) {
                    dereference(this, p);
                    dereference(this, i);
                    dereference(this, v);
                    THROW("Insert Function: Array index out of bounds: %d > %d", index, size)
                }
                if (index < 0) {
                    index = size + index;
                    if (index < 0) {
                        dereference(this, p);
                        dereference(this, i);
                        dereference(this, v);
                        THROW("Insert Function: Array index less than zero: %d", index)
                    }
                }
                if (index == size) {
                    array_push(array, p);
                } else {
                    array_insert(array, index, p);
                }
                push(this, p);
                reference(p);
                dereference(this, v);
            } else {
                const char *is = value_name(v.is);
                dereference(this, p);
                dereference(this, i);
                dereference(this, v);
                THROW("Insert Function: Expected `Array` for 1st argument, but was `%s`.", is)
            }
            break;
        }
        case OP_DELETE: {
            HymnValue i = pop(this);
            HymnValue v = pop(this);
            if (hymn_is_array(v)) {
                if (!hymn_is_int(i)) {
                    dereference(this, i);
                    dereference(this, v);
                    THROW("Integer required to delete from array.")
                }
                HymnArray *array = hymn_as_array(v);
                int64_t size = array->length;
                int64_t index = hymn_as_int(i);
                if (index >= size) {
                    dereference(this, i);
                    dereference(this, v);
                    THROW("Array index out of bounds %d > %d.", index, size)
                }
                if (index < 0) {
                    index = size + index;
                    if (index < 0) {
                        dereference(this, i);
                        dereference(this, v);
                        THROW("Array index out of bounds %d.", index)
                    }
                }
                HymnValue value = array_remove_index(array, index);
                push(this, value);
                dereference(this, v);
            } else if (hymn_is_table(v)) {
                if (!hymn_is_string(i)) {
                    dereference(this, i);
                    dereference(this, v);
                    THROW("String required to delete from table.")
                }
                HymnTable *table = hymn_as_table(v);
                HymnObjectString *name = hymn_as_hymn_string(i);
                HymnValue value = table_remove(table, name);
                if (hymn_is_undefined(value)) {
                    value.is = HYMN_VALUE_NONE;
                } else {
                    dereference_string(this, name);
                }
                push(this, value);
                dereference_string(this, name);
                dereference(this, v);
            } else {
                dereference(this, i);
                dereference(this, v);
                THROW("Expected array or table for `delete` function.")
            }
            break;
        }
        case OP_COPY: {
            HymnValue value = pop(this);
            switch (value.is) {
            case HYMN_VALUE_NONE:
            case HYMN_VALUE_BOOL:
            case HYMN_VALUE_INTEGER:
            case HYMN_VALUE_FLOAT:
            case HYMN_VALUE_STRING:
            case HYMN_VALUE_FUNC:
            case HYMN_VALUE_FUNC_NATIVE:
                push(this, value);
                break;
            case HYMN_VALUE_ARRAY: {
                HymnArray *copy = new_array_copy(hymn_as_array(value));
                HymnValue new = hymn_new_array_value(copy);
                push(this, new);
                reference(new);
                dereference(this, value);
                break;
            }
            case HYMN_VALUE_TABLE: {
                HymnTable *copy = new_table_copy(hymn_as_table(value));
                HymnValue new = hymn_new_table_value(copy);
                push(this, new);
                reference(new);
                dereference(this, value);
                break;
            }
            default:
                push(this, hymn_new_none());
            }
            break;
        }
        case OP_SLICE: {
            HymnValue b = pop(this);
            HymnValue a = pop(this);
            HymnValue v = pop(this);
            if (!hymn_is_int(a)) {
                dereference(this, a);
                dereference(this, b);
                dereference(this, v);
                THROW("Integer required for slice expression.")
            }
            int64_t start = hymn_as_int(a);
            if (hymn_is_string(v)) {
                HymnString *original = hymn_as_string(v);
                int64_t size = (int64_t)hymn_string_len(original);
                int64_t end;
                if (hymn_is_int(b)) {
                    end = hymn_as_int(b);
                } else if (hymn_is_none(b)) {
                    end = size;
                } else {
                    dereference(this, a);
                    dereference(this, b);
                    dereference(this, v);
                    THROW("Integer required for slice expression.")
                }
                if (end > size) {
                    dereference(this, a);
                    dereference(this, b);
                    dereference(this, v);
                    THROW("String index out of bounds %d > %d.", end, size)
                }
                if (end < 0) {
                    end = size + end;
                    if (end < 0) {
                        dereference(this, a);
                        dereference(this, b);
                        dereference(this, v);
                        THROW("String index out of bounds %d.", end)
                    }
                }
                if (start >= end) {
                    dereference(this, a);
                    dereference(this, b);
                    dereference(this, v);
                    THROW("String start index %d > end index %d.", start, end)
                }
                HymnString *sub = hymn_substring(original, (size_t)start, (size_t)end);
                machine_push_intern_string(this, sub);
            } else if (hymn_is_array(v)) {
                HymnArray *array = hymn_as_array(v);
                int64_t size = array->length;
                int64_t end;
                if (hymn_is_int(b)) {
                    end = hymn_as_int(b);
                } else if (hymn_is_none(b)) {
                    end = size;
                } else {
                    dereference(this, a);
                    dereference(this, b);
                    dereference(this, v);
                    THROW("Integer required for slice expression.")
                }
                if (end > size) {
                    dereference(this, a);
                    dereference(this, b);
                    dereference(this, v);
                    THROW("Array index out of bounds %d > %d.", end, size)
                }
                if (end < 0) {
                    end = size + end;
                    if (end < 0) {
                        dereference(this, a);
                        dereference(this, b);
                        dereference(this, v);
                        THROW("Array index out of bounds %d.", end)
                    }
                }
                if (start >= end) {
                    dereference(this, a);
                    dereference(this, b);
                    dereference(this, v);
                    THROW("Array start index %d >= end index %d.", start, end)
                }
                HymnArray *copy = new_array_slice(array, start, end);
                HymnValue new = hymn_new_array_value(copy);
                reference(new);
                push(this, new);
            } else {
                dereference(this, a);
                dereference(this, b);
                dereference(this, v);
                THROW("Expected string or array for `slice` function.")
            }
            dereference(this, v);
            break;
        }
        case OP_CLEAR: {
            HymnValue value = pop(this);
            switch (value.is) {
            case HYMN_VALUE_BOOL:
                push(this, hymn_new_bool(false));
                break;
            case HYMN_VALUE_INTEGER:
                push(this, hymn_new_int(0));
                break;
            case HYMN_VALUE_FLOAT:
                push(this, hymn_new_float(0.0f));
                break;
            case HYMN_VALUE_STRING:
                machine_push_intern_string(this, hymn_new_string(""));
                break;
            case HYMN_VALUE_ARRAY: {
                HymnArray *array = hymn_as_array(value);
                array_clear(this, array);
                push(this, value);
                break;
            }
            case HYMN_VALUE_TABLE: {
                HymnTable *table = hymn_as_table(value);
                table_clear(this, table);
                push(this, value);
                break;
            }
            case HYMN_VALUE_UNDEFINED:
            case HYMN_VALUE_NONE:
            case HYMN_VALUE_FUNC:
            case HYMN_VALUE_FUNC_NATIVE:
            case HYMN_VALUE_POINTER:
                push(this, hymn_new_none());
                break;
            }
            break;
        }
        case OP_KEYS: {
            HymnValue value = pop(this);
            if (!hymn_is_table(value)) {
                dereference(this, value);
                THROW("Expected table for `keys` function.")
            } else {
                HymnTable *table = hymn_as_table(value);
                HymnArray *array = table_keys(table);
                HymnValue keys = hymn_new_array_value(array);
                reference(keys);
                push(this, keys);
                dereference(this, value);
            }
            break;
        }
        case OP_INDEX: {
            HymnValue b = pop(this);
            HymnValue a = pop(this);
            switch (a.is) {
            case HYMN_VALUE_STRING: {
                if (!hymn_is_string(b)) {
                    dereference(this, a);
                    dereference(this, b);
                    THROW("Expected substring for 2nd argument of `index` function.")
                }
                size_t index = 0;
                bool found = string_find(hymn_as_string(a), hymn_as_string(b), &index);
                if (found) {
                    push(this, hymn_new_int((int64_t)index));
                } else {
                    push(this, hymn_new_int(-1));
                }
                dereference(this, a);
                dereference(this, b);
                break;
            }
            case HYMN_VALUE_ARRAY:
                push(this, hymn_new_int(array_index_of(hymn_as_array(a), b)));
                dereference(this, a);
                dereference(this, b);
                break;
            case HYMN_VALUE_TABLE: {
                HymnObjectString *key = table_key_of(hymn_as_table(a), b);
                if (key == NULL) {
                    push(this, hymn_new_none());
                } else {
                    push(this, hymn_new_string_value(key));
                }
                dereference(this, a);
                dereference(this, b);
                break;
            }
            default:
                dereference(this, a);
                dereference(this, b);
                THROW("Expected string, array, or table for `index` function.")
            }
            break;
        }
        case OP_TYPE: {
            HymnValue value = pop(this);
            switch (value.is) {
            case HYMN_VALUE_UNDEFINED:
            case HYMN_VALUE_NONE:
                machine_push_intern_string(this, hymn_new_string(STRING_NONE_TYPE));
                break;
            case HYMN_VALUE_BOOL:
                machine_push_intern_string(this, hymn_new_string(STRING_BOOL));
                break;
            case HYMN_VALUE_INTEGER:
                machine_push_intern_string(this, hymn_new_string(STRING_INTEGER));
                break;
            case HYMN_VALUE_FLOAT:
                machine_push_intern_string(this, hymn_new_string(STRING_FLOAT));
                break;
            case HYMN_VALUE_STRING:
                machine_push_intern_string(this, hymn_new_string(STRING_STRING));
                dereference(this, value);
                break;
            case HYMN_VALUE_ARRAY:
                machine_push_intern_string(this, hymn_new_string(STRING_ARRAY));
                dereference(this, value);
                break;
            case HYMN_VALUE_TABLE:
                machine_push_intern_string(this, hymn_new_string(STRING_TABLE));
                dereference(this, value);
                break;
            case HYMN_VALUE_FUNC:
                machine_push_intern_string(this, hymn_new_string(STRING_FUNC));
                dereference(this, value);
                break;
            case HYMN_VALUE_FUNC_NATIVE:
                machine_push_intern_string(this, hymn_new_string(STRING_NATIVE));
                break;
            case HYMN_VALUE_POINTER:
                machine_push_intern_string(this, hymn_new_string(STRING_POINTER));
                break;
            }
            break;
        }
        case OP_TO_INTEGER: {
            HymnValue value = pop(this);
            if (hymn_is_int(value)) {
                push(this, value);
            } else if (hymn_is_float(value)) {
                int64_t number = (int64_t)hymn_as_float(value);
                push(this, hymn_new_int(number));
            } else if (hymn_is_string(value)) {
                HymnString *string = hymn_as_string(value);
                char *end = NULL;
                double number = string_to_double(string, &end);
                if (string == end) {
                    push(this, hymn_new_none());
                } else {
                    push(this, hymn_new_int((int64_t)number));
                }
                dereference(this, value);
            } else {
                dereference(this, value);
                THROW("Can't cast to an integer.")
            }
            break;
        }
        case OP_TO_FLOAT: {
            HymnValue value = pop(this);
            if (hymn_is_int(value)) {
                double number = (double)hymn_as_int(value);
                push(this, hymn_new_float(number));
            } else if (hymn_is_float(value)) {
                push(this, value);
            } else if (hymn_is_string(value)) {
                HymnString *string = hymn_as_string(value);
                char *end = NULL;
                double number = string_to_double(string, &end);
                if (string == end) {
                    push(this, hymn_new_none());
                } else {
                    push(this, hymn_new_float(number));
                }
                dereference(this, value);
            } else {
                dereference(this, value);
                THROW("Can't cast to a float.")
            }
            break;
        }
        case OP_TO_STRING: {
            HymnValue value = pop(this);
            machine_push_intern_string(this, value_to_string(value));
            dereference(this, value);
            break;
        }
        case OP_PRINT: {
            HymnValue value = pop(this);
            HymnString *string = value_to_string(value);
            this->print("%s\n", string);
            hymn_string_delete(string);
            dereference(this, value);
            break;
        }
        case OP_THROW: {
            frame = machine_exception(this);
            if (frame == NULL) return;
            break;
        }
        case OP_DUPLICATE: {
            HymnValue top = peek(this, 1);
            push(this, top);
            ;
            reference(top);
            break;
        }
        case OP_USE: {
            HymnValue file = pop(this);
            if (hymn_is_string(file)) {
                frame = machine_import(this, hymn_as_hymn_string(file));
                dereference(this, file);
                if (frame == NULL) return;
            } else {
                dereference(this, file);
                THROW("Expected string for 'use' command.")
            }
            break;
        }
        default:
            UNREACHABLE();
        }
    }
}

static char *machine_interpret(Hymn *this) {
    machine_run(this);
    char *error = NULL;
    if (this->error) {
        error = string_to_chars(this->error);
        hymn_string_delete(this->error);
        this->error = NULL;
    }
    return error;
}

// END VM

static void print_stdout(const char *format, ...) {
    va_list args;

    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

Hymn *new_hymn() {
    Hymn *this = hymn_calloc(1, sizeof(Hymn));
    reset_stack(this);

    set_init(&this->strings);
    table_init(&this->globals);

    HymnObjectString *search_this = machine_intern_string(this, hymn_new_string("<parent>" PATH_SEP_STRING "<path>.hm"));
    reference_string(search_this);

    HymnObjectString *search_relative = machine_intern_string(this, hymn_new_string("." PATH_SEP_STRING "<path>.hm"));
    reference_string(search_relative);

    HymnObjectString *search_modules = machine_intern_string(this, hymn_new_string("." PATH_SEP_STRING "modules" PATH_SEP_STRING "<path>.hm"));
    reference_string(search_modules);

    HymnObjectString *paths = machine_intern_string(this, hymn_new_string("__paths"));
    reference_string(paths);

    this->paths = new_array(3);
    this->paths->items[0] = hymn_new_string_value(search_this);
    this->paths->items[1] = hymn_new_string_value(search_relative);
    this->paths->items[2] = hymn_new_string_value(search_modules);

    HymnValue paths_value = hymn_new_array_value(this->paths);
    table_put(&this->globals, paths, paths_value);
    reference_string(paths);
    reference(paths_value);

    HymnObjectString *imports = machine_intern_string(this, hymn_new_string("__imports"));
    reference_string(imports);

    this->imports = new_table();
    HymnValue imports_value = hymn_new_table_value(this->imports);
    table_put(&this->globals, imports, imports_value);
    reference_string(imports);
    reference(imports_value);

    this->print = print_stdout;

    return this;
}

void hymn_delete(Hymn *this) {
    {
        HymnTable *globals = &this->globals;
        unsigned int bins = globals->bins;
        for (unsigned int i = 0; i < bins; i++) {
            HymnTableItem *item = globals->items[i];
            HymnTableItem *previous = NULL;
            while (item != NULL) {
                HymnTableItem *next = item->next;
                if (hymn_is_native(item->value)) {
                    if (previous == NULL) {
                        globals->items[i] = next;
                    } else {
                        previous->next = next;
                    }
                    native_function_delete(hymn_as_native(item->value));
                    free(item);
                    globals->size -= 1;
                } else {
                    previous = item;
                }
                item = next;
            }
        }
        table_release(this, &this->globals);
        assert(this->globals.size == 0);
    }

    {
        HymnSet *strings = &this->strings;
        unsigned int bins = strings->bins;
        for (unsigned int i = 0; i < bins; i++) {
            HymnSetItem *item = strings->items[i];
            while (item != NULL) {
                HymnSetItem *next = item->next;
                dereference_string(this, item->string);
                item = next;
            }
        }
        assert(strings->size == 0);
        free(strings->items);
    }

    hymn_string_delete(this->error);

    free(this);
}

void hymn_add_function(Hymn *this, const char *name, HymnNativeCall func) {
    HymnObjectString *intern = machine_intern_string(this, hymn_new_string(name));
    reference_string(intern);
    HymnString *copy = string_copy(intern->string);
    HymnNativeFunction *value = new_native_function(copy, func);
    table_put(&this->globals, intern, hymn_new_native(value));
}

void hymn_add_pointer(Hymn *this, const char *name, void *pointer) {
    HymnObjectString *intern = machine_intern_string(this, hymn_new_string(name));
    reference_string(intern);
    table_put(&this->globals, intern, hymn_new_pointer(pointer));
}

char *hymn_debug(Hymn *this, const char *script) {
    HymnString *source = hymn_read_file(script);

    struct CompileReturn result = compile(this, script, source);

    HymnFunction *main = result.func;

    char *error = result.error;
    if (error) {
        function_delete(main);
        return error;
    }

    disassemble_byte_code(&main->code, script);

    HymnValuePool *constants = &main->code.constants;
    for (int i = 0; i < constants->count; i++) {
        HymnValue value = constants->values[i];
        if (hymn_is_func(value)) {
            HymnFunction *func = hymn_as_func(value);
            disassemble_byte_code(&func->code, func->name);
        }
    }

    return NULL;
}

char *hymn_do(Hymn *this, const char *script, const char *source) {
    struct CompileReturn result = compile(this, script, source);

    HymnFunction *main = result.func;

    char *error = result.error;
    if (error) {
        function_delete(main);
        return error;
    }

    HymnValue function = hymn_new_func_value(main);
    reference(function);

    push(this, function);
    call(this, main, 0);

    error = machine_interpret(this);
    if (error) {
        return error;
    }

    assert(this->stack_top == this->stack);
    reset_stack(this);

    return NULL;
}

char *hymn_read(Hymn *this, const char *script) {
    HymnString *source = hymn_read_file(script);
    char *error = hymn_do(this, script, source);
    hymn_string_delete(source);
    return error;
}
