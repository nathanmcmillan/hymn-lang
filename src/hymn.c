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
#define PATH_SEP_OTHER '\\'
#define PATH_SEP_OTHER_STRING "\\"
#define UNREACHABLE() __builtin_unreachable()
#elif _MSC_VER
#include <direct.h>
#include <windows.h>
#define getcwd _getcwd
#define PATH_MAX FILENAME_MAX
#define PATH_SEP '\\'
#define PATH_SEP_STRING "\\"
#define PATH_SEP_OTHER '/'
#define PATH_SEP_OTHER_STRING "/"
#define UNREACHABLE() __assume(0)
#endif

#include "hymn.h"

// #define HYMN_DEBUG_TRACE
// #define HYMN_DEBUG_STACK
// #define HYMN_DEBUG_MEMORY

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

static HymnString *path_convert(HymnString *path) {
    size_t size = hymn_string_len(path);
    HymnString *convert = string_copy(path);
    for (size_t i = 0; i < size; i++) {
        if (convert[i] == PATH_SEP_OTHER) {
            convert[i] = PATH_SEP;
        }
    }
    return convert;
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
typedef struct Instruction Instruction;

static const float LOAD_FACTOR = 0.80f;
static const unsigned int INITIAL_BINS = 1 << 3;
static const unsigned int MAXIMUM_BINS = 1 << 30;

enum TokenType {
    TOKEN_ADD,
    TOKEN_AND,
    TOKEN_ASSIGN,
    TOKEN_ASSIGN_ADD,
    TOKEN_ASSIGN_BIT_AND,
    TOKEN_ASSIGN_BIT_LEFT_SHIFT,
    TOKEN_ASSIGN_BIT_OR,
    TOKEN_ASSIGN_BIT_RIGHT_SHIFT,
    TOKEN_ASSIGN_BIT_XOR,
    TOKEN_ASSIGN_DIVIDE,
    TOKEN_ASSIGN_MODULO,
    TOKEN_ASSIGN_MULTIPLY,
    TOKEN_ASSIGN_SUBTRACT,
    TOKEN_BEGIN,
    TOKEN_BIT_AND,
    TOKEN_BIT_LEFT_SHIFT,
    TOKEN_BIT_NOT,
    TOKEN_BIT_OR,
    TOKEN_BIT_RIGHT_SHIFT,
    TOKEN_BIT_XOR,
    TOKEN_BREAK,
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
    OP_GET_TWO_LOCAL,
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
    OP_USE,
    OP_FOR,
    OP_FOR_LOOP
};

enum FunctionType {
    TYPE_FUNCTION,
    TYPE_SCRIPT,
};

static void compile_with_precedence(Compiler *C, enum Precedence precedence);
static void compile_call(Compiler *C, bool assign);
static void compile_group(Compiler *C, bool assign);
static void compile_none(Compiler *C, bool assign);
static void compile_true(Compiler *C, bool assign);
static void compile_false(Compiler *C, bool assign);
static void compile_integer(Compiler *C, bool assign);
static void compile_float(Compiler *C, bool assign);
static void compile_string(Compiler *C, bool assign);
static void compile_array(Compiler *C, bool assign);
static void compile_table(Compiler *C, bool assign);
static void compile_variable(Compiler *C, bool assign);
static void compile_unary(Compiler *C, bool assign);
static void compile_binary(Compiler *C, bool assign);
static void compile_dot(Compiler *C, bool assign);
static void compile_square(Compiler *C, bool assign);
static void compile_and(Compiler *C, bool assign);
static void compile_or(Compiler *C, bool assign);
static void array_push_expression(Compiler *C, bool assign);
static void array_insert_expression(Compiler *C, bool assign);
static void array_pop_expression(Compiler *C, bool assign);
static void delete_expression(Compiler *C, bool assign);
static void len_expression(Compiler *C, bool assign);
static void cast_integer_expression(Compiler *C, bool assign);
static void cast_float_expression(Compiler *C, bool assign);
static void cast_string_expression(Compiler *C, bool assign);
static void clear_expression(Compiler *C, bool assign);
static void copy_expression(Compiler *C, bool assign);
static void index_expression(Compiler *C, bool assign);
static void keys_expression(Compiler *C, bool assign);
static void type_expression(Compiler *C, bool assign);
static void declaration(Compiler *C);
static void statement(Compiler *C);
static void expression_statement(Compiler *C);
static void expression(Compiler *C);

static void reference_string(HymnObjectString *string);
static void reference(HymnValue value);
static void dereference_string(Hymn *H, HymnObjectString *string);
static void dereference(Hymn *H, HymnValue value);

static char *machine_interpret(Hymn *H);

struct JumpList {
    int jump;
    int depth;
    HymnByteCode *code;
    struct JumpList *next;
};

struct LoopList {
    int start;
    int depth;
    HymnByteCode *code;
    bool is_for;
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
    struct JumpList *jump_for;
    HymnString *error;
};

struct Instruction {
    int index;
    uint8_t instruction;
    Instruction *next;
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
    [TOKEN_ASSIGN_ADD] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_ASSIGN_BIT_AND] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_ASSIGN_BIT_LEFT_SHIFT] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_ASSIGN_BIT_OR] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_ASSIGN_BIT_RIGHT_SHIFT] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_ASSIGN_BIT_XOR] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_ASSIGN_DIVIDE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_ASSIGN_MODULO] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_ASSIGN_MULTIPLY] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_ASSIGN_SUBTRACT] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_BEGIN] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_BIT_AND] = {NULL, compile_binary, PRECEDENCE_BITS},
    [TOKEN_BIT_LEFT_SHIFT] = {NULL, compile_binary, PRECEDENCE_BITS},
    [TOKEN_BIT_NOT] = {compile_unary, NULL, PRECEDENCE_NONE},
    [TOKEN_BIT_OR] = {NULL, compile_binary, PRECEDENCE_BITS},
    [TOKEN_BIT_RIGHT_SHIFT] = {NULL, compile_binary, PRECEDENCE_BITS},
    [TOKEN_BIT_XOR] = {NULL, compile_binary, PRECEDENCE_BITS},
    [TOKEN_BREAK] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_CLEAR] = {clear_expression, NULL, PRECEDENCE_NONE},
    [TOKEN_COLON] = {NULL, NULL, PRECEDENCE_NONE},
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
    [TOKEN_SEMICOLON] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_STRING] = {compile_string, NULL, PRECEDENCE_NONE},
    [TOKEN_SUBTRACT] = {compile_unary, compile_binary, PRECEDENCE_TERM},
    [TOKEN_THROW] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_TO_FLOAT] = {cast_float_expression, NULL, PRECEDENCE_NONE},
    [TOKEN_TO_INTEGER] = {cast_integer_expression, NULL, PRECEDENCE_NONE},
    [TOKEN_TO_STRING] = {cast_string_expression, NULL, PRECEDENCE_NONE},
    [TOKEN_TRUE] = {compile_true, NULL, PRECEDENCE_NONE},
    [TOKEN_TRY] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_TYPE_FUNC] = {type_expression, NULL, PRECEDENCE_NONE},
    [TOKEN_UNDEFINED] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_USE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_VALUE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_WHILE] = {NULL, NULL, PRECEDENCE_NONE},
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
    default: return "?";
    }
}

static size_t string_mix_hashcode(HymnString *key) {
    size_t length = hymn_string_len(key);
    size_t hash = 0;
    for (size_t i = 0; i < length; i++) {
        hash = 31 * hash + (size_t)key[i];
    }
    return hash ^ (hash >> 16);
}

static HymnObjectString *new_hymn_string_with_hash(HymnString *string, size_t hash) {
    HymnObjectString *object = hymn_calloc(1, sizeof(HymnObjectString));
    object->string = string;
    object->hash = hash;
    return object;
}

HymnObjectString *hymn_new_string_object(HymnString *string) {
    return new_hymn_string_with_hash(string, string_mix_hashcode(string));
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
            HymnValue old = item->value;
            item->value = value;
            return old;
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

static HymnTableItem *table_next(HymnTable *this, HymnObjectString *key) {
    unsigned int bins = this->bins;
    if (key == NULL) {
        for (unsigned int i = 0; i < bins; i++) {
            HymnTableItem *item = this->items[i];
            if (item != NULL) {
                return item;
            }
        }
        return NULL;
    }
    unsigned int bin = table_get_bin(this, key->hash);
    {
        HymnTableItem *item = this->items[bin];
        while (item != NULL) {
            HymnTableItem *next = item->next;
            if (key == item->key) {
                if (next != NULL) {
                    return next;
                }
            }
            item = next;
        }
    }
    for (unsigned int i = bin + 1; i < bins; i++) {
        HymnTableItem *item = this->items[i];
        if (item != NULL) {
            return item;
        }
    }
    return NULL;
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
            this->size--;
            return value;
        }
        previous = item;
        item = item->next;
    }
    return hymn_new_undefined();
}

static void table_clear(Hymn *H, HymnTable *this) {
    this->size = 0;
    unsigned int bins = this->bins;
    for (unsigned int i = 0; i < bins; i++) {
        HymnTableItem *item = this->items[i];
        while (item != NULL) {
            HymnTableItem *next = item->next;
            dereference(H, item->value);
            dereference_string(H, item->key);
            free(item);
            item = next;
        }
        this->items[i] = NULL;
    }
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
    size_t hash = string_mix_hashcode(add);
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
    size_t hash = string_mix_hashcode(remove);
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
            this->size--;
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

static void set_release(Hymn *H, HymnSet *set) {
    set_clear(H, set);
    free(set->items);
}

static HymnFunction *current_func(Compiler *C) {
    return C->scope->func;
}

static HymnByteCode *current(Compiler *C) {
    return &current_func(C)->code;
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

static void compile_error(Compiler *C, Token *token, const char *format, ...) {
    if (C->error != NULL) return;

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

    size_t begin = beginning_of_line(C->source, token->start);
    size_t end = end_of_line(C->source, C->size, token->start);

    error = string_append_second_previous_line(C->source, error, begin);
    error = string_append_previous_line(C->source, error, begin);
    error = string_append_format(error, "%.*s\n", end - begin, &C->source[begin]);
    for (int i = 0; i < (int)(token->start - begin); i++) {
        error = hymn_string_append_char(error, ' ');
    }
    error = hymn_string_append(error, ANSI_COLOR_RED);
    for (int i = 0; i < token->length; i++) {
        error = hymn_string_append_char(error, '^');
    }
    error = hymn_string_append(error, ANSI_COLOR_RESET);

    error = string_append_format(error, "\nat %s:%d\n", C->script, token->row);

    C->error = error;

    C->previous.type = TOKEN_EOF;
    C->current.type = TOKEN_EOF;
}

static char next_char(Compiler *C) {
    size_t pos = C->pos;
    if (pos == C->size) {
        return '\0';
    }
    char c = C->source[pos];
    C->pos = pos + 1;
    if (c == '\n') {
        C->row++;
        C->column = 0;
    } else {
        C->column++;
    }
    return c;
}

static char peek_char(Compiler *C) {
    if (C->pos == C->size) {
        return '\0';
    }
    return C->source[C->pos];
}

static void token(Compiler *C, enum TokenType type) {
    Token *current = &C->current;
    current->type = type;
    current->row = C->row;
    current->column = C->column;
    if (C->pos == 0) {
        current->start = 0;
    } else {
        current->start = C->pos - 1;
    }
    current->length = 1;
}

static void token_special(Compiler *C, enum TokenType type, size_t offset, size_t length) {
    Token *current = &C->current;
    current->type = type;
    current->row = C->row;
    current->column = C->column;
    if (C->pos < offset) {
        current->start = 0;
    } else {
        current->start = C->pos - offset;
    }
    current->length = (int)length;
}

static void value_token(Compiler *C, enum TokenType type, size_t start, size_t end) {
    Token *current = &C->current;
    current->type = type;
    current->row = C->row;
    current->column = C->column;
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
        if (size == 6) return ident_trie(ident, 1, "tring", TOKEN_TO_STRING);
        break;
    case 'k':
        if (size == 4) return ident_trie(ident, 1, "eys", TOKEN_KEYS);
        break;
    case 'c':
        if (size == 4) return ident_trie(ident, 1, "opy", TOKEN_COPY);
        if (size == 5) return ident_trie(ident, 1, "lear", TOKEN_CLEAR);
        if (size == 8) return ident_trie(ident, 1, "ontinue", TOKEN_CONTINUE);
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

static void push_ident_token(Compiler *C, size_t start, size_t end) {
    const char *ident = &C->source[start];
    size_t size = end - start;
    enum TokenType keyword = ident_keyword(ident, size);
    if (keyword != TOKEN_UNDEFINED) {
        value_token(C, keyword, start, end);
    } else {
        value_token(C, TOKEN_IDENT, start, end);
    }
}

static bool is_digit(char c) {
    return '0' <= c && c <= '9';
}

static bool is_ident(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

static void advance(Compiler *C) {
    C->previous = C->current;
    if (C->previous.type == TOKEN_EOF) {
        return;
    }
    while (true) {
        char c = next_char(C);
        switch (c) {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            c = peek_char(C);
            while (c != '\0' && (c == ' ' || c == '\t' || c == '\r' || c == '\n')) {
                next_char(C);
                c = peek_char(C);
            }
            continue;
        case '!':
            if (peek_char(C) == '=') {
                next_char(C);
                token_special(C, TOKEN_NOT_EQUAL, 2, 2);
            } else {
                token(C, TOKEN_NOT);
            }
            return;
        case '=':
            if (peek_char(C) == '=') {
                next_char(C);
                token_special(C, TOKEN_EQUAL, 2, 2);
            } else {
                token(C, TOKEN_ASSIGN);
            }
            return;
        case '-': {
            if (peek_char(C) == '-') {
                next_char(C);
                c = peek_char(C);
                while (c != '\n' && c != '\0') {
                    next_char(C);
                    c = peek_char(C);
                }
                continue;
            } else if (peek_char(C) == '=') {
                next_char(C);
                token_special(C, TOKEN_ASSIGN_SUBTRACT, 2, 2);
                return;
            } else {
                token(C, TOKEN_SUBTRACT);
                return;
            }
        }
        case '+':
            if (peek_char(C) == '=') {
                next_char(C);
                token_special(C, TOKEN_ASSIGN_ADD, 2, 2);
            } else {
                token(C, TOKEN_ADD);
            }
            return;
        case '*':
            if (peek_char(C) == '=') {
                next_char(C);
                token_special(C, TOKEN_ASSIGN_MULTIPLY, 2, 2);
            } else {
                token(C, TOKEN_MULTIPLY);
            }
            return;
        case '/':
            if (peek_char(C) == '=') {
                next_char(C);
                token_special(C, TOKEN_ASSIGN_DIVIDE, 2, 2);
            } else {
                token(C, TOKEN_DIVIDE);
            }
            return;
        case '%':
            if (peek_char(C) == '=') {
                next_char(C);
                token_special(C, TOKEN_ASSIGN_MODULO, 2, 2);
            } else {
                token(C, TOKEN_MODULO);
            }
            return;
        case '&':
            if (peek_char(C) == '=') {
                next_char(C);
                token_special(C, TOKEN_ASSIGN_BIT_AND, 2, 2);
            } else {
                token(C, TOKEN_BIT_AND);
            }
            return;
        case '|':
            if (peek_char(C) == '=') {
                next_char(C);
                token_special(C, TOKEN_ASSIGN_BIT_OR, 2, 2);
            } else {
                token(C, TOKEN_BIT_OR);
            }
            return;
        case '^':
            if (peek_char(C) == '=') {
                next_char(C);
                token_special(C, TOKEN_ASSIGN_BIT_XOR, 2, 2);
            } else {
                token(C, TOKEN_BIT_XOR);
            }
            return;
        case '>':
            if (peek_char(C) == '=') {
                next_char(C);
                token_special(C, TOKEN_GREATER_EQUAL, 2, 2);
            } else if (peek_char(C) == '>') {
                next_char(C);
                if (peek_char(C) == '=') {
                    next_char(C);
                    token_special(C, TOKEN_ASSIGN_BIT_RIGHT_SHIFT, 2, 2);
                } else {
                    token_special(C, TOKEN_BIT_RIGHT_SHIFT, 2, 2);
                }
            } else {
                token(C, TOKEN_GREATER);
            }
            return;
        case '<':
            if (peek_char(C) == '=') {
                next_char(C);
                token_special(C, TOKEN_LESS_EQUAL, 2, 2);
            } else if (peek_char(C) == '<') {
                next_char(C);
                if (peek_char(C) == '=') {
                    next_char(C);
                    token_special(C, TOKEN_ASSIGN_BIT_LEFT_SHIFT, 2, 2);
                } else {
                    token_special(C, TOKEN_BIT_LEFT_SHIFT, 2, 2);
                }
            } else {
                token(C, TOKEN_LESS);
            }
            return;
        case '~': token(C, TOKEN_BIT_NOT); return;
        case ',': token(C, TOKEN_COMMA); return;
        case '.': token(C, TOKEN_DOT); return;
        case '(': token(C, TOKEN_LEFT_PAREN); return;
        case ')': token(C, TOKEN_RIGHT_PAREN); return;
        case '[': token(C, TOKEN_LEFT_SQUARE); return;
        case ']': token(C, TOKEN_RIGHT_SQUARE); return;
        case '{': token(C, TOKEN_LEFT_CURLY); return;
        case '}': token(C, TOKEN_RIGHT_CURLY); return;
        case ':': token(C, TOKEN_COLON); return;
        case ';': token(C, TOKEN_SEMICOLON); return;
        case '\0': token(C, TOKEN_EOF); return;
        case '"': {
            size_t start = C->pos;
            while (true) {
                c = next_char(C);
                if (c == '\\') {
                    next_char(C);
                    continue;
                } else if (c == '"' || c == '\0') {
                    break;
                }
            }
            size_t end = C->pos - 1;
            value_token(C, TOKEN_STRING, start, end);
            return;
        }
        case '\'': {
            size_t start = C->pos;
            while (true) {
                c = next_char(C);
                if (c == '\\') {
                    next_char(C);
                    continue;
                } else if (c == '\'' || c == '\0') {
                    break;
                }
            }
            size_t end = C->pos - 1;
            value_token(C, TOKEN_STRING, start, end);
            return;
        }
        default: {
            if (is_digit(c)) {
                size_t start = C->pos - 1;
                while (is_digit(peek_char(C))) {
                    next_char(C);
                }
                bool discrete = true;
                if (peek_char(C) == '.') {
                    discrete = false;
                    next_char(C);
                    while (is_digit(peek_char(C))) {
                        next_char(C);
                    }
                }
                size_t end = C->pos;
                if (discrete) {
                    value_token(C, TOKEN_INTEGER, start, end);
                } else {
                    value_token(C, TOKEN_FLOAT, start, end);
                }
                return;
            } else if (is_ident(c)) {
                size_t start = C->pos - 1;
                while (is_ident(peek_char(C))) {
                    next_char(C);
                }
                size_t end = C->pos;
                push_ident_token(C, start, end);
                return;
            } else {
                compile_error(C, &C->current, "Unknown character: `%c`", c);
            }
        }
        }
    }
}

bool hymn_value_false(HymnValue value) {
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

bool hymn_values_equal(HymnValue a, HymnValue b) {
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
    case HYMN_VALUE_POINTER:
        return hymn_is_pointer(b) && hymn_as_pointer(a) == hymn_as_pointer(b);
    default: return false;
    }
}

bool hymn_match_values(HymnValue a, HymnValue b) {
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

static void value_pool_init(HymnValuePool *this) {
    this->count = 0;
    this->capacity = 8;
    this->values = hymn_malloc(8 * sizeof(HymnValue));
}

static int value_pool_add(HymnValuePool *this, HymnValue value) {
    int count = this->count;
    for (int c = 0; c < count; c++) {
        if (hymn_match_values(this->values[c], value)) {
            return c;
        }
    }
    if (count + 1 > this->capacity) {
        this->capacity *= 2;
        this->values = hymn_realloc(this->values, this->capacity * sizeof(HymnValue));
    }
    this->values[count] = value;
    this->count = count + 1;
    return count;
}

static void byte_code_init(HymnByteCode *this) {
    this->count = 0;
    this->capacity = 8;
    this->instructions = hymn_malloc(8 * sizeof(uint8_t));
    this->lines = hymn_malloc(8 * sizeof(int));
    value_pool_init(&this->constants);
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

HymnArray *hymn_new_array(int64_t length) {
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
        if (hymn_match_values(match, items[i])) {
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

HymnTable *hymn_new_table() {
    HymnTable *this = hymn_calloc(1, sizeof(HymnTable));
    table_init(this);
    return this;
}

static HymnTable *new_table_copy(HymnTable *from) {
    HymnTable *this = hymn_new_table();
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
    if (hymn_match_values(match, item->value)) return item->key;

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
        if (hymn_match_values(match, item->value)) return item->key;
    }
}

static void value_pool_delete(HymnValuePool *this) {
    free(this->values);
}

static void scope_init(Compiler *C, Scope *scope, enum FunctionType type) {
    scope->enclosing = C->scope;
    C->scope = scope;

    scope->local_count = 0;
    scope->depth = 0;
    scope->func = new_function(C->script);
    scope->type = type;

    if (type != TYPE_SCRIPT) {
        scope->func->name = hymn_substring(C->source, C->previous.start, C->previous.start + C->previous.length);
    }

    Local *local = &scope->locals[scope->local_count++];
    local->depth = 0;
    local->name.start = 0;
    local->name.length = 0;
}

static inline Compiler new_compiler(const char *script, const char *source, Hymn *H, Scope *scope) {
    Compiler C = {0};
    C.row = 1;
    C.column = 1;
    C.script = script;
    C.source = source;
    C.size = strlen(source);
    C.previous.type = TOKEN_UNDEFINED;
    C.current.type = TOKEN_UNDEFINED;
    C.H = H;
    scope_init(&C, scope, TYPE_SCRIPT);
    return C;
}

static void byte_code_delete(HymnByteCode *this) {
    free(this->instructions);
    free(this->lines);
    value_pool_delete(&this->constants);
}

static uint8_t byte_code_new_constant(Compiler *C, HymnValue value) {
    HymnByteCode *code = current(C);
    int constant = value_pool_add(&code->constants, value);
    if (constant > UINT8_MAX) {
        compile_error(C, &C->previous, "Too many constants");
        constant = 0;
    }
    return (uint8_t)constant;
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

static void emit(Compiler *C, uint8_t i) {
    write_byte(current(C), i, C->previous.row);
}

static void emit_short(Compiler *C, uint8_t i, uint8_t b) {
    int row = C->previous.row;
    HymnByteCode *code = current(C);
    write_byte(code, i, row);
    write_byte(code, b, row);
}

static void emit_word(Compiler *C, uint8_t i, uint8_t b, uint8_t n) {
    int row = C->previous.row;
    HymnByteCode *code = current(C);
    write_byte(code, i, row);
    write_byte(code, b, row);
    write_byte(code, n, row);
}

static uint8_t emit_constant(Compiler *C, HymnValue value) {
    uint8_t constant = byte_code_new_constant(C, value);
    emit_short(C, OP_CONSTANT, constant);
    return constant;
}

static Rule *token_rule(enum TokenType type) {
    return &rules[type];
}

static HymnObjectString *machine_intern_string(Hymn *H, HymnString *string) {
    HymnObjectString *object = set_add_or_get(&H->strings, string);
    if (object->string != string) {
        hymn_string_delete(string);
    }
    return object;
}

static HymnValue compile_intern_string(Hymn *H, HymnString *string) {
    HymnObjectString *object = set_add_or_get(&H->strings, string);
    if (object->string == string) {
        reference_string(object);
    } else {
        hymn_string_delete(string);
    }
    return hymn_new_string_value(object);
}

static bool check_assign(Compiler *C) {
    switch (C->current.type) {
    case TOKEN_ASSIGN:
    case TOKEN_ASSIGN_ADD:
    case TOKEN_ASSIGN_BIT_AND:
    case TOKEN_ASSIGN_BIT_LEFT_SHIFT:
    case TOKEN_ASSIGN_BIT_OR:
    case TOKEN_ASSIGN_BIT_RIGHT_SHIFT:
    case TOKEN_ASSIGN_BIT_XOR:
    case TOKEN_ASSIGN_DIVIDE:
    case TOKEN_ASSIGN_MODULO:
    case TOKEN_ASSIGN_MULTIPLY:
    case TOKEN_ASSIGN_SUBTRACT:
        return true;
    default:
        return false;
    }
}

static bool check(Compiler *C, enum TokenType type) {
    return C->current.type == type;
}

static bool match(Compiler *C, enum TokenType type) {
    if (!check(C, type)) {
        return false;
    }
    advance(C);
    return true;
}

static void compile_with_precedence(Compiler *C, enum Precedence precedence) {
    advance(C);
    Rule *rule = token_rule(C->previous.type);
    void (*prefix)(Compiler *, bool) = rule->prefix;
    if (prefix == NULL) {
        compile_error(C, &C->previous, "Expected expression following `%.*s`", C->previous.length, &C->source[C->previous.start]);
        return;
    }
    bool assign = precedence <= PRECEDENCE_ASSIGN;
    prefix(C, assign);
    while (precedence <= token_rule(C->current.type)->precedence) {
        advance(C);
        void (*infix)(Compiler *, bool) = token_rule(C->previous.type)->infix;
        if (infix == NULL) {
            compile_error(C, &C->previous, "Expected infix");
            return;
        }
        infix(C, assign);
    }
    if (assign && check_assign(C)) {
        advance(C);
        compile_error(C, &C->current, "Invalid assignment target");
    }
}

static void consume(Compiler *C, enum TokenType type, const char *error) {
    if (C->current.type == type) {
        advance(C);
        return;
    }
    compile_error(C, &C->current, error);
}

static uint8_t push_hidden_local(Compiler *C) {
    Scope *scope = C->scope;
    if (scope->local_count == HYMN_UINT8_COUNT) {
        compile_error(C, &C->previous, "Too many local variables in scope");
        return 0;
    }
    uint8_t index = (uint8_t)scope->local_count++;
    Local *local = &scope->locals[index];
    local->name = (Token){0};
    local->depth = scope->depth;
    return index;
}

static uint8_t arguments(Compiler *C) {
    uint8_t count = 0;
    if (!check(C, TOKEN_RIGHT_PAREN)) {
        do {
            expression(C);
            if (count == UINT8_MAX) {
                compile_error(C, &C->previous, "Can't have more than 255 function arguments");
                break;
            }
            count++;
        } while (match(C, TOKEN_COMMA));
    }
    consume(C, TOKEN_RIGHT_PAREN, "Expected `)` after function arguments");
    return count;
}

static void compile_call(Compiler *C, bool assign) {
    (void)assign;
    uint8_t count = arguments(C);
    emit_short(C, OP_CALL, count);
}

static void compile_group(Compiler *C, bool assign) {
    (void)assign;
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "Expected right parenthesis");
}

static void compile_none(Compiler *C, bool assign) {
    (void)assign;
    emit(C, OP_NONE);
}

static void compile_true(Compiler *C, bool assign) {
    (void)assign;
    emit(C, OP_TRUE);
}

static void compile_false(Compiler *C, bool assign) {
    (void)assign;
    emit(C, OP_FALSE);
}

static void compile_integer(Compiler *C, bool assign) {
    (void)assign;
    Token *previous = &C->previous;
    int64_t number = (int64_t)strtoll(&C->source[previous->start], NULL, 10);
    emit_constant(C, hymn_new_int(number));
}

static void compile_float(Compiler *C, bool assign) {
    (void)assign;
    Token *previous = &C->previous;
    double number = strtod(&C->source[previous->start], NULL);
    emit_constant(C, hymn_new_float(number));
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

static void compile_string(Compiler *C, bool assign) {
    (void)assign;
    Token *previous = &C->previous;
    HymnString *s = parse_string_literal(C->source, previous->start, previous->length);
    emit_constant(C, compile_intern_string(C->H, s));
}

static uint8_t ident_constant(Compiler *C, Token *token) {
    HymnString *string = hymn_substring(C->source, token->start, token->start + token->length);
    return byte_code_new_constant(C, compile_intern_string(C->H, string));
}

static void begin_scope(Compiler *C) {
    C->scope->depth++;
}

static void end_scope(Compiler *C) {
    Scope *scope = C->scope;
    scope->depth--;
    while (scope->local_count > 0 && scope->locals[scope->local_count - 1].depth > scope->depth) {
        emit(C, OP_POP);
        scope->local_count--;
    }
}

static void compile_array(Compiler *C, bool assign) {
    (void)assign;
    emit_constant(C, hymn_new_array_value(NULL));
    if (match(C, TOKEN_RIGHT_SQUARE)) {
        return;
    }
    while (!check(C, TOKEN_RIGHT_SQUARE) && !check(C, TOKEN_EOF)) {
        emit(C, OP_DUPLICATE);
        expression(C);
        emit_short(C, OP_ARRAY_PUSH, OP_POP);
        if (!check(C, TOKEN_RIGHT_SQUARE)) {
            consume(C, TOKEN_COMMA, "Expected `,`");
        }
    }
    consume(C, TOKEN_RIGHT_SQUARE, "Expected `]` declaring array");
}

static void compile_table(Compiler *C, bool assign) {
    (void)assign;
    emit_constant(C, hymn_new_table_value(NULL));
    if (match(C, TOKEN_RIGHT_CURLY)) {
        return;
    }
    while (!check(C, TOKEN_RIGHT_CURLY) && !check(C, TOKEN_EOF)) {
        emit(C, OP_DUPLICATE);
        consume(C, TOKEN_IDENT, "Expected property name");
        uint8_t name = ident_constant(C, &C->previous);
        consume(C, TOKEN_COLON, "Expected `:`");
        expression(C);
        emit_short(C, OP_SET_PROPERTY, name);
        emit(C, OP_POP);
        if (!check(C, TOKEN_RIGHT_CURLY)) {
            consume(C, TOKEN_COMMA, "Expected `,`");
        }
    }
    consume(C, TOKEN_RIGHT_CURLY, "Expected `}` declaring table");
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

static void push_local(Compiler *C, Token name) {
    Scope *scope = C->scope;
    if (scope->local_count == HYMN_UINT8_COUNT) {
        compile_error(C, &name, "Too many local variables in scope");
        return;
    }
    Local *local = &scope->locals[scope->local_count++];
    local->name = name;
    local->depth = -1;
}

static bool ident_match(Compiler *C, Token *a, Token *b) {
    if (a->length != b->length) {
        return false;
    }
    return memcmp(&C->source[a->start], &C->source[b->start], a->length) == 0;
}

static uint8_t variable(Compiler *C, const char *error) {
    consume(C, TOKEN_IDENT, error);
    Scope *scope = C->scope;
    if (scope->depth == 0) {
        return ident_constant(C, &C->previous);
    }
    Token *name = &C->previous;
    for (int i = scope->local_count - 1; i >= 0; i--) {
        Local *local = &scope->locals[i];
        if (local->depth != -1 && local->depth < scope->depth) {
            break;
        } else if (ident_match(C, name, &local->name)) {
            compile_error(C, name, "Variable `%.*s` already exists in this scope", name->length, &C->source[name->start]);
        }
    }
    push_local(C, *name);
    return 0;
}

static void local_initialize(Compiler *C) {
    Scope *scope = C->scope;
    if (scope->depth == 0) {
        return;
    }
    scope->locals[scope->local_count - 1].depth = scope->depth;
}

static void finalize_variable(Compiler *C, uint8_t global) {
    if (C->scope->depth > 0) {
        local_initialize(C);
        return;
    }
    emit_short(C, OP_DEFINE_GLOBAL, global);
}

static void define_new_variable(Compiler *C) {
    uint8_t global = variable(C, "Expected variable name");
    consume(C, TOKEN_ASSIGN, "Expected `=` after variable");
    expression(C);
    finalize_variable(C, global);
}

static int resolve_local(Compiler *C, Token *name) {
    Scope *scope = C->scope;
    for (int i = scope->local_count - 1; i >= 0; i--) {
        Local *local = &scope->locals[i];
        if (ident_match(C, name, &local->name)) {
            if (local->depth == -1) {
                compile_error(C, name, "Local variable `%.*s` referenced before assignment", name->length, &C->source[name->start]);
            }
            return i;
        }
    }
    return -1;
}

static void named_variable(Compiler *C, Token token, bool assign) {
    uint8_t get;
    uint8_t set;
    int var = resolve_local(C, &token);
    if (var != -1) {
        get = OP_GET_LOCAL;
        set = OP_SET_LOCAL;
    } else {
        get = OP_GET_GLOBAL;
        set = OP_SET_GLOBAL;
        var = ident_constant(C, &token);
    }
    if (assign && check_assign(C)) {
        enum TokenType type = C->current.type;
        advance(C);
        if (type != TOKEN_ASSIGN) {
            emit_short(C, get, (uint8_t)var);
        }
        expression(C);
        switch (type) {
        case TOKEN_ASSIGN_ADD: emit(C, OP_ADD); break;
        case TOKEN_ASSIGN_BIT_AND: emit(C, OP_BIT_AND); break;
        case TOKEN_ASSIGN_BIT_LEFT_SHIFT: emit(C, OP_BIT_LEFT_SHIFT); break;
        case TOKEN_ASSIGN_BIT_OR: emit(C, OP_BIT_OR); break;
        case TOKEN_ASSIGN_BIT_RIGHT_SHIFT: emit(C, OP_BIT_RIGHT_SHIFT); break;
        case TOKEN_ASSIGN_BIT_XOR: emit(C, OP_BIT_XOR); break;
        case TOKEN_ASSIGN_DIVIDE: emit(C, OP_DIVIDE); break;
        case TOKEN_ASSIGN_MODULO: emit(C, OP_MODULO); break;
        case TOKEN_ASSIGN_MULTIPLY: emit(C, OP_MULTIPLY); break;
        case TOKEN_ASSIGN_SUBTRACT: emit(C, OP_SUBTRACT); break;
        default: break;
        }
        emit_short(C, set, (uint8_t)var);
    } else {
        emit_short(C, get, (uint8_t)var);
    }
}

static void compile_variable(Compiler *C, bool assign) {
    named_variable(C, C->previous, assign);
}

static void compile_unary(Compiler *C, bool assign) {
    (void)assign;
    enum TokenType type = C->previous.type;
    compile_with_precedence(C, PRECEDENCE_UNARY);
    switch (type) {
    case TOKEN_NOT: emit(C, OP_NOT); break;
    case TOKEN_SUBTRACT: emit(C, OP_NEGATE); break;
    case TOKEN_BIT_NOT: emit(C, OP_BIT_NOT); break;
    default: return;
    }
}

static void compile_binary(Compiler *C, bool assign) {
    (void)assign;
    enum TokenType type = C->previous.type;
    Rule *rule = token_rule(type);
    compile_with_precedence(C, (enum Precedence)(rule->precedence + 1));
    switch (type) {
    case TOKEN_ADD: emit(C, OP_ADD); break;
    case TOKEN_SUBTRACT: emit(C, OP_SUBTRACT); break;
    case TOKEN_MODULO: emit(C, OP_MODULO); break;
    case TOKEN_MULTIPLY: emit(C, OP_MULTIPLY); break;
    case TOKEN_DIVIDE: emit(C, OP_DIVIDE); break;
    case TOKEN_EQUAL: emit(C, OP_EQUAL); break;
    case TOKEN_NOT_EQUAL: emit(C, OP_NOT_EQUAL); break;
    case TOKEN_LESS: emit(C, OP_LESS); break;
    case TOKEN_LESS_EQUAL: emit(C, OP_LESS_EQUAL); break;
    case TOKEN_GREATER: emit(C, OP_GREATER); break;
    case TOKEN_GREATER_EQUAL: emit(C, OP_GREATER_EQUAL); break;
    case TOKEN_BIT_OR: emit(C, OP_BIT_OR); break;
    case TOKEN_BIT_AND: emit(C, OP_BIT_AND); break;
    case TOKEN_BIT_XOR: emit(C, OP_BIT_XOR); break;
    case TOKEN_BIT_LEFT_SHIFT: emit(C, OP_BIT_LEFT_SHIFT); break;
    case TOKEN_BIT_RIGHT_SHIFT: emit(C, OP_BIT_RIGHT_SHIFT); break;
    default: return;
    }
}

static void compile_dot(Compiler *C, bool assign) {
    consume(C, TOKEN_IDENT, "Expected property name after `.`");
    uint8_t name = ident_constant(C, &C->previous);
    if (assign && match(C, TOKEN_ASSIGN)) {
        expression(C);
        emit_short(C, OP_SET_PROPERTY, name);
    } else {
        emit_short(C, OP_GET_PROPERTY, name);
    }
}

static void compile_square(Compiler *C, bool assign) {
    if (match(C, TOKEN_COLON)) {
        emit_constant(C, hymn_new_int(0));
        if (match(C, TOKEN_RIGHT_SQUARE)) {
            emit_constant(C, hymn_new_none());
        } else {
            expression(C);
            consume(C, TOKEN_RIGHT_SQUARE, "Expected `]` after expression");
        }
        emit(C, OP_SLICE);
    } else {
        expression(C);
        if (match(C, TOKEN_COLON)) {
            if (match(C, TOKEN_RIGHT_SQUARE)) {
                emit_constant(C, hymn_new_none());
            } else {
                expression(C);
                consume(C, TOKEN_RIGHT_SQUARE, "Expected `]` after expression");
            }
            emit(C, OP_SLICE);
        } else {
            consume(C, TOKEN_RIGHT_SQUARE, "Expected `]` after expression");
            if (assign && match(C, TOKEN_ASSIGN)) {
                expression(C);
                emit(C, OP_SET_DYNAMIC);
            } else {
                emit(C, OP_GET_DYNAMIC);
            }
        }
    }
}

static int emit_jump(Compiler *C, uint8_t instruction) {
    emit(C, instruction);
    emit_short(C, UINT8_MAX, UINT8_MAX);
    return current(C)->count - 2;
}

static void patch_jump(Compiler *C, int jump) {
    if (jump == -1) {
        return;
    }
    HymnByteCode *code = current(C);
    int count = code->count;
    int offset = count - jump - 2;
    if (offset > UINT16_MAX) {
        compile_error(C, &C->previous, "Jump offset too large");
        return;
    }
    code->instructions[jump] = (offset >> 8) & UINT8_MAX;
    code->instructions[jump + 1] = offset & UINT8_MAX;
}

static struct JumpList *add_jump(Compiler *C, struct JumpList *list, enum OpCode instruction) {
    struct JumpList *jump = hymn_calloc(1, sizeof(struct JumpList));
    jump->jump = emit_jump(C, instruction);
    jump->depth = C->scope->depth;
    jump->code = current(C);
    jump->next = list;
    return jump;
}

static void free_jump_and_list(Compiler *C) {
    struct JumpList *jump = C->jump_and;
    HymnByteCode *code = current(C);
    int depth = C->scope->depth;
    while (jump != NULL) {
        if (jump->code != code || jump->depth < depth) {
            break;
        }
        patch_jump(C, jump->jump);
        struct JumpList *next = jump->next;
        free(jump);
        jump = next;
    }
    C->jump_and = jump;
}

static void free_jump_or_list(Compiler *C) {
    struct JumpList *jump = C->jump_or;
    HymnByteCode *code = current(C);
    int depth = C->scope->depth;
    while (jump != NULL) {
        if (jump->code != code || jump->depth < depth) {
            break;
        }
        patch_jump(C, jump->jump);
        struct JumpList *next = jump->next;
        free(jump);
        jump = next;
    }
    C->jump_or = jump;
}

static void free_jumps(Compiler *C, struct JumpList *jump) {
    while (jump != NULL) {
        patch_jump(C, jump->jump);
        struct JumpList *next = jump->next;
        free(jump);
        jump = next;
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
    free_jump_and_list(C);
    compile_with_precedence(C, PRECEDENCE_OR);
}

static int next(uint8_t instruction) {
    switch (instruction) {
    case OP_POP_N:
    case OP_SET_GLOBAL:
    case OP_SET_LOCAL:
    case OP_SET_PROPERTY:
    case OP_INCREMENT:
    case OP_CALL:
    case OP_TAIL_CALL:
    case OP_CONSTANT:
    case OP_DEFINE_GLOBAL:
    case OP_GET_GLOBAL:
    case OP_GET_LOCAL:
    case OP_GET_PROPERTY:
        return 2;
    case OP_GET_TWO_LOCAL:
    case OP_ADD_TWO_LOCAL:
    case OP_JUMP:
    case OP_JUMP_IF_FALSE:
    case OP_JUMP_IF_TRUE:
    case OP_JUMP_IF_EQUAL:
    case OP_JUMP_IF_NOT_EQUAL:
    case OP_JUMP_IF_LESS:
    case OP_JUMP_IF_GREATER:
    case OP_JUMP_IF_LESS_EQUAL:
    case OP_JUMP_IF_GREATER_EQUAL:
    case OP_LOOP:
    case OP_INCREMENT_LOCAL_AND_SET:;
    case OP_INCREMENT_LOCAL:
        return 3;
    case OP_FOR:
    case OP_FOR_LOOP:
        return 4;
    default:
        return 1;
    }
}

static bool adjustable(Instruction *important, uint8_t *instructions, int target) {
    Instruction *view = important;
    while (view != NULL) {
        int i = view->index;
        uint8_t instruction = view->instruction;
        switch (instruction) {
        case OP_JUMP:
        case OP_JUMP_IF_FALSE:
        case OP_JUMP_IF_TRUE:
        case OP_JUMP_IF_EQUAL:
        case OP_JUMP_IF_NOT_EQUAL:
        case OP_JUMP_IF_LESS:
        case OP_JUMP_IF_GREATER:
        case OP_JUMP_IF_LESS_EQUAL:
        case OP_JUMP_IF_GREATER_EQUAL: {
            if (i < target) {
                uint16_t jump = (uint16_t)((instructions[i + 1] << 8) | instructions[i + 2]);
                if (i + 3 + jump == target) {
                    return false;
                }
            }
            break;
        }
        case OP_FOR: {
            if (i < target) {
                uint16_t jump = (uint16_t)((instructions[i + 2] << 8) | instructions[i + 3]);
                if (i + 3 + jump == target) {
                    return false;
                }
            }
            break;
        }
        case OP_LOOP: {
            if (i >= target) {
                uint16_t jump = (uint16_t)((instructions[i + 1] << 8) | instructions[i + 2]);
                if (i + 3 - jump == target) {
                    return false;
                }
            }
            break;
        }
        case OP_FOR_LOOP: {
            if (i >= target) {
                uint16_t jump = (uint16_t)((instructions[i + 2] << 8) | instructions[i + 3]);
                if (i + 3 - jump == target) {
                    return false;
                }
            }
            break;
        }
        }
        view = view->next;
    }
    return true;
}

static int rewrite(Instruction *important, uint8_t *instructions, int *lines, int count, int start, int shift) {
    Instruction *view = important;
    while (view != NULL) {
        int i = view->index;
        uint8_t instruction = view->instruction;
        switch (instruction) {
        case OP_JUMP:
        case OP_JUMP_IF_FALSE:
        case OP_JUMP_IF_TRUE:
        case OP_JUMP_IF_EQUAL:
        case OP_JUMP_IF_NOT_EQUAL:
        case OP_JUMP_IF_LESS:
        case OP_JUMP_IF_GREATER:
        case OP_JUMP_IF_LESS_EQUAL:
        case OP_JUMP_IF_GREATER_EQUAL: {
            if (i < start) {
                uint16_t jump = (uint16_t)((instructions[i + 1] << 8) | instructions[i + 2]);
                if (i + 3 + jump > start) {
                    jump -= (uint16_t)shift;
                    instructions[i + 1] = (jump >> 8) & UINT8_MAX;
                    instructions[i + 2] = jump & UINT8_MAX;
                }
            }
            break;
        }
        case OP_FOR: {
            if (i < start) {
                uint16_t jump = (uint16_t)((instructions[i + 2] << 8) | instructions[i + 3]);
                if (i + 3 + jump > start) {
                    jump -= (uint16_t)shift;
                    instructions[i + 2] = (jump >> 8) & UINT8_MAX;
                    instructions[i + 3] = jump & UINT8_MAX;
                }
            }
            break;
        }
        case OP_LOOP: {
            if (i >= start) {
                uint16_t jump = (uint16_t)((instructions[i + 1] << 8) | instructions[i + 2]);
                if (i + 3 - jump < start) {
                    jump -= (uint16_t)shift;
                    instructions[i + 1] = (jump >> 8) & UINT8_MAX;
                    instructions[i + 2] = jump & UINT8_MAX;
                }
            }
            break;
        }
        case OP_FOR_LOOP: {
            if (i >= start) {
                uint16_t jump = (uint16_t)((instructions[i + 2] << 8) | instructions[i + 3]);
                if (i + 3 - jump < start) {
                    jump -= (uint16_t)shift;
                    instructions[i + 2] = (jump >> 8) & UINT8_MAX;
                    instructions[i + 3] = jump & UINT8_MAX;
                }
            }
            break;
        }
        }
        if (i >= start) {
            view->index = i - shift;
        }
        view = view->next;
    }
    count -= shift;
    for (int c = start; c < count; c++) {
        int n = c + shift;
        instructions[c] = instructions[n];
        lines[c] = lines[n];
    }
    return shift;
}

static void update(Instruction *important, uint8_t *instructions, int i, uint8_t instruction) {
    instructions[i] = instruction;
    Instruction *view = important;
    while (view != NULL) {
        if (i == view->index) {
            view->instruction = instruction;
            return;
        }
        view = view->next;
    }
    fprintf(stderr, "Optimization failed to find instruction to update.\n");
    exit(1);
}

static void deleter(Instruction **important, int i) {
    Instruction *view = *important;
    Instruction *previous = NULL;
    while (view != NULL) {
        if (i == view->index) {
            Instruction *next = view->next;
            free(view);
            if (previous == NULL) {
                *important = next;
            } else {
                previous->next = next;
            }
            return;
        }
        previous = view;
        view = view->next;
    }
    fprintf(stderr, "Optimization failed to find instruction to delete.\n");
    exit(1);
}

static Instruction *interest(uint8_t *instructions, int count) {
    Instruction *head = NULL;
    Instruction *tail = NULL;
    int i = 0;
    while (i < count) {
        uint8_t instruction = instructions[i];
        switch (instruction) {
        case OP_JUMP:
        case OP_JUMP_IF_FALSE:
        case OP_JUMP_IF_TRUE:
        case OP_JUMP_IF_EQUAL:
        case OP_JUMP_IF_NOT_EQUAL:
        case OP_JUMP_IF_LESS:
        case OP_JUMP_IF_GREATER:
        case OP_JUMP_IF_LESS_EQUAL:
        case OP_JUMP_IF_GREATER_EQUAL:
        case OP_FOR:
        case OP_LOOP:
        case OP_FOR_LOOP: {
            Instruction *important = hymn_calloc(1, sizeof(Instruction));
            important->index = i;
            important->instruction = instruction;
            if (tail == NULL) {
                head = important;
            } else {
                tail->next = important;
            }
            tail = important;
            break;
        }
        }
        i += next(instruction);
    }
    return head;
}

static void optimize(Compiler *C) {
    HymnByteCode *code = current(C);
    uint8_t *instructions = code->instructions;
    int *lines = code->lines;
    int count = code->count;
    Instruction *important = interest(instructions, count);
    int one = 0;
    while (one < count) {

#define SET(I, O) instructions[I] = O
#define UPDATE(I, O) update(important, instructions, I, O)
#define REWRITE(S, X) count -= rewrite(important, instructions, lines, count, one + S, X)
#define REPEAT continue
#define NEXT goto next
#define JUMP_IF(T, F)                        \
    if (second == OP_JUMP_IF_TRUE) {         \
        REWRITE(0, 1);                       \
        UPDATE(one, T);                      \
        REPEAT;                              \
    } else if (second == OP_JUMP_IF_FALSE) { \
        REWRITE(0, 1);                       \
        UPDATE(one, F);                      \
        REPEAT;                              \
    }

        uint8_t first = instructions[one];
        int two = one + next(first);
        if (two >= count) break;
        uint8_t second = instructions[two];

        if (!adjustable(important, instructions, one) || !adjustable(important, instructions, two)) {
            one = two;
            continue;
        }

        switch (first) {
        case OP_CALL: {
            if (second == OP_RETURN) {
                SET(one, OP_TAIL_CALL);
                REPEAT;
            }
            break;
        }
        case OP_POP: {
            if (second == OP_POP) {
                REWRITE(0, 1);
                SET(one, OP_POP_TWO);
                REPEAT;
            }
            break;
        }
        case OP_POP_TWO: {
            if (second == OP_POP) {
                SET(one, OP_POP_N);
                SET(one + 1, 3);
                REPEAT;
            }
            break;
        }
        case OP_POP_N: {
            if (second == OP_POP) {
                uint8_t pop = instructions[one + 1];
                if (pop < UINT8_MAX - 1) {
                    REWRITE(1, 1);
                    SET(one + 1, pop + 1);
                    REPEAT;
                }
            }
            break;
        }
        case OP_GET_LOCAL: {
            if (second == OP_GET_LOCAL) {
                SET(one, OP_GET_TWO_LOCAL);
                REWRITE(2, 1);
                REPEAT;
            } else if (second == OP_CONSTANT) {
                int three = two + next(second);
                uint8_t third = three < count ? instructions[three] : UINT8_MAX;
                if (third == OP_ADD) {
                    HymnValue value = code->constants.values[code->instructions[two + 1]];
                    if (hymn_is_int(value)) {
                        int64_t add = hymn_as_int(value);
                        if (add >= 0 && add <= UINT8_MAX) {
                            uint8_t local = code->instructions[one + 1];
                            REWRITE(0, 2);
                            SET(one, OP_INCREMENT_LOCAL);
                            SET(one + 1, local);
                            SET(one + 2, (uint8_t)add);
                            REPEAT;
                        }
                    }
                }
            }
            break;
        }
        case OP_GET_TWO_LOCAL: {
            if (second == OP_ADD) {
                SET(one, OP_ADD_TWO_LOCAL);
                REWRITE(3, 1);
                REPEAT;
            }
            break;
        }
        case OP_INCREMENT_LOCAL: {
            if (second == OP_SET_LOCAL) {
                if (instructions[one + 1] == instructions[one + 4]) {
                    SET(one, OP_INCREMENT_LOCAL_AND_SET);
                    REWRITE(3, 2);
                    REPEAT;
                }
            }
            break;
        }
        case OP_INCREMENT_LOCAL_AND_SET: {
            if (second == OP_POP) {
                REWRITE(3, 1);
                REPEAT;
            }
            break;
        }
        case OP_CONSTANT: {
            if (second == OP_NEGATE) {
                HymnValue value = code->constants.values[instructions[one + 1]];
                if (hymn_is_int(value)) {
                    value.as.i = -value.as.i;
                } else if (hymn_is_float(value)) {
                    value.as.f = -value.as.f;
                }
                uint8_t constant = byte_code_new_constant(C, value);
                SET(one + 1, constant);
                REWRITE(2, 1);
                REPEAT;
            } else if (second == OP_ADD) {
                HymnValue value = code->constants.values[code->instructions[one + 1]];
                if (hymn_is_int(value)) {
                    int64_t add = hymn_as_int(value);
                    if (add >= 0 && add <= UINT8_MAX) {
                        SET(one, OP_INCREMENT);
                        SET(one + 1, (uint8_t)add);
                        REWRITE(2, 1);
                        REPEAT;
                    }
                }
            }
            break;
        }
        case OP_EQUAL: {
            JUMP_IF(OP_JUMP_IF_EQUAL, OP_JUMP_IF_NOT_EQUAL)
            break;
        }
        case OP_NOT_EQUAL: {
            JUMP_IF(OP_JUMP_IF_NOT_EQUAL, OP_JUMP_IF_EQUAL)
            break;
        }
        case OP_LESS: {
            JUMP_IF(OP_JUMP_IF_LESS, OP_JUMP_IF_GREATER_EQUAL)
            break;
        }
        case OP_GREATER: {
            JUMP_IF(OP_JUMP_IF_GREATER, OP_JUMP_IF_LESS_EQUAL)
            break;
        }
        case OP_LESS_EQUAL: {
            JUMP_IF(OP_JUMP_IF_LESS_EQUAL, OP_JUMP_IF_GREATER)
            break;
        }
        case OP_GREATER_EQUAL: {
            JUMP_IF(OP_JUMP_IF_GREATER_EQUAL, OP_JUMP_IF_LESS)
            break;
        }
        case OP_TRUE: {
            if (second == OP_JUMP_IF_TRUE) {
                REWRITE(0, 1);
                UPDATE(one, OP_JUMP);
                REPEAT;
            } else if (second == OP_JUMP_IF_FALSE) {
                deleter(&important, two);
                REWRITE(0, 4);
                REPEAT;
            }
            break;
        }
        case OP_FALSE: {
            if (second == OP_JUMP_IF_TRUE) {
                deleter(&important, two);
                REWRITE(0, 4);
                REPEAT;
            } else if (second == OP_JUMP_IF_FALSE) {
                REWRITE(0, 1);
                UPDATE(one, OP_JUMP);
                REPEAT;
            }
            break;
        }
        case OP_NOT: {
            if (second == OP_JUMP_IF_TRUE) {
                REWRITE(0, 1);
                UPDATE(one, OP_JUMP_IF_FALSE);
                REPEAT;
            } else if (second == OP_JUMP_IF_FALSE) {
                REWRITE(0, 1);
                UPDATE(one, OP_JUMP_IF_TRUE);
                REPEAT;
            }
            break;
        }
        }

        one = two;
    }

    while (important != NULL) {
        Instruction *next = important->next;
        free(important);
        important = next;
    }

    code->count = count;
}

static HymnFunction *end_function(Compiler *C) {
    emit(C, OP_NONE);
    emit(C, OP_RETURN);
    optimize(C);
    HymnFunction *func = C->scope->func;
    C->scope = C->scope->enclosing;
    return func;
}

static void compile_function(Compiler *C, enum FunctionType type) {
    Scope scope = {0};
    scope_init(C, &scope, type);

    begin_scope(C);

    consume(C, TOKEN_LEFT_PAREN, "Expected `(` after function name");

    if (!check(C, TOKEN_RIGHT_PAREN)) {
        do {
            C->scope->func->arity++;
            if (C->scope->func->arity > UINT8_MAX) {
                compile_error(C, &C->previous, "Can't have more than 255 function parameters");
            }
            uint8_t parameter = variable(C, "Expected parameter name");
            finalize_variable(C, parameter);
        } while (match(C, TOKEN_COMMA));
    }

    consume(C, TOKEN_RIGHT_PAREN, "Expected `)` after function parameters");

    while (!check(C, TOKEN_END) && !check(C, TOKEN_EOF)) {
        declaration(C);
    }

    end_scope(C);
    consume(C, TOKEN_END, "Expected `end` after function body");

    HymnFunction *func = end_function(C);
    emit_constant(C, hymn_new_func_value(func));
}

static void declare_function(Compiler *C) {
    uint8_t global = variable(C, "Expected function name");
    local_initialize(C);
    compile_function(C, TYPE_FUNCTION);
    finalize_variable(C, global);
}

static void declaration(Compiler *C) {
    if (match(C, TOKEN_LET)) {
        define_new_variable(C);
    } else if (match(C, TOKEN_FUNCTION)) {
        declare_function(C);
    } else {
        statement(C);
    }
}

static void block(Compiler *C) {
    begin_scope(C);
    while (!check(C, TOKEN_END) && !check(C, TOKEN_EOF)) {
        declaration(C);
    }
    end_scope(C);
}

static void if_statement(Compiler *C) {
    expression(C);
    int jump = emit_jump(C, OP_JUMP_IF_FALSE);

    free_jump_or_list(C);

    begin_scope(C);
    while (!check(C, TOKEN_ELIF) && !check(C, TOKEN_ELSE) && !check(C, TOKEN_END) && !check(C, TOKEN_EOF)) {
        declaration(C);
    }
    end_scope(C);

    if (check(C, TOKEN_END)) {
        patch_jump(C, jump);
        free_jump_and_list(C);
    } else {
        struct JumpList jump_end = {0};
        jump_end.jump = emit_jump(C, OP_JUMP);
        struct JumpList *tail = &jump_end;

        while (match(C, TOKEN_ELIF)) {
            patch_jump(C, jump);
            free_jump_and_list(C);

            expression(C);
            jump = emit_jump(C, OP_JUMP_IF_FALSE);

            free_jump_or_list(C);

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
        free_jump_and_list(C);

        if (match(C, TOKEN_ELSE)) {
            block(C);
        }

        patch_jump(C, jump_end.jump);
        free_jumps(C, jump_end.next);
    }

    consume(C, TOKEN_END, "If statement is missing \"end\"");
}

static void emit_loop(Compiler *C, int start) {
    emit(C, OP_LOOP);
    int offset = current(C)->count - start + 2;
    if (offset > UINT16_MAX) {
        compile_error(C, &C->previous, "Loop is too large");
    }
    emit_short(C, (offset >> 8) & UINT8_MAX, offset & UINT8_MAX);
}

static void patch_jump_list(Compiler *C) {
    while (C->jump != NULL) {
        int depth;
        if (C->loop == NULL) {
            depth = 1;
        } else {
            depth = C->loop->depth + 1;
        }
        if (C->jump->depth < depth) {
            break;
        }
        patch_jump(C, C->jump->jump);
        struct JumpList *next = C->jump->next;
        free(C->jump);
        C->jump = next;
    }
}

static void patch_jump_for_list(Compiler *C) {
    while (C->jump_for != NULL) {
        int depth;
        if (C->loop == NULL) {
            depth = 1;
        } else {
            depth = C->loop->depth;
        }
        if (C->jump_for->depth < depth) {
            break;
        }
        patch_jump(C, C->jump_for->jump);
        struct JumpList *next = C->jump_for->next;
        free(C->jump_for);
        C->jump_for = next;
    }
}

static void iterator_statement(Compiler *C, bool pair) {
    local_initialize(C);

    uint8_t index = (uint8_t)C->scope->local_count;
    uint8_t value = index + 1;

    uint8_t object = index - 1;

    push_hidden_local(C);

    if (pair) {
        variable(C, "Missing variable name in for loop");
        local_initialize(C);
        consume(C, TOKEN_IN, "Missing `in` in for loop");
        C->scope->locals[index].name = C->scope->locals[object].name;
    } else {
        push_hidden_local(C);
        C->scope->locals[value].name = C->scope->locals[object].name;
    }

    C->scope->locals[object].name = (Token){0};

    // IN

    expression(C);

    emit_short(C, OP_FOR, object);
    emit_short(C, UINT8_MAX, UINT8_MAX);

    int start = current(C)->count;
    int jump = start - 2;

    struct LoopList loop = {.start = start, .depth = C->scope->depth + 1, .next = C->loop, .is_for = true};
    C->loop = &loop;

    // BODY

    block(C);

    // LOOP

    patch_jump_for_list(C);

    emit_short(C, OP_FOR_LOOP, object);
    int offset = current(C)->count - start + 2;
    if (offset > UINT16_MAX) {
        compile_error(C, &C->previous, "Loop is too large");
    }
    emit_short(C, (offset >> 8) & UINT8_MAX, offset & UINT8_MAX);

    // END

    C->loop = loop.next;

    patch_jump(C, jump);
    patch_jump_list(C);

    end_scope(C);

    consume(C, TOKEN_END, "Missing `end` in for loop");
}

static void for_statement(Compiler *C) {
    begin_scope(C);

    // ASSIGN

    uint8_t index = (uint8_t)C->scope->local_count;

    variable(C, "Missing variable name in for loop");

    if (match(C, TOKEN_ASSIGN)) {
        expression(C);
        local_initialize(C);
        consume(C, TOKEN_COMMA, "Missing `,` in for loop");
    } else if (match(C, TOKEN_COMMA)) {
        iterator_statement(C, true);
        return;
    } else if (match(C, TOKEN_IN)) {
        iterator_statement(C, false);
        return;
    } else {
        compile_error(C, &C->previous, "Missing either `=`, `in`, or `,` in for loop");
        return;
    }

    // COMPARE

    int compare = current(C)->count;

    expression(C);

    int jump = emit_jump(C, OP_JUMP_IF_FALSE);

    // INCREMENT

    int increment = current(C)->count;

    struct LoopList loop = {.start = increment, .depth = C->scope->depth + 1, .next = C->loop, .is_for = true};
    C->loop = &loop;

    if (match(C, TOKEN_COMMA)) {
        expression(C);
    } else {
        emit_word(C, OP_INCREMENT_LOCAL_AND_SET, index, 1);
    }

    HymnByteCode *code = current(C);

    int count = code->count - increment;
    uint8_t *instructions = hymn_malloc(count * sizeof(uint8_t));
    int *lines = hymn_malloc(count * sizeof(int));
    memcpy(instructions, &code->instructions[increment], count * sizeof(uint8_t));
    memcpy(lines, &code->lines[increment], count * sizeof(int));
    code->count = increment;

    // BODY

    block(C);

    // INCREMENT

    patch_jump_for_list(C);

    while (code->count + count > code->capacity) {
        code->capacity *= 2;
        code->instructions = hymn_realloc(code->instructions, code->capacity * sizeof(uint8_t));
        code->lines = hymn_realloc(code->lines, code->capacity * sizeof(int));
    }
    memcpy(&code->instructions[code->count], instructions, count * sizeof(uint8_t));
    memcpy(&code->lines[code->count], lines, count * sizeof(int));
    code->count += count;
    free(instructions);
    free(lines);

    emit_loop(C, compare);

    // END

    C->loop = loop.next;

    patch_jump(C, jump);
    patch_jump_list(C);

    end_scope(C);

    consume(C, TOKEN_END, "Missing `end` in for loop");
}

static void while_statement(Compiler *C) {
    int start = current(C)->count;

    struct LoopList loop = {.start = start, .depth = C->scope->depth + 1, .next = C->loop, .is_for = false};
    C->loop = &loop;

    expression(C);

    int jump = emit_jump(C, OP_JUMP_IF_FALSE);

    block(C);
    emit_loop(C, start);

    C->loop = loop.next;

    patch_jump(C, jump);
    patch_jump_list(C);

    consume(C, TOKEN_END, "While: Missing 'end'");
}

static void return_statement(Compiler *C) {
    if (C->scope->type == TYPE_SCRIPT) {
        compile_error(C, &C->previous, "Return: Outside of function");
    }
    if (check(C, TOKEN_END)) {
        emit(C, OP_NONE);
    } else {
        expression(C);
    }
    emit(C, OP_RETURN);
}

static void pop_stack_loop(Compiler *C) {
    int depth = C->loop->depth;
    Scope *scope = C->scope;
    for (int i = scope->local_count; i > 0; i--) {
        if (scope->locals[i - 1].depth < depth) {
            return;
        }
        emit(C, OP_POP);
    }
}

static void break_statement(Compiler *C) {
    if (C->loop == NULL) {
        compile_error(C, &C->previous, "Break Error: Outside of loop");
    }
    pop_stack_loop(C);
    struct JumpList *jump_next = C->jump;
    struct JumpList *jump = hymn_malloc(sizeof(struct JumpList));
    jump->jump = emit_jump(C, OP_JUMP);
    jump->depth = C->loop->depth;
    jump->next = jump_next;
    C->jump = jump;
}

static void continue_statement(Compiler *C) {
    if (C->loop == NULL) {
        compile_error(C, &C->previous, "Continue Error: Outside of loop");
    }
    pop_stack_loop(C);
    if (C->loop->is_for) {
        struct JumpList *jump_next = C->jump_for;
        struct JumpList *jump = hymn_malloc(sizeof(struct JumpList));
        jump->jump = emit_jump(C, OP_JUMP);
        jump->depth = C->loop->depth;
        jump->next = jump_next;
        C->jump_for = jump;
    } else {
        emit_loop(C, C->loop->start);
    }
}

static void try_statement(Compiler *C) {
    HymnByteCode *code = current(C);

    HymnExceptList *except = hymn_calloc(1, sizeof(HymnExceptList));
    except->locals = C->scope->local_count;
    except->start = code->count;

    HymnFunction *func = current_func(C);
    except->next = func->except;
    func->except = except;

    begin_scope(C);
    while (!check(C, TOKEN_EXCEPT) && !check(C, TOKEN_EOF)) {
        declaration(C);
    }
    end_scope(C);

    int jump = emit_jump(C, OP_JUMP);

    consume(C, TOKEN_EXCEPT, "Try statement is missing `except`");

    except->end = code->count;

    begin_scope(C);
    uint8_t message = variable(C, "Try: Missing variable after 'except'");
    finalize_variable(C, message);
    while (!check(C, TOKEN_END) && !check(C, TOKEN_EOF)) {
        declaration(C);
    }
    end_scope(C);

    consume(C, TOKEN_END, "Try: Missing 'end'");

    patch_jump(C, jump);
}

static void print_statement(Compiler *C) {
    consume(C, TOKEN_LEFT_PAREN, "Expected `(` around print statement");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "Expected `)` around print statement");
    emit(C, OP_PRINT);
}

static void use_statement(Compiler *C) {
    expression(C);
    emit(C, OP_USE);
}

static void throw_statement(Compiler *C) {
    expression(C);
    emit(C, OP_THROW);
}

static void statement(Compiler *C) {
    if (match(C, TOKEN_PRINT)) {
        print_statement(C);
    } else if (match(C, TOKEN_USE)) {
        use_statement(C);
    } else if (match(C, TOKEN_IF)) {
        if_statement(C);
    } else if (match(C, TOKEN_FOR)) {
        for_statement(C);
    } else if (match(C, TOKEN_WHILE)) {
        while_statement(C);
    } else if (match(C, TOKEN_RETURN)) {
        return_statement(C);
    } else if (match(C, TOKEN_BREAK)) {
        break_statement(C);
    } else if (match(C, TOKEN_CONTINUE)) {
        continue_statement(C);
    } else if (match(C, TOKEN_TRY)) {
        try_statement(C);
    } else if (match(C, TOKEN_THROW)) {
        throw_statement(C);
    } else if (match(C, TOKEN_BEGIN)) {
        block(C);
        consume(C, TOKEN_END, "Expected `end` after block");
    } else {
        expression_statement(C);
    }
}

static void array_push_expression(Compiler *C, bool assign) {
    (void)assign;
    consume(C, TOKEN_LEFT_PAREN, "Expected `(` after push");
    expression(C);
    consume(C, TOKEN_COMMA, "Expected `,` between push arguments");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "Expected `)` after push expression");
    emit(C, OP_ARRAY_PUSH);
}

static void array_insert_expression(Compiler *C, bool assign) {
    (void)assign;
    consume(C, TOKEN_LEFT_PAREN, "Expected `(` after insert");
    expression(C);
    consume(C, TOKEN_COMMA, "Expected `,` between insert arguments");
    expression(C);
    consume(C, TOKEN_COMMA, "Expected `,` between insert arguments");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "Expected `)` after insert expression");
    emit(C, OP_ARRAY_INSERT);
}

static void array_pop_expression(Compiler *C, bool assign) {
    (void)assign;
    consume(C, TOKEN_LEFT_PAREN, "Expected `(` after pop");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "Expected `)` after pop expression");
    emit(C, OP_ARRAY_POP);
}

static void delete_expression(Compiler *C, bool assign) {
    (void)assign;
    consume(C, TOKEN_LEFT_PAREN, "Expected `(` after delete");
    expression(C);
    consume(C, TOKEN_COMMA, "Expected `,` between delete arguments");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "Expected `)` after delete expression");
    emit(C, OP_DELETE);
}

static void len_expression(Compiler *C, bool assign) {
    (void)assign;
    consume(C, TOKEN_LEFT_PAREN, "Expected `(` after len");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "Expected `)` after len expression");
    emit(C, OP_LEN);
}

static void cast_integer_expression(Compiler *C, bool assign) {
    (void)assign;
    consume(C, TOKEN_LEFT_PAREN, "Expected `(` after integer");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "Expected `)` after integer expression");
    emit(C, OP_TO_INTEGER);
}

static void cast_float_expression(Compiler *C, bool assign) {
    (void)assign;
    consume(C, TOKEN_LEFT_PAREN, "Expected `(` after float");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "Expected `)` after float expression");
    emit(C, OP_TO_FLOAT);
}

static void cast_string_expression(Compiler *C, bool assign) {
    (void)assign;
    consume(C, TOKEN_LEFT_PAREN, "Expected `(` after string");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "Expected `)` after string expression");
    emit(C, OP_TO_STRING);
}

static void type_expression(Compiler *C, bool assign) {
    (void)assign;
    consume(C, TOKEN_LEFT_PAREN, "Expected `(` after type");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "Expected `)` after type expression");
    emit(C, OP_TYPE);
}

static void clear_expression(Compiler *C, bool assign) {
    (void)assign;
    consume(C, TOKEN_LEFT_PAREN, "Expected `(` after clear");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "Expected `)` after clear expression");
    emit(C, OP_CLEAR);
}

static void copy_expression(Compiler *C, bool assign) {
    (void)assign;
    consume(C, TOKEN_LEFT_PAREN, "Expected `(` after copy");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "Expected `)` after copy expression");
    emit(C, OP_COPY);
}

static void keys_expression(Compiler *C, bool assign) {
    (void)assign;
    consume(C, TOKEN_LEFT_PAREN, "Expected `(` after keys");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "Expected `)` after keys expression");
    emit(C, OP_KEYS);
}

static void index_expression(Compiler *C, bool assign) {
    (void)assign;
    consume(C, TOKEN_LEFT_PAREN, "Missing '(' for paramters in `index` function");
    expression(C);
    consume(C, TOKEN_COMMA, "Expected 2 arguments for `index` function");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "Missing ')' after parameters in `index` function");
    emit(C, OP_INDEX);
}

static void expression_statement(Compiler *C) {
    expression(C);
    emit(C, OP_POP);
}

static void expression(Compiler *C) {
    compile_with_precedence(C, PRECEDENCE_ASSIGN);
}

static HymnFrame *parent_frame(Hymn *H, int offset) {
    int frame_count = H->frame_count;
    if (offset > frame_count) return NULL;
    return &H->frames[frame_count - offset];
}

static HymnFrame *current_frame(Hymn *H) {
    return &H->frames[H->frame_count - 1];
}

struct CompileReturn {
    HymnFunction *func;
    char *error;
};

static struct CompileReturn compile(Hymn *H, const char *script, const char *source) {
    Scope scope = {0};

    Compiler compiler = new_compiler(script, source, H, &scope);
    Compiler *C = &compiler;

    advance(C);
    while (!match(C, TOKEN_EOF)) {
        declaration(C);
    }

    HymnFunction *func = end_function(C);
    char *error = NULL;

    if (C->error) {
        error = string_to_chars(C->error);
        hymn_string_delete(C->error);
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
        } else if (pointer_set_has(set, table)) {
            return hymn_new_string("{ .. }");
        } else {
            pointer_set_add(set, table);
        }
        unsigned int size = table->size;
        HymnObjectString **keys = hymn_malloc(size * sizeof(HymnObjectString *));
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
        for (unsigned int i = 0; i < size; i++) {
            if (i != 0) {
                string = hymn_string_append(string, ", ");
            }
            HymnObjectString *key = keys[i];
            HymnValue item = table_get(table, key);
            HymnString *add = value_to_string_recusive(item, set, true);
            string = string_append_format(string, "%s: %s", key->string, add);
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

static void reset_stack(Hymn *H) {
    H->stack_top = H->stack;
    H->frame_count = 0;
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

#ifdef HYMN_DEBUG_MEMORY
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
#ifdef HYMN_DEBUG_MEMORY
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
#ifdef HYMN_DEBUG_MEMORY
        debug_reference(value);
#endif
    }
}
#endif

#ifdef HYMN_NO_MEMORY_MANAGE
static void dereference_string(Hymn *H, HymnObjectString *string) {
    (void)this;
    (void)string;
}
#else
static void dereference_string(Hymn *H, HymnObjectString *string) {
#ifdef HYMN_DEBUG_MEMORY
    debug_dereference(hymn_new_string_value(string));
#endif
    int count = --(string->object.count);
    assert(count >= 0);
    if (count == 0) {
        set_remove(&H->strings, string->string);
        hymn_string_delete(string->string);
        free(string);
    }
}
#endif

#ifdef HYMN_NO_MEMORY_MANAGE
static void dereference(Hymn *H, HymnValue value) {
    (void)this;
    (void)value;
}
#else
static void dereference(Hymn *H, HymnValue value) {
    switch (value.is) {
    case HYMN_VALUE_STRING: {
        HymnObjectString *string = hymn_as_hymn_string(value);
        dereference_string(H, string);
        break;
    }
    case HYMN_VALUE_ARRAY: {
#ifdef HYMN_DEBUG_MEMORY
        debug_dereference(value);
#endif
        HymnArray *array = hymn_as_array(value);
        int count = --(array->object.count);
        assert(count >= 0);
        if (count == 0) {
            array_delete(H, array);
        }
        break;
    }
    case HYMN_VALUE_TABLE: {
#ifdef HYMN_DEBUG_MEMORY
        debug_dereference(value);
#endif
        HymnTable *table = hymn_as_table(value);
        int count = --(table->object.count);
        assert(count >= 0);
        if (count == 0) {
            table_delete(H, table);
        }
        break;
    }
    case HYMN_VALUE_FUNC: {
#ifdef HYMN_DEBUG_MEMORY
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

static void push(Hymn *H, HymnValue value) {
    *H->stack_top = value;
    H->stack_top++;
}

static HymnValue peek(Hymn *H, int dist) {
    assert(&H->stack_top[-dist] >= H->stack);
    return H->stack_top[-dist];
}

static HymnValue pop(Hymn *H) {
    assert(&H->stack_top[-1] >= H->stack);
    H->stack_top--;
    return *H->stack_top;
}

static void machine_push_intern_string(Hymn *H, HymnString *string) {
    HymnObjectString *intern = machine_intern_string(H, string);
    reference_string(intern);
    push(H, hymn_new_string_value(intern));
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

static HymnString *machine_stacktrace(Hymn *H) {
    HymnString *trace = hymn_new_string("");

    for (int i = H->frame_count - 1; i >= 0; i--) {
        HymnFrame *frame = &H->frames[i];
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

static HymnFrame *machine_push_error(Hymn *H, HymnString *error) {
    HymnObjectString *message = machine_intern_string(H, error);
    reference_string(message);
    push(H, hymn_new_string_value(message));
    return machine_exception(H);
}

static HymnFrame *machine_throw_existing_error(Hymn *H, char *error) {
    HymnString *message = hymn_new_string(error);
    free(error);
    return machine_push_error(H, message);
}

static HymnFrame *machine_throw_error(Hymn *H, const char *format, ...) {
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

    HymnString *trace = machine_stacktrace(H);
    error = hymn_string_append(error, "\n\n");
    error = hymn_string_append(error, trace);
    hymn_string_delete(trace);

    return machine_push_error(H, error);
}

static HymnFrame *machine_throw_error_string(Hymn *H, HymnString *string) {
    HymnFrame *frame = machine_throw_error(H, string);
    hymn_string_delete(string);
    return frame;
}

static HymnFrame *call(Hymn *H, HymnFunction *func, int count) {
    if (count != func->arity) {
        return machine_throw_error(H, "Expected %d function arguments but found %d.", func->arity, count);
    } else if (H->frame_count == HYMN_FRAMES_MAX) {
        return machine_throw_error(H, "Stack overflow");
    }

    HymnFrame *frame = &H->frames[H->frame_count++];
    frame->func = func;
    frame->ip = func->code.instructions;
    frame->stack = H->stack_top - count - 1;

    return frame;
}

static HymnFrame *call_value(Hymn *H, HymnValue value, int count) {
    switch (value.is) {
    case HYMN_VALUE_FUNC:
        return call(H, hymn_as_func(value), count);
    case HYMN_VALUE_FUNC_NATIVE: {
        HymnNativeCall func = hymn_as_native(value)->func;
        HymnValue result = func(H, count, H->stack_top - count);
        reference(result);
        HymnValue *top = H->stack_top - (count + 1);
        while (H->stack_top != top) {
            dereference(H, pop(H));
        }
        push(H, result);
        return current_frame(H);
    }
    default: {
        const char *is = value_name(value.is);
        return machine_throw_error(H, "Call: Requires `Function`, but was `%s`.", is);
    }
    }
}

static HymnFrame *machine_import(Hymn *H, HymnObjectString *file) {
    HymnTable *imports = H->imports;

    HymnString *script = NULL;
    int p = 1;
    while (true) {
        HymnFrame *frame = parent_frame(H, p);
        if (frame == NULL) break;
        script = frame->func->script;
        if (script) break;
        p++;
    }

    HymnString *look = path_convert(file->string);
    HymnString *parent = script ? path_parent(script) : NULL;

    HymnObjectString *module = NULL;

    HymnArray *paths = H->paths;
    int64_t size = paths->length;
    for (int64_t i = 0; i < size; i++) {
        HymnValue value = paths->items[i];
        if (!hymn_is_string(value)) {
            continue;
        }
        HymnString *question = hymn_as_string(value);

        HymnString *replace = string_replace(question, "<path>", look);
        HymnString *path = parent ? string_replace(replace, "<parent>", parent) : string_copy(replace);

        HymnObjectString *use = machine_intern_string(H, path_absolute(path));
        reference_string(use);

        hymn_string_delete(path);
        hymn_string_delete(replace);

        if (!hymn_is_undefined(table_get(imports, use))) {
            dereference_string(H, use);
            hymn_string_delete(look);
            if (parent) hymn_string_delete(parent);
            return current_frame(H);
        }

        if (hymn_file_exists(use->string)) {
            module = use;
            break;
        }

        dereference_string(H, use);
    }

    if (module == NULL) {
        HymnString *missing = hymn_string_format("import not found: %s\n", look);

        for (int64_t i = 0; i < size; i++) {
            HymnValue value = paths->items[i];
            if (!hymn_is_string(value)) {
                continue;
            }
            HymnString *question = hymn_as_string(value);

            HymnString *replace = string_replace(question, "<path>", look);
            HymnString *path = parent ? string_replace(replace, "<parent>", parent) : string_copy(replace);
            HymnString *use = path_absolute(path);

            missing = string_append_format(missing, "\nno file %s", use);

            hymn_string_delete(path);
            hymn_string_delete(replace);
            hymn_string_delete(use);
        }

        hymn_string_delete(look);
        if (parent) hymn_string_delete(parent);

        return machine_throw_error_string(H, missing);
    }

    hymn_string_delete(look);
    if (parent) hymn_string_delete(parent);

    table_put(imports, module, hymn_new_bool(true));

    HymnString *source = hymn_read_file(module->string);

    struct CompileReturn result = compile(H, module->string, source);

    HymnFunction *func = result.func;
    char *error = result.error;

    hymn_string_delete(source);

    if (error) {
        function_delete(func);
        return machine_throw_existing_error(H, error);
    }

    HymnValue function = hymn_new_func_value(func);
    reference(function);

    push(H, function);
    call(H, func, 0);

    error = machine_interpret(H);
    if (error) {
        return machine_throw_existing_error(H, error);
    }

    return current_frame(H);
}

static size_t debug_constant_instruction(HymnString **debug, const char *name, HymnByteCode *code, size_t index) {
    uint8_t constant = code->instructions[index + 1];
    *debug = string_append_format(*debug, "%s: [%d] [", name, constant);
    HymnString *value = debug_value_to_string(code->constants.values[constant]);
    *debug = hymn_string_append(*debug, value);
    hymn_string_delete(value);
    *debug = hymn_string_append(*debug, "]");
    return index + 2;
}

static size_t debug_byte_instruction(HymnString **debug, const char *name, HymnByteCode *code, size_t index) {
    uint8_t byte = code->instructions[index + 1];
    *debug = string_append_format(*debug, "%s: [%d]", name, byte);
    return index + 2;
}

static size_t debug_jump_instruction(HymnString **debug, const char *name, int sign, HymnByteCode *code, size_t index) {
    uint16_t jump = (uint16_t)(code->instructions[index + 1] << 8) | (uint16_t)code->instructions[index + 2];
    *debug = string_append_format(*debug, "%s: [%zu] -> [%zu]", name, index, index + 3 + sign * jump);
    return index + 3;
}

static size_t debug_three_byte_instruction(HymnString **debug, const char *name, HymnByteCode *code, size_t index) {
    uint8_t byte = code->instructions[index + 1];
    uint8_t next = code->instructions[index + 2];
    *debug = string_append_format(*debug, "%s: [%d] [%d]", name, byte, next);
    return index + 3;
}

static size_t debug_for_loop_instruction(HymnString **debug, const char *name, int sign, HymnByteCode *code, size_t index) {
    uint8_t slot = code->instructions[index + 1];
    uint16_t jump = (uint16_t)(code->instructions[index + 2] << 8) | (uint16_t)code->instructions[index + 3];
    *debug = string_append_format(*debug, "%s: [%d] [%zu] -> [%zu]", name, slot, index, index + 4 + sign * jump);
    return index + 4;
}

static size_t debug_instruction(HymnString **debug, const char *name, size_t index) {
    *debug = string_append_format(*debug, "%s", name);
    return index + 1;
}

static size_t disassemble_instruction(HymnString **debug, HymnByteCode *code, size_t index) {
    *debug = string_append_format(*debug, "%04zu ", index);
    if (index > 0 && code->lines[index] == code->lines[index - 1]) {
        *debug = hymn_string_append(*debug, "   | ");
    } else {
        *debug = string_append_format(*debug, "%4d ", code->lines[index]);
    }
    uint8_t instruction = code->instructions[index];
    switch (instruction) {
    case OP_ADD: return debug_instruction(debug, "OP_ADD", index);
    case OP_ADD_TWO_LOCAL: return debug_three_byte_instruction(debug, "OP_ADD_TWO_LOCAL", code, index);
    case OP_ARRAY_INSERT: return debug_instruction(debug, "OP_ARRAY_INSERT", index);
    case OP_ARRAY_POP: return debug_instruction(debug, "OP_ARRAY_POP", index);
    case OP_ARRAY_PUSH: return debug_instruction(debug, "OP_ARRAY_PUSH", index);
    case OP_BIT_AND: return debug_instruction(debug, "OP_BIT_AND", index);
    case OP_BIT_LEFT_SHIFT: return debug_instruction(debug, "OP_BIT_LEFT_SHIFT", index);
    case OP_BIT_NOT: return debug_instruction(debug, "OP_BIT_NOT", index);
    case OP_BIT_OR: return debug_instruction(debug, "OP_BIT_OR", index);
    case OP_BIT_RIGHT_SHIFT: return debug_instruction(debug, "OP_BIT_RIGHT_SHIFT", index);
    case OP_BIT_XOR: return debug_instruction(debug, "OP_BIT_XOR", index);
    case OP_CALL: return debug_byte_instruction(debug, "OP_CALL", code, index);
    case OP_CLEAR: return debug_instruction(debug, "OP_CLEAR", index);
    case OP_CONSTANT: return debug_constant_instruction(debug, "OP_CONSTANT", code, index);
    case OP_COPY: return debug_instruction(debug, "OP_COPY", index);
    case OP_DEFINE_GLOBAL: return debug_constant_instruction(debug, "OP_DEFINE_GLOBAL", code, index);
    case OP_DELETE: return debug_instruction(debug, "OP_DELETE", index);
    case OP_DIVIDE: return debug_instruction(debug, "OP_DIVIDE", index);
    case OP_DUPLICATE: return debug_instruction(debug, "OP_DUPLICATE", index);
    case OP_EQUAL: return debug_instruction(debug, "OP_EQUAL", index);
    case OP_FALSE: return debug_instruction(debug, "OP_FALSE", index);
    case OP_FOR: return debug_for_loop_instruction(debug, "OP_FOR", 1, code, index);
    case OP_FOR_LOOP: return debug_for_loop_instruction(debug, "OP_FOR_LOOP", -1, code, index);
    case OP_GET_DYNAMIC: return debug_instruction(debug, "OP_GET_DYNAMIC", index);
    case OP_GET_GLOBAL: return debug_constant_instruction(debug, "OP_GET_GLOBAL", code, index);
    case OP_GET_LOCAL: return debug_byte_instruction(debug, "OP_GET_LOCAL", code, index);
    case OP_GET_PROPERTY: return debug_constant_instruction(debug, "OP_GET_PROPERTY", code, index);
    case OP_GET_TWO_LOCAL: return debug_three_byte_instruction(debug, "OP_GET_TWO_LOCAL", code, index);
    case OP_GREATER: return debug_instruction(debug, "OP_GREATER", index);
    case OP_GREATER_EQUAL: return debug_instruction(debug, "OP_GREATER_EQUAL", index);
    case OP_INCREMENT: return debug_byte_instruction(debug, "OP_INCREMENT", code, index);
    case OP_INCREMENT_LOCAL: return debug_three_byte_instruction(debug, "OP_INCREMENT_LOCAL", code, index);
    case OP_INCREMENT_LOCAL_AND_SET: return debug_three_byte_instruction(debug, "OP_INCREMENT_LOCAL_AND_SET", code, index);
    case OP_INDEX: return debug_instruction(debug, "OP_INDEX", index);
    case OP_JUMP: return debug_jump_instruction(debug, "OP_JUMP", 1, code, index);
    case OP_JUMP_IF_EQUAL: return debug_jump_instruction(debug, "OP_JUMP_IF_EQUAL", 1, code, index);
    case OP_JUMP_IF_FALSE: return debug_jump_instruction(debug, "OP_JUMP_IF_FALSE", 1, code, index);
    case OP_JUMP_IF_GREATER: return debug_jump_instruction(debug, "OP_JUMP_IF_GREATER", 1, code, index);
    case OP_JUMP_IF_GREATER_EQUAL: return debug_jump_instruction(debug, "OP_JUMP_IF_GREATER_EQUAL", 1, code, index);
    case OP_JUMP_IF_LESS: return debug_jump_instruction(debug, "OP_JUMP_IF_LESS", 1, code, index);
    case OP_JUMP_IF_LESS_EQUAL: return debug_jump_instruction(debug, "OP_JUMP_IF_LESS_EQUAL", 1, code, index);
    case OP_JUMP_IF_NOT_EQUAL: return debug_jump_instruction(debug, "OP_JUMP_IF_NOT_EQUAL", 1, code, index);
    case OP_JUMP_IF_TRUE: return debug_jump_instruction(debug, "OP_JUMP_IF_TRUE", 1, code, index);
    case OP_KEYS: return debug_instruction(debug, "OP_KEYS", index);
    case OP_LEN: return debug_instruction(debug, "OP_LEN", index);
    case OP_LESS: return debug_instruction(debug, "OP_LESS", index);
    case OP_LESS_EQUAL: return debug_instruction(debug, "OP_LESS_EQUAL", index);
    case OP_LOOP: return debug_jump_instruction(debug, "OP_LOOP", -1, code, index);
    case OP_MODULO: return debug_instruction(debug, "OP_MODULO", index);
    case OP_MULTIPLY: return debug_instruction(debug, "OP_MULTIPLY", index);
    case OP_NEGATE: return debug_instruction(debug, "OP_NEGATE", index);
    case OP_NONE: return debug_instruction(debug, "OP_NONE", index);
    case OP_NOT: return debug_instruction(debug, "OP_NOT", index);
    case OP_NOT_EQUAL: return debug_instruction(debug, "OP_NOT_EQUAL", index);
    case OP_POP: return debug_instruction(debug, "OP_POP", index);
    case OP_POP_N: return debug_byte_instruction(debug, "OP_POP_N", code, index);
    case OP_POP_TWO: return debug_instruction(debug, "OP_POP_TWO", index);
    case OP_PRINT: return debug_instruction(debug, "OP_PRINT", index);
    case OP_RETURN: return debug_instruction(debug, "OP_RETURN", index);
    case OP_SET_DYNAMIC: return debug_instruction(debug, "OP_SET_DYNAMIC", index);
    case OP_SET_GLOBAL: return debug_constant_instruction(debug, "OP_SET_GLOBAL", code, index);
    case OP_SET_LOCAL: return debug_byte_instruction(debug, "OP_SET_LOCAL", code, index);
    case OP_SET_PROPERTY: return debug_constant_instruction(debug, "OP_SET_PROPERTY", code, index);
    case OP_SLICE: return debug_instruction(debug, "OP_SLICE", index);
    case OP_SUBTRACT: return debug_instruction(debug, "OP_SUBTRACT", index);
    case OP_TAIL_CALL: return debug_byte_instruction(debug, "OP_TAIL_CALL", code, index);
    case OP_THROW: return debug_instruction(debug, "OP_THROW", index);
    case OP_TO_FLOAT: return debug_instruction(debug, "OP_TO_FLOAT", index);
    case OP_TO_INTEGER: return debug_instruction(debug, "OP_TO_INTEGER", index);
    case OP_TO_STRING: return debug_instruction(debug, "OP_TO_STRING", index);
    case OP_TRUE: return debug_instruction(debug, "OP_TRUE", index);
    case OP_TYPE: return debug_instruction(debug, "OP_TYPE", index);
    case OP_USE: return debug_instruction(debug, "OP_USE", index);
    default: *debug = string_append_format(*debug, "UNKNOWN OPCODE %d\n", instruction); return index + 1;
    }
}

void disassemble_byte_code(HymnByteCode *code, const char *name) {
    printf("\n-- %s --\n", name != NULL ? name : "NULL");
    HymnString *debug = hymn_new_string("");
    size_t offset = 0;
    while (offset < (size_t)code->count) {
        offset = disassemble_instruction(&debug, code, offset);
        printf("%s\n", debug);
        hymn_string_zero(debug);
    }
    hymn_string_delete(debug);
}

#define READ_BYTE(frame) (*frame->ip++)

#define READ_SHORT(frame) (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))

#define READ_CONSTANT(frame) (frame->func->code.constants.values[READ_BYTE(frame)])

#define THROW(...)                                 \
    frame = machine_throw_error(H, ##__VA_ARGS__); \
    if (frame == NULL) return;                     \
    HYMN_DISPATCH;

#define ARITHMETIC_OP(_arithmetic_)                                                   \
    HymnValue b = pop(H);                                                             \
    HymnValue a = pop(H);                                                             \
    if (hymn_is_int(a)) {                                                             \
        if (hymn_is_int(b)) {                                                         \
            a.as.i _arithmetic_ b.as.i;                                               \
            push(H, a);                                                               \
        } else if (hymn_is_float(b)) {                                                \
            HymnValue new = hymn_new_float((double)a.as.i);                           \
            new.as.f _arithmetic_ b.as.f;                                             \
            push(H, new);                                                             \
        } else {                                                                      \
            dereference(H, a);                                                        \
            dereference(H, b);                                                        \
            THROW("Operation Error: 2nd value must be `Integer` or `Float`")          \
        }                                                                             \
    } else if (hymn_is_float(a)) {                                                    \
        if (hymn_is_int(b)) {                                                         \
            a.as.f _arithmetic_(double) b.as.i;                                       \
            push(H, a);                                                               \
        } else if (hymn_is_float(b)) {                                                \
            a.as.f _arithmetic_ b.as.f;                                               \
            push(H, a);                                                               \
        } else {                                                                      \
            dereference(H, a);                                                        \
            dereference(H, b);                                                        \
            THROW("Operation Error: 1st and 2nd values must be `Integer` or `Float`") \
        }                                                                             \
    } else {                                                                          \
        dereference(H, a);                                                            \
        dereference(H, b);                                                            \
        THROW("Operation Error: 1st and 2nd values must be `Integer` or `Float`")     \
    }

#define COMPARE_OP(compare)                                                          \
    HymnValue b = pop(H);                                                            \
    HymnValue a = pop(H);                                                            \
    if (hymn_is_int(a)) {                                                            \
        if (hymn_is_int(b)) {                                                        \
            push(H, hymn_new_bool(hymn_as_int(a) compare hymn_as_int(b)));           \
        } else if (hymn_is_float(b)) {                                               \
            push(H, hymn_new_bool((double)hymn_as_int(a) compare hymn_as_float(b))); \
        } else {                                                                     \
            dereference(H, a);                                                       \
            dereference(H, b);                                                       \
            THROW("Operands must be numbers")                                        \
        }                                                                            \
    } else if (hymn_is_float(a)) {                                                   \
        if (hymn_is_int(b)) {                                                        \
            push(H, hymn_new_bool(hymn_as_float(a) compare(double) hymn_as_int(b))); \
        } else if (hymn_is_float(b)) {                                               \
            push(H, hymn_new_bool(hymn_as_float(a) compare hymn_as_float(b)));       \
        } else {                                                                     \
            dereference(H, a);                                                       \
            dereference(H, b);                                                       \
            THROW("Operands must be numbers")                                        \
        }                                                                            \
    } else {                                                                         \
        dereference(H, a);                                                           \
        dereference(H, b);                                                           \
        THROW("Operands must be numbers")                                            \
    }

#define JUMP_COMPARE_OP(compare)                                      \
    HymnValue b = pop(H);                                             \
    HymnValue a = pop(H);                                             \
    bool answer;                                                      \
    if (hymn_is_int(a)) {                                             \
        if (hymn_is_int(b)) {                                         \
            answer = hymn_as_int(a) compare hymn_as_int(b);           \
        } else if (hymn_is_float(b)) {                                \
            answer = (double)hymn_as_int(a) compare hymn_as_float(b); \
        } else {                                                      \
            dereference(H, a);                                        \
            dereference(H, b);                                        \
            THROW("Comparison: Operands must be numbers")             \
        }                                                             \
    } else if (hymn_is_float(a)) {                                    \
        if (hymn_is_int(b)) {                                         \
            answer = hymn_as_float(a) compare(double) hymn_as_int(b); \
        } else if (hymn_is_float(b)) {                                \
            answer = hymn_as_float(a) compare hymn_as_float(b);       \
        } else {                                                      \
            dereference(H, a);                                        \
            dereference(H, b);                                        \
            THROW("Comparison: Operands must be numbers")             \
        }                                                             \
    } else {                                                          \
        dereference(H, a);                                            \
        dereference(H, b);                                            \
        THROW("Comparison: Operands must be numbers")                 \
    }                                                                 \
    uint16_t jump = READ_SHORT(frame);                                \
    if (answer) {                                                     \
        frame->ip += jump;                                            \
    }

static void machine_run(Hymn *H) {
    HymnFrame *frame = current_frame(H);

#define HYMN_DISPATCH goto dispatch

dispatch:
#ifdef HYMN_DEBUG_STACK
    if (H->stack_top != H->stack) {
        HymnString *debug = hymn_new_string("");
        for (HymnValue *i = H->stack; i != H->stack_top; i++) {
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
#ifdef HYMN_DEBUG_TRACE
    {
        HymnString *debug = hymn_new_string("");
        disassemble_instruction(&debug, &frame->func->code, (int)(frame->ip - frame->func->code.instructions));
        printf("%s\n", debug);
        hymn_string_delete(debug);
    }
#endif
    switch (READ_BYTE(frame)) {
    case OP_RETURN: {
        HymnValue result = pop(H);
        H->frame_count--;
        if (H->frame_count == 0 || frame->func->name == NULL) {
            dereference(H, pop(H));
            return;
        }
        while (H->stack_top != frame->stack) {
            dereference(H, pop(H));
        }
        push(H, result);
        frame = current_frame(H);
        HYMN_DISPATCH;
    }
    case OP_POP: {
        dereference(H, pop(H));
        HYMN_DISPATCH;
    }
    case OP_POP_TWO: {
        dereference(H, pop(H));
        dereference(H, pop(H));
        HYMN_DISPATCH;
    }
    case OP_POP_N: {
        int count = READ_BYTE(frame);
        while (count--) {
            dereference(H, pop(H));
        }
        HYMN_DISPATCH;
    }
    case OP_TRUE: {
        push(H, hymn_new_bool(true));
        HYMN_DISPATCH;
    }
    case OP_FALSE: {
        push(H, hymn_new_bool(false));
        HYMN_DISPATCH;
    }
    case OP_NONE: {
        push(H, hymn_new_none());
        HYMN_DISPATCH;
    }
    case OP_CALL: {
        int count = READ_BYTE(frame);
        HymnValue value = peek(H, count + 1);
        frame = call_value(H, value, count);
        if (frame == NULL) return;
        HYMN_DISPATCH;
    }
    case OP_TAIL_CALL: {
        // TODO
        int count = READ_BYTE(frame);
        HymnValue value = peek(H, count + 1);
        frame = call_value(H, value, count);
        if (frame == NULL) return;
        HYMN_DISPATCH;
    }
    case OP_JUMP: {
        uint16_t jump = READ_SHORT(frame);
        frame->ip += jump;
        HYMN_DISPATCH;
    }
    case OP_JUMP_IF_FALSE: {
        HymnValue a = pop(H);
        uint16_t jump = READ_SHORT(frame);
        if (hymn_value_false(a)) {
            frame->ip += jump;
        }
        dereference(H, a);
        HYMN_DISPATCH;
    }
    case OP_JUMP_IF_TRUE: {
        HymnValue a = pop(H);
        uint16_t jump = READ_SHORT(frame);
        if (!hymn_value_false(a)) {
            frame->ip += jump;
        }
        dereference(H, a);
        HYMN_DISPATCH;
    }
    case OP_JUMP_IF_EQUAL: {
        HymnValue b = pop(H);
        HymnValue a = pop(H);
        uint16_t jump = READ_SHORT(frame);
        if (hymn_values_equal(a, b)) {
            frame->ip += jump;
        }
        dereference(H, a);
        dereference(H, b);
        HYMN_DISPATCH;
    }
    case OP_JUMP_IF_NOT_EQUAL: {
        HymnValue b = pop(H);
        HymnValue a = pop(H);
        uint16_t jump = READ_SHORT(frame);
        if (!hymn_values_equal(a, b)) {
            frame->ip += jump;
        }
        dereference(H, a);
        dereference(H, b);
        HYMN_DISPATCH;
    }
    case OP_JUMP_IF_LESS: {
        JUMP_COMPARE_OP(<);
        HYMN_DISPATCH;
    }
    case OP_JUMP_IF_LESS_EQUAL: {
        JUMP_COMPARE_OP(<=);
        HYMN_DISPATCH;
    }
    case OP_JUMP_IF_GREATER: {
        JUMP_COMPARE_OP(>);
        HYMN_DISPATCH;
    }
    case OP_JUMP_IF_GREATER_EQUAL: {
        JUMP_COMPARE_OP(>=);
        HYMN_DISPATCH;
    }
    case OP_LOOP: {
        uint16_t jump = READ_SHORT(frame);
        frame->ip -= jump;
        HYMN_DISPATCH;
    }
    case OP_FOR: {
        uint8_t slot = READ_BYTE(frame);
        HymnValue object = frame->stack[slot];
        H->stack_top += 2;
        if (hymn_is_table(object)) {
            HymnTable *table = hymn_as_table(object);
            HymnTableItem *next = table_next(table, NULL);
            if (next == NULL) {
                frame->stack[slot + 1] = hymn_new_none();
                frame->stack[slot + 2] = hymn_new_none();
                uint16_t jump = READ_SHORT(frame);
                frame->ip += jump;
            } else {
                frame->stack[slot + 1] = hymn_new_string_value(next->key);
                frame->stack[slot + 2] = next->value;
                reference_string(next->key);
                reference(next->value);
                frame->ip += 2;
            }
        } else if (hymn_is_array(object)) {
            HymnArray *array = hymn_as_array(object);
            if (array->length == 0) {
                frame->stack[slot + 1] = hymn_new_none();
                frame->stack[slot + 2] = hymn_new_none();
                uint16_t jump = READ_SHORT(frame);
                frame->ip += jump;
            } else {
                HymnValue item = array->items[0];
                frame->stack[slot + 1] = hymn_new_int(0);
                frame->stack[slot + 2] = item;
                reference(item);
                frame->ip += 2;
            }
        } else {
            frame->stack[slot + 1] = hymn_new_none();
            frame->stack[slot + 2] = hymn_new_none();
            THROW("Loop: Expected table or array")
        }
        HYMN_DISPATCH;
    }
    case OP_FOR_LOOP: {
        uint8_t slot = READ_BYTE(frame);
        HymnValue object = frame->stack[slot];
        uint8_t index = slot + 1;
        uint8_t value = slot + 2;
        if (hymn_is_table(object)) {
            HymnTable *table = hymn_as_table(object);
            HymnObjectString *key = hymn_as_hymn_string(frame->stack[index]);
            HymnTableItem *next = table_next(table, key);
            if (next == NULL) {
                frame->ip += 2;
            } else {
                dereference(H, frame->stack[index]);
                dereference(H, frame->stack[value]);
                frame->stack[index] = hymn_new_string_value(next->key);
                frame->stack[value] = next->value;
                reference_string(next->key);
                reference(next->value);
                uint16_t jump = READ_SHORT(frame);
                frame->ip -= jump;
            }
        } else {
            HymnArray *array = hymn_as_array(object);
            int64_t key = hymn_as_int(frame->stack[index]) + 1;
            if (key >= array->length) {
                frame->ip += 2;
            } else {
                dereference(H, frame->stack[value]);
                HymnValue item = array->items[key];
                frame->stack[index].as.i++;
                frame->stack[value] = item;
                reference(item);
                uint16_t jump = READ_SHORT(frame);
                frame->ip -= jump;
            }
        }
        HYMN_DISPATCH;
    }
    case OP_EQUAL: {
        HymnValue b = pop(H);
        HymnValue a = pop(H);
        push(H, hymn_new_bool(hymn_values_equal(a, b)));
        dereference(H, a);
        dereference(H, b);
        HYMN_DISPATCH;
    }
    case OP_NOT_EQUAL: {
        HymnValue b = pop(H);
        HymnValue a = pop(H);
        push(H, hymn_new_bool(!hymn_values_equal(a, b)));
        dereference(H, a);
        dereference(H, b);
        HYMN_DISPATCH;
    }
    case OP_LESS: {
        COMPARE_OP(<);
        HYMN_DISPATCH;
    }
    case OP_LESS_EQUAL: {
        COMPARE_OP(<=);
        HYMN_DISPATCH;
    }
    case OP_GREATER: {
        COMPARE_OP(>);
        HYMN_DISPATCH;
    }
    case OP_GREATER_EQUAL: {
        COMPARE_OP(>=);
        HYMN_DISPATCH;
    }
    case OP_ADD: {
        HymnValue b = pop(H);
        HymnValue a = pop(H);
        if (hymn_is_none(a)) {
            if (hymn_is_string(b)) {
                machine_push_intern_string(H, value_concat(a, b));
            } else {
                dereference(H, a);
                dereference(H, b);
                THROW("Add: 1st and 2nd values can't be added")
            }
        } else if (hymn_is_bool(a)) {
            if (hymn_is_string(b)) {
                machine_push_intern_string(H, value_concat(a, b));
            } else {
                dereference(H, a);
                dereference(H, b);
                THROW("Add: 1st and 2nd values can't be added")
            }
        } else if (hymn_is_int(a)) {
            if (hymn_is_int(b)) {
                a.as.i += b.as.i;
                push(H, a);
            } else if (hymn_is_float(b)) {
                b.as.f += a.as.i;
                push(H, a);
            } else if (hymn_is_string(b)) {
                machine_push_intern_string(H, value_concat(a, b));
            } else {
                dereference(H, a);
                dereference(H, b);
                THROW("Add: 1st and 2nd values can't be added")
            }
        } else if (hymn_is_float(a)) {
            if (hymn_is_int(b)) {
                a.as.f += b.as.i;
                push(H, a);
            } else if (hymn_is_float(b)) {
                a.as.f += b.as.f;
                push(H, a);
            } else if (hymn_is_string(b)) {
                machine_push_intern_string(H, value_concat(a, b));
            } else {
                dereference(H, a);
                dereference(H, b);
                THROW("Add: 1st and 2nd values can't be added")
            }
        } else if (hymn_is_string(a)) {
            machine_push_intern_string(H, value_concat(a, b));
        } else {
            THROW("Add: 1st and 2nd values can't be added")
        }
        dereference(H, a);
        dereference(H, b);
        HYMN_DISPATCH;
    }
    case OP_ADD_TWO_LOCAL: {
        HymnValue a = frame->stack[READ_BYTE(frame)];
        HymnValue b = frame->stack[READ_BYTE(frame)];
        if (hymn_is_none(a)) {
            if (hymn_is_string(b)) {
                machine_push_intern_string(H, value_concat(a, b));
            } else {
                dereference(H, a);
                dereference(H, b);
                THROW("Add: 1st and 2nd values can't be added")
            }
        } else if (hymn_is_bool(a)) {
            if (hymn_is_string(b)) {
                machine_push_intern_string(H, value_concat(a, b));
            } else {
                dereference(H, a);
                dereference(H, b);
                THROW("Add: 1st and 2nd values can't be added")
            }
        } else if (hymn_is_int(a)) {
            if (hymn_is_int(b)) {
                a.as.i += b.as.i;
                push(H, a);
            } else if (hymn_is_float(b)) {
                b.as.f += a.as.i;
                push(H, a);
            } else if (hymn_is_string(b)) {
                machine_push_intern_string(H, value_concat(a, b));
            } else {
                dereference(H, a);
                dereference(H, b);
                THROW("Add: 1st and 2nd values can't be added")
            }
        } else if (hymn_is_float(a)) {
            if (hymn_is_int(b)) {
                a.as.f += b.as.i;
                push(H, a);
            } else if (hymn_is_float(b)) {
                a.as.f += b.as.f;
                push(H, a);
            } else if (hymn_is_string(b)) {
                machine_push_intern_string(H, value_concat(a, b));
            } else {
                dereference(H, a);
                dereference(H, b);
                THROW("Add: 1st and 2nd values can't be added")
            }
        } else if (hymn_is_string(a)) {
            machine_push_intern_string(H, value_concat(a, b));
        } else {
            THROW("Add: 1st and 2nd values can't be added")
        }
        HYMN_DISPATCH;
    }
    case OP_INCREMENT: {
        HymnValue a = pop(H);
        uint8_t increment = READ_BYTE(frame);
        if (hymn_is_none(a)) {
            dereference(H, a);
            THROW("Increment: 1st and 2nd values can't be added")
        } else if (hymn_is_bool(a)) {
            dereference(H, a);
            THROW("Increment: 1st and 2nd values can't be added")
        } else if (hymn_is_int(a)) {
            a.as.i += (int64_t)increment;
            push(H, a);
        } else if (hymn_is_float(a)) {
            a.as.f += (double)increment;
            push(H, a);
        } else if (hymn_is_string(a)) {
            machine_push_intern_string(H, value_concat(a, hymn_new_int((int64_t)increment)));
        } else {
            THROW("Increment: 1st and 2nd values can't be added")
        }
        dereference(H, a);
        HYMN_DISPATCH;
    }
    case OP_SUBTRACT: {
        ARITHMETIC_OP(-=);
        HYMN_DISPATCH;
    }
    case OP_MULTIPLY: {
        ARITHMETIC_OP(*=);
        HYMN_DISPATCH;
    }
    case OP_DIVIDE: {
        ARITHMETIC_OP(/=);
        HYMN_DISPATCH;
    }
    case OP_MODULO: {
        HymnValue b = pop(H);
        HymnValue a = pop(H);
        if (hymn_is_int(a)) {
            if (hymn_is_int(b)) {
                a.as.i %= b.as.i;
                push(H, a);
            } else {
                dereference(H, a);
                dereference(H, b);
                THROW("Modulo Error: 2nd value must be `Integer`")
            }
        } else {
            dereference(H, a);
            dereference(H, b);
            THROW("Modulo Error: 1st and 2nd values must be `Integer`")
        }
        HYMN_DISPATCH;
    }
    case OP_BIT_NOT: {
        HymnValue value = pop(H);
        if (hymn_is_int(value)) {
            value.as.i = ~value.as.i;
            push(H, value);
        } else {
            dereference(H, value);
            THROW("Bitwise operand must integer")
        }
        HYMN_DISPATCH;
    }
    case OP_BIT_OR: {
        HymnValue b = pop(H);
        HymnValue a = pop(H);
        if (hymn_is_int(a)) {
            if (hymn_is_int(b)) {
                a.as.i |= b.as.i;
                push(H, a);
            } else {
                dereference(H, a);
                dereference(H, b);
                THROW("Bit Or Error: 2nd value must be `Integer`")
            }
        } else {
            dereference(H, a);
            dereference(H, b);
            THROW("Bit Or Error: 1st and 2nd values must be `Integer`")
        }
        HYMN_DISPATCH;
    }
    case OP_BIT_AND: {
        HymnValue b = pop(H);
        HymnValue a = pop(H);
        if (hymn_is_int(a)) {
            if (hymn_is_int(b)) {
                a.as.i &= b.as.i;
                push(H, a);
            } else {
                dereference(H, a);
                dereference(H, b);
                THROW("Bit And Error: 2nd value must be `Integer`")
            }
        } else {
            dereference(H, a);
            dereference(H, b);
            THROW("Bit And Error: 1st and 2nd values must be `Integer`")
        }
        HYMN_DISPATCH;
    }
    case OP_BIT_XOR: {
        HymnValue b = pop(H);
        HymnValue a = pop(H);
        if (hymn_is_int(a)) {
            if (hymn_is_int(b)) {
                a.as.i ^= b.as.i;
                push(H, a);
            } else {
                dereference(H, a);
                dereference(H, b);
                THROW("Bit Xor Error: 2nd value must be `Integer`")
            }
        } else {
            dereference(H, a);
            dereference(H, b);
            THROW("Bit Xor Error: 1st and 2nd values must be `Integer`")
        }
        HYMN_DISPATCH;
    }
    case OP_BIT_LEFT_SHIFT: {
        HymnValue b = pop(H);
        HymnValue a = pop(H);
        if (hymn_is_int(a)) {
            if (hymn_is_int(b)) {
                a.as.i <<= b.as.i;
                push(H, a);
            } else {
                dereference(H, a);
                dereference(H, b);
                THROW("Bit Left Shift Error: 2nd value must be `Integer`")
            }
        } else {
            dereference(H, a);
            dereference(H, b);
            THROW("Bit Left Shift Error: 1st and 2nd values must be `Integer`")
        }
        HYMN_DISPATCH;
    }
    case OP_BIT_RIGHT_SHIFT: {
        HymnValue b = pop(H);
        HymnValue a = pop(H);
        if (hymn_is_int(a)) {
            if (hymn_is_int(b)) {
                a.as.i >>= b.as.i;
                push(H, a);
            } else {
                dereference(H, a);
                dereference(H, b);
                THROW("Bit Right Shift Error: 2nd value must be `Integer`")
            }
        } else {
            dereference(H, a);
            dereference(H, b);
            THROW("Bit Right Shift Error: 1st and 2nd values must be `Integer`")
        }
        HYMN_DISPATCH;
    }
    case OP_NEGATE: {
        HymnValue value = pop(H);
        if (hymn_is_int(value)) {
            value.as.i = -value.as.i;
        } else if (hymn_is_float(value)) {
            value.as.f = -value.as.f;
        } else {
            dereference(H, value);
            THROW("Negate: Must be a number")
        }
        push(H, value);
        HYMN_DISPATCH;
    }
    case OP_NOT: {
        HymnValue value = pop(H);
        if (hymn_is_bool(value)) {
            value.as.b = !value.as.b;
        } else {
            dereference(H, value);
            THROW("Not: Operand must be a boolean")
        }
        push(H, value);
        HYMN_DISPATCH;
    }
    case OP_CONSTANT: {
        HymnValue constant = READ_CONSTANT(frame);
        switch (constant.is) {
        case HYMN_VALUE_ARRAY: {
            constant = hymn_new_array_value(hymn_new_array(0));
            break;
        }
        case HYMN_VALUE_TABLE: {
            constant = hymn_new_table_value(hymn_new_table());
            break;
        }
        default:
            break;
        }
        reference(constant);
        push(H, constant);
        HYMN_DISPATCH;
    }
    case OP_DEFINE_GLOBAL: {
        HymnObjectString *name = hymn_as_hymn_string(READ_CONSTANT(frame));
        HymnValue value = pop(H);
        HymnValue previous = table_put(&H->globals, name, value);
        if (!hymn_is_undefined(previous)) {
            dereference(H, previous);
            THROW("Global `%s` was previously defined.", name->string)
        }
        HYMN_DISPATCH;
    }
    case OP_SET_GLOBAL: {
        HymnObjectString *name = hymn_as_hymn_string(READ_CONSTANT(frame));
        HymnValue value = peek(H, 1);
        HymnValue previous = table_put(&H->globals, name, value);
        if (hymn_is_undefined(previous)) {
            THROW("Undefined variable `%s`.", name->string)
        } else {
            dereference(H, previous);
        }
        reference(value);
        HYMN_DISPATCH;
    }
    case OP_GET_GLOBAL: {
        HymnObjectString *name = hymn_as_hymn_string(READ_CONSTANT(frame));
        HymnValue get = table_get(&H->globals, name);
        if (hymn_is_undefined(get)) {
            THROW("Undefined variable `%s`.", name->string)
        }
        reference(get);
        push(H, get);
        HYMN_DISPATCH;
    }
    case OP_SET_LOCAL: {
        uint8_t slot = READ_BYTE(frame);
        HymnValue value = peek(H, 1);
        dereference(H, frame->stack[slot]);
        frame->stack[slot] = value;
        reference(value);
        HYMN_DISPATCH;
    }
    case OP_GET_LOCAL: {
        uint8_t slot = READ_BYTE(frame);
        HymnValue value = frame->stack[slot];
        reference(value);
        push(H, value);
        HYMN_DISPATCH;
    }
    case OP_GET_TWO_LOCAL: {
        uint8_t slot_a = READ_BYTE(frame);
        uint8_t slot_b = READ_BYTE(frame);
        HymnValue value_a = frame->stack[slot_a];
        HymnValue value_b = frame->stack[slot_b];
        reference(value_a);
        reference(value_b);
        push(H, value_a);
        push(H, value_b);
        HYMN_DISPATCH;
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
        push(H, value);
        HYMN_DISPATCH;
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
        HYMN_DISPATCH;
    }
    case OP_SET_PROPERTY: {
        HymnValue value = pop(H);
        HymnValue table_value = pop(H);
        if (!hymn_is_table(table_value)) {
            const char *is = value_name(table_value.is);
            dereference(H, value);
            dereference(H, table_value);
            THROW("Set Property: Expected `Table` but was `%s`", is)
        }
        HymnTable *table = hymn_as_table(table_value);
        HymnObjectString *name = hymn_as_hymn_string(READ_CONSTANT(frame));
        HymnValue previous = table_put(table, name, value);
        if (hymn_is_undefined(previous)) {
            reference_string(name);
        } else {
            dereference(H, previous);
        }
        push(H, value);
        reference(value);
        dereference(H, table_value);
        HYMN_DISPATCH;
    }
    case OP_GET_PROPERTY: {
        HymnValue v = pop(H);
        if (!hymn_is_table(v)) {
            const char *is = value_name(v.is);
            dereference(H, v);
            THROW("Get Property: Requires `Table` but was `%s`", is)
        }
        HymnTable *table = hymn_as_table(v);
        HymnObjectString *name = hymn_as_hymn_string(READ_CONSTANT(frame));
        HymnValue g = table_get(table, name);
        if (hymn_is_undefined(g)) {
            g.is = HYMN_VALUE_NONE;
        } else {
            reference(g);
        }
        dereference(H, v);
        push(H, g);
        HYMN_DISPATCH;
    }
    case OP_SET_DYNAMIC: {
        HymnValue value = pop(H);
        HymnValue property = pop(H);
        HymnValue object = pop(H);
        if (hymn_is_array(object)) {
            if (!hymn_is_int(property)) {
                dereference(H, value);
                dereference(H, property);
                dereference(H, object);
                THROW("Dynamic Set: `Integer` required to set `Array` index")
            }
            HymnArray *array = hymn_as_array(object);
            int64_t size = array->length;
            int64_t index = hymn_as_int(property);
            if (index > size) {
                dereference(H, value);
                dereference(H, property);
                dereference(H, object);
                THROW("Dynamic Set: Array index out of bounds %d > %d.", index, size)
            }
            if (index < 0) {
                index = size + index;
                if (index < 0) {
                    dereference(H, value);
                    dereference(H, property);
                    dereference(H, object);
                    THROW("Dynamic Set: Array index out of bounds %d.", index)
                }
            }
            if (index == size) {
                array_push(array, value);
            } else {
                dereference(H, array->items[index]);
                array->items[index] = value;
            }
        } else if (hymn_is_table(object)) {
            if (!hymn_is_string(property)) {
                const char *is = value_name(property.is);
                dereference(H, value);
                dereference(H, property);
                dereference(H, object);
                THROW("Dynamic Set: Requires `String` to set `Table` property, but was `%s`.", is)
            }
            HymnTable *table = hymn_as_table(object);
            HymnObjectString *name = hymn_as_hymn_string(property);
            HymnValue previous = table_put(table, name, value);
            if (hymn_is_undefined(previous)) {
                reference_string(name);
            } else {
                dereference(H, previous);
            }
        } else {
            const char *is = value_name(object.is);
            dereference(H, value);
            dereference(H, property);
            dereference(H, object);
            THROW("Dynamic Set: 1st argument requires `Array` or `Table`, but was `%s`.", is)
        }
        push(H, value);
        dereference(H, object);
        reference(value);
        HYMN_DISPATCH;
    }
    case OP_GET_DYNAMIC: {
        HymnValue i = pop(H);
        HymnValue v = pop(H);
        switch (v.is) {
        case HYMN_VALUE_STRING: {
            if (!hymn_is_int(i)) {
                const char *is = value_name(v.is);
                dereference(H, i);
                dereference(H, v);
                THROW("Get Dynamic: Requires `Integer` to get string character from index, but was `%s`", is)
            }
            HymnString *string = hymn_as_string(v);
            int64_t size = (int64_t)hymn_string_len(string);
            int64_t index = hymn_as_int(i);
            if (index >= size) {
                dereference(H, i);
                dereference(H, v);
                THROW("String index out of bounds %d >= %d", index, size)
            }
            if (index < 0) {
                index = size + index;
                if (index < 0) {
                    dereference(H, i);
                    dereference(H, v);
                    THROW("String index out of bounds %d", index)
                }
            }
            char c = string[index];
            machine_push_intern_string(H, char_to_string(c));
            dereference(H, v);
            break;
        }
        case HYMN_VALUE_ARRAY: {
            if (!hymn_is_int(i)) {
                dereference(H, i);
                dereference(H, v);
                THROW("Integer required to get array index")
            }
            HymnArray *array = hymn_as_array(v);
            int64_t size = array->length;
            int64_t index = hymn_as_int(i);
            if (index >= size) {
                dereference(H, i);
                dereference(H, v);
                THROW("Array index out of bounds %d >= %d", index, size)
            }
            if (index < 0) {
                index = size + index;
                if (index < 0) {
                    dereference(H, i);
                    dereference(H, v);
                    THROW("Array index out of bounds %d", index)
                }
            }
            HymnValue g = array_get(array, index);
            reference(g);
            push(H, g);
            dereference(H, v);
            break;
        }
        case HYMN_VALUE_TABLE: {
            if (!hymn_is_string(i)) {
                const char *is = value_name(i.is);
                dereference(H, i);
                dereference(H, v);
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
            push(H, g);
            dereference(H, i);
            dereference(H, v);
            break;
        }
        default: {
            const char *is = value_name(v.is);
            dereference(H, i);
            dereference(H, v);
            THROW("Dynamic Get: 1st argument requires `Array` or `Table`, but was `%s`.", is)
        }
        }
        HYMN_DISPATCH;
    }
    case OP_LEN: {
        HymnValue value = pop(H);
        switch (value.is) {
        case HYMN_VALUE_STRING: {
            int64_t len = (int64_t)hymn_string_len(hymn_as_string(value));
            push(H, hymn_new_int(len));
            break;
        }
        case HYMN_VALUE_ARRAY: {
            int64_t len = hymn_as_array(value)->length;
            push(H, hymn_new_int(len));
            break;
        }
        case HYMN_VALUE_TABLE: {
            int64_t len = (int64_t)hymn_as_table(value)->size;
            push(H, hymn_new_int(len));
            break;
        }
        default:
            dereference(H, value);
            THROW("Len: Expected `Array` or `Table`")
        }
        dereference(H, value);
        HYMN_DISPATCH;
    }
    case OP_ARRAY_POP: {
        HymnValue a = pop(H);
        if (!hymn_is_array(a)) {
            const char *is = value_name(a.is);
            dereference(H, a);
            THROW("Pop: Expected `Array` for 1st argument, but was `%s`.", is)
        } else {
            HymnValue value = array_pop(hymn_as_array(a));
            push(H, value);
            dereference(H, a);
        }
        HYMN_DISPATCH;
    }
    case OP_ARRAY_PUSH: {
        HymnValue value = pop(H);
        HymnValue array = pop(H);
        if (!hymn_is_array(array)) {
            const char *is = value_name(value.is);
            dereference(H, array);
            dereference(H, value);
            THROW("Push: Expected `Array` for 1st argument, but was `%s`.", is)
        } else {
            array_push(hymn_as_array(array), value);
            push(H, value);
            reference(value);
            dereference(H, array);
        }
        HYMN_DISPATCH;
    }
    case OP_ARRAY_INSERT: {
        HymnValue p = pop(H);
        HymnValue i = pop(H);
        HymnValue v = pop(H);
        if (hymn_is_array(v)) {
            if (!hymn_is_int(i)) {
                const char *is = value_name(i.is);
                dereference(H, p);
                dereference(H, i);
                dereference(H, v);
                THROW("Insert Function: Expected `Integer` for 2nd argument, but was `%s`.", is)
            }
            HymnArray *array = hymn_as_array(v);
            int64_t size = array->length;
            int64_t index = hymn_as_int(i);
            if (index > size) {
                dereference(H, p);
                dereference(H, i);
                dereference(H, v);
                THROW("Insert Function: Array index out of bounds: %d > %d", index, size)
            }
            if (index < 0) {
                index = size + index;
                if (index < 0) {
                    dereference(H, p);
                    dereference(H, i);
                    dereference(H, v);
                    THROW("Insert Function: Array index less than zero: %d", index)
                }
            }
            if (index == size) {
                array_push(array, p);
            } else {
                array_insert(array, index, p);
            }
            push(H, p);
            reference(p);
            dereference(H, v);
        } else {
            const char *is = value_name(v.is);
            dereference(H, p);
            dereference(H, i);
            dereference(H, v);
            THROW("Insert Function: Expected `Array` for 1st argument, but was `%s`.", is)
        }
        HYMN_DISPATCH;
    }
    case OP_DELETE: {
        HymnValue i = pop(H);
        HymnValue v = pop(H);
        if (hymn_is_array(v)) {
            if (!hymn_is_int(i)) {
                dereference(H, i);
                dereference(H, v);
                THROW("Integer required to delete from array")
            }
            HymnArray *array = hymn_as_array(v);
            int64_t size = array->length;
            int64_t index = hymn_as_int(i);
            if (index >= size) {
                dereference(H, i);
                dereference(H, v);
                THROW("Array index out of bounds %d > %d.", index, size)
            }
            if (index < 0) {
                index = size + index;
                if (index < 0) {
                    dereference(H, i);
                    dereference(H, v);
                    THROW("Array index out of bounds %d.", index)
                }
            }
            HymnValue value = array_remove_index(array, index);
            push(H, value);
            dereference(H, v);
        } else if (hymn_is_table(v)) {
            if (!hymn_is_string(i)) {
                dereference(H, i);
                dereference(H, v);
                THROW("String required to delete from table")
            }
            HymnTable *table = hymn_as_table(v);
            HymnObjectString *name = hymn_as_hymn_string(i);
            HymnValue value = table_remove(table, name);
            if (hymn_is_undefined(value)) {
                value.is = HYMN_VALUE_NONE;
            } else {
                dereference_string(H, name);
            }
            push(H, value);
            dereference(H, v);
            dereference_string(H, name);
        } else {
            dereference(H, i);
            dereference(H, v);
            THROW("Expected array or table for `delete` function")
        }
        HYMN_DISPATCH;
    }
    case OP_COPY: {
        HymnValue value = pop(H);
        switch (value.is) {
        case HYMN_VALUE_NONE:
        case HYMN_VALUE_BOOL:
        case HYMN_VALUE_INTEGER:
        case HYMN_VALUE_FLOAT:
        case HYMN_VALUE_STRING:
        case HYMN_VALUE_FUNC:
        case HYMN_VALUE_FUNC_NATIVE:
            push(H, value);
            break;
        case HYMN_VALUE_ARRAY: {
            HymnArray *copy = new_array_copy(hymn_as_array(value));
            HymnValue new = hymn_new_array_value(copy);
            push(H, new);
            reference(new);
            dereference(H, value);
            break;
        }
        case HYMN_VALUE_TABLE: {
            HymnTable *copy = new_table_copy(hymn_as_table(value));
            HymnValue new = hymn_new_table_value(copy);
            push(H, new);
            reference(new);
            dereference(H, value);
            break;
        }
        default:
            push(H, hymn_new_none());
        }
        HYMN_DISPATCH;
    }
    case OP_SLICE: {
        HymnValue b = pop(H);
        HymnValue a = pop(H);
        HymnValue v = pop(H);
        if (!hymn_is_int(a)) {
            dereference(H, a);
            dereference(H, b);
            dereference(H, v);
            THROW("Integer required for slice expression")
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
                dereference(H, a);
                dereference(H, b);
                dereference(H, v);
                THROW("Integer required for slice expression")
            }
            if (end > size) {
                dereference(H, a);
                dereference(H, b);
                dereference(H, v);
                THROW("String index out of bounds %d > %d", end, size)
            }
            if (end < 0) {
                end = size + end;
                if (end < 0) {
                    dereference(H, a);
                    dereference(H, b);
                    dereference(H, v);
                    THROW("String index out of bounds %d", end)
                }
            }
            if (start >= end) {
                dereference(H, a);
                dereference(H, b);
                dereference(H, v);
                THROW("String start index %d > end index %d", start, end)
            }
            HymnString *sub = hymn_substring(original, (size_t)start, (size_t)end);
            machine_push_intern_string(H, sub);
        } else if (hymn_is_array(v)) {
            HymnArray *array = hymn_as_array(v);
            int64_t size = array->length;
            int64_t end;
            if (hymn_is_int(b)) {
                end = hymn_as_int(b);
            } else if (hymn_is_none(b)) {
                end = size;
            } else {
                dereference(H, a);
                dereference(H, b);
                dereference(H, v);
                THROW("Integer required for slice expression")
            }
            if (end > size) {
                dereference(H, a);
                dereference(H, b);
                dereference(H, v);
                THROW("Array index out of bounds %d > %d", end, size)
            }
            if (end < 0) {
                end = size + end;
                if (end < 0) {
                    dereference(H, a);
                    dereference(H, b);
                    dereference(H, v);
                    THROW("Array index out of bounds %d", end)
                }
            }
            if (start >= end) {
                dereference(H, a);
                dereference(H, b);
                dereference(H, v);
                THROW("Array start index %d >= end index %d", start, end)
            }
            HymnArray *copy = new_array_slice(array, start, end);
            HymnValue new = hymn_new_array_value(copy);
            reference(new);
            push(H, new);
        } else {
            dereference(H, a);
            dereference(H, b);
            dereference(H, v);
            THROW("Expected string or array for `slice` function")
        }
        dereference(H, v);
        HYMN_DISPATCH;
    }
    case OP_CLEAR: {
        HymnValue value = pop(H);
        switch (value.is) {
        case HYMN_VALUE_BOOL:
            push(H, hymn_new_bool(false));
            break;
        case HYMN_VALUE_INTEGER:
            push(H, hymn_new_int(0));
            break;
        case HYMN_VALUE_FLOAT:
            push(H, hymn_new_float(0.0f));
            break;
        case HYMN_VALUE_STRING:
            machine_push_intern_string(H, hymn_new_string(""));
            break;
        case HYMN_VALUE_ARRAY: {
            HymnArray *array = hymn_as_array(value);
            array_clear(H, array);
            push(H, value);
            break;
        }
        case HYMN_VALUE_TABLE: {
            HymnTable *table = hymn_as_table(value);
            table_clear(H, table);
            push(H, value);
            break;
        }
        case HYMN_VALUE_UNDEFINED:
        case HYMN_VALUE_NONE:
        case HYMN_VALUE_FUNC:
        case HYMN_VALUE_FUNC_NATIVE:
        case HYMN_VALUE_POINTER:
            push(H, hymn_new_none());
            break;
        }
        HYMN_DISPATCH;
    }
    case OP_KEYS: {
        HymnValue value = pop(H);
        if (!hymn_is_table(value)) {
            dereference(H, value);
            THROW("Expected table for `keys` function")
        } else {
            HymnTable *table = hymn_as_table(value);
            HymnArray *array = table_keys(table);
            HymnValue keys = hymn_new_array_value(array);
            reference(keys);
            push(H, keys);
            dereference(H, value);
        }
        HYMN_DISPATCH;
    }
    case OP_INDEX: {
        HymnValue b = pop(H);
        HymnValue a = pop(H);
        switch (a.is) {
        case HYMN_VALUE_STRING: {
            if (!hymn_is_string(b)) {
                dereference(H, a);
                dereference(H, b);
                THROW("Expected substring for 2nd argument of `index` function")
            }
            size_t index = 0;
            bool found = string_find(hymn_as_string(a), hymn_as_string(b), &index);
            if (found) {
                push(H, hymn_new_int((int64_t)index));
            } else {
                push(H, hymn_new_int(-1));
            }
            dereference(H, a);
            dereference(H, b);
            break;
        }
        case HYMN_VALUE_ARRAY:
            push(H, hymn_new_int(array_index_of(hymn_as_array(a), b)));
            dereference(H, a);
            dereference(H, b);
            break;
        case HYMN_VALUE_TABLE: {
            HymnObjectString *key = table_key_of(hymn_as_table(a), b);
            if (key == NULL) {
                push(H, hymn_new_none());
            } else {
                push(H, hymn_new_string_value(key));
            }
            dereference(H, a);
            dereference(H, b);
            break;
        }
        default:
            dereference(H, a);
            dereference(H, b);
            THROW("Index function expects `String`, `Array`, or `Table`")
        }
        HYMN_DISPATCH;
    }
    case OP_TYPE: {
        HymnValue value = pop(H);
        switch (value.is) {
        case HYMN_VALUE_UNDEFINED:
        case HYMN_VALUE_NONE:
            machine_push_intern_string(H, hymn_new_string(STRING_NONE_TYPE));
            break;
        case HYMN_VALUE_BOOL:
            machine_push_intern_string(H, hymn_new_string(STRING_BOOL));
            break;
        case HYMN_VALUE_INTEGER:
            machine_push_intern_string(H, hymn_new_string(STRING_INTEGER));
            break;
        case HYMN_VALUE_FLOAT:
            machine_push_intern_string(H, hymn_new_string(STRING_FLOAT));
            break;
        case HYMN_VALUE_STRING:
            machine_push_intern_string(H, hymn_new_string(STRING_STRING));
            dereference(H, value);
            break;
        case HYMN_VALUE_ARRAY:
            machine_push_intern_string(H, hymn_new_string(STRING_ARRAY));
            dereference(H, value);
            break;
        case HYMN_VALUE_TABLE:
            machine_push_intern_string(H, hymn_new_string(STRING_TABLE));
            dereference(H, value);
            break;
        case HYMN_VALUE_FUNC:
            machine_push_intern_string(H, hymn_new_string(STRING_FUNC));
            dereference(H, value);
            break;
        case HYMN_VALUE_FUNC_NATIVE:
            machine_push_intern_string(H, hymn_new_string(STRING_NATIVE));
            break;
        case HYMN_VALUE_POINTER:
            machine_push_intern_string(H, hymn_new_string(STRING_POINTER));
            break;
        }
        HYMN_DISPATCH;
    }
    case OP_TO_INTEGER: {
        HymnValue value = pop(H);
        if (hymn_is_int(value)) {
            push(H, value);
        } else if (hymn_is_float(value)) {
            int64_t number = (int64_t)hymn_as_float(value);
            push(H, hymn_new_int(number));
        } else if (hymn_is_string(value)) {
            HymnString *string = hymn_as_string(value);
            char *end = NULL;
            double number = string_to_double(string, &end);
            if (string == end) {
                push(H, hymn_new_none());
            } else {
                push(H, hymn_new_int((int64_t)number));
            }
            dereference(H, value);
        } else {
            dereference(H, value);
            THROW("Can't cast to an integer")
        }
        HYMN_DISPATCH;
    }
    case OP_TO_FLOAT: {
        HymnValue value = pop(H);
        if (hymn_is_int(value)) {
            double number = (double)hymn_as_int(value);
            push(H, hymn_new_float(number));
        } else if (hymn_is_float(value)) {
            push(H, value);
        } else if (hymn_is_string(value)) {
            HymnString *string = hymn_as_string(value);
            char *end = NULL;
            double number = string_to_double(string, &end);
            if (string == end) {
                push(H, hymn_new_none());
            } else {
                push(H, hymn_new_float(number));
            }
            dereference(H, value);
        } else {
            dereference(H, value);
            THROW("Can't cast to a float")
        }
        HYMN_DISPATCH;
    }
    case OP_TO_STRING: {
        HymnValue value = pop(H);
        machine_push_intern_string(H, value_to_string(value));
        dereference(H, value);
        HYMN_DISPATCH;
    }
    case OP_PRINT: {
        HymnValue value = pop(H);
        HymnString *string = value_to_string(value);
        H->print("%s\n", string);
        hymn_string_delete(string);
        dereference(H, value);
        HYMN_DISPATCH;
    }
    case OP_THROW: {
        frame = machine_exception(H);
        if (frame == NULL) return;
        HYMN_DISPATCH;
    }
    case OP_DUPLICATE: {
        HymnValue top = peek(H, 1);
        push(H, top);
        reference(top);
        HYMN_DISPATCH;
    }
    case OP_USE: {
        HymnValue file = pop(H);
        if (hymn_is_string(file)) {
            frame = machine_import(H, hymn_as_hymn_string(file));
            dereference(H, file);
            if (frame == NULL) return;
        } else {
            dereference(H, file);
            THROW("Expected `String` for `use` command")
        }
        HYMN_DISPATCH;
    }
    default:
        UNREACHABLE();
    }
}

static char *machine_interpret(Hymn *H) {
    machine_run(H);
    char *error = NULL;
    if (H->error) {
        error = string_to_chars(H->error);
        hymn_string_delete(H->error);
        H->error = NULL;
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
    Hymn *H = hymn_calloc(1, sizeof(Hymn));
    reset_stack(H);

    set_init(&H->strings);
    table_init(&H->globals);

    HymnObjectString *search_this = machine_intern_string(H, hymn_new_string("<parent>" PATH_SEP_STRING "<path>.hm"));
    reference_string(search_this);

    HymnObjectString *search_relative = machine_intern_string(H, hymn_new_string("." PATH_SEP_STRING "<path>.hm"));
    reference_string(search_relative);

    HymnObjectString *search_libs = machine_intern_string(H, hymn_new_string("." PATH_SEP_STRING "libs" PATH_SEP_STRING "<path>.hm"));
    reference_string(search_libs);

    HymnObjectString *paths = machine_intern_string(H, hymn_new_string("__paths"));
    reference_string(paths);

    H->paths = hymn_new_array(3);
    H->paths->items[0] = hymn_new_string_value(search_this);
    H->paths->items[1] = hymn_new_string_value(search_relative);
    H->paths->items[2] = hymn_new_string_value(search_libs);

    HymnValue paths_value = hymn_new_array_value(H->paths);
    table_put(&H->globals, paths, paths_value);
    reference_string(paths);
    reference(paths_value);

    HymnObjectString *imports = machine_intern_string(H, hymn_new_string("__imports"));
    reference_string(imports);

    H->imports = hymn_new_table();
    HymnValue imports_value = hymn_new_table_value(H->imports);
    table_put(&H->globals, imports, imports_value);
    reference_string(imports);
    reference(imports_value);

    H->print = print_stdout;

    return H;
}

void hymn_delete(Hymn *H) {
    {
        HymnTable *globals = &H->globals;
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
                    globals->size--;
                } else {
                    previous = item;
                }
                item = next;
            }
        }
        table_release(H, &H->globals);
        assert(H->globals.size == 0);
    }

    HymnSet *strings = &H->strings;
    {
        unsigned int bins = strings->bins;
        for (unsigned int i = 0; i < bins; i++) {
            HymnSetItem *item = strings->items[i];
            while (item != NULL) {
                HymnSetItem *next = item->next;
                dereference_string(H, item->string);
                item = next;
            }
        }
    }
#ifdef HYMN_DEBUG_MEMORY
    {
        unsigned int bins = strings->bins;
        for (unsigned int i = 0; i < bins; i++) {
            HymnSetItem *item = strings->items[i];
            while (item != NULL) {
                fprintf(stderr, "ENDING STRING REFERENCE: %s\n", item->string->string);
                item = item->next;
            }
        }
    }
#endif
    assert(strings->size == 0);
    free(strings->items);

    hymn_string_delete(H->error);

    free(H);
}

void hymn_add_function(Hymn *H, const char *name, HymnNativeCall func) {
    HymnObjectString *string = machine_intern_string(H, hymn_new_string(name));
    HymnString *string_name = string_copy(string->string);
    HymnNativeFunction *value = new_native_function(string_name, func);
    HymnValue previous = table_put(&H->globals, string, hymn_new_native(value));
    if (hymn_is_undefined(previous)) {
        reference_string(string);
    } else {
        dereference(H, previous);
    }
}

void hymn_add_pointer(Hymn *H, const char *name, void *pointer) {
    HymnObjectString *string = machine_intern_string(H, hymn_new_string(name));
    HymnValue previous = table_put(&H->globals, string, hymn_new_pointer(pointer));
    if (hymn_is_undefined(previous)) {
        reference_string(string);
    } else {
        dereference(H, previous);
    }
}

char *hymn_debug(Hymn *H, const char *script, const char *source) {
    HymnString *code = NULL;
    if (source == NULL) {
        code = hymn_read_file(script);
    } else {
        code = hymn_new_string(source);
    }

    struct CompileReturn result = compile(H, script, code);

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

char *hymn_call(Hymn *H, const char *name, int arguments) {

    // TODO: NEED A NON-INTERN STRING LOOKUP FOR GLOBALS

    HymnObjectString *string = machine_intern_string(H, hymn_new_string(name));

    HymnValue function = table_get(&H->globals, string);
    if (hymn_is_undefined(function)) {
        dereference_string(H, string);
        return NULL;
    }
    reference(function);

    push(H, function);
    call_value(H, function, arguments);

    char *error = machine_interpret(H);
    if (error) {
        return error;
    }

    assert(H->stack_top == H->stack);
    reset_stack(H);

    return NULL;
}

char *hymn_run(Hymn *H, const char *script, const char *source) {
    struct CompileReturn result = compile(H, script, source);

    HymnFunction *main = result.func;

    char *error = result.error;
    if (error) {
        function_delete(main);
        return error;
    }

    HymnValue function = hymn_new_func_value(main);
    reference(function);

    push(H, function);
    call(H, main, 0);

    error = machine_interpret(H);
    if (error) {
        return error;
    }

    assert(H->stack_top == H->stack);
    reset_stack(H);

    return NULL;
}

char *hymn_do(Hymn *H, const char *source) {
    return hymn_run(H, NULL, source);
}

char *hymn_read(Hymn *H, const char *script) {
    HymnString *source = hymn_read_file(script);
    char *error = hymn_run(H, script, source);
    hymn_string_delete(source);
    return error;
}
