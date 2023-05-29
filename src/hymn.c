/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "hymn.h"

void *hymn_malloc(size_t size) {
    void *mem = malloc(size);
    if (mem) {
        return mem;
    }
    fprintf(stderr, "malloc failed.\n");
    exit(1);
}

void *hymn_calloc(size_t count, size_t size) {
    void *mem = calloc(count, size);
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

void *hymn_malloc_int(int count, size_t size) {
    if (count < 0) {
        fprintf(stderr, "malloc negative count.\n");
        exit(1);
    }
    void *mem = malloc((size_t)count * size);
    if (mem) {
        return mem;
    }
    fprintf(stderr, "malloc failed.\n");
    exit(1);
}

void *hymn_calloc_int(int count, size_t size) {
    if (count < 0) {
        fprintf(stderr, "calloc negative count.\n");
        exit(1);
    }
    void *mem = calloc((size_t)count, size);
    if (mem) {
        return mem;
    }
    fprintf(stderr, "calloc failed.\n");
    exit(1);
}

void *hymn_realloc_int(void *mem, int count, size_t size) {
    if (count < 0) {
        fprintf(stderr, "realloc negative count.\n");
        exit(1);
    }
    mem = realloc(mem, (size_t)count * size);
    if (mem) {
        return mem;
    }
    fprintf(stderr, "realloc failed.\n");
    exit(1);
}

static void hymn_mem_copy(void *dest, void *src, int count, size_t size) {
    if (count < 0) {
        fprintf(stderr, "memcpy negative count.\n");
        exit(1);
    }
    memcpy(dest, src, (size_t)count * size);
}

static HymnStringHead *string_head_init(size_t length, size_t capacity) {
    size_t memory = sizeof(HymnStringHead) + capacity + 1;
    HymnStringHead *head = (HymnStringHead *)hymn_malloc(memory);
    memset(head, 0, memory);
    head->length = length;
    head->capacity = capacity;
    return head;
}

HymnString *hymn_new_string_with_capacity(size_t capacity) {
    HymnStringHead *head = string_head_init(0, capacity);
    return (HymnString *)(head + 1);
}

HymnString *hymn_new_string_with_length(const char *chars, size_t length) {
    HymnStringHead *head = string_head_init(length, length);
    char *string = (char *)(head + 1);
    memcpy(string, chars, length);
    return (HymnString *)string;
}

HymnString *hymn_new_empty_string(size_t length) {
    HymnStringHead *head = string_head_init(length, length);
    char *string = (char *)(head + 1);
    return (HymnString *)string;
}

HymnString *hymn_new_string(const char *chars) {
    size_t length = strlen(chars);
    return hymn_new_string_with_length(chars, length);
}

HymnString *hymn_substring(const char *init, size_t start, size_t end) {
    size_t length = end - start;
    HymnStringHead *head = string_head_init(length, length);
    char *string = (char *)(head + 1);
    memcpy(string, &init[start], length);
    string[length] = '\0';
    return (HymnString *)string;
}

static bool space(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

void hymn_string_trim(HymnString *string) {
    size_t len = hymn_string_len(string);
    size_t start = 0;
    while (start < len) {
        char c = string[start];
        if (!space(c)) {
            break;
        }
        start++;
    }
    if (start == len) {
        hymn_string_zero(string);
    } else {
        size_t end = len - 1;
        while (end > start) {
            char c = string[end];
            if (!space(c)) {
                break;
            }
            end--;
        }
        end++;
        size_t offset = start;
        size_t size = end - start;
        for (size_t i = 0; i < size; i++) {
            string[i] = string[offset++];
        }
        HymnStringHead *head = hymn_string_head(string);
        head->length = size;
        string[size] = '\0';
    }
}

HymnString *hymn_string_copy(HymnString *string) {
    HymnStringHead *head = hymn_string_head(string);
    return hymn_new_string_with_length(string, head->length);
}

void hymn_string_delete(HymnString *string) {
    if (string == NULL) {
        return;
    }
    free((char *)string - sizeof(HymnStringHead));
}

void hymn_string_zero(HymnString *string) {
    HymnStringHead *head = hymn_string_head(string);
    head->length = 0;
    string[0] = '\0';
}

static HymnStringHead *string_resize(HymnStringHead *head, size_t capacity) {
    size_t memory = sizeof(HymnStringHead) + capacity + 1;
    HymnStringHead *new = hymn_realloc(head, memory);
    new->capacity = capacity;
    return new;
}

HymnString *hymn_string_append(HymnString *string, const char *b) {
    HymnStringHead *head = hymn_string_head(string);
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

HymnString *hymn_string_append_char(HymnString *string, const char b) {
    HymnStringHead *head = hymn_string_head(string);
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

HymnString *hymn_string_append_substring(HymnString *string, const char *b, size_t start, size_t end) {
    HymnStringHead *head = hymn_string_head(string);
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

static bool string_starts_with(const char *t, const char *s) {
    size_t tlen = strlen(t);
    size_t slen = strlen(s);
    return tlen < slen ? false : memcmp(t, s, slen) == 0;
}

bool hymn_string_starts_with(HymnString *s, const char *using) {
    size_t slen = hymn_string_len(s);
    size_t ulen = strlen(using);
    return slen < ulen ? false : memcmp(s, using, ulen) == 0;
}

static bool string_find(HymnString *string, HymnString *sub, size_t *out) {
    HymnStringHead *head = hymn_string_head(string);
    HymnStringHead *head_sub = hymn_string_head(sub);
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
            if (sub[k] != string[i + k]) {
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

HymnString *hymn_string_replace(HymnString *string, const char *find, const char *replace) {
    HymnStringHead *head = hymn_string_head(string);
    size_t len = head->length;
    size_t len_sub = strlen(find);
    if (len == 0) return hymn_new_string("");
    if (len_sub == 0 || len_sub > len) return hymn_string_copy(string);
    HymnString *out = hymn_new_string_with_capacity(len);
    size_t end = len - len_sub + 1;
    size_t pos = 0;
    for (size_t i = 0; i < end; i++) {
        bool match = true;
        for (size_t k = 0; k < len_sub; k++) {
            if (find[k] != string[i + k]) {
                match = false;
                break;
            }
        }
        if (match) {
            out = hymn_string_append_substring(out, string, pos, i);
            out = hymn_string_append(out, replace);
            pos = i + len_sub;
            i = pos - 1;
        }
    }
    if (pos < len) out = hymn_string_append_substring(out, string, pos, len);
    return out;
}

static HymnString *char_to_string(char ch) {
    HymnString *s = hymn_new_empty_string(1);
    s[0] = ch;
    return s;
}

HymnString *hymn_int_to_string(HymnInt number) {
    size_t len = (size_t)snprintf(NULL, 0, "%lld", number);
    char *str = hymn_malloc(len + 1);
    snprintf(str, len + 1, "%lld", number);
    HymnString *s = hymn_new_string_with_length(str, len);
    free(str);
    return s;
}

HymnString *hymn_float_to_string(HymnFloat number) {
    size_t len = (size_t)snprintf(NULL, 0, "%g", number);
    char *str = hymn_malloc(len + 1);
    snprintf(str, len + 1, "%g", number);
    HymnString *s = hymn_new_string_with_length(str, len);
    free(str);
    return s;
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
    size_t len = (size_t)vsnprintf(NULL, 0, format, args);
    va_end(args);
    char *chars = hymn_malloc((len + 1) * sizeof(char));
    va_start(args, format);
    len = (size_t)vsnprintf(chars, len + 1, format, args);
    va_end(args);
    HymnString *str = hymn_new_string_with_length(chars, len);
    free(chars);
    return str;
}

static HymnString *string_append_format(HymnString *this, const char *format, ...) {
    va_list args;

    va_start(args, format);
    size_t len = (size_t)vsnprintf(NULL, 0, format, args);
    va_end(args);
    char *chars = hymn_malloc((len + 1) * sizeof(char));
    va_start(args, format);
    len = (size_t)vsnprintf(chars, len + 1, format, args);
    va_end(args);
    this = hymn_string_append(this, chars);
    free(chars);
    return this;
}

HymnString *hymn_working_directory(void) {
    char path[PATH_MAX];
    if (getcwd(path, sizeof(path)) != NULL) {
        return hymn_new_string(path);
    }
    return NULL;
}

HymnString *hymn_path_convert(HymnString *path) {
    size_t size = hymn_string_len(path);
    HymnString *convert = hymn_string_copy(path);
    for (size_t i = 0; i < size; i++) {
        if (convert[i] == PATH_SEP_OTHER) {
            convert[i] = PATH_SEP;
        }
    }
    return convert;
}

HymnString *hymn_path_normalize(HymnString *path) {
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

HymnString *hymn_path_parent(HymnString *path) {
    size_t size = hymn_string_len(path);
    if (size < 2) {
        return hymn_string_copy(path);
    }
    size_t index = size - 2;
    while (true) {
        if (index == 0 || path[index] == PATH_SEP) {
            return hymn_substring(path, 0, index);
        }
        index--;
    }
}

HymnString *hymn_path_absolute(HymnString *path) {
    HymnString *working = hymn_working_directory();
    if (hymn_string_starts_with(path, working)) {
        hymn_string_delete(working);
        return hymn_path_normalize(path);
    }
    working = hymn_string_append_char(working, PATH_SEP);
    working = hymn_string_append(working, path);
    HymnString *normal = hymn_path_normalize(working);
    hymn_string_delete(working);
    return normal;
}

size_t hymn_file_size(const char *path) {
    FILE *open = fopen(path, "r");
    if (open == NULL) {
        return 0;
    }
    size_t size = 0;
    int ch;
    while ((ch = fgetc(open)) != EOF) {
        size++;
    }
    fclose(open);
    return size;
}

HymnString *hymn_read_file(const char *path) {
    size_t size = hymn_file_size(path);
    FILE *open = fopen(path, "r");
    if (open == NULL) {
        return NULL;
    }
    HymnString *string = hymn_new_string_with_capacity(size);
    HymnStringHead *head = hymn_string_head(string);
    for (size_t i = 0; i < size; i++) {
        string[i] = (char)fgetc(open);
    }
    fclose(open);
    head->length = size;
    return string;
}

bool hymn_file_exists(const char *path) {
    struct stat b;
    return stat(path, &b) == 0;
}

#ifdef _MSC_VER
#define ANSI_COLOR_RED ""
#define ANSI_COLOR_RESET ""
#else
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\x1b[0m"
#endif

typedef struct JumpList JumpList;
typedef struct LoopList LoopList;

typedef struct Token Token;
typedef struct Local Local;
typedef struct Rule Rule;
typedef struct Scope Scope;
typedef struct Compiler Compiler;
typedef struct CompileResult CompileResult;

typedef struct Instruction Instruction;
typedef struct Optimizer Optimizer;

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
    TOKEN_OPCODES,
    TOKEN_STACK,
    TOKEN_REFERENCE,
    TOKEN_DELETE,
    TOKEN_DIVIDE,
    TOKEN_DOT,
    TOKEN_ECHO,
    TOKEN_ELIF,
    TOKEN_ELSE,
    TOKEN_EOF,
    TOKEN_EQUAL,
    TOKEN_ERROR,
    TOKEN_EXCEPT,
    TOKEN_EXISTS,
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
    TOKEN_SRC,
    TOKEN_INTEGER,
    TOKEN_KEYS,
    TOKEN_LEFT_CURLY,
    TOKEN_LEFT_PAREN,
    TOKEN_LEFT_SQUARE,
    TOKEN_LEN,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_SET,
    TOKEN_MODULO,
    TOKEN_MULTIPLY,
    TOKEN_NONE,
    TOKEN_NOT,
    TOKEN_NOT_EQUAL,
    TOKEN_OR,
    TOKEN_POINTER,
    TOKEN_POP,
    TOKEN_PRINT,
    TOKEN_PUSH,
    TOKEN_RETURN,
    TOKEN_RIGHT_CURLY,
    TOKEN_RIGHT_PAREN,
    TOKEN_RIGHT_SQUARE,
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

enum StringStatus {
    STRING_STATUS_NONE,
    STRING_STATUS_BEGIN,
    STRING_STATUS_ADD,
    STRING_STATUS_CLOSE,
    STRING_STATUS_CONTINUE,
};

enum OpCode {
    OP_ADD,
    OP_ADD_LOCALS,
    OP_INCREMENT,
    OP_INSERT,
    OP_ARRAY_POP,
    OP_ARRAY_PUSH,
    OP_ARRAY_PUSH_LOCALS,
    OP_BIT_AND,
    OP_BIT_LEFT_SHIFT,
    OP_BIT_NOT,
    OP_BIT_OR,
    OP_BIT_RIGHT_SHIFT,
    OP_BIT_XOR,
    OP_CALL,
    OP_TAIL_CALL,
    OP_SELF,
    OP_CLEAR,
    OP_CONSTANT,
    OP_COPY,
    OP_CODES,
    OP_STACK,
    OP_REFERENCE,
    OP_DEFINE_GLOBAL,
    OP_DELETE,
    OP_DIVIDE,
    OP_DUPLICATE,
    OP_ECHO,
    OP_EQUAL,
    OP_EXISTS,
    OP_FALSE,
    OP_GET_DYNAMIC,
    OP_GET_GLOBAL,
    OP_GET_GLOBAL_PROPERTY,
    OP_GET_LOCAL,
    OP_GET_LOCALS,
    OP_GET_PROPERTY,
    OP_GREATER,
    OP_GREATER_EQUAL,
    OP_INDEX,
    OP_SOURCE,
    OP_JUMP,
    OP_JUMP_IF_EQUAL,
    OP_JUMP_IF_NOT_EQUAL,
    OP_JUMP_IF_LESS,
    OP_JUMP_IF_GREATER,
    OP_JUMP_IF_GREATER_LOCALS,
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
    OP_MODULO_LOCALS,
    OP_MULTIPLY,
    OP_NEGATE,
    OP_NEW_ARRAY,
    OP_NEW_TABLE,
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
    OP_INCREMENT_LOOP,
    OP_SLICE,
    OP_SUBTRACT,
    OP_THROW,
    OP_FLOAT,
    OP_INT,
    OP_STRING,
    OP_TRUE,
    OP_TYPE,
    OP_USE,
    OP_FOR,
    OP_FOR_LOOP,
    OP_VOID,
};

enum FunctionType {
    TYPE_FUNCTION,
    TYPE_SCRIPT,
    TYPE_DIRECT,
    TYPE_REPL,
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
static void compile_pointer(Compiler *C, bool assign);
static void compile_square(Compiler *C, bool assign);
static void compile_and(Compiler *C, bool assign);
static void compile_or(Compiler *C, bool assign);
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
static void exists_expression(Compiler *C, bool assign);
static void source_expression(Compiler *C, bool assign);
static void opcode_expression(Compiler *C, bool assign);
static void stack_expression(Compiler *C, bool assign);
static void reference_expression(Compiler *C, bool assign);
static void function_expression(Compiler *C, bool assign);
static void declaration(Compiler *C);
static void statement(Compiler *C);
static void expression_statement(Compiler *C);
static void expression(Compiler *C);

static char *interpret(Hymn *H);

struct JumpList {
    int jump;
    int depth;
    HymnByteCode *code;
    JumpList *next;
};

struct LoopList {
    int start;
    int depth;
    HymnByteCode *code;
    LoopList *next;
    bool is_for;
    char padding[7];
};

struct Token {
    HymnInt integer;
    HymnFloat floating;
    int row;
    int column;
    size_t start;
    unsigned int length;
    enum TokenType type;
};

struct Local {
    Token name;
    int depth;
    char padding[4];
};

struct Rule {
    void (*prefix)(Compiler *, bool);
    void (*infix)(Compiler *, bool);
    enum Precedence precedence;
    char padding[4];
};

struct Scope {
    struct Scope *enclosing;
    HymnFunction *func;
    size_t begin;
    Local locals[HYMN_UINT8_COUNT];
    int local_count;
    int depth;
    enum FunctionType type;
    char padding[4];
};

struct Compiler {
    Hymn *H;
    Scope *scope;
    LoopList *loop;
    JumpList *jump;
    JumpList *jump_or;
    JumpList *jump_and;
    JumpList *jump_for;
    HymnString *error;
    const char *script;
    const char *source;
    size_t pos;
    size_t size;
    int pop;
    int barrier;
    int row;
    int column;
    Token previous;
    Token current;
    int string_format;
    enum StringStatus string_status;
    bool interactive;
    char padding[7];
};

struct CompileResult {
    HymnFunction *func;
    char *error;
};

HymnValue hymn_new_undefined(void) {
    return (HymnValue){.is = HYMN_VALUE_UNDEFINED, .as = {.i = 0}};
}

HymnValue hymn_new_none(void) {
    return (HymnValue){.is = HYMN_VALUE_NONE, .as = {.i = 0}};
}

HymnValue hymn_new_bool(bool v) {
    return (HymnValue){.is = HYMN_VALUE_BOOL, .as = {.b = v}};
}

HymnValue hymn_new_int(HymnInt v) {
    return (HymnValue){.is = HYMN_VALUE_INTEGER, .as = {.i = v}};
}

HymnValue hymn_new_float(HymnFloat v) {
    return (HymnValue){.is = HYMN_VALUE_FLOAT, .as = {.f = v}};
}

HymnValue hymn_new_native(HymnNativeFunction *v) {
    return (HymnValue){.is = HYMN_VALUE_FUNC_NATIVE, .as = {.o = (void *)v}};
}

HymnValue hymn_new_pointer(void *v) {
    return (HymnValue){.is = HYMN_VALUE_POINTER, .as = {.p = v}};
}

HymnValue hymn_new_string_value(HymnObjectString *v) {
    return (HymnValue){.is = HYMN_VALUE_STRING, .as = {.o = (void *)v}};
}

HymnValue hymn_new_array_value(HymnArray *v) {
    return (HymnValue){.is = HYMN_VALUE_ARRAY, .as = {.o = (void *)v}};
}

HymnValue hymn_new_table_value(HymnTable *v) {
    return (HymnValue){.is = HYMN_VALUE_TABLE, .as = {.o = (void *)v}};
}

HymnValue hymn_new_func_value(HymnFunction *v) {
    return (HymnValue){.is = HYMN_VALUE_FUNC, .as = {.o = (void *)v}};
}

bool hymn_as_bool(HymnValue v) {
    return (v).as.b;
}

HymnInt hymn_as_int(HymnValue v) {
    return (v).as.i;
}

HymnFloat hymn_as_float(HymnValue v) {
    return (v).as.f;
}

HymnNativeFunction *hymn_as_native(HymnValue v) {
    return (HymnNativeFunction *)(v).as.o;
}

void *hymn_as_pointer(HymnValue v) {
    return (v).as.p;
}

void *hymn_as_object(HymnValue v) {
    return (void *)(v).as.o;
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

static Rule rules[] = {
    [TOKEN_ADD] = {NULL, compile_binary, PRECEDENCE_TERM, {0}},
    [TOKEN_AND] = {NULL, compile_and, PRECEDENCE_AND, {0}},
    [TOKEN_ASSIGN] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_ASSIGN_ADD] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_ASSIGN_BIT_AND] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_ASSIGN_BIT_LEFT_SHIFT] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_ASSIGN_BIT_OR] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_ASSIGN_BIT_RIGHT_SHIFT] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_ASSIGN_BIT_XOR] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_ASSIGN_DIVIDE] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_ASSIGN_MODULO] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_ASSIGN_MULTIPLY] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_ASSIGN_SUBTRACT] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_BIT_AND] = {NULL, compile_binary, PRECEDENCE_BITS, {0}},
    [TOKEN_BIT_LEFT_SHIFT] = {NULL, compile_binary, PRECEDENCE_BITS, {0}},
    [TOKEN_BIT_NOT] = {compile_unary, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_BIT_OR] = {NULL, compile_binary, PRECEDENCE_BITS, {0}},
    [TOKEN_BIT_RIGHT_SHIFT] = {NULL, compile_binary, PRECEDENCE_BITS, {0}},
    [TOKEN_BIT_XOR] = {NULL, compile_binary, PRECEDENCE_BITS, {0}},
    [TOKEN_BREAK] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_CLEAR] = {clear_expression, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_COLON] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_COMMA] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_CONTINUE] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_COPY] = {copy_expression, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_OPCODES] = {opcode_expression, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_STACK] = {stack_expression, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_REFERENCE] = {reference_expression, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_DELETE] = {delete_expression, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_DIVIDE] = {NULL, compile_binary, PRECEDENCE_FACTOR, {0}},
    [TOKEN_DOT] = {NULL, compile_dot, PRECEDENCE_CALL, {0}},
    [TOKEN_ECHO] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_ELIF] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_ELSE] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_EOF] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_EQUAL] = {NULL, compile_binary, PRECEDENCE_EQUALITY, {0}},
    [TOKEN_ERROR] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_EXCEPT] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_EXISTS] = {exists_expression, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_FALSE] = {compile_false, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_FLOAT] = {compile_float, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_FOR] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_FUNCTION] = {function_expression, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_GREATER] = {NULL, compile_binary, PRECEDENCE_COMPARE, {0}},
    [TOKEN_GREATER_EQUAL] = {NULL, compile_binary, PRECEDENCE_COMPARE, {0}},
    [TOKEN_IDENT] = {compile_variable, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_SRC] = {source_expression, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_IF] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_IN] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_INDEX] = {index_expression, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_INSERT] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_INTEGER] = {compile_integer, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_KEYS] = {keys_expression, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_LEFT_CURLY] = {compile_table, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_LEFT_PAREN] = {compile_group, compile_call, PRECEDENCE_CALL, {0}},
    [TOKEN_LEFT_SQUARE] = {compile_array, compile_square, PRECEDENCE_CALL, {0}},
    [TOKEN_LEN] = {len_expression, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_LESS] = {NULL, compile_binary, PRECEDENCE_COMPARE, {0}},
    [TOKEN_LESS_EQUAL] = {NULL, compile_binary, PRECEDENCE_COMPARE, {0}},
    [TOKEN_SET] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_MODULO] = {NULL, compile_binary, PRECEDENCE_FACTOR, {0}},
    [TOKEN_MULTIPLY] = {NULL, compile_binary, PRECEDENCE_FACTOR, {0}},
    [TOKEN_NONE] = {compile_none, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_NOT] = {compile_unary, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_NOT_EQUAL] = {NULL, compile_binary, PRECEDENCE_EQUALITY, {0}},
    [TOKEN_OR] = {NULL, compile_or, PRECEDENCE_OR, {0}},
    [TOKEN_POINTER] = {NULL, compile_pointer, PRECEDENCE_CALL, {0}},
    [TOKEN_POP] = {array_pop_expression, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_PRINT] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_PUSH] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_RETURN] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_RIGHT_CURLY] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_RIGHT_PAREN] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_RIGHT_SQUARE] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_STRING] = {compile_string, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_SUBTRACT] = {compile_unary, compile_binary, PRECEDENCE_TERM, {0}},
    [TOKEN_THROW] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_TO_FLOAT] = {cast_float_expression, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_TO_INTEGER] = {cast_integer_expression, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_TO_STRING] = {cast_string_expression, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_TRUE] = {compile_true, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_TRY] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_TYPE_FUNC] = {type_expression, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_UNDEFINED] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_USE] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_VALUE] = {NULL, NULL, PRECEDENCE_NONE, {0}},
    [TOKEN_WHILE] = {NULL, NULL, PRECEDENCE_NONE, {0}},
};

const char *hymn_value_type(enum HymnValueType type) {
    switch (type) {
    case HYMN_VALUE_UNDEFINED: return "undefined";
    case HYMN_VALUE_NONE: return "none";
    case HYMN_VALUE_BOOL: return "boolean";
    case HYMN_VALUE_INTEGER: return "integer";
    case HYMN_VALUE_FLOAT: return "float";
    case HYMN_VALUE_STRING: return "string";
    case HYMN_VALUE_ARRAY: return "array";
    case HYMN_VALUE_TABLE: return "table";
    case HYMN_VALUE_FUNC: return "function";
    case HYMN_VALUE_FUNC_NATIVE: return "native";
    case HYMN_VALUE_POINTER: return "pointer";
    default: return "?";
    }
}

static unsigned int string_mix_code(HymnString *key) {
    size_t length = hymn_string_len(key);
    unsigned int hash = 0;
    for (size_t i = 0; i < length; i++) {
        hash = 31 * hash + (unsigned int)key[i];
    }
    return hash ^ (hash >> 16);
}

static unsigned int string_mix_code_const(const char *key) {
    size_t length = strlen(key);
    unsigned int hash = 0;
    for (size_t i = 0; i < length; i++) {
        hash = 31 * hash + (unsigned int)key[i];
    }
    return hash ^ (hash >> 16);
}

static HymnObjectString *new_hymn_string_with_hash(HymnString *string, unsigned int hash) {
    HymnObjectString *object = hymn_calloc(1, sizeof(HymnObjectString));
    object->string = string;
    object->hash = hash;
    return object;
}

HymnObjectString *hymn_new_string_object(HymnString *string) {
    return new_hymn_string_with_hash(string, string_mix_code(string));
}

static void table_init(HymnTable *this) {
    this->size = 0;
    this->bins = INITIAL_BINS;
    this->items = hymn_calloc(this->bins, sizeof(HymnTableItem *));
}

static unsigned int table_get_bin(HymnTable *this, unsigned int hash) {
    return (this->bins - 1U) & hash;
}

static void table_resize(HymnTable *this) {
    unsigned int old_bins = this->bins;

    if (old_bins >= MAXIMUM_BINS) {
        return;
    }

    unsigned int bins = old_bins << 1U;

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
    if (this->size >= (int)((float)this->bins * LOAD_FACTOR)) {
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

HymnValue hymn_table_get(HymnTable *this, const char *key) {
    unsigned int hash = string_mix_code_const(key);
    unsigned int bin = table_get_bin(this, hash);
    HymnTableItem *item = this->items[bin];
    while (item != NULL) {
        if (hymn_string_equal(key, item->key->string)) {
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
            hymn_dereference(H, item->value);
            hymn_dereference_string(H, item->key);
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

void hymn_set_property(Hymn *H, HymnTable *table, HymnObjectString *name, HymnValue value) {
    hymn_reference(value);
    HymnValue previous = table_put(table, name, value);
    if (hymn_is_undefined(previous)) {
        hymn_reference_string(name);
    } else {
        hymn_dereference(H, previous);
    }
}

void hymn_set_property_const(Hymn *H, HymnTable *table, const char *name, HymnValue value) {
    HymnObjectString *key = hymn_new_intern_string(H, name);
    hymn_set_property(H, table, key, value);
}

static void set_init(HymnSet *this) {
    this->size = 0;
    this->bins = INITIAL_BINS;
    this->items = hymn_calloc(this->bins, sizeof(HymnSetItem *));
}

static unsigned int set_get_bin(HymnSet *this, unsigned int hash) {
    return (this->bins - 1U) & hash;
}

static void set_resize(HymnSet *this) {
    unsigned int old_bins = this->bins;

    if (old_bins >= MAXIMUM_BINS) {
        return;
    }

    unsigned int bins = old_bins << 1U;

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
    unsigned int hash = string_mix_code(add);
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
    if (this->size >= (int)((float)this->bins * LOAD_FACTOR)) {
        set_resize(this);
    }
    return new;
}

static HymnObjectString *set_remove(HymnSet *this, HymnString *remove) {
    unsigned int hash = string_mix_code(remove);
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

static HymnByteCode *current(Compiler *C) {
    return &C->scope->func->code;
}

static void compile_error(Compiler *C, Token *token, const char *format, ...) {
    if (C->error != NULL) {
        return;
    }

    if (C->interactive && token->type == TOKEN_EOF) {
        C->error = hymn_new_string("<eof>");
        goto clean;
    }

    va_list ap;
    va_start(ap, format);
    size_t len = (size_t)vsnprintf(NULL, 0, format, ap);
    va_end(ap);
    char *chars = hymn_malloc((len + 1) * sizeof(char));
    va_start(ap, format);
    len = (size_t)vsnprintf(chars, len + 1, format, ap);
    va_end(ap);

    HymnString *error = hymn_new_string_with_capacity(len + 128);
    error = hymn_string_append(error, chars);

    free(chars);

    if (token->type != TOKEN_EOF && token->length > 0) {
        const char *source = C->source;
        const size_t size = C->size;
        const size_t start = token->start;

        size_t begin = start;
        while (true) {
            if (source[begin] == '\n') {
                begin++;
                break;
            }
            if (begin == 0) break;
            begin--;
        }

        while (true) {
            if (source[begin] != ' ' || begin == size) break;
            begin++;
        }

        size_t end = start;
        while (true) {
            if (source[end] == '\n' || end == size) break;
            end++;
        }

        if (begin < end) {
            error = string_append_format(error, "\n  %.*s\n  ", end - begin, &source[begin]);
            for (int i = 0; i < (int)(start - begin); i++) {
                error = hymn_string_append_char(error, ' ');
            }
            error = hymn_string_append(error, ANSI_COLOR_RED);
            for (unsigned int i = 0; i < token->length; i++) {
                error = hymn_string_append_char(error, '^');
            }
            error = hymn_string_append(error, ANSI_COLOR_RESET);
        }
    }

    error = string_append_format(error, "\n  at %s:%d", C->script == NULL ? "script" : C->script, token->row);

    C->error = error;

clean:
    C->current.type = TOKEN_EOF;
    C->previous.type = TOKEN_EOF;
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

static char peek_two_char(Compiler *C) {
    if (C->pos + 1 >= C->size) {
        return '\0';
    }
    return C->source[C->pos + 1];
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
    current->length = (unsigned int)length;
}

static void value_token(Compiler *C, enum TokenType type, size_t start, size_t end) {
    Token *current = &C->current;
    current->type = type;
    current->row = C->row;
    current->column = C->column;
    current->start = start;
    current->length = (unsigned int)(end - start);
}

static void int_token(Compiler *C, enum TokenType type, size_t start, size_t end, HymnInt integer) {
    Token *current = &C->current;
    current->type = type;
    current->row = C->row;
    current->column = C->column;
    current->start = start;
    current->length = (unsigned int)(end - start);
    current->integer = integer;
}

static void float_token(Compiler *C, enum TokenType type, size_t start, size_t end, HymnFloat floating) {
    Token *current = &C->current;
    current->type = type;
    current->row = C->row;
    current->column = C->column;
    current->start = start;
    current->length = (unsigned int)(end - start);
    current->floating = floating;
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
        if (size == 5) return ident_trie(ident, 1, "reak", TOKEN_BREAK);
        break;
    case 'd':
        if (size == 6) return ident_trie(ident, 1, "elete", TOKEN_DELETE);
        break;
    case 'r':
        if (size == 6) return ident_trie(ident, 1, "eturn", TOKEN_RETURN);
        break;
    case 's':
        if (size == 3) {
            if (ident[1] == 'e' && ident[2] == 't') return TOKEN_SET;
            if (ident[1] == 't' && ident[2] == 'r') return TOKEN_TO_STRING;
        }
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
        if (size == 3) return ident_trie(ident, 1, "en", TOKEN_LEN);
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
        if (size == 6) {
            if (ident[1] == 'x') {
                if (ident[2] == 'c') return ident_trie(ident, 3, "ept", TOKEN_EXCEPT);
                if (ident[2] == 'i') return ident_trie(ident, 3, "sts", TOKEN_EXISTS);
            }
        } else if (size == 4) {
            if (ident[1] == 'l') {
                if (ident[2] == 's') {
                    if (ident[3] == 'e') {
                        return TOKEN_ELSE;
                    }
                } else if (ident[2] == 'i' && ident[3] == 'f') {
                    return TOKEN_ELIF;
                }
            } else if (ident[1] == 'c') {
                return ident_trie(ident, 2, "ho", TOKEN_ECHO);
            }
        }
        break;
    case 'f':
        if (size == 3) return ident_trie(ident, 1, "or", TOKEN_FOR);
        if (size == 4) return ident_trie(ident, 1, "unc", TOKEN_FUNCTION);
        if (size == 5) {
            if (ident[1] == 'a') return ident_trie(ident, 2, "lse", TOKEN_FALSE);
            if (ident[1] == 'l') return ident_trie(ident, 2, "oat", TOKEN_TO_FLOAT);
        }
        break;
    case '_':
        if (size == 6) return ident_trie(ident, 1, "stack", TOKEN_STACK);
        if (size == 7) return ident_trie(ident, 1, "source", TOKEN_SRC);
        if (size == 8) return ident_trie(ident, 1, "opcodes", TOKEN_OPCODES);
        if (size == 10) return ident_trie(ident, 1, "reference", TOKEN_REFERENCE);
        break;
    default:
        break;
    }
    return TOKEN_UNDEFINED;
}

static void push_ident_token(Compiler *C, size_t start, size_t end) {
    const char *ident = &C->source[start];
    size_t size = end - start;
    enum TokenType keyword = ident_keyword(ident, size);
    if (keyword != TOKEN_UNDEFINED && C->previous.type != TOKEN_DOT) {
        value_token(C, keyword, start, end);
    } else {
        value_token(C, TOKEN_IDENT, start, end);
    }
}

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static bool is_ident(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
}

static enum StringStatus string_status(Compiler *C) {
    size_t i = C->pos;
    const char *source = C->source;
    const size_t size = C->size;
    bool expression = false;
    int brackets = 1;
    while (true) {
        if (i >= size) return false;
        switch (source[i]) {
        case '}':
            if (brackets > 1) {
                expression = true;
                i++;
                brackets--;
                continue;
            }
            return expression ? STRING_STATUS_BEGIN : STRING_STATUS_CONTINUE;
        case '"':
            return STRING_STATUS_NONE;
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            i++;
            continue;
        case '{':
            expression = true;
            i++;
            brackets++;
            continue;
        default:
            expression = true;
            i++;
            continue;
        }
    }
}

static void parse_string(Compiler *C, size_t start) {
    while (true) {
        char c = next_char(C);
        if (c == '\\') {
            next_char(C);
            continue;
        } else if (c == '$') {
            if (peek_char(C) == '{') {
                next_char(C);
                enum StringStatus status = string_status(C);
                if (status == STRING_STATUS_BEGIN) {
                    C->string_format = 1;
                    C->string_status = STRING_STATUS_BEGIN;
                    size_t end = C->pos - 2;
                    value_token(C, TOKEN_STRING, start, end);
                    return;
                } else if (status == STRING_STATUS_CONTINUE) {
                    C->string_status = STRING_STATUS_CONTINUE;
                    size_t end = C->pos - 2;
                    value_token(C, TOKEN_STRING, start, end);
                    while (true) {
                        c = next_char(C);
                        if (c == '}' || c == '\0') return;
                    }
                } else {
                    continue;
                }
            }
        } else if (c == '"' || c == '\0') {
            break;
        }
    }
    size_t end = C->pos - 1;
    value_token(C, TOKEN_STRING, start, end);
}

static void advance(Compiler *C) {
    C->previous = C->current;
    if (C->previous.type == TOKEN_EOF) {
        return;
    }
    switch (C->string_status) {
    case STRING_STATUS_BEGIN:
        C->string_status = STRING_STATUS_ADD;
        token(C, TOKEN_ADD);
        return;
    case STRING_STATUS_ADD:
        C->string_status = STRING_STATUS_NONE;
        token(C, TOKEN_LEFT_PAREN);
        return;
    case STRING_STATUS_CLOSE:
        C->string_status = STRING_STATUS_CONTINUE;
        token(C, TOKEN_ADD);
        return;
    case STRING_STATUS_CONTINUE: {
        C->string_status = STRING_STATUS_NONE;
        size_t start = C->pos;
        parse_string(C, start);
        return;
    }
    default:
        break;
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
        case '#': {
            next_char(C);
            c = peek_char(C);
            while (c != '\n' && c != '\0') {
                next_char(C);
                c = peek_char(C);
            }
            continue;
        }
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
            if (peek_char(C) == '=') {
                next_char(C);
                token_special(C, TOKEN_ASSIGN_SUBTRACT, 2, 2);
                return;
            } else if (peek_char(C) == '>') {
                next_char(C);
                token_special(C, TOKEN_POINTER, 2, 2);
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
        case '{':
            if (C->string_format >= 1) {
                C->string_format++;
            }
            token(C, TOKEN_LEFT_CURLY);
            return;
        case '}':
            if (C->string_format == 1) {
                C->string_format = 0;
                C->string_status = STRING_STATUS_CLOSE;
                token(C, TOKEN_RIGHT_PAREN);
                return;
            } else if (C->string_format > 1) {
                C->string_format--;
            }
            token(C, TOKEN_RIGHT_CURLY);
            return;
        case ':': token(C, TOKEN_COLON); return;
        case '\0': token(C, TOKEN_EOF); return;
        case '"': {
            size_t start = C->pos;
            parse_string(C, start);
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
                if (c == '0') {
                    const char p = peek_char(C);
                    if (p == 'b') {
                        next_char(C);
                        while (true) {
                            c = peek_char(C);
                            if (c != '0' && c != '1') {
                                break;
                            }
                            next_char(C);
                        }
                        size_t end = C->pos;
                        long long number = (long long)strtoll(&C->source[start + 2], NULL, 2);
                        int_token(C, TOKEN_INTEGER, start, end, number);
                        return;
                    } else if (p == 'x') {
                        next_char(C);
                        while (true) {
                            c = peek_char(C);
                            if (!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'f')) {
                                break;
                            }
                            next_char(C);
                        }
                        size_t end = C->pos;
                        long long number = (long long)strtoll(&C->source[start + 2], NULL, 16);
                        int_token(C, TOKEN_INTEGER, start, end, number);
                        return;
                    }
                }
                while (is_digit(peek_char(C))) {
                    next_char(C);
                }
                const char p = peek_char(C);
                if (p == '.') {
                    next_char(C);
                    while (is_digit(peek_char(C))) {
                        next_char(C);
                    }
                    const char n = peek_char(C);
                    if (n == 'e' || n == 'E') {
                        next_char(C);
                        const char e = peek_char(C);
                        if (e == '-' || e == '+') next_char(C);
                        while (is_digit(peek_char(C))) {
                            next_char(C);
                        }
                    }
                    size_t end = C->pos;
                    double number = atof(&C->source[start]);
                    float_token(C, TOKEN_FLOAT, start, end, number);
                    return;
                } else if (p == 'e' || p == 'E') {
                    next_char(C);
                    const char n = peek_char(C);
                    if (n == '-' || n == '+') next_char(C);
                    while (is_digit(peek_char(C))) {
                        next_char(C);
                    }
                    size_t end = C->pos;
                    double number = atof(&C->source[start]);
                    if (trunc(number) == number) {
                        int_token(C, TOKEN_INTEGER, start, end, (long long)number);
                    } else {
                        float_token(C, TOKEN_FLOAT, start, end, number);
                    }
                    return;
                }
                size_t end = C->pos;
                long long number = atoll(&C->source[start]);
                int_token(C, TOKEN_INTEGER, start, end, number);
                return;
            } else if (is_ident(c)) {
                size_t start = C->pos - 1;
                while (true) {
                    c = peek_char(C);
                    if (is_ident(c)) {
                        next_char(C);
                        continue;
                    } else if (c == '-') {
                        if (is_ident(peek_two_char(C))) {
                            next_char(C);
                            next_char(C);
                            continue;
                        }
                    }
                    break;
                }
                size_t end = C->pos;
                push_ident_token(C, start, end);
                return;
            } else {
                token(C, TOKEN_ERROR);
                compile_error(C, &C->current, "unknown character: %c", c);
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
        case HYMN_VALUE_FLOAT: return (HymnFloat)hymn_as_int(a) == hymn_as_float(b);
        default: return false;
        }
    case HYMN_VALUE_FLOAT:
        switch (b.is) {
        case HYMN_VALUE_INTEGER: return hymn_as_float(a) == (HymnFloat)hymn_as_int(b);
        case HYMN_VALUE_FLOAT: return hymn_as_float(a) == hymn_as_float(b);
        default: return false;
        }
    case HYMN_VALUE_STRING:
    case HYMN_VALUE_ARRAY:
    case HYMN_VALUE_TABLE:
    case HYMN_VALUE_FUNC:
    case HYMN_VALUE_FUNC_NATIVE:
        return b.is == a.is && hymn_as_object(a) == hymn_as_object(b);
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
    case HYMN_VALUE_FUNC_NATIVE:
        return hymn_as_object(a) == hymn_as_object(b);
    case HYMN_VALUE_POINTER: return hymn_as_pointer(a) == hymn_as_pointer(b);
    default:
        return false;
    }
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
    if (count >= this->capacity) {
        this->capacity *= 2;
        this->values = hymn_realloc_int(this->values, this->capacity, sizeof(HymnValue));
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

static HymnNativeFunction *new_native_function(HymnObjectString *name, HymnNativeCall func) {
    HymnNativeFunction *native = hymn_calloc(1, sizeof(HymnNativeFunction));
    native->name = name;
    native->func = func;
    hymn_reference_string(name);
    return native;
}

static void array_init_with_capacity(HymnArray *this, HymnInt length, HymnInt capacity) {
    if (capacity == 0) {
        this->items = NULL;
    } else {
        this->items = hymn_calloc((size_t)capacity, sizeof(HymnValue));
    }
    this->length = length;
    this->capacity = capacity;
}

static HymnArray *new_array_with_capacity(HymnInt length, HymnInt capacity) {
    HymnArray *this = hymn_calloc(1, sizeof(HymnArray));
    array_init_with_capacity(this, length, capacity);
    return this;
}

HymnArray *hymn_new_array(HymnInt length) {
    return new_array_with_capacity(length, length);
}

static HymnArray *new_array_slice(HymnArray *from, HymnInt start, HymnInt end) {
    HymnInt length = end - start;
    size_t size = (size_t)length * sizeof(HymnValue);
    HymnArray *this = hymn_calloc(1, sizeof(HymnArray));
    this->items = hymn_malloc(size);
    memcpy(this->items, &from->items[start], size);
    this->length = length;
    this->capacity = length;
    for (HymnInt i = 0; i < length; i++) {
        hymn_reference(this->items[i]);
    }
    return this;
}

static HymnArray *new_array_copy(HymnArray *from) {
    return new_array_slice(from, 0, from->length);
}

static void array_update_capacity(HymnArray *this, HymnInt length) {
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

void hymn_array_push(HymnArray *this, HymnValue value) {
    HymnInt length = this->length + 1;
    array_update_capacity(this, length);
    this->length = length;
    this->items[length - 1] = value;
}

void hymn_array_insert(HymnArray *this, HymnInt index, HymnValue value) {
    HymnInt length = this->length + 1;
    array_update_capacity(this, length);
    this->length = length;
    HymnValue *items = this->items;
    for (HymnInt i = length - 1; i > index; i--) {
        items[i] = items[i - 1];
    }
    items[index] = value;
}

HymnValue hymn_array_get(HymnArray *this, HymnInt index) {
    if (index >= this->length) {
        return hymn_new_undefined();
    }
    return this->items[index];
}

HymnInt hymn_array_index_of(HymnArray *this, HymnValue match) {
    HymnInt len = this->length;
    HymnValue *items = this->items;
    for (HymnInt i = 0; i < len; i++) {
        if (hymn_match_values(match, items[i])) {
            return i;
        }
    }
    return -1;
}

HymnValue hymn_array_pop(HymnArray *this) {
    if (this->length == 0) {
        return hymn_new_none();
    }
    return this->items[--this->length];
}

HymnValue hymn_array_remove_index(HymnArray *this, HymnInt index) {
    HymnInt len = --this->length;
    HymnValue *items = this->items;
    HymnValue deleted = items[index];
    while (index < len) {
        items[index] = items[index + 1];
        index++;
    }
    return deleted;
}

void hymn_array_clear(Hymn *H, HymnArray *this) {
    HymnInt len = this->length;
    HymnValue *items = this->items;
    for (HymnInt i = 0; i < len; i++) {
        hymn_dereference(H, items[i]);
    }
    this->length = 0;
}

void hymn_array_delete(Hymn *H, HymnArray *this) {
    hymn_array_clear(H, this);
    free(this->items);
    free(this);
}

HymnTable *hymn_new_table(void) {
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
            hymn_reference_string(item->key);
            hymn_reference(item->value);
            item = item->next;
        }
    }
    return this;
}

static HymnArray *table_keys(HymnTable *this) {
    int size = this->size;
    HymnArray *array = hymn_new_array((HymnInt)size);
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
                if (strcmp(string, hymn_as_string(keys[insert])) < 0) {
                    for (unsigned int swap = total; swap > insert; swap--) {
                        keys[swap] = keys[swap - 1];
                    }
                    break;
                }
                insert++;
            }
            HymnValue value = hymn_new_string_value(item->key);
            hymn_reference(value);
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

static void scope_init(Compiler *C, Scope *scope, enum FunctionType type, size_t begin) {

    Scope *enclosing = C->scope;

    scope->enclosing = enclosing;
    C->scope = scope;

    scope->local_count = 0;
    scope->depth = 0;
    scope->type = type;
    scope->begin = begin;
    scope->func = hymn_calloc(1, sizeof(HymnFunction));
    byte_code_init(&scope->func->code);
    if (C->script != NULL) {
        scope->func->script = hymn_new_string(C->script);
    }
    scope->func->parent = enclosing == NULL ? NULL : enclosing->func;

    if (type == TYPE_FUNCTION) {
        scope->func->name = hymn_substring(C->source, C->previous.start, C->previous.start + C->previous.length);
    }

    Local *local = &scope->locals[scope->local_count++];
    local->depth = 0;
    local->name.start = 0;
    local->name.length = 0;
}

static void byte_code_delete(HymnByteCode *this) {
    free(this->instructions);
    free(this->lines);
    free(this->constants.values);
}

static uint8_t byte_code_new_constant(Compiler *C, HymnValue value) {
    HymnByteCode *code = current(C);
    int constant = value_pool_add(&code->constants, value);
    if (constant > UINT8_MAX) {
        compile_error(C, &C->previous, "too many constants");
        constant = 0;
    }
    return (uint8_t)constant;
}

static void write_byte(HymnByteCode *code, uint8_t b, int row) {
    int count = code->count;
    if (count >= code->capacity) {
        code->capacity *= 2;
        code->instructions = hymn_realloc_int(code->instructions, code->capacity, sizeof(uint8_t));
        code->lines = hymn_realloc_int(code->lines, code->capacity, sizeof(int));
    }
    code->instructions[count] = b;
    code->lines[count] = row;
    code->count = count + 1;
}

static void emit(Compiler *C, uint8_t i) {
    write_byte(current(C), i, C->previous.row);
}

static void emit_pop(Compiler *C) {
    HymnByteCode *code = current(C);
    write_byte(code, OP_POP, C->previous.row);
    C->pop = code->count;
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

HymnObjectString *hymn_intern_string(Hymn *H, HymnString *string) {
    HymnObjectString *object = set_add_or_get(&H->strings, string);
    if (object->string != string) {
        hymn_string_delete(string);
    }
    return object;
}

HymnObjectString *hymn_new_intern_string(Hymn *H, const char *value) {
    return hymn_intern_string(H, hymn_new_string(value));
}

static HymnValue compile_intern_string(Hymn *H, HymnString *string) {
    HymnObjectString *object = set_add_or_get(&H->strings, string);
    if (object->string == string) {
        hymn_reference_string(object);
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
    if (C->current.type != type) {
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
        compile_error(C, &C->previous, "expression expected near '%.*s'", C->previous.length, &C->source[C->previous.start]);
        return;
    }
    bool assign = precedence <= PRECEDENCE_ASSIGN;
    prefix(C, assign);
    while (precedence <= token_rule(C->current.type)->precedence) {
        advance(C);
        void (*infix)(Compiler *, bool) = token_rule(C->previous.type)->infix;
        if (infix == NULL) {
            compile_error(C, &C->previous, "expected infix");
            return;
        }
        infix(C, assign);
    }
    if (assign && check_assign(C)) {
        advance(C);
        compile_error(C, &C->current, "invalid assignment");
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
        compile_error(C, &C->previous, "too many local variables in scope");
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
                compile_error(C, &C->previous, "too many function arguments");
                break;
            }
            count++;
        } while (match(C, TOKEN_COMMA));
    }
    consume(C, TOKEN_RIGHT_PAREN, "function has no closing ')'");
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
    consume(C, TOKEN_RIGHT_PAREN, "parenthesis group has no closing ')'");
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
    emit_constant(C, hymn_new_int(C->current.integer));
}

static void compile_float(Compiler *C, bool assign) {
    (void)assign;
    emit_constant(C, hymn_new_float(C->current.floating));
}

static char escape_sequence(const char c) {
    switch (c) {
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
    default:
        return c;
    }
}

static HymnString *parse_string_literal(const char *string, size_t start, size_t len) {
    const size_t end = start + len;
    HymnString *literal = hymn_new_string_with_capacity(len);
    for (size_t i = start; i < end; i++) {
        const char c = string[i];
        if (c == '\\' && i + 1 < end) {
            const char e = escape_sequence(string[i + 1]);
            literal = hymn_string_append_char(literal, e);
            i++;
        } else {
            literal = hymn_string_append_char(literal, c);
        }
    }
    return literal;
}

static HymnValue string_literal(Compiler *C) {
    Token *previous = &C->previous;
    HymnString *string = parse_string_literal(C->source, previous->start, previous->length);
    while (check(C, TOKEN_STRING)) {
        Token *current = &C->current;
        HymnString *and = parse_string_literal(C->source, current->start, current->length);
        string = hymn_string_append(string, and);
        hymn_string_delete(and);
        advance(C);
    }
    return compile_intern_string(C->H, string);
}

static void compile_string(Compiler *C, bool assign) {
    (void)assign;
    emit_constant(C, string_literal(C));
}

static uint8_t ident_constant_string(Compiler *C) {
    return byte_code_new_constant(C, string_literal(C));
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
        emit_pop(C);
        scope->local_count--;
    }
    C->barrier = scope->func->code.count;
}

static void compile_array(Compiler *C, bool assign) {
    (void)assign;
    emit(C, OP_NEW_ARRAY);
    if (match(C, TOKEN_RIGHT_SQUARE)) {
        return;
    }
    while (!check(C, TOKEN_RIGHT_SQUARE) && !check(C, TOKEN_EOF)) {
        emit(C, OP_DUPLICATE);
        expression(C);
        emit(C, OP_ARRAY_PUSH);
        if (!check(C, TOKEN_RIGHT_SQUARE)) {
            consume(C, TOKEN_COMMA, "expected ',' between array elements");
        }
    }
    consume(C, TOKEN_RIGHT_SQUARE, "expected ']' at end of array declaration");
}

static void compile_table(Compiler *C, bool assign) {
    (void)assign;
    emit(C, OP_NEW_TABLE);
    if (match(C, TOKEN_RIGHT_CURLY)) {
        return;
    }
    while (!check(C, TOKEN_RIGHT_CURLY) && !check(C, TOKEN_EOF)) {
        emit(C, OP_DUPLICATE);
        uint8_t name;
        if (match(C, TOKEN_IDENT)) {
            name = ident_constant(C, &C->previous);
        } else if (match(C, TOKEN_STRING)) {
            name = ident_constant_string(C);
        } else {
            name = UINT8_MAX;
            compile_error(C, &C->current, "expected table key");
        }
        consume(C, TOKEN_COLON, "expected ':' between table key and value");
        expression(C);
        emit_short(C, OP_SET_PROPERTY, name);
        emit_pop(C);
        if (!check(C, TOKEN_RIGHT_CURLY)) {
            consume(C, TOKEN_COMMA, "expected ',' between table elements");
        }
    }
    consume(C, TOKEN_RIGHT_CURLY, "expected '}' at end of table declaration");
}

static void function_delete(HymnFunction *this) {
    if (this->parent != NULL) {
        HymnFunction *parent = this->parent;
        HymnValuePool *constants = &parent->code.constants;
        int count = constants->count;
        HymnValue *values = constants->values;
        for (int i = 0; i < count; i++) {
            HymnValue value = values[i];
            if (hymn_is_func(value)) {
                HymnFunction *func = hymn_as_func(value);
                if (func == this) {
                    values[i] = hymn_new_undefined();
                    break;
                }
            }
        }
    }
    HymnValuePool *constants = &this->code.constants;
    int count = constants->count;
    HymnValue *values = constants->values;
    for (int i = 0; i < count; i++) {
        HymnValue value = values[i];
        if (hymn_is_func(value)) {
            HymnFunction *func = hymn_as_func(value);
            func->parent = NULL;
            if (func->count == 0) {
                function_delete(func);
            }
        }
    }
    byte_code_delete(&this->code);
    hymn_string_delete(this->name);
    hymn_string_delete(this->script);
    hymn_string_delete(this->source);
    HymnExceptList *except = this->except;
    while (except != NULL) {
        HymnExceptList *next = except->next;
        free(except);
        except = next;
    }
    free(this);
}

static void native_function_delete(Hymn *H, HymnNativeFunction *this) {
    hymn_dereference_string(H, this->name);
    free(this);
}

static void push_local(Compiler *C, Token name) {
    Scope *scope = C->scope;
    if (scope->local_count == HYMN_UINT8_COUNT) {
        compile_error(C, &name, "too many local variables in scope");
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
            compile_error(C, name, "variable '%.*s' already exists in scope", name->length, &C->source[name->start]);
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

static void type_declaration(Compiler *C) {
    if (match(C, TOKEN_COLON)) {
        enum TokenType type = C->current.type;
        switch (type) {
        case TOKEN_NONE:
        case TOKEN_TO_FLOAT:
        case TOKEN_TO_STRING:
        case TOKEN_TO_INTEGER:
            advance(C);
            return;
        default:
            compile_error(C, &C->previous, "unavailable type declaration");
            return;
        }
    }
}

static void define_new_variable(Compiler *C) {
    uint8_t global = variable(C, "expected a variable name");
    type_declaration(C);
    consume(C, TOKEN_ASSIGN, "expected '=' after variable");
    expression(C);
    finalize_variable(C, global);
}

static int resolve_local(Compiler *C, Token *name) {
    Scope *scope = C->scope;
    for (int i = scope->local_count - 1; i >= 0; i--) {
        Local *local = &scope->locals[i];
        if (ident_match(C, name, &local->name)) {
            if (local->depth == -1) {
                compile_error(C, name, "local variable '%.*s' referenced before assignment", name->length, &C->source[name->start]);
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
    consume(C, TOKEN_IDENT, "expected table key after '.'");
    uint8_t name = ident_constant(C, &C->previous);
    if (assign && match(C, TOKEN_ASSIGN)) {
        expression(C);
        emit_short(C, OP_SET_PROPERTY, name);
    } else {
        emit_short(C, OP_GET_PROPERTY, name);
    }
}

static void compile_pointer(Compiler *C, bool assign) {
    (void)assign;
    consume(C, TOKEN_IDENT, "expected table key after '->'");
    uint8_t name = ident_constant(C, &C->previous);
    consume(C, TOKEN_LEFT_PAREN, "expected '(' after function name");
    emit_short(C, OP_SELF, name);
    uint8_t count = arguments(C);
    if (count == UINT8_MAX) {
        compile_error(C, &C->previous, "too many function arguments");
        return;
    }
    emit_short(C, OP_CALL, (uint8_t)(count + 1));
}

static void compile_square(Compiler *C, bool assign) {
    if (match(C, TOKEN_COLON)) {
        emit_constant(C, hymn_new_int(0));
        if (match(C, TOKEN_RIGHT_SQUARE)) {
            emit_constant(C, hymn_new_none());
        } else {
            expression(C);
            consume(C, TOKEN_RIGHT_SQUARE, "expected ']' after square bracket expression");
        }
        emit(C, OP_SLICE);
    } else {
        expression(C);
        if (match(C, TOKEN_COLON)) {
            if (match(C, TOKEN_RIGHT_SQUARE)) {
                emit_constant(C, hymn_new_none());
            } else {
                expression(C);
                consume(C, TOKEN_RIGHT_SQUARE, "expected ']' after square bracket expression");
            }
            emit(C, OP_SLICE);
        } else {
            consume(C, TOKEN_RIGHT_SQUARE, "expected ']' after square bracket expression");
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
        compile_error(C, &C->previous, "jump offset too large");
        return;
    }
    code->instructions[jump] = (uint8_t)((offset >> 8) & UINT8_MAX);
    code->instructions[jump + 1] = (uint8_t)(offset & UINT8_MAX);
}

static JumpList *add_jump(Compiler *C, JumpList *list, enum OpCode instruction) {
    JumpList *jump = hymn_calloc(1, sizeof(JumpList));
    jump->jump = emit_jump(C, (uint8_t)instruction);
    jump->depth = C->scope->depth;
    jump->code = current(C);
    jump->next = list;
    return jump;
}

static void free_jump_and_list(Compiler *C) {
    JumpList *jump = C->jump_and;
    HymnByteCode *code = current(C);
    int depth = C->scope->depth;
    while (jump != NULL) {
        if (jump->code != code || jump->depth < depth) {
            break;
        }
        patch_jump(C, jump->jump);
        JumpList *next = jump->next;
        free(jump);
        jump = next;
    }
    C->jump_and = jump;
}

static void free_jump_or_list(Compiler *C) {
    JumpList *jump = C->jump_or;
    HymnByteCode *code = current(C);
    int depth = C->scope->depth;
    while (jump != NULL) {
        if (jump->code != code || jump->depth < depth) {
            break;
        }
        patch_jump(C, jump->jump);
        JumpList *next = jump->next;
        free(jump);
        jump = next;
    }
    C->jump_or = jump;
}

static void free_jumps(Compiler *C, JumpList *jump) {
    while (jump != NULL) {
        patch_jump(C, jump->jump);
        JumpList *next = jump->next;
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
    case OP_CALL:
    case OP_CONSTANT:
    case OP_DEFINE_GLOBAL:
    case OP_EXISTS:
    case OP_GET_GLOBAL:
    case OP_GET_LOCAL:
    case OP_GET_PROPERTY:
    case OP_INCREMENT:
    case OP_POP_N:
    case OP_PRINT:
    case OP_SELF:
    case OP_SET_GLOBAL:
    case OP_SET_LOCAL:
    case OP_SET_PROPERTY:
    case OP_TAIL_CALL:
        return 2;
    case OP_ADD_LOCALS:
    case OP_ARRAY_PUSH_LOCALS:
    case OP_GET_GLOBAL_PROPERTY:
    case OP_GET_LOCALS:
    case OP_INCREMENT_LOCAL:
    case OP_INCREMENT_LOCAL_AND_SET:
    case OP_JUMP:
    case OP_JUMP_IF_EQUAL:
    case OP_JUMP_IF_FALSE:
    case OP_JUMP_IF_GREATER:
    case OP_JUMP_IF_GREATER_EQUAL:
    case OP_JUMP_IF_LESS:
    case OP_JUMP_IF_LESS_EQUAL:
    case OP_JUMP_IF_NOT_EQUAL:
    case OP_JUMP_IF_TRUE:
    case OP_LOOP:
    case OP_MODULO_LOCALS:
        return 3;
    case OP_FOR:
    case OP_FOR_LOOP:
        return 4;
    case OP_INCREMENT_LOOP:
    case OP_JUMP_IF_GREATER_LOCALS:
        return 5;
    default:
        return 1;
    }
}

struct Instruction {
    Instruction *next;
    int index;
    uint8_t instruction;
    char padding[3];
};

struct Optimizer {
    HymnByteCode *code;
    Instruction *important;
    HymnExceptList *except;
};

#define GET_JUMP(instructions, index, x, y) (((int)instructions[index + x] << 8) | (int)instructions[index + y])

#define UPDATE_JUMP(instructions, index, x, y, jump)              \
    instructions[index + x] = (uint8_t)((jump >> 8) & UINT8_MAX); \
    instructions[index + y] = (uint8_t)(jump & UINT8_MAX);

#define IS_ADJUSTABLE(off, compare, T, operator)                                         \
    if (index compare target) {                                                          \
        int offset = index + off;                                                        \
        int jump = ((int)instructions[offset - 2] << 8) | (int)instructions[offset - 1]; \
        if (offset operator jump == target) {                                            \
            return view;                                                                 \
        }                                                                                \
    }                                                                                    \
    break;

static Instruction *adjustable(Optimizer *optimizer, int target) {
    Instruction *view = optimizer->important;
    uint8_t *instructions = optimizer->code->instructions;
    while (view != NULL) {
        int index = view->index;
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
            IS_ADJUSTABLE(3, <, target, +)
        }
        case OP_FOR: {
            IS_ADJUSTABLE(4, <, target, +)
        }
        case OP_JUMP_IF_GREATER_LOCALS: {
            IS_ADJUSTABLE(5, <, target, +)
        }
        case OP_LOOP: {
            IS_ADJUSTABLE(3, >=, target, -)
        }
        case OP_FOR_LOOP: {
            IS_ADJUSTABLE(4, >=, target, -)
        }
        case OP_INCREMENT_LOOP: {
            IS_ADJUSTABLE(5, >=, target, -)
        }
        default:
            break;
        }
        view = view->next;
    }
    return NULL;
}

static void rewriting(Optimizer *optimizer, int start, int shift, bool inside) {
    uint8_t *instructions = optimizer->code->instructions;
    Instruction *view = optimizer->important;
    while (view != NULL) {
        int i = view->index;
        switch (view->instruction) {
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
                int jump = GET_JUMP(instructions, i, 1, 2);
                int destination = i + 3 + jump;
                assert((destination < start || destination > start + shift) || !inside);
                if (destination > start) {
                    jump -= shift;
                    UPDATE_JUMP(instructions, i, 1, 2, jump)
                } else if (destination >= start && destination <= start + shift && inside) {
                    // not sure about this
                    jump -= next(optimizer->code->instructions[destination]);
                    UPDATE_JUMP(instructions, i, 1, 2, jump)
                }
            }
            break;
        }
        case OP_JUMP_IF_GREATER_LOCALS: {
            if (i < start) {
                int jump = GET_JUMP(instructions, i, 3, 4);
                int destination = i + 5 + jump;
                assert((destination < start || destination > start + shift) || !inside);
                if (destination > start) {
                    jump -= shift;
                    UPDATE_JUMP(instructions, i, 3, 4, jump)
                } else if (destination >= start && destination <= start + shift && inside) {
                    // not sure about this
                    jump -= next(optimizer->code->instructions[destination]);
                    UPDATE_JUMP(instructions, i, 3, 4, jump)
                }
            }
            break;
        }
        case OP_FOR: {
            if (i < start) {
                int jump = GET_JUMP(instructions, i, 2, 3);
                int destination = i + 4 + jump;
                assert((destination < start || destination > start + shift) || !inside);
                if (destination > start) {
                    jump -= shift;
                    UPDATE_JUMP(instructions, i, 2, 3, jump)
                }
            }
            break;
        }
        case OP_LOOP: {
            if (i >= start) {
                int jump = GET_JUMP(instructions, i, 1, 2);
                int destination = i + 3 - jump;
                assert((destination < start || destination > start + shift) || !inside);
                if (destination < start) {
                    jump -= shift;
                    UPDATE_JUMP(instructions, i, 1, 2, jump)
                } else if (destination >= start && destination <= start + shift && inside) {
                    // not sure about this
                    jump -= next(optimizer->code->instructions[destination]);
                    UPDATE_JUMP(instructions, i, 1, 2, jump)
                }
            }
            break;
        }
        case OP_FOR_LOOP: {
            if (i >= start) {
                int jump = GET_JUMP(instructions, i, 2, 3);
                int destination = i + 3 - jump;
                assert((destination < start || destination > start + shift) || !inside);
                if (destination < start) {
                    jump -= shift;
                    UPDATE_JUMP(instructions, i, 2, 3, jump)
                } else if (destination >= start && destination <= start + shift && inside) {
                    // not sure about this
                    jump -= next(optimizer->code->instructions[destination]);
                    UPDATE_JUMP(instructions, i, 2, 3, jump)
                }
            }
            break;
        }
        case OP_INCREMENT_LOOP: {
            if (i >= start) {
                int jump = GET_JUMP(instructions, i, 3, 4);
                int destination = i + 5 - jump;
                assert((destination < start || destination > start + shift) || !inside);
                if (destination < start) {
                    jump -= shift;
                    UPDATE_JUMP(instructions, i, 3, 4, jump)
                } else if (destination >= start && destination <= start + shift && inside) {
                    // not sure about this
                    jump -= next(optimizer->code->instructions[destination]);
                    UPDATE_JUMP(instructions, i, 3, 4, jump)
                }
            }
            break;
        }
        default: break;
        }
        if (i >= start) {
            view->index = i - shift;
        }
        view = view->next;
    }

    int *lines = optimizer->code->lines;
    int count = optimizer->code->count - shift;
    for (int c = start; c < count; c++) {
        int n = c + shift;
        instructions[c] = instructions[n];
        lines[c] = lines[n];
    }
    optimizer->code->count = count;

    HymnExceptList *except = optimizer->except;
    while (except != NULL) {
        if (start < except->start) {
            except->start -= shift;
            except->end -= shift;
        } else if (start < except->end) {
            except->end -= shift;
        }
        except = except->next;
    }
}

static void rewrite(Optimizer *optimizer, int start, int shift) {
    rewriting(optimizer, start, shift, true);
}

static void update(Optimizer *optimizer, int index, uint8_t instruction) {
    Instruction *view = optimizer->important;
    while (view != NULL) {
        if (index == view->index) {
            view->instruction = instruction;
            optimizer->code->instructions[index] = instruction;
            return;
        }
        view = view->next;
    }
    fprintf(stderr, "optimization failed to find instruction to update\n");
    exit(1);
}

static void move(Optimizer *optimizer, int index, uint8_t instruction, int to) {
    Instruction *view = optimizer->important;
    while (view != NULL) {
        if (index == view->index) {
            view->index = to;
            view->instruction = instruction;
            optimizer->code->instructions[to] = instruction;
            return;
        }
        view = view->next;
    }
    fprintf(stderr, "optimization failed to find instruction to move\n");
    exit(1);
}

static void deletion(Optimizer *optimizer, int index) {
    Instruction *view = optimizer->important;
    Instruction *previous = NULL;
    while (view != NULL) {
        if (index == view->index) {
            Instruction *next = view->next;
            free(view);
            if (previous == NULL) {
                optimizer->important = next;
            } else {
                previous->next = next;
            }
            return;
        }
        previous = view;
        view = view->next;
    }
    fprintf(stderr, "optimization failed to find instruction to delete\n");
    exit(1);
}

static void interest(Optimizer *optimizer) {
    uint8_t *instructions = optimizer->code->instructions;
    int count = optimizer->code->count;
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
        case OP_JUMP_IF_GREATER_LOCALS:
        case OP_JUMP_IF_LESS_EQUAL:
        case OP_JUMP_IF_GREATER_EQUAL:
        case OP_FOR:
        case OP_LOOP:
        case OP_FOR_LOOP:
        case OP_INCREMENT_LOOP: {
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
        default:
            break;
        }
        i += next(instruction);
    }
    optimizer->important = head;
}

static void optimize(Compiler *C) {

    Scope *scope = C->scope;
    HymnFunction *func = scope->func;

    Optimizer optimizer = {0};
    optimizer.code = &func->code;
    optimizer.except = func->except;

    if (optimizer.code->count <= 2) {
        return;
    }

    interest(&optimizer);

#define COUNT() optimizer.code->count
#define INSTRUCTION(I) optimizer.code->instructions[I]
#define CONSTANT(I) optimizer.code->constants.values[optimizer.code->instructions[I]]
#define SET(I, O) optimizer.code->instructions[I] = O
#define JUMP_IF(T, F)                        \
    if (second == OP_JUMP_IF_TRUE) {         \
        rewrite(&optimizer, one, 1);         \
        update(&optimizer, one, T);          \
        continue;                            \
    } else if (second == OP_JUMP_IF_FALSE) { \
        rewrite(&optimizer, one, 1);         \
        update(&optimizer, one, F);          \
        continue;                            \
    }

    int one = 0;

    while (one < COUNT()) {

        uint8_t first = INSTRUCTION(one);
        int two = one + next(first);
        if (two >= COUNT()) {
            break;
        }
        uint8_t second = INSTRUCTION(two);

        switch (first) {
        case OP_INCREMENT_LOCAL_AND_SET: {
            if (second == OP_LOOP) {
                // set to increment loop and reuse loop jump
                move(&optimizer, two, OP_INCREMENT_LOOP, one);
                // delete loop
                rewrite(&optimizer, two, 1);
                // subtract 1 to account for one less byte to jump after deleting instruction
                uint8_t *instructions = optimizer.code->instructions;
                int jump = GET_JUMP(instructions, one, 3, 4) - 1;
                UPDATE_JUMP(instructions, one, 3, 4, jump)
                continue;
            }
            break;
        }
        case OP_POP:
        case OP_POP_TWO: {
            if (second == OP_VOID) {
                rewriting(&optimizer, one, 1, false);
                continue;
            }
            break;
        }
        case OP_POP_N: {
            if (second == OP_VOID) {
                rewriting(&optimizer, one, 2, false);
                continue;
            }
            break;
        }
        default:
            break;
        }

        if (adjustable(&optimizer, two) != NULL) {
            goto next;
        }

        switch (first) {
        case OP_GET_LOCAL: {
            if (second == OP_GET_LOCAL) {
                SET(one, OP_GET_LOCALS);
                rewrite(&optimizer, one + 2, 1);
                continue;
            }
            break;
        }
        case OP_GET_LOCALS: {
            if (second == OP_LESS_EQUAL) {
                int three = two + next(second);
                uint8_t third = three < COUNT() ? INSTRUCTION(three) : UINT8_MAX;
                if (third == OP_JUMP_IF_FALSE) {
                    move(&optimizer, three, OP_JUMP_IF_GREATER_LOCALS, one);
                    rewrite(&optimizer, two, 2);
                    continue;
                }
            }
            break;
        }
        case OP_POP: {
            if (second == OP_POP) {
                rewriting(&optimizer, one, 1, false);
                SET(one, OP_POP_TWO);
                continue;
            }
            break;
        }
        case OP_POP_TWO: {
            if (second == OP_POP) {
                SET(one, OP_POP_N);
                SET(one + 1, 3);
                continue;
            }
            break;
        }
        case OP_POP_N: {
            if (second == OP_POP) {
                uint8_t pop = INSTRUCTION(one + 1);
                if (pop < UINT8_MAX - 1) {
                    rewriting(&optimizer, one + 1, 1, false);
                    SET(one + 1, (uint8_t)(pop + 1));
                    continue;
                }
            }
            break;
        }
        default:
            break;
        }

        if (adjustable(&optimizer, one) != NULL) {
            goto next;
        }

        switch (first) {
        case OP_RETURN: {
            if (second == OP_VOID) {
                rewrite(&optimizer, two, 1);
                continue;
            }
            break;
        }
        case OP_CALL: {
            if (second == OP_RETURN) {
                SET(one, OP_TAIL_CALL);
                continue;
            }
            break;
        }
        case OP_GET_GLOBAL: {
            if (second == OP_GET_PROPERTY) {
                SET(one, OP_GET_GLOBAL_PROPERTY);
                rewrite(&optimizer, one + 2, 1);
                continue;
            }
            break;
        }
        case OP_GET_LOCAL: {
            if (second == OP_CONSTANT) {
                int three = two + next(second);
                uint8_t third = three < COUNT() ? INSTRUCTION(three) : UINT8_MAX;
                if (third == OP_ADD) {
                    HymnValue value = CONSTANT(two + 1);
                    if (hymn_is_int(value)) {
                        HymnInt add = hymn_as_int(value);
                        if (add >= 0 && add <= UINT8_MAX) {
                            uint8_t local = INSTRUCTION(one + 1);
                            rewrite(&optimizer, one, 2);
                            SET(one, OP_INCREMENT_LOCAL);
                            SET(one + 1, local);
                            SET(one + 2, (uint8_t)add);
                            continue;
                        }
                    }
                }
            }
            break;
        }
        case OP_GET_LOCALS: {
            if (second == OP_ADD) {
                SET(one, OP_ADD_LOCALS);
                rewrite(&optimizer, one + 3, 1);
                continue;
            } else if (second == OP_MODULO) {
                SET(one, OP_MODULO_LOCALS);
                rewrite(&optimizer, one + 3, 1);
                continue;
            } else if (second == OP_ARRAY_PUSH) {
                SET(one, OP_ARRAY_PUSH_LOCALS);
                rewrite(&optimizer, one + 3, 1);
                continue;
            }
            break;
        }
        case OP_INCREMENT_LOCAL: {
            if (second == OP_SET_LOCAL) {
                if (INSTRUCTION(one + 1) == INSTRUCTION(one + 4)) {
                    int three = two + next(second);
                    uint8_t third = three < COUNT() ? INSTRUCTION(three) : UINT8_MAX;
                    if (third == OP_POP) {
                        SET(one, OP_INCREMENT_LOCAL_AND_SET);
                        rewrite(&optimizer, one + 3, 3);
                        continue;
                    }
                }
            }
            break;
        }
        case OP_CONSTANT: {
            if (second == OP_NEGATE) {
                HymnValue value = CONSTANT(one + 1);
                if (hymn_is_int(value)) {
                    value.as.i = -value.as.i;
                } else if (hymn_is_float(value)) {
                    value.as.f = -value.as.f;
                }
                uint8_t constant = byte_code_new_constant(C, value);
                SET(one + 1, constant);
                rewrite(&optimizer, one + 2, 1);
                continue;
            } else if (second == OP_ADD) {
                HymnValue value = CONSTANT(one + 1);
                if (hymn_is_int(value)) {
                    HymnInt add = hymn_as_int(value);
                    if (add >= 0 && add <= UINT8_MAX) {
                        SET(one, OP_INCREMENT);
                        SET(one + 1, (uint8_t)add);
                        rewrite(&optimizer, one + 2, 1);
                        continue;
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
                rewrite(&optimizer, one, 1);
                update(&optimizer, one, OP_JUMP);
                continue;
            } else if (second == OP_JUMP_IF_FALSE) {
                deletion(&optimizer, two);
                rewrite(&optimizer, one, 4);
                continue;
            }
            break;
        }
        case OP_FALSE: {
            if (second == OP_JUMP_IF_TRUE) {
                deletion(&optimizer, two);
                rewrite(&optimizer, one, 4);
                continue;
            } else if (second == OP_JUMP_IF_FALSE) {
                rewrite(&optimizer, one, 1);
                update(&optimizer, one, OP_JUMP);
                continue;
            }
            break;
        }
        case OP_NOT: {
            if (second == OP_JUMP_IF_TRUE) {
                rewrite(&optimizer, one, 1);
                update(&optimizer, one, OP_JUMP_IF_FALSE);
                continue;
            } else if (second == OP_JUMP_IF_FALSE) {
                rewrite(&optimizer, one, 1);
                update(&optimizer, one, OP_JUMP_IF_TRUE);
                continue;
            }
            break;
        }
        default:
            break;
        }

    next:
        one = two;
    }

    Instruction *important = optimizer.important;
    while (important != NULL) {
        Instruction *next = important->next;
        free(important);
        important = next;
    }
}

static void echo_if_none(Compiler *C) {
    HymnByteCode *code = &C->scope->func->code;
    int count = code->count;
    if (C->barrier == count) return;
    if (C->pop == count) code->instructions[count - 1] = OP_ECHO;
}

static HymnFunction *end_function(Compiler *C) {
    Scope *scope = C->scope;
    HymnFunction *func = scope->func;
    if (scope->type == TYPE_DIRECT || scope->type == TYPE_REPL) echo_if_none(C);
    emit(C, OP_VOID);
#ifndef HYMN_NO_OPTIMIZE
    optimize(C);
#endif
    if (scope->type == TYPE_FUNCTION) func->source = hymn_substring(C->source, scope->begin, C->previous.start + C->previous.length);
    C->scope = scope->enclosing;
    return func;
}

static void compile_function(Compiler *C, enum FunctionType type, size_t begin) {
    Scope scope = {0};
    scope_init(C, &scope, type, begin);

    begin_scope(C);

    consume(C, TOKEN_LEFT_PAREN, "expected '(' after function name");

    HymnFunction *func = C->scope->func;

    if (!check(C, TOKEN_RIGHT_PAREN)) {
        do {
            func->arity++;
            if (func->arity > UINT8_MAX) {
                compile_error(C, &C->previous, "too many function parameters");
            }
            uint8_t parameter = variable(C, "expected parameter name");
            finalize_variable(C, parameter);
            type_declaration(C);
        } while (match(C, TOKEN_COMMA));
    }

    consume(C, TOKEN_RIGHT_PAREN, "expected ')' after function parameters");
    type_declaration(C);
    consume(C, TOKEN_LEFT_CURLY, "expected '{' after function parameters");

    while (!check(C, TOKEN_RIGHT_CURLY) && !check(C, TOKEN_EOF)) {
        declaration(C);
    }

    consume(C, TOKEN_RIGHT_CURLY, "expected '}' at end of function body");

    end_function(C);
    emit_constant(C, hymn_new_func_value(func));
}

static void function_expression(Compiler *C, bool assign) {
    (void)assign;
    compile_function(C, TYPE_FUNCTION, C->previous.start);
}

static void declare_function(Compiler *C) {
    size_t begin = C->previous.start;
    uint8_t global = variable(C, "expected function name");
    local_initialize(C);
    compile_function(C, TYPE_FUNCTION, begin);
    finalize_variable(C, global);
}

static void declaration(Compiler *C) {
    if (match(C, TOKEN_SET)) {
        define_new_variable(C);
    } else if (match(C, TOKEN_FUNCTION)) {
        declare_function(C);
    } else {
        statement(C);
    }
}

static void block(Compiler *C) {
    begin_scope(C);
    while (!check(C, TOKEN_RIGHT_CURLY) && !check(C, TOKEN_EOF)) {
        declaration(C);
    }
    end_scope(C);
}

static void if_statement(Compiler *C) {
    expression(C);
    int jump = emit_jump(C, OP_JUMP_IF_FALSE);

    free_jump_or_list(C);

    consume(C, TOKEN_LEFT_CURLY, "expected '{' after if statement");
    begin_scope(C);
    while (!check(C, TOKEN_RIGHT_CURLY) && !check(C, TOKEN_EOF)) {
        declaration(C);
    }
    end_scope(C);

    consume(C, TOKEN_RIGHT_CURLY, "expected '}' at end of if statement body");

    if (check(C, TOKEN_ELIF) || check(C, TOKEN_ELSE)) {
        JumpList jump_end = {0};
        jump_end.jump = emit_jump(C, OP_JUMP);
        JumpList *tail = &jump_end;

        while (match(C, TOKEN_ELIF)) {
            patch_jump(C, jump);
            free_jump_and_list(C);

            expression(C);
            jump = emit_jump(C, OP_JUMP_IF_FALSE);

            free_jump_or_list(C);

            consume(C, TOKEN_LEFT_CURLY, "expected '{' after elif statement");
            begin_scope(C);
            while (!check(C, TOKEN_RIGHT_CURLY) && !check(C, TOKEN_EOF)) {
                declaration(C);
            }
            end_scope(C);
            consume(C, TOKEN_RIGHT_CURLY, "expected '}' at end of elif statement body");

            JumpList *next = hymn_calloc(1, sizeof(JumpList));
            next->jump = emit_jump(C, OP_JUMP);

            tail->next = next;
            tail = next;
        }

        patch_jump(C, jump);
        free_jump_and_list(C);

        if (match(C, TOKEN_ELSE)) {
            consume(C, TOKEN_LEFT_CURLY, "expected '{' after else statement");
            block(C);
            consume(C, TOKEN_RIGHT_CURLY, "expected '}' at end of else statement body");
        }

        patch_jump(C, jump_end.jump);
        free_jumps(C, jump_end.next);
    } else {
        patch_jump(C, jump);
        free_jump_and_list(C);
    }
}

static void emit_loop(Compiler *C, int start) {
    emit(C, OP_LOOP);
    int offset = current(C)->count - start + 2;
    if (offset > UINT16_MAX) {
        compile_error(C, &C->previous, "loop is too large");
    }
    emit_short(C, (uint8_t)((offset >> 8) & UINT8_MAX), (uint8_t)(offset & UINT8_MAX));
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
        JumpList *next = C->jump->next;
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
        JumpList *next = C->jump_for->next;
        free(C->jump_for);
        C->jump_for = next;
    }
}

static void iterator_statement(Compiler *C, bool pair) {
    local_initialize(C);

    int index = C->scope->local_count;

    if (index <= 0 || index >= UINT8_MAX) {
        compile_error(C, &C->current, "too many local variables in iterator");
        return;
    }

    uint8_t value = (uint8_t)(index + 1);
    uint8_t object = (uint8_t)(index - 1);

    push_hidden_local(C);

    if (pair) {
        variable(C, "expected variable name in for loop");
        local_initialize(C);
        consume(C, TOKEN_IN, "expected 'in' after variable name in for loop");
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

    LoopList loop = {.start = start, .depth = C->scope->depth + 1, .next = C->loop, .is_for = true};
    C->loop = &loop;

    // BODY

    consume(C, TOKEN_LEFT_CURLY, "expected '{' after for loop declaration");
    block(C);

    // LOOP

    patch_jump_for_list(C);

    emit_short(C, OP_FOR_LOOP, object);
    int offset = current(C)->count - start + 2;
    if (offset > UINT16_MAX) {
        compile_error(C, &C->previous, "loop is too large");
    }
    emit_short(C, (uint8_t)((offset >> 8) & UINT8_MAX), (uint8_t)(offset & UINT8_MAX));

    // END

    C->loop = loop.next;

    patch_jump(C, jump);
    patch_jump_list(C);

    end_scope(C);

    consume(C, TOKEN_RIGHT_CURLY, "expected '}' at end of for loop");
}

static void for_statement(Compiler *C) {
    begin_scope(C);

    // ASSIGN

    uint8_t index = (uint8_t)C->scope->local_count;

    variable(C, "expected variable name in for loop");

    if (match(C, TOKEN_ASSIGN)) {
        expression(C);
        local_initialize(C);
        consume(C, TOKEN_COMMA, "expected ',' in for loop after variable assignment");
    } else if (match(C, TOKEN_COMMA)) {
        iterator_statement(C, true);
        return;
    } else if (match(C, TOKEN_IN)) {
        iterator_statement(C, false);
        return;
    } else {
        compile_error(C, &C->previous, "incomplete for loop declaration");
        return;
    }

    // COMPARE

    int compare = current(C)->count;

    expression(C);

    int jump = emit_jump(C, OP_JUMP_IF_FALSE);

    // INCREMENT

    int increment = current(C)->count;

    LoopList loop = {.start = increment, .depth = C->scope->depth + 1, .next = C->loop, .is_for = true};
    C->loop = &loop;

    if (match(C, TOKEN_COMMA)) {
        expression_statement(C);
    } else {
        emit_word(C, OP_INCREMENT_LOCAL_AND_SET, index, 1);
    }

    HymnByteCode *code = current(C);

    int count = code->count - increment;
    uint8_t *instructions = hymn_malloc_int(count, sizeof(uint8_t));
    int *lines = hymn_malloc_int(count, sizeof(int));
    hymn_mem_copy(instructions, &code->instructions[increment], count, sizeof(uint8_t));
    hymn_mem_copy(lines, &code->lines[increment], count, sizeof(int));
    code->count = increment;

    // BODY

    consume(C, TOKEN_LEFT_CURLY, "expected '{' after for loop declaration");
    block(C);

    // INCREMENT

    patch_jump_for_list(C);

    while (code->count + count > code->capacity) {
        code->capacity *= 2;
        code->instructions = hymn_realloc_int(code->instructions, code->capacity, sizeof(uint8_t));
        code->lines = hymn_realloc_int(code->lines, code->capacity, sizeof(int));
    }
    hymn_mem_copy(&code->instructions[code->count], instructions, count, sizeof(uint8_t));
    hymn_mem_copy(&code->lines[code->count], lines, count, sizeof(int));
    code->count += count;
    free(instructions);
    free(lines);

    emit_loop(C, compare);

    // END

    C->loop = loop.next;

    patch_jump(C, jump);
    patch_jump_list(C);

    end_scope(C);

    consume(C, TOKEN_RIGHT_CURLY, "expected '}' at end of for loop");
}

static void while_statement(Compiler *C) {
    int start = current(C)->count;

    LoopList loop = {.start = start, .depth = C->scope->depth + 1, .next = C->loop, .is_for = false};
    C->loop = &loop;

    expression(C);

    int jump = emit_jump(C, OP_JUMP_IF_FALSE);

    consume(C, TOKEN_LEFT_CURLY, "expected '{' after while loop declaration");
    block(C);
    emit_loop(C, start);

    C->loop = loop.next;

    patch_jump(C, jump);
    patch_jump_list(C);

    consume(C, TOKEN_RIGHT_CURLY, "expected '}' at end of while loop");
}

static void return_statement(Compiler *C) {
    if (C->scope->type != TYPE_FUNCTION) {
        compile_error(C, &C->previous, "return statement outside of function");
    }
    if (check(C, TOKEN_RIGHT_CURLY)) {
        emit(C, OP_VOID);
    } else {
        expression(C);
        emit(C, OP_RETURN);
    }
}

static void pop_stack_loop(Compiler *C) {
    int depth = C->loop->depth;
    Scope *scope = C->scope;
    for (int i = scope->local_count; i > 0; i--) {
        if (scope->locals[i - 1].depth < depth) {
            return;
        }
        emit_pop(C);
    }
}

static void break_statement(Compiler *C) {
    if (C->loop == NULL) {
        compile_error(C, &C->previous, "break statement outside of loop");
    }
    pop_stack_loop(C);
    JumpList *jump_next = C->jump;
    JumpList *jump = hymn_malloc(sizeof(JumpList));
    jump->jump = emit_jump(C, OP_JUMP);
    jump->depth = C->loop->depth;
    jump->next = jump_next;
    C->jump = jump;
}

static void continue_statement(Compiler *C) {
    if (C->loop == NULL) {
        compile_error(C, &C->previous, "continue statement outside of loop");
    }
    pop_stack_loop(C);
    if (C->loop->is_for) {
        JumpList *jump_next = C->jump_for;
        JumpList *jump = hymn_malloc(sizeof(JumpList));
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

    HymnFunction *func = C->scope->func;
    except->next = func->except;
    func->except = except;

    consume(C, TOKEN_LEFT_CURLY, "expected '{' after try declaration");
    begin_scope(C);
    while (!check(C, TOKEN_RIGHT_CURLY) && !check(C, TOKEN_EOF)) {
        declaration(C);
    }
    end_scope(C);

    int jump = emit_jump(C, OP_JUMP);

    consume(C, TOKEN_RIGHT_CURLY, "expected '}' at end of try statement");
    consume(C, TOKEN_EXCEPT, "expected 'except' at end of try statement");

    except->end = code->count;

    begin_scope(C);
    uint8_t message = variable(C, "expected variable name in exception declaration");
    finalize_variable(C, message);
    consume(C, TOKEN_LEFT_CURLY, "expected '{' after exception declaration");
    while (!check(C, TOKEN_RIGHT_CURLY) && !check(C, TOKEN_EOF)) {
        declaration(C);
    }
    end_scope(C);

    consume(C, TOKEN_RIGHT_CURLY, "expected '}' at end of exception statement");

    patch_jump(C, jump);
}

static void echo_statement(Compiler *C) {
    expression(C);
    emit(C, OP_ECHO);
}

static void print_statement(Compiler *C) {
    consume(C, TOKEN_LEFT_PAREN, "expected opening '(' in call to 'print'");
    expression(C);
    consume(C, TOKEN_COMMA, "not enough arguments in call to 'print' (expected 2)");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "expected closing ')' in call to 'print'");
    emit(C, OP_PRINT);
}

static void push_statement(Compiler *C) {
    consume(C, TOKEN_LEFT_PAREN, "expected opening '(' in call to 'push'");
    expression(C);
    consume(C, TOKEN_COMMA, "not enough arguments in call to 'push' (expected 2)");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "expected closing ')' in call to 'push'");
    emit(C, OP_ARRAY_PUSH);
}

static void insert_statement(Compiler *C) {
    consume(C, TOKEN_LEFT_PAREN, "expected opening '(' in call to 'insert'");
    expression(C);
    consume(C, TOKEN_COMMA, "not enough arguments in call to 'insert' (expected 3)");
    expression(C);
    consume(C, TOKEN_COMMA, "not enough arguments in call to 'insert' (expected 3)");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "expected closing ')' in call to 'insert'");
    emit(C, OP_INSERT);
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
    if (match(C, TOKEN_ECHO)) {
        echo_statement(C);
    } else if (match(C, TOKEN_PRINT)) {
        print_statement(C);
    } else if (match(C, TOKEN_PUSH)) {
        push_statement(C);
    } else if (match(C, TOKEN_INSERT)) {
        insert_statement(C);
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
    } else if (match(C, TOKEN_LEFT_CURLY)) {
        block(C);
        consume(C, TOKEN_RIGHT_CURLY, "expected '}' at end of block statement");
    } else {
        expression_statement(C);
    }
}

static void array_pop_expression(Compiler *C, bool assign) {
    (void)assign;
    consume(C, TOKEN_LEFT_PAREN, "expected opening '(' in call to 'pop'");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "expected closing ')' in call to 'pop'");
    emit(C, OP_ARRAY_POP);
}

static void delete_expression(Compiler *C, bool assign) {
    (void)assign;
    consume(C, TOKEN_LEFT_PAREN, "expected opening '(' in call to 'delete'");
    expression(C);
    consume(C, TOKEN_COMMA, "not enough arguments in call to 'delete' (expected 2)");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "expected closing ')' in call to 'delete'");
    emit(C, OP_DELETE);
}

static void len_expression(Compiler *C, bool assign) {
    (void)assign;
    consume(C, TOKEN_LEFT_PAREN, "expected opening '(' in call to 'len'");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "expected closing ')' in call to 'len'");
    emit(C, OP_LEN);
}

static void cast_integer_expression(Compiler *C, bool assign) {
    (void)assign;
    consume(C, TOKEN_LEFT_PAREN, "expected opening '(' in call to 'int'");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "expected closing ')' in call to 'int'");
    emit(C, OP_INT);
}

static void cast_float_expression(Compiler *C, bool assign) {
    (void)assign;
    consume(C, TOKEN_LEFT_PAREN, "expected opening '(' in call to 'float'");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "expected closing ')' in call to 'float'");
    emit(C, OP_FLOAT);
}

static void cast_string_expression(Compiler *C, bool assign) {
    (void)assign;
    consume(C, TOKEN_LEFT_PAREN, "expected opening '(' in call to 'str'");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "expected closing ')' in call to 'str'");
    emit(C, OP_STRING);
}

static void type_expression(Compiler *C, bool assign) {
    (void)assign;
    consume(C, TOKEN_LEFT_PAREN, "expected opening '(' in call to 'type'");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "expected closing ')' in call to 'type'");
    emit(C, OP_TYPE);
}

static void clear_expression(Compiler *C, bool assign) {
    (void)assign;
    consume(C, TOKEN_LEFT_PAREN, "expected opening '(' in call to 'clear'");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "expected closing ')' in call to 'clear'");
    emit(C, OP_CLEAR);
}

static void copy_expression(Compiler *C, bool assign) {
    (void)assign;
    consume(C, TOKEN_LEFT_PAREN, "expected opening '(' in call to 'copy'");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "expected closing ')' in call to 'copy'");
    emit(C, OP_COPY);
}

static void keys_expression(Compiler *C, bool assign) {
    (void)assign;
    consume(C, TOKEN_LEFT_PAREN, "expected opening '(' in call to 'keys'");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "expected closing ')' in call to 'keys'");
    emit(C, OP_KEYS);
}

static void index_expression(Compiler *C, bool assign) {
    (void)assign;
    consume(C, TOKEN_LEFT_PAREN, "expected opening '(' in call to 'index'");
    expression(C);
    consume(C, TOKEN_COMMA, "not enough arguments in call to 'index' (expected 2)");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "expected closing ')' in call to 'index'");
    emit(C, OP_INDEX);
}

static void exists_expression(Compiler *C, bool assign) {
    (void)assign;
    consume(C, TOKEN_LEFT_PAREN, "expected opening '(' in call to 'exists'");
    expression(C);
    consume(C, TOKEN_COMMA, "not enough arguments in call to 'exists' (expected 2)");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "expected closing ')' in call to 'exists'");
    emit(C, OP_EXISTS);
}

static void source_expression(Compiler *C, bool assign) {
    (void)assign;
    consume(C, TOKEN_LEFT_PAREN, "expected opening '(' in call to '_source'");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "expected closing ')' in call to '_source'");
    emit(C, OP_SOURCE);
}

static void opcode_expression(Compiler *C, bool assign) {
    (void)assign;
    consume(C, TOKEN_LEFT_PAREN, "expected opening '(' in call to '_opcodes'");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "expected closing ')' in call to '_opcodes'");
    emit(C, OP_CODES);
}

static void stack_expression(Compiler *C, bool assign) {
    (void)assign;
    consume(C, TOKEN_LEFT_PAREN, "expected opening '(' in call to '_stack'");
    consume(C, TOKEN_RIGHT_PAREN, "expected closing ')' in call to '_stack'");
    emit(C, OP_STACK);
}

static void reference_expression(Compiler *C, bool assign) {
    (void)assign;
    consume(C, TOKEN_LEFT_PAREN, "expected opening '(' in call to '_reference'");
    expression(C);
    consume(C, TOKEN_RIGHT_PAREN, "expected closing ')' in call to '_reference'");
    emit(C, OP_REFERENCE);
}

static void expression_statement(Compiler *C) {
    expression(C);
    emit_pop(C);
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

static CompileResult compile(Hymn *H, const char *script, const char *source, enum FunctionType type) {
    Scope scope = {0};
    Compiler C = {0};
    C.row = 1;
    C.column = 1;
    C.script = script;
    C.source = source;
    C.interactive = type == TYPE_REPL;
    C.size = strlen(source);
    C.previous.type = TOKEN_UNDEFINED;
    C.current.type = TOKEN_UNDEFINED;
    C.string_status = STRING_STATUS_NONE;
    C.H = H;
    C.pop = -1;
    C.barrier = -1;
    scope_init(&C, &scope, type, 0);

    advance(&C);
    while (!match(&C, TOKEN_EOF)) {
        declaration(&C);
    }

    HymnFunction *func = end_function(&C);

    if (C.error != NULL) {
        char *error = string_to_chars(C.error);
        hymn_string_delete(C.error);
        function_delete(func);
        return (CompileResult){.func = NULL, .error = error};
    }

    return (CompileResult){.func = func, .error = NULL};
}

HymnString *hymn_quote_string(HymnString *string) {
    size_t len = hymn_string_len(string);
    size_t extra = 2;
    for (size_t s = 0; s < len; s++) {
        char c = string[s];
        if (c == '\\' || c == '\"') {
            extra++;
        }
    }
    HymnString *quoted = hymn_new_string_with_capacity(len + extra);
    quoted[0] = '"';
    size_t q = 1;
    for (size_t s = 0; s < len; s++) {
        char c = string[s];
        if (c == '\\') {
            quoted[q++] = '\\';
            quoted[q++] = '\\';
        } else if (c == '"') {
            quoted[q++] = '\\';
            quoted[q++] = '"';
        } else {
            quoted[q++] = c;
        }
    }
    HymnStringHead *head = hymn_string_head(quoted);
    head->length = head->capacity;
    quoted[len + extra - 1] = '"';
    quoted[len + extra] = '\0';
    return quoted;
}

struct PointerSet {
    int count;
    int capacity;
    void **items;
};

static bool pointer_set_has(struct PointerSet *set, void *pointer) {
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

static void pointer_set_add(struct PointerSet *set, void *pointer) {
    if (set->items) {
        int count = set->count;
        if (count >= set->capacity) {
            set->capacity *= 2;
            set->items = hymn_realloc_int(set->items, set->capacity, sizeof(void *));
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
    case HYMN_VALUE_UNDEFINED: return hymn_new_string("undefined");
    case HYMN_VALUE_NONE: return hymn_new_string("none");
    case HYMN_VALUE_BOOL: return hymn_as_bool(value) ? hymn_new_string("true") : hymn_new_string("false");
    case HYMN_VALUE_INTEGER: return hymn_int_to_string(hymn_as_int(value));
    case HYMN_VALUE_FLOAT: return hymn_float_to_string(hymn_as_float(value));
    case HYMN_VALUE_STRING: {
        if (quote) return hymn_quote_string(hymn_as_string(value));
        return hymn_string_copy(hymn_as_string(value));
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
        for (HymnInt i = 0; i < array->length; i++) {
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
        int size = table->size;
        HymnObjectString **keys = hymn_malloc_int(size, sizeof(HymnObjectString *));
        unsigned int total = 0;
        unsigned int bins = table->bins;
        for (unsigned int i = 0; i < bins; i++) {
            HymnTableItem *item = table->items[i];
            while (item != NULL) {
                HymnString *string = item->key->string;
                unsigned int insert = 0;
                while (insert != total) {
                    if (strcmp(string, keys[insert]->string) < 0) {
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
        for (int i = 0; i < size; i++) {
            if (i != 0) {
                string = hymn_string_append(string, ", ");
            }
            HymnObjectString *key = keys[i];
            HymnValue item = table_get(table, key);
            HymnString *add = value_to_string_recusive(item, set, true);
            HymnString *quoting = hymn_quote_string(key->string);
            string = hymn_string_append(string, quoting);
            string = hymn_string_append(string, ": ");
            string = hymn_string_append(string, add);
            hymn_string_delete(quoting);
            hymn_string_delete(add);
        }
        string = hymn_string_append(string, " }");
        free(keys);
        return string;
    }
    case HYMN_VALUE_FUNC: {
        HymnFunction *func = hymn_as_func(value);
        if (func->name) return hymn_string_copy(func->name);
        if (func->script) return hymn_string_copy(func->script);
        return hymn_new_string("script");
    }
    case HYMN_VALUE_FUNC_NATIVE: return hymn_string_copy(hymn_as_native(value)->name->string);
    case HYMN_VALUE_POINTER: return hymn_string_format("%p", hymn_as_pointer(value));
    default:
        break;
    }
    return hymn_new_string("?");
}

HymnString *hymn_value_to_string(HymnValue value) {
    struct PointerSet set = {.count = 0, .capacity = 0, .items = NULL};
    HymnString *string = value_to_string_recusive(value, &set, false);
    free(set.items);
    return string;
}

static HymnString *value_concat(HymnValue a, HymnValue b) {
    HymnString *string = hymn_value_to_string(a);
    HymnString *second = hymn_value_to_string(b);
    string = hymn_string_append(string, second);
    hymn_string_delete(second);
    return string;
}

static HymnString *debug_value_to_string(HymnValue value) {
    HymnString *string = hymn_value_to_string(value);
    HymnString *format = hymn_string_format("%s: %s", hymn_value_type(value.is), string);
    hymn_string_delete(string);
    return format;
}

static void reset_stack(Hymn *H) {
    H->stack_top = H->stack;
    H->frame_count = 0;
}

#ifdef HYMN_NO_MEMORY
void hymn_reference_string(HymnObjectString *string) {
    (void)string;
}
#else
void hymn_reference_string(HymnObjectString *string) {
    string->count++;
}
#endif

#ifdef HYMN_NO_MEMORY
void hymn_reference(HymnValue value) {
    (void)value;
}
#else
void hymn_reference(HymnValue value) {
    switch (value.is) {
    case HYMN_VALUE_STRING:
        ((HymnObjectString *)value.as.o)->count++;
        return;
    case HYMN_VALUE_ARRAY:
        ((HymnArray *)value.as.o)->count++;
        return;
    case HYMN_VALUE_TABLE:
        ((HymnTable *)value.as.o)->count++;
        return;
    case HYMN_VALUE_FUNC:
        ((HymnFunction *)value.as.o)->count++;
        return;
    case HYMN_VALUE_FUNC_NATIVE:
        ((HymnNativeFunction *)value.as.o)->count++;
        return;
    default:
        return;
    }
}
#endif

#ifdef HYMN_NO_MEMORY
void hymn_dereference_string(Hymn *H, HymnObjectString *string) {
    (void)H;
    (void)string;
}
#else
void hymn_dereference_string(Hymn *H, HymnObjectString *string) {
    int count = --string->count;
    assert(count >= 0);
    if (count == 0) {
        set_remove(&H->strings, string->string);
        hymn_string_delete(string->string);
        free(string);
    }
}
#endif

#ifdef HYMN_NO_MEMORY
void hymn_dereference(Hymn *H, HymnValue value) {
    (void)H;
    (void)value;
}
#else
void hymn_dereference(Hymn *H, HymnValue value) {
    switch (value.is) {
    case HYMN_VALUE_STRING: {
        HymnObjectString *string = (HymnObjectString *)value.as.o;
        hymn_dereference_string(H, string);
        return;
    }
    case HYMN_VALUE_ARRAY: {
        HymnArray *array = (HymnArray *)value.as.o;
        int count = --array->count;
        assert(count >= 0);
        if (count == 0) {
            hymn_array_delete(H, array);
        }
        return;
    }
    case HYMN_VALUE_TABLE: {
        HymnTable *table = (HymnTable *)value.as.o;
        int count = --table->count;
        assert(count >= 0);
        if (count == 0) {
            table_delete(H, table);
        }
        return;
    }
    case HYMN_VALUE_FUNC: {
        HymnFunction *func = (HymnFunction *)value.as.o;
        int count = --func->count;
        assert(count >= 0);
        if (count == 0) {
            function_delete(func);
        }
        return;
    }
    case HYMN_VALUE_FUNC_NATIVE: {
        HymnNativeFunction *func = (HymnNativeFunction *)value.as.o;
        int count = --func->count;
        assert(count >= 0);
        if (count == 0) {
            native_function_delete(H, func);
        }
        return;
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

static void push_string(Hymn *H, HymnString *string) {
    HymnObjectString *intern = hymn_intern_string(H, string);
    hymn_reference_string(intern);
    push(H, hymn_new_string_value(intern));
}

static HymnFrame *exception(Hymn *H) {
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
        HymnValue message = pop(H);
        if (except != NULL) {
            while (H->stack_top != &frame->stack[except->locals]) {
                hymn_dereference(H, pop(H));
            }
            frame->ip = &instructions[except->end];
            push(H, message);
            return frame;
        }
        while (H->stack_top != frame->stack) {
            hymn_dereference(H, pop(H));
        }
        H->frame_count--;
        if (H->frame_count == 0 || func->name == NULL) {
            assert(H->error == NULL);
            H->error = hymn_value_to_string(message);
            hymn_dereference(H, message);
            return NULL;
        }
        push(H, message);
        frame = current_frame(H);
    }
}

static HymnString *stacktrace(Hymn *H) {
    HymnString *trace = hymn_new_string("");
    for (int i = H->frame_count - 1; i >= 0; i--) {
        HymnFrame *frame = &H->frames[i];
        HymnFunction *func = frame->func;
        int row = func->code.lines[frame->ip - func->code.instructions - 1];
        if (func->name == NULL) {
            if (func->script == NULL) {
                trace = string_append_format(trace, "  at script:%d", row);
            } else {
                trace = string_append_format(trace, "  at %s:%d", func->script, row);
            }
        } else if (func->script == NULL) {
            trace = string_append_format(trace, "  at %s script:%d", func->name, row);
        } else {
            trace = string_append_format(trace, "  at %s %s:%d", func->name, func->script, row);
        }
        if (i > 0) trace = hymn_string_append_char(trace, '\n');
    }
    return trace;
}

static HymnFrame *push_error(Hymn *H, HymnString *error) {
    HymnObjectString *message = hymn_intern_string(H, error);
    hymn_reference_string(message);
    push(H, hymn_new_string_value(message));
    return exception(H);
}

static HymnFrame *throw_existing_error(Hymn *H, char *error) {
    HymnString *message = hymn_new_string(error);
    free(error);
    return push_error(H, message);
}

static HymnFrame *throw_error(Hymn *H, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    size_t len = (size_t)vsnprintf(NULL, 0, format, ap);
    va_end(ap);
    char *chars = hymn_malloc((len + 1) * sizeof(char));
    va_start(ap, format);
    len = (size_t)vsnprintf(chars, len + 1, format, ap);
    va_end(ap);

    HymnString *error = hymn_new_string_with_capacity(len + 128);
    error = hymn_string_append(error, chars);

    free(chars);

    HymnString *trace = stacktrace(H);
    error = hymn_string_append(error, "\n");
    error = hymn_string_append(error, trace);
    hymn_string_delete(trace);

    return push_error(H, error);
}

static HymnFrame *throw_exception(Hymn *H, const char *name) {

    HymnString *trace = stacktrace(H);
    HymnString *error = hymn_new_string_with_capacity(hymn_string_len(H->exception) + hymn_string_len(trace) + 128);
    error = hymn_string_append(error, H->exception);

    hymn_string_delete(H->exception);
    H->exception = NULL;

    error = hymn_string_append(error, "\n  in ");
    error = hymn_string_append(error, name);
    HymnFrame *frame = &H->frames[H->frame_count - 1];
    HymnFunction *func = frame->func;
    int row = func->code.lines[frame->ip - func->code.instructions - 1];
    if (func->script == NULL) {
        error = string_append_format(error, " script:%d\n", row);
    } else {
        error = string_append_format(error, " %s:%d\n", func->script, row);
    }
    error = hymn_string_append(error, trace);
    hymn_string_delete(trace);

    return push_error(H, error);
}

static HymnFrame *throw_error_string(Hymn *H, HymnString *string) {
    HymnFrame *frame = throw_error(H, string);
    hymn_string_delete(string);
    return frame;
}

HymnValue hymn_new_exception(Hymn *H, const char *error) {
    H->exception = hymn_new_string(error);
    return hymn_new_none();
}

HymnValue hymn_arity_exception(Hymn *H, int expected, int actual) {
    H->exception = hymn_string_format("expected: %d function argument(s) but was: %d", expected, actual);
    return hymn_new_none();
}

HymnValue hymn_type_exception(Hymn *H, enum HymnValueType expected, enum HymnValueType actual) {
    H->exception = hymn_string_format("expected type: %s but was: %s", hymn_value_type(expected), hymn_value_type(actual));
    return hymn_new_none();
}

static HymnFrame *call(Hymn *H, HymnFunction *func, int count) {
    if (count != func->arity) {
        if (count < func->arity) return throw_error(H, "not enough arguments in call to '%s' (expected %d)", func->name, func->arity);
        return throw_error(H, "too many arguments in call to '%s' (expected %d)", func->name, func->arity);
    } else if (H->frame_count == HYMN_FRAMES_MAX) {
        return throw_error(H, "stack overflow");
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
        HymnNativeFunction *native = hymn_as_native(value);
        HymnNativeCall func = native->func;
        HymnValue result = func(H, count, H->stack_top - count);
        HymnValue *top = H->stack_top - count - 1;
        while (H->stack_top != top) {
            hymn_dereference(H, pop(H));
        }
        if (H->exception != NULL) {
            return throw_exception(H, native->name->string);
        } else {
            hymn_reference(result);
            push(H, result);
            return current_frame(H);
        }
    }
    default: {
        const char *is = hymn_value_type(value.is);
        return throw_error(H, "can't call %s (expected function)", is);
    }
    }
}

static HymnFrame *import(Hymn *H, HymnObjectString *file) {
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

    HymnString *look = hymn_path_convert(file->string);
    HymnString *parent = script ? hymn_path_parent(script) : NULL;

    HymnObjectString *module = NULL;

    HymnArray *paths = H->paths;
    HymnInt size = paths->length;
    for (HymnInt i = 0; i < size; i++) {
        HymnValue value = paths->items[i];
        if (!hymn_is_string(value)) {
            continue;
        }
        HymnString *question = hymn_as_string(value);

        HymnString *replace = hymn_string_replace(question, "<path>", look);
        HymnString *path = parent ? hymn_string_replace(replace, "<parent>", parent) : hymn_string_copy(replace);

        HymnObjectString *use = hymn_intern_string(H, hymn_path_absolute(path));
        hymn_reference_string(use);

        hymn_string_delete(path);
        hymn_string_delete(replace);

        if (!hymn_is_undefined(table_get(imports, use))) {
            hymn_dereference_string(H, use);
            hymn_string_delete(look);
            if (parent) hymn_string_delete(parent);
            return current_frame(H);
        }

        if (hymn_file_exists(use->string)) {
            module = use;
            break;
        }

        hymn_dereference_string(H, use);
    }

    if (module == NULL) {
        HymnString *missing = hymn_string_format("import not found: %s", look);

        for (HymnInt i = 0; i < size; i++) {
            HymnValue value = paths->items[i];
            if (!hymn_is_string(value)) {
                continue;
            }
            HymnString *question = hymn_as_string(value);

            HymnString *replace = hymn_string_replace(question, "<path>", look);
            HymnString *path = parent ? hymn_string_replace(replace, "<parent>", parent) : hymn_string_copy(replace);
            HymnString *use = hymn_path_absolute(path);

            missing = string_append_format(missing, "\n  no file: %s", use);

            hymn_string_delete(path);
            hymn_string_delete(replace);
            hymn_string_delete(use);
        }

        hymn_string_delete(look);
        if (parent) hymn_string_delete(parent);

        return throw_error_string(H, missing);
    }

    hymn_string_delete(look);
    if (parent != NULL) {
        hymn_string_delete(parent);
    }

    table_put(imports, module, hymn_new_bool(true));

    HymnString *source = hymn_read_file(module->string);
    if (source == NULL) {
        HymnString *failed = hymn_string_format("error reading file: %s\n", module->string);
        return throw_error_string(H, failed);
    }

    CompileResult result = compile(H, module->string, source, TYPE_SCRIPT);

    hymn_string_delete(source);

    char *error = result.error;
    if (error != NULL) {
        return throw_existing_error(H, error);
    }

    HymnFunction *func = result.func;
    HymnValue function = hymn_new_func_value(func);
    func->count = 1;

    push(H, function);
    call(H, func, 0);

    error = interpret(H);
    if (error != NULL) return throw_existing_error(H, error);

    return current_frame(H);
}

static int debug_constant_instruction(HymnString **debug, const char *name, HymnByteCode *code, int index) {
    uint8_t constant = code->instructions[index + 1];
    HymnString *value = debug_value_to_string(code->constants.values[constant]);
    *debug = string_append_format(*debug, "%s: [%d] [%s]", name, constant, value);
    hymn_string_delete(value);
    return index + 2;
}

static int debug_two_constant_instruction(HymnString **debug, const char *name, HymnByteCode *code, int index) {
    uint8_t constant = code->instructions[index + 1];
    HymnString *value = debug_value_to_string(code->constants.values[constant]);
    uint8_t constant2 = code->instructions[index + 2];
    HymnString *value2 = debug_value_to_string(code->constants.values[constant2]);
    *debug = string_append_format(*debug, "%s: [%d] [%s] & [%d] [%s]", name, constant, value, constant2, value2);
    hymn_string_delete(value);
    hymn_string_delete(value2);
    return index + 3;
}

static int debug_byte_instruction(HymnString **debug, const char *name, HymnByteCode *code, int index) {
    uint8_t byte = code->instructions[index + 1];
    *debug = string_append_format(*debug, "%s: [%d]", name, byte);
    return index + 2;
}

static int debug_jump_instruction(HymnString **debug, const char *name, int sign, HymnByteCode *code, int index) {
    int jump = ((int)code->instructions[index + 1] << 8) | (int)code->instructions[index + 2];
    *debug = string_append_format(*debug, "%s: [%zu] -> [%zu]", name, index, sign < 0 ? index + 3 - jump : index + 3 + jump);
    return index + 3;
}

static int debug_register_jump_instruction(HymnString **debug, const char *name, HymnByteCode *code, int index) {
    uint8_t slot_a = code->instructions[index + 1];
    uint8_t slot_b = code->instructions[index + 2];
    int jump = ((int)code->instructions[index + 3] << 8) | (int)code->instructions[index + 4];
    *debug = string_append_format(*debug, "%s: [%d] [%d] ? [%zu] -> [%zu]", name, slot_a, slot_b, index, index + 5 + jump);
    return index + 5;
}

static int debug_three_byte_instruction(HymnString **debug, const char *name, HymnByteCode *code, int index) {
    uint8_t byte = code->instructions[index + 1];
    uint8_t next = code->instructions[index + 2];
    *debug = string_append_format(*debug, "%s: [%d] [%d]", name, byte, next);
    return index + 3;
}

static int debug_for_loop_instruction(HymnString **debug, const char *name, int sign, HymnByteCode *code, int index) {
    uint8_t slot = code->instructions[index + 1];
    int jump = ((int)code->instructions[index + 2] << 8) | (int)code->instructions[index + 3];
    *debug = string_append_format(*debug, "%s: [%d] [%zu] -> [%zu]", name, slot, index, sign < 0 ? index + 4 - jump : index + 4 + jump);
    return index + 4;
}

static int debug_increment_loop_instruction(HymnString **debug, const char *name, HymnByteCode *code, int index) {
    uint8_t slot = code->instructions[index + 1];
    uint8_t increment = code->instructions[index + 2];
    int jump = ((int)code->instructions[index + 3] << 8) | (int)code->instructions[index + 4];
    *debug = string_append_format(*debug, "%s: [%d] [%d] & [%zu] -> [%zu]", name, slot, increment, index, index + 5 - jump);
    return index + 5;
}

static int debug_instruction(HymnString **debug, const char *name, int index) {
    *debug = string_append_format(*debug, "%s", name);
    return index + 1;
}

static int disassemble_instruction(HymnString **debug, HymnByteCode *code, int index) {
    *debug = string_append_format(*debug, "%04zu ", index);
    if (index > 0 && code->lines[index] == code->lines[index - 1]) {
        *debug = hymn_string_append(*debug, "   | ");
    } else {
        *debug = string_append_format(*debug, "%4d ", code->lines[index]);
    }
    uint8_t instruction = code->instructions[index];
    switch (instruction) {
    case OP_ADD: return debug_instruction(debug, "OP_ADD", index);
    case OP_ADD_LOCALS: return debug_three_byte_instruction(debug, "OP_ADD_LOCALS", code, index);
    case OP_INSERT: return debug_instruction(debug, "OP_INSERT", index);
    case OP_ARRAY_POP: return debug_instruction(debug, "OP_ARRAY_POP", index);
    case OP_ARRAY_PUSH: return debug_instruction(debug, "OP_ARRAY_PUSH", index);
    case OP_ARRAY_PUSH_LOCALS: return debug_three_byte_instruction(debug, "OP_ARRAY_PUSH_LOCALS", code, index);
    case OP_BIT_AND: return debug_instruction(debug, "OP_BIT_AND", index);
    case OP_BIT_LEFT_SHIFT: return debug_instruction(debug, "OP_BIT_LEFT_SHIFT", index);
    case OP_BIT_NOT: return debug_instruction(debug, "OP_BIT_NOT", index);
    case OP_BIT_OR: return debug_instruction(debug, "OP_BIT_OR", index);
    case OP_BIT_RIGHT_SHIFT: return debug_instruction(debug, "OP_BIT_RIGHT_SHIFT", index);
    case OP_BIT_XOR: return debug_instruction(debug, "OP_BIT_XOR", index);
    case OP_CALL: return debug_byte_instruction(debug, "OP_CALL", code, index);
    case OP_SELF: return debug_constant_instruction(debug, "OP_SELF", code, index);
    case OP_CLEAR: return debug_instruction(debug, "OP_CLEAR", index);
    case OP_CONSTANT: return debug_constant_instruction(debug, "OP_CONSTANT", code, index);
    case OP_NEW_ARRAY: return debug_instruction(debug, "OP_NEW_ARRAY", index);
    case OP_NEW_TABLE: return debug_instruction(debug, "OP_NEW_TABLE", index);
    case OP_COPY: return debug_instruction(debug, "OP_COPY", index);
    case OP_DEFINE_GLOBAL: return debug_constant_instruction(debug, "OP_DEFINE_GLOBAL", code, index);
    case OP_CODES: return debug_instruction(debug, "OP_CODES", index);
    case OP_STACK: return debug_instruction(debug, "OP_STACK", index);
    case OP_REFERENCE: return debug_instruction(debug, "OP_REFERENCE", index);
    case OP_DELETE: return debug_instruction(debug, "OP_DELETE", index);
    case OP_DIVIDE: return debug_instruction(debug, "OP_DIVIDE", index);
    case OP_DUPLICATE: return debug_instruction(debug, "OP_DUPLICATE", index);
    case OP_EQUAL: return debug_instruction(debug, "OP_EQUAL", index);
    case OP_ECHO: return debug_instruction(debug, "OP_ECHO", index);
    case OP_EXISTS: return debug_instruction(debug, "OP_EXISTS", index);
    case OP_FALSE: return debug_instruction(debug, "OP_FALSE", index);
    case OP_FOR: return debug_for_loop_instruction(debug, "OP_FOR", 1, code, index);
    case OP_FOR_LOOP: return debug_for_loop_instruction(debug, "OP_FOR_LOOP", -1, code, index);
    case OP_GET_DYNAMIC: return debug_instruction(debug, "OP_GET_DYNAMIC", index);
    case OP_GET_GLOBAL: return debug_constant_instruction(debug, "OP_GET_GLOBAL", code, index);
    case OP_GET_GLOBAL_PROPERTY: return debug_two_constant_instruction(debug, "OP_GET_GLOBAL_PROPERTY", code, index);
    case OP_GET_LOCAL: return debug_byte_instruction(debug, "OP_GET_LOCAL", code, index);
    case OP_GET_PROPERTY: return debug_constant_instruction(debug, "OP_GET_PROPERTY", code, index);
    case OP_GET_LOCALS: return debug_three_byte_instruction(debug, "OP_GET_LOCALS", code, index);
    case OP_GREATER: return debug_instruction(debug, "OP_GREATER", index);
    case OP_GREATER_EQUAL: return debug_instruction(debug, "OP_GREATER_EQUAL", index);
    case OP_INCREMENT: return debug_byte_instruction(debug, "OP_INCREMENT", code, index);
    case OP_INCREMENT_LOCAL: return debug_three_byte_instruction(debug, "OP_INCREMENT_LOCAL", code, index);
    case OP_INCREMENT_LOCAL_AND_SET: return debug_three_byte_instruction(debug, "OP_INCREMENT_LOCAL_AND_SET", code, index);
    case OP_INCREMENT_LOOP: return debug_increment_loop_instruction(debug, "OP_INCREMENT_LOOP", code, index);
    case OP_INDEX: return debug_instruction(debug, "OP_INDEX", index);
    case OP_SOURCE: return debug_instruction(debug, "OP_SOURCE", index);
    case OP_JUMP: return debug_jump_instruction(debug, "OP_JUMP", 1, code, index);
    case OP_JUMP_IF_EQUAL: return debug_jump_instruction(debug, "OP_JUMP_IF_EQUAL", 1, code, index);
    case OP_JUMP_IF_FALSE: return debug_jump_instruction(debug, "OP_JUMP_IF_FALSE", 1, code, index);
    case OP_JUMP_IF_GREATER: return debug_jump_instruction(debug, "OP_JUMP_IF_GREATER", 1, code, index);
    case OP_JUMP_IF_GREATER_LOCALS: return debug_register_jump_instruction(debug, "OP_JUMP_IF_GREATER_LOCALS", code, index);
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
    case OP_MODULO_LOCALS: return debug_three_byte_instruction(debug, "OP_MODULO_LOCALS", code, index);
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
    case OP_VOID: return debug_instruction(debug, "OP_VOID", index);
    case OP_SET_DYNAMIC: return debug_instruction(debug, "OP_SET_DYNAMIC", index);
    case OP_SET_GLOBAL: return debug_constant_instruction(debug, "OP_SET_GLOBAL", code, index);
    case OP_SET_LOCAL: return debug_byte_instruction(debug, "OP_SET_LOCAL", code, index);
    case OP_SET_PROPERTY: return debug_constant_instruction(debug, "OP_SET_PROPERTY", code, index);
    case OP_SLICE: return debug_instruction(debug, "OP_SLICE", index);
    case OP_SUBTRACT: return debug_instruction(debug, "OP_SUBTRACT", index);
    case OP_TAIL_CALL: return debug_byte_instruction(debug, "OP_TAIL_CALL", code, index);
    case OP_THROW: return debug_instruction(debug, "OP_THROW", index);
    case OP_FLOAT: return debug_instruction(debug, "OP_FLOAT", index);
    case OP_INT: return debug_instruction(debug, "OP_INT", index);
    case OP_STRING: return debug_instruction(debug, "OP_STRING", index);
    case OP_TRUE: return debug_instruction(debug, "OP_TRUE", index);
    case OP_TYPE: return debug_instruction(debug, "OP_TYPE", index);
    case OP_USE: return debug_instruction(debug, "OP_USE", index);
    default: *debug = string_append_format(*debug, "UNKNOWN_OPCODE %d\n", instruction); return index + 1;
    }
}

static HymnString *disassemble_byte_code(HymnByteCode *code) {
    HymnString *debug = hymn_new_string("");
    if (code->count > 0) {
        int offset = disassemble_instruction(&debug, code, 0);
        while (offset < code->count) {
            debug = hymn_string_append_char(debug, '\n');
            offset = disassemble_instruction(&debug, code, offset);
        }
    }
    return debug;
}

#define READ_BYTE(F) (*F->ip++)

#define READ_SHORT(F) (F->ip += 2, (((int)F->ip[-2] << 8) | (int)F->ip[-1]))

#define GET_CONSTANT(F, B) (F->func->code.constants.values[B])

#define READ_CONSTANT(F) (GET_CONSTANT(F, READ_BYTE(F)))

#define THROW(...)                         \
    frame = throw_error(H, ##__VA_ARGS__); \
    if (frame == NULL) return;             \
    goto dispatch;

#define COMPARE_OP(compare)                                                                       \
    HymnValue b = pop(H);                                                                         \
    HymnValue a = pop(H);                                                                         \
    if (hymn_is_int(a)) {                                                                         \
        if (hymn_is_int(b)) {                                                                     \
            push(H, hymn_new_bool(hymn_as_int(a) compare hymn_as_int(b)));                        \
        } else if (hymn_is_float(b)) {                                                            \
            push(H, hymn_new_bool((HymnFloat)hymn_as_int(a) compare hymn_as_float(b)));           \
        } else {                                                                                  \
            const char *is_a = hymn_value_type(a.is);                                             \
            const char *is_b = hymn_value_type(b.is);                                             \
            hymn_dereference(H, a);                                                               \
            hymn_dereference(H, b);                                                               \
            THROW("comparison '" #compare "' can't use %s and %s (expected numbers)", is_a, is_b) \
        }                                                                                         \
    } else if (hymn_is_float(a)) {                                                                \
        if (hymn_is_int(b)) {                                                                     \
            push(H, hymn_new_bool(hymn_as_float(a) compare(HymnFloat) hymn_as_int(b)));           \
        } else if (hymn_is_float(b)) {                                                            \
            push(H, hymn_new_bool(hymn_as_float(a) compare hymn_as_float(b)));                    \
        } else {                                                                                  \
            const char *is_a = hymn_value_type(a.is);                                             \
            const char *is_b = hymn_value_type(b.is);                                             \
            hymn_dereference(H, a);                                                               \
            hymn_dereference(H, b);                                                               \
            THROW("comparison '" #compare "' can't use %s and %s (expected numbers)", is_a, is_b) \
        }                                                                                         \
    } else {                                                                                      \
        const char *is_a = hymn_value_type(a.is);                                                 \
        const char *is_b = hymn_value_type(b.is);                                                 \
        hymn_dereference(H, a);                                                                   \
        hymn_dereference(H, b);                                                                   \
        THROW("comparison '" #compare "' can't use %s and %s (expected numbers)", is_a, is_b)     \
    }

#define JUMP_COMPARE_OP(compare)                                         \
    HymnValue b = pop(H);                                                \
    HymnValue a = pop(H);                                                \
    bool answer;                                                         \
    if (hymn_is_int(a)) {                                                \
        if (hymn_is_int(b)) {                                            \
            answer = hymn_as_int(a) compare hymn_as_int(b);              \
        } else if (hymn_is_float(b)) {                                   \
            answer = (HymnFloat)hymn_as_int(a) compare hymn_as_float(b); \
        } else {                                                         \
            hymn_dereference(H, a);                                      \
            hymn_dereference(H, b);                                      \
            THROW("comparison: operands must be numbers")                \
        }                                                                \
    } else if (hymn_is_float(a)) {                                       \
        if (hymn_is_int(b)) {                                            \
            answer = hymn_as_float(a) compare(HymnFloat) hymn_as_int(b); \
        } else if (hymn_is_float(b)) {                                   \
            answer = hymn_as_float(a) compare hymn_as_float(b);          \
        } else {                                                         \
            hymn_dereference(H, a);                                      \
            hymn_dereference(H, b);                                      \
            THROW("comparison: operands must be numbers")                \
        }                                                                \
    } else {                                                             \
        hymn_dereference(H, a);                                          \
        hymn_dereference(H, b);                                          \
        THROW("comparison: operands must be numbers")                    \
    }                                                                    \
    int jump = READ_SHORT(frame);                                        \
    if (answer) {                                                        \
        frame->ip += jump;                                               \
    }

static void run(Hymn *H) {
    HymnFrame *frame = current_frame(H);

dispatch:
    switch (READ_BYTE(frame)) {
    case OP_VOID: {
        H->frame_count--;
        bool done = H->frame_count == 0 || frame->func->name == NULL;
        while (H->stack_top != frame->stack) {
            hymn_dereference(H, pop(H));
        }
        if (done) return;
        push(H, hymn_new_none());
        frame = current_frame(H);
        goto dispatch;
    }
    case OP_RETURN: {
        HymnValue result = pop(H);
        H->frame_count--;
        bool done = H->frame_count == 0 || frame->func->name == NULL;
        while (H->stack_top != frame->stack) {
            hymn_dereference(H, pop(H));
        }
        if (done) return;
        push(H, result);
        frame = current_frame(H);
        goto dispatch;
    }
    case OP_POP: {
        hymn_dereference(H, pop(H));
        goto dispatch;
    }
    case OP_POP_TWO: {
        hymn_dereference(H, pop(H));
        hymn_dereference(H, pop(H));
        goto dispatch;
    }
    case OP_POP_N: {
        int count = READ_BYTE(frame);
        while (count--) {
            hymn_dereference(H, pop(H));
        }
        goto dispatch;
    }
    case OP_TRUE: {
        push(H, hymn_new_bool(true));
        goto dispatch;
    }
    case OP_FALSE: {
        push(H, hymn_new_bool(false));
        goto dispatch;
    }
    case OP_NONE: {
        push(H, hymn_new_none());
        goto dispatch;
    }
    case OP_CALL: {
        int count = READ_BYTE(frame);
        HymnValue value = peek(H, count + 1);
        frame = call_value(H, value, count);
        if (frame == NULL) return;
        goto dispatch;
    }
    case OP_TAIL_CALL: {
        int count = READ_BYTE(frame);
        HymnValue value = peek(H, count + 1);
        if (!hymn_is_func(value)) {
            frame = call_value(H, value, count);
        } else {
            HymnFunction *func = hymn_as_func(value);
            if (count != func->arity) {
                if (count < func->arity) frame = throw_error(H, "not enough arguments in call to '%s' (expected %d)", func->name, func->arity);
                frame = throw_error(H, "too many arguments in call to '%s' (expected %d)", func->name, func->arity);
                if (frame == NULL) return;
            } else {
                HymnValue *top = H->stack_top;
                HymnValue *new_frame = top - count - 1;
                HymnValue *bottom = frame->stack;
                HymnValue *shift = new_frame;
                while (bottom != new_frame) {
                    hymn_dereference(H, *bottom);
                    if (shift != top) {
                        *bottom = *shift;
                        shift++;
                    }
                    bottom++;
                }
                while (shift != top) {
                    *bottom = *shift;
                    shift++;
                    bottom++;
                }
                H->stack_top = frame->stack + count + 1;
                frame->func = func;
                frame->ip = func->code.instructions;
            }
        }
        if (frame == NULL) return;
        goto dispatch;
    }
    case OP_SELF: {
        HymnValue table = peek(H, 1);
        if (!hymn_is_table(table)) {
            const char *is = hymn_value_type(table.is);
            THROW("can't get property of %s (expected table)", is)
        }
        HymnObjectString *name = hymn_as_hymn_string(READ_CONSTANT(frame));
        HymnValue fun = table_get(hymn_as_table(table), name);
        *(H->stack_top - 1) = fun;
        hymn_reference(fun);
        *H->stack_top = table;
        H->stack_top++;
        goto dispatch;
    }
    case OP_JUMP: {
        int jump = READ_SHORT(frame);
        frame->ip += jump;
        goto dispatch;
    }
    case OP_JUMP_IF_FALSE: {
        HymnValue a = pop(H);
        int jump = READ_SHORT(frame);
        if (hymn_value_false(a)) {
            frame->ip += jump;
        }
        hymn_dereference(H, a);
        goto dispatch;
    }
    case OP_JUMP_IF_TRUE: {
        HymnValue a = pop(H);
        int jump = READ_SHORT(frame);
        if (!hymn_value_false(a)) {
            frame->ip += jump;
        }
        hymn_dereference(H, a);
        goto dispatch;
    }
    case OP_JUMP_IF_EQUAL: {
        HymnValue b = pop(H);
        HymnValue a = pop(H);
        int jump = READ_SHORT(frame);
        if (hymn_values_equal(a, b)) {
            frame->ip += jump;
        }
        hymn_dereference(H, a);
        hymn_dereference(H, b);
        goto dispatch;
    }
    case OP_JUMP_IF_NOT_EQUAL: {
        HymnValue b = pop(H);
        HymnValue a = pop(H);
        int jump = READ_SHORT(frame);
        if (!hymn_values_equal(a, b)) {
            frame->ip += jump;
        }
        hymn_dereference(H, a);
        hymn_dereference(H, b);
        goto dispatch;
    }
    case OP_JUMP_IF_LESS: {
        JUMP_COMPARE_OP(<)
        goto dispatch;
    }
    case OP_JUMP_IF_LESS_EQUAL: {
        JUMP_COMPARE_OP(<=)
        goto dispatch;
    }
    case OP_JUMP_IF_GREATER: {
        JUMP_COMPARE_OP(>)
        goto dispatch;
    }
    case OP_JUMP_IF_GREATER_LOCALS: {
        HymnValue a = frame->stack[READ_BYTE(frame)];
        HymnValue b = frame->stack[READ_BYTE(frame)];
        int jump = READ_SHORT(frame);
        bool answer;
        if (hymn_is_int(a)) {
            if (hymn_is_int(b)) {
                answer = hymn_as_int(a) > hymn_as_int(b);
            } else if (hymn_is_float(b)) {
                answer = (HymnFloat)hymn_as_int(a) > hymn_as_float(b);
            } else {
                THROW("comparison: operands must be numbers")
            }
        } else if (hymn_is_float(a)) {
            if (hymn_is_int(b)) {
                answer = hymn_as_float(a) > (HymnFloat)hymn_as_int(b);
            } else if (hymn_is_float(b)) {
                answer = hymn_as_float(a) > hymn_as_float(b);
            } else {
                THROW("comparison: operands must be numbers")
            }
        } else {
            THROW("comparison: operands must be numbers")
        }
        if (answer) {
            frame->ip += jump;
        }
        goto dispatch;
    }
    case OP_JUMP_IF_GREATER_EQUAL: {
        JUMP_COMPARE_OP(>=)
        goto dispatch;
    }
    case OP_LOOP: {
        int jump = READ_SHORT(frame);
        frame->ip -= jump;
        goto dispatch;
    }
    case OP_INCREMENT_LOOP: {
        int slot = READ_BYTE(frame);
        int increment = READ_BYTE(frame);
        int jump = READ_SHORT(frame);
        HymnValue value = frame->stack[slot];
        if (hymn_is_int(value)) {
            value.as.i += (HymnInt)increment;
        } else if (hymn_is_float(value)) {
            value.as.f += (HymnFloat)increment;
        } else {
            const char *is = hymn_value_type(value.is);
            THROW("expected a number but was '%s'", is)
        }
        frame->stack[slot] = value;
        frame->ip -= jump;
        goto dispatch;
    }
    case OP_FOR: {
        int slot = READ_BYTE(frame);
        HymnValue object = frame->stack[slot];
        H->stack_top += 2;
        if (hymn_is_table(object)) {
            HymnTable *table = hymn_as_table(object);
            HymnTableItem *next = table_next(table, NULL);
            if (next == NULL) {
                frame->stack[slot + 1] = hymn_new_none();
                frame->stack[slot + 2] = hymn_new_none();
                int jump = READ_SHORT(frame);
                frame->ip += jump;
            } else {
                frame->stack[slot + 1] = hymn_new_string_value(next->key);
                frame->stack[slot + 2] = next->value;
                hymn_reference_string(next->key);
                hymn_reference(next->value);
                frame->ip += 2;
            }
        } else if (hymn_is_array(object)) {
            HymnArray *array = hymn_as_array(object);
            if (array->length == 0) {
                frame->stack[slot + 1] = hymn_new_none();
                frame->stack[slot + 2] = hymn_new_none();
                int jump = READ_SHORT(frame);
                frame->ip += jump;
            } else {
                HymnValue item = array->items[0];
                frame->stack[slot + 1] = hymn_new_int(0);
                frame->stack[slot + 2] = item;
                hymn_reference(item);
                frame->ip += 2;
            }
        } else {
            frame->stack[slot + 1] = hymn_new_none();
            frame->stack[slot + 2] = hymn_new_none();
            const char *is = hymn_value_type(object.is);
            THROW("can't iterate over %s (expected array or table)", is)
        }
        goto dispatch;
    }
    case OP_FOR_LOOP: {
        int slot = READ_BYTE(frame);
        HymnValue object = frame->stack[slot];
        int index = slot + 1;
        int value = slot + 2;
        if (hymn_is_table(object)) {
            HymnTable *table = hymn_as_table(object);
            HymnObjectString *key = hymn_as_hymn_string(frame->stack[index]);
            HymnTableItem *next = table_next(table, key);
            if (next == NULL) {
                frame->ip += 2;
            } else {
                hymn_dereference(H, frame->stack[index]);
                hymn_dereference(H, frame->stack[value]);
                frame->stack[index] = hymn_new_string_value(next->key);
                frame->stack[value] = next->value;
                hymn_reference_string(next->key);
                hymn_reference(next->value);
                int jump = READ_SHORT(frame);
                frame->ip -= jump;
            }
        } else {
            HymnArray *array = hymn_as_array(object);
            HymnInt key = hymn_as_int(frame->stack[index]) + 1;
            if (key >= array->length) {
                frame->ip += 2;
            } else {
                hymn_dereference(H, frame->stack[value]);
                HymnValue item = array->items[key];
                frame->stack[index].as.i++;
                frame->stack[value] = item;
                hymn_reference(item);
                int jump = READ_SHORT(frame);
                frame->ip -= jump;
            }
        }
        goto dispatch;
    }
    case OP_EQUAL: {
        HymnValue b = pop(H);
        HymnValue a = pop(H);
        push(H, hymn_new_bool(hymn_values_equal(a, b)));
        hymn_dereference(H, a);
        hymn_dereference(H, b);
        goto dispatch;
    }
    case OP_NOT_EQUAL: {
        HymnValue b = pop(H);
        HymnValue a = pop(H);
        push(H, hymn_new_bool(!hymn_values_equal(a, b)));
        hymn_dereference(H, a);
        hymn_dereference(H, b);
        goto dispatch;
    }
    case OP_LESS: {
        COMPARE_OP(<)
        goto dispatch;
    }
    case OP_LESS_EQUAL: {
        COMPARE_OP(<=)
        goto dispatch;
    }
    case OP_GREATER: {
        COMPARE_OP(>)
        goto dispatch;
    }
    case OP_GREATER_EQUAL: {
        COMPARE_OP(>=)
        goto dispatch;
    }
    case OP_ADD: {
        HymnValue b = pop(H);
        HymnValue a = pop(H);
        if (hymn_is_none(a)) {
            if (hymn_is_string(b)) {
                push_string(H, value_concat(a, b));
            } else {
                goto bad_add;
            }
        } else if (hymn_is_bool(a)) {
            if (hymn_is_string(b)) {
                push_string(H, value_concat(a, b));
            } else {
                goto bad_add;
            }
        } else if (hymn_is_int(a)) {
            if (hymn_is_int(b)) {
                a.as.i += b.as.i;
                push(H, a);
            } else if (hymn_is_float(b)) {
                b.as.f += (HymnFloat)a.as.i;
                push(H, a);
            } else if (hymn_is_string(b)) {
                push_string(H, value_concat(a, b));
            } else {
                goto bad_add;
            }
        } else if (hymn_is_float(a)) {
            if (hymn_is_int(b)) {
                a.as.f += (HymnFloat)b.as.i;
                push(H, a);
            } else if (hymn_is_float(b)) {
                a.as.f += b.as.f;
                push(H, a);
            } else if (hymn_is_string(b)) {
                push_string(H, value_concat(a, b));
            } else {
                goto bad_add;
            }
        } else if (hymn_is_string(a)) {
            push_string(H, value_concat(a, b));
        } else {
            goto bad_add;
        }
        hymn_dereference(H, a);
        hymn_dereference(H, b);
        goto dispatch;
    bad_add:;
        const char *is_a = hymn_value_type(a.is);
        const char *is_b = hymn_value_type(b.is);
        hymn_dereference(H, a);
        hymn_dereference(H, b);
        THROW("can't add %s and %s", is_a, is_b)
    }
    case OP_ADD_LOCALS: {
        HymnValue a = frame->stack[READ_BYTE(frame)];
        HymnValue b = frame->stack[READ_BYTE(frame)];
        if (hymn_is_none(a)) {
            if (hymn_is_string(b)) {
                push_string(H, value_concat(a, b));
            } else {
                goto bad_add_two;
            }
        } else if (hymn_is_bool(a)) {
            if (hymn_is_string(b)) {
                push_string(H, value_concat(a, b));
            } else {
                goto bad_add_two;
            }
        } else if (hymn_is_int(a)) {
            if (hymn_is_int(b)) {
                a.as.i += b.as.i;
                push(H, a);
            } else if (hymn_is_float(b)) {
                b.as.f += (HymnFloat)a.as.i;
                push(H, a);
            } else if (hymn_is_string(b)) {
                push_string(H, value_concat(a, b));
            } else {
                goto bad_add_two;
            }
        } else if (hymn_is_float(a)) {
            if (hymn_is_int(b)) {
                a.as.f += (HymnFloat)b.as.i;
                push(H, a);
            } else if (hymn_is_float(b)) {
                a.as.f += b.as.f;
                push(H, a);
            } else if (hymn_is_string(b)) {
                push_string(H, value_concat(a, b));
            } else {
                goto bad_add_two;
            }
        } else if (hymn_is_string(a)) {
            push_string(H, value_concat(a, b));
        } else {
            goto bad_add_two;
        }
        goto dispatch;
    bad_add_two:;
        const char *is_a = hymn_value_type(a.is);
        const char *is_b = hymn_value_type(b.is);
        hymn_dereference(H, a);
        hymn_dereference(H, b);
        THROW("can't add %s and %s", is_a, is_b)
    }
    case OP_INCREMENT: {
        HymnValue a = pop(H);
        HymnInt increment = READ_BYTE(frame);
        if (hymn_is_none(a)) {
            goto bad_increment;
        } else if (hymn_is_bool(a)) {
            goto bad_increment;
        } else if (hymn_is_int(a)) {
            a.as.i += increment;
            push(H, a);
        } else if (hymn_is_float(a)) {
            a.as.f += (HymnFloat)increment;
            push(H, a);
        } else if (hymn_is_string(a)) {
            push_string(H, value_concat(a, hymn_new_int(increment)));
        } else {
            goto bad_increment;
        }
        hymn_dereference(H, a);
        goto dispatch;
    bad_increment:;
        const char *is = hymn_value_type(a.is);
        hymn_dereference(H, a);
        THROW("can't increment %s", is)
    }
    case OP_SUBTRACT: {
        HymnValue b = pop(H);
        HymnValue a = pop(H);
        if (hymn_is_int(a)) {
            if (hymn_is_int(b)) {
                a.as.i -= b.as.i;
                push(H, a);
            } else if (hymn_is_float(b)) {
                HymnValue new = hymn_new_float((HymnFloat)a.as.i);
                new.as.f -= b.as.f;
                push(H, new);
            } else {
                goto bad_subtract;
            }
        } else if (hymn_is_float(a)) {
            if (hymn_is_int(b)) {
                a.as.f -= (HymnFloat)b.as.i;
                push(H, a);
            } else if (hymn_is_float(b)) {
                a.as.f -= b.as.f;
                push(H, a);
            } else {
                goto bad_subtract;
            }
        } else {
            goto bad_subtract;
        }
        goto dispatch;
    bad_subtract:;
        const char *is_a = hymn_value_type(a.is);
        const char *is_b = hymn_value_type(b.is);
        hymn_dereference(H, a);
        hymn_dereference(H, b);
        THROW("can't subtract %s and %s (expected numbers)", is_a, is_b)
    }
    case OP_MULTIPLY: {
        HymnValue b = pop(H);
        HymnValue a = pop(H);
        if (hymn_is_int(a)) {
            if (hymn_is_int(b)) {
                a.as.i *= b.as.i;
                push(H, a);
            } else if (hymn_is_float(b)) {
                HymnValue new = hymn_new_float((HymnFloat)a.as.i);
                new.as.f *= b.as.f;
                push(H, new);
            } else {
                goto bad_multiply;
            }
        } else if (hymn_is_float(a)) {
            if (hymn_is_int(b)) {
                a.as.f *= (HymnFloat)b.as.i;
                push(H, a);
            } else if (hymn_is_float(b)) {
                a.as.f *= b.as.f;
                push(H, a);
            } else {
                goto bad_multiply;
            }
        } else {
            goto bad_multiply;
        }
        goto dispatch;
    bad_multiply:;
        const char *is_a = hymn_value_type(a.is);
        const char *is_b = hymn_value_type(b.is);
        hymn_dereference(H, a);
        hymn_dereference(H, b);
        THROW("can't multiply %s and %s (expected numbers)", is_a, is_b)
    }
    case OP_DIVIDE: {
        HymnValue b = pop(H);
        HymnValue a = pop(H);
        if (hymn_is_int(a)) {
            if (hymn_is_int(b)) {
                a.as.i /= b.as.i;
                push(H, a);
            } else if (hymn_is_float(b)) {
                HymnValue new = hymn_new_float((HymnFloat)a.as.i);
                new.as.f /= b.as.f;
                push(H, new);
            } else {
                goto bad_divide;
            }
        } else if (hymn_is_float(a)) {
            if (hymn_is_int(b)) {
                a.as.f /= (HymnFloat)b.as.i;
                push(H, a);
            } else if (hymn_is_float(b)) {
                a.as.f /= b.as.f;
                push(H, a);
            } else {
                goto bad_divide;
            }
        } else {
            goto bad_divide;
        }
        goto dispatch;
    bad_divide:;
        const char *is_a = hymn_value_type(a.is);
        const char *is_b = hymn_value_type(b.is);
        hymn_dereference(H, a);
        hymn_dereference(H, b);
        THROW("can't divide %s and %s (expected numbers)", is_a, is_b)
    }
    case OP_MODULO: {
        HymnValue b = pop(H);
        HymnValue a = pop(H);
        if (hymn_is_int(a)) {
            if (hymn_is_int(b)) {
                a.as.i %= b.as.i;
                push(H, a);
            } else {
                goto bad_modulo;
            }
        } else {
            goto bad_modulo;
        }
        goto dispatch;
    bad_modulo:;
        const char *is_a = hymn_value_type(a.is);
        const char *is_b = hymn_value_type(b.is);
        hymn_dereference(H, a);
        hymn_dereference(H, b);
        THROW("can't modulo %s and %s (expected integers)", is_a, is_b)
    }
    case OP_MODULO_LOCALS: {
        HymnValue a = frame->stack[READ_BYTE(frame)];
        HymnValue b = frame->stack[READ_BYTE(frame)];
        if (hymn_is_int(a)) {
            if (hymn_is_int(b)) {
                a.as.i %= b.as.i;
                push(H, a);
            } else {
                goto bad_modulo_locals;
            }
        } else {
            goto bad_modulo_locals;
        }
        goto dispatch;
    bad_modulo_locals:;
        const char *is_a = hymn_value_type(a.is);
        const char *is_b = hymn_value_type(b.is);
        THROW("can't modulo %s and %s (expected integers)", is_a, is_b)
    }
    case OP_BIT_NOT: {
        HymnValue value = pop(H);
        if (hymn_is_int(value)) {
            value.as.i = ~value.as.i;
            push(H, value);
        } else {
            const char *is = hymn_value_type(value.is);
            hymn_dereference(H, value);
            THROW("bitwise '~' can't use %s (expected integer)", is)
        }
        goto dispatch;
    }
    case OP_BIT_OR: {
        HymnValue b = pop(H);
        HymnValue a = pop(H);
        if (hymn_is_int(a) && hymn_is_int(b)) {
            a.as.i |= b.as.i;
            push(H, a);
        } else {
            const char *is_a = hymn_value_type(a.is);
            const char *is_b = hymn_value_type(b.is);
            hymn_dereference(H, a);
            hymn_dereference(H, b);
            THROW("bitwise '|' can't use %s and %s (expected integers)", is_a, is_b)
        }
        goto dispatch;
    }
    case OP_BIT_AND: {
        HymnValue b = pop(H);
        HymnValue a = pop(H);
        if (hymn_is_int(a) && hymn_is_int(b)) {
            a.as.i &= b.as.i;
            push(H, a);
        } else {
            const char *is_a = hymn_value_type(a.is);
            const char *is_b = hymn_value_type(b.is);
            hymn_dereference(H, a);
            hymn_dereference(H, b);
            THROW("bitwise '&' can't use %s and %s (expected integers)", is_a, is_b)
        }
        goto dispatch;
    }
    case OP_BIT_XOR: {
        HymnValue b = pop(H);
        HymnValue a = pop(H);
        if (hymn_is_int(a) && hymn_is_int(b)) {
            a.as.i ^= b.as.i;
            push(H, a);
        } else {
            const char *is_a = hymn_value_type(a.is);
            const char *is_b = hymn_value_type(b.is);
            hymn_dereference(H, a);
            hymn_dereference(H, b);
            THROW("bitwise '^' can't use %s and %s (expected integers)", is_a, is_b)
        }
        goto dispatch;
    }
    case OP_BIT_LEFT_SHIFT: {
        HymnValue b = pop(H);
        HymnValue a = pop(H);
        if (hymn_is_int(a) && hymn_is_int(b)) {
            a.as.i <<= b.as.i;
            push(H, a);
        } else {
            const char *is_a = hymn_value_type(a.is);
            const char *is_b = hymn_value_type(b.is);
            hymn_dereference(H, a);
            hymn_dereference(H, b);
            THROW("bitwise '<<' can't use %s and %s (expected integers)", is_a, is_b)
        }
        goto dispatch;
    }
    case OP_BIT_RIGHT_SHIFT: {
        HymnValue b = pop(H);
        HymnValue a = pop(H);
        if (hymn_is_int(a) && hymn_is_int(b)) {
            a.as.i >>= b.as.i;
            push(H, a);
        } else {
            const char *is_a = hymn_value_type(a.is);
            const char *is_b = hymn_value_type(b.is);
            hymn_dereference(H, a);
            hymn_dereference(H, b);
            THROW("bitwise '>>' can't use %s and %s (expected integers)", is_a, is_b)
        }
        goto dispatch;
    }
    case OP_NEGATE: {
        HymnValue value = pop(H);
        if (hymn_is_int(value)) {
            value.as.i = -value.as.i;
        } else if (hymn_is_float(value)) {
            value.as.f = -value.as.f;
        } else {
            const char *is = hymn_value_type(value.is);
            hymn_dereference(H, value);
            THROW("negation '-' can't use %s (expected number)", is)
        }
        push(H, value);
        goto dispatch;
    }
    case OP_NOT: {
        HymnValue value = pop(H);
        if (hymn_is_bool(value)) {
            value.as.b = !value.as.b;
        } else {
            const char *is = hymn_value_type(value.is);
            hymn_dereference(H, value);
            THROW("not '!' can't use %s (expected boolean)", is)
        }
        push(H, value);
        goto dispatch;
    }
    case OP_CONSTANT: {
        HymnValue constant = READ_CONSTANT(frame);
        hymn_reference(constant);
        push(H, constant);
        goto dispatch;
    }
    case OP_NEW_ARRAY: {
        HymnValue constant = hymn_new_array_value(hymn_new_array(0));
        hymn_reference(constant);
        push(H, constant);
        goto dispatch;
    }
    case OP_NEW_TABLE: {
        HymnValue constant = hymn_new_table_value(hymn_new_table());
        hymn_reference(constant);
        push(H, constant);
        goto dispatch;
    }
    case OP_DEFINE_GLOBAL: {
        HymnObjectString *name = hymn_as_hymn_string(READ_CONSTANT(frame));
        HymnValue value = pop(H);
        HymnValue previous = table_put(&H->globals, name, value);
        if (hymn_is_undefined(previous)) {
            hymn_reference_string(name);
        } else {
            table_put(&H->globals, name, previous);
            hymn_dereference(H, value);
            THROW("multiple global definitions of '%s'", name->string)
        }
        goto dispatch;
    }
    case OP_SET_GLOBAL: {
        HymnObjectString *name = hymn_as_hymn_string(READ_CONSTANT(frame));
        HymnValue value = peek(H, 1);
        HymnValue previous = table_put(&H->globals, name, value);
        if (hymn_is_undefined(previous)) {
            hymn_reference_string(name);
        } else {
            hymn_dereference(H, previous);
        }
        hymn_reference(value);
        goto dispatch;
    }
    case OP_GET_GLOBAL: {
        HymnObjectString *name = hymn_as_hymn_string(READ_CONSTANT(frame));
        HymnValue get = table_get(&H->globals, name);
        if (hymn_is_undefined(get)) {
            THROW("undefined global '%s'", name->string)
        }
        hymn_reference(get);
        push(H, get);
        goto dispatch;
    }
    case OP_GET_GLOBAL_PROPERTY: {
        HymnObjectString *name = hymn_as_hymn_string(READ_CONSTANT(frame));
        HymnObjectString *property = hymn_as_hymn_string(READ_CONSTANT(frame));
        HymnValue global = table_get(&H->globals, name);
        if (hymn_is_undefined(global)) {
            THROW("undefined global '%s'", name->string)
        } else if (!hymn_is_table(global)) {
            const char *is = hymn_value_type(global.is);
            THROW("can't get property of %s (expected table)", is)
        }
        HymnTable *table = hymn_as_table(global);
        HymnValue get = table_get(table, property);
        if (hymn_is_undefined(get)) {
            get.is = HYMN_VALUE_NONE;
        } else {
            hymn_reference(get);
        }
        push(H, get);
        goto dispatch;
    }
    case OP_SET_LOCAL: {
        int slot = READ_BYTE(frame);
        HymnValue value = peek(H, 1);
        hymn_dereference(H, frame->stack[slot]);
        frame->stack[slot] = value;
        hymn_reference(value);
        goto dispatch;
    }
    case OP_GET_LOCAL: {
        int slot = READ_BYTE(frame);
        HymnValue value = frame->stack[slot];
        hymn_reference(value);
        push(H, value);
        goto dispatch;
    }
    case OP_GET_LOCALS: {
        int slot_a = READ_BYTE(frame);
        int slot_b = READ_BYTE(frame);
        HymnValue value_a = frame->stack[slot_a];
        HymnValue value_b = frame->stack[slot_b];
        hymn_reference(value_a);
        hymn_reference(value_b);
        push(H, value_a);
        push(H, value_b);
        goto dispatch;
    }
    case OP_INCREMENT_LOCAL: {
        int slot = READ_BYTE(frame);
        int increment = READ_BYTE(frame);
        HymnValue value = frame->stack[slot];
        if (hymn_is_int(value)) {
            value.as.i += (HymnInt)increment;
        } else if (hymn_is_float(value)) {
            value.as.f += (HymnFloat)increment;
        } else {
            const char *is = hymn_value_type(value.is);
            THROW("can't increment %s (expected number)", is)
        }
        push(H, value);
        goto dispatch;
    }
    case OP_INCREMENT_LOCAL_AND_SET: {
        int slot = READ_BYTE(frame);
        int increment = READ_BYTE(frame);
        HymnValue value = frame->stack[slot];
        if (hymn_is_int(value)) {
            value.as.i += (HymnInt)increment;
        } else if (hymn_is_float(value)) {
            value.as.f += (HymnFloat)increment;
        } else {
            const char *is = hymn_value_type(value.is);
            THROW("can't increment %s (expected number)", is)
        }
        frame->stack[slot] = value;
        goto dispatch;
    }
    case OP_SET_PROPERTY: {
        HymnValue value = pop(H);
        HymnValue table_value = pop(H);
        if (!hymn_is_table(table_value)) {
            const char *is = hymn_value_type(table_value.is);
            hymn_dereference(H, value);
            hymn_dereference(H, table_value);
            THROW("can't set property of %s (expected table)", is)
        }
        HymnTable *table = hymn_as_table(table_value);
        HymnObjectString *name = hymn_as_hymn_string(READ_CONSTANT(frame));
        hymn_set_property(H, table, name, value);
        push(H, value);
        hymn_dereference(H, table_value);
        goto dispatch;
    }
    case OP_GET_PROPERTY: {
        HymnValue value = pop(H);
        if (!hymn_is_table(value)) {
            const char *is = hymn_value_type(value.is);
            hymn_dereference(H, value);
            THROW("can't get property of %s (expected table)", is)
        }
        HymnTable *table = hymn_as_table(value);
        HymnObjectString *name = hymn_as_hymn_string(READ_CONSTANT(frame));
        HymnValue get = table_get(table, name);
        if (hymn_is_undefined(get)) {
            get.is = HYMN_VALUE_NONE;
        } else {
            hymn_reference(get);
        }
        hymn_dereference(H, value);
        push(H, get);
        goto dispatch;
    }
    case OP_EXISTS: {
        HymnValue value = pop(H);
        HymnValue object = pop(H);
        if (!hymn_is_table(object)) {
            const char *is = hymn_value_type(object.is);
            hymn_dereference(H, value);
            hymn_dereference(H, object);
            THROW("call to 'exists' can't use %s for 1st argument (expected table)", is)
        }
        if (!hymn_is_string(value)) {
            const char *is = hymn_value_type(value.is);
            hymn_dereference(H, value);
            hymn_dereference(H, object);
            THROW("call to 'exists' can't use %s for 2nd argument (expected string)", is)
        }
        HymnTable *table = hymn_as_table(object);
        HymnObjectString *name = hymn_as_hymn_string(value);
        HymnValue g = table_get(table, name);
        if (hymn_is_undefined(g)) {
            push(H, hymn_new_bool(false));
        } else {
            push(H, hymn_new_bool(true));
        }
        hymn_dereference(H, value);
        hymn_dereference(H, object);
        goto dispatch;
    }
    case OP_SET_DYNAMIC: {
        HymnValue value = pop(H);
        HymnValue property = pop(H);
        HymnValue object = pop(H);
        if (hymn_is_array(object)) {
            if (!hymn_is_int(property)) {
                const char *is = hymn_value_type(property.is);
                hymn_dereference(H, value);
                hymn_dereference(H, property);
                hymn_dereference(H, object);
                THROW("array assignment index can't be %s (expected integer)", is)
            }
            HymnArray *array = hymn_as_array(object);
            HymnInt size = array->length;
            HymnInt index = hymn_as_int(property);
            if (index > size) {
                hymn_dereference(H, value);
                hymn_dereference(H, property);
                hymn_dereference(H, object);
                THROW("array assignment index out of bounds: %d > %d", index, size)
            }
            if (index < 0) {
                index = size + index;
                if (index < 0) {
                    hymn_dereference(H, value);
                    hymn_dereference(H, property);
                    hymn_dereference(H, object);
                    THROW("negative array assignment index: %d", index)
                }
            }
            if (index == size) {
                hymn_array_push(array, value);
            } else {
                hymn_dereference(H, array->items[index]);
                array->items[index] = value;
            }
        } else if (hymn_is_table(object)) {
            if (!hymn_is_string(property)) {
                const char *is = hymn_value_type(property.is);
                hymn_dereference(H, value);
                hymn_dereference(H, property);
                hymn_dereference(H, object);
                THROW("table assignment key can't be %s (expected string)", is)
            }
            HymnTable *table = hymn_as_table(object);
            HymnObjectString *name = hymn_as_hymn_string(property);
            HymnValue previous = table_put(table, name, value);
            if (hymn_is_undefined(previous)) {
                hymn_reference_string(name);
            } else {
                hymn_dereference(H, previous);
            }
        } else {
            const char *is = hymn_value_type(object.is);
            hymn_dereference(H, value);
            hymn_dereference(H, property);
            hymn_dereference(H, object);
            THROW("can't assign value to %s (expected array or table)", is)
        }
        push(H, value);
        hymn_dereference(H, object);
        hymn_reference(value);
        goto dispatch;
    }
    case OP_GET_DYNAMIC: {
        HymnValue i = pop(H);
        HymnValue v = pop(H);
        switch (v.is) {
        case HYMN_VALUE_STRING: {
            if (!hymn_is_int(i)) {
                const char *is = hymn_value_type(i.is);
                hymn_dereference(H, i);
                hymn_dereference(H, v);
                THROW("string index can't be %s (expected integer)", is)
            }
            HymnString *string = hymn_as_string(v);
            HymnInt size = (HymnInt)hymn_string_len(string);
            HymnInt index = hymn_as_int(i);
            if (index >= size) {
                hymn_dereference(H, i);
                hymn_dereference(H, v);
                THROW("string index out of bounds: %d >= %d", index, size)
            }
            if (index < 0) {
                index = size + index;
                if (index < 0) {
                    hymn_dereference(H, i);
                    hymn_dereference(H, v);
                    THROW("negative string index: %d", index)
                }
            }
            char c = string[index];
            push_string(H, char_to_string(c));
            hymn_dereference(H, v);
            break;
        }
        case HYMN_VALUE_ARRAY: {
            if (!hymn_is_int(i)) {
                const char *is = hymn_value_type(i.is);
                hymn_dereference(H, i);
                hymn_dereference(H, v);
                THROW("array index can't be %s (expected integer)", is)
            }
            HymnArray *array = hymn_as_array(v);
            HymnInt size = array->length;
            HymnInt index = hymn_as_int(i);
            if (index >= size) {
                hymn_dereference(H, i);
                hymn_dereference(H, v);
                THROW("array index out of bounds: %d >= %d", index, size)
            }
            if (index < 0) {
                index = size + index;
                if (index < 0) {
                    hymn_dereference(H, i);
                    hymn_dereference(H, v);
                    THROW("negative array index: %d", index)
                }
            }
            HymnValue g = hymn_array_get(array, index);
            hymn_reference(g);
            push(H, g);
            hymn_dereference(H, v);
            break;
        }
        case HYMN_VALUE_TABLE: {
            if (!hymn_is_string(i)) {
                const char *is = hymn_value_type(i.is);
                hymn_dereference(H, i);
                hymn_dereference(H, v);
                THROW("table key can't be %s (expected string)", is)
            }
            HymnTable *table = hymn_as_table(v);
            HymnObjectString *name = hymn_as_hymn_string(i);
            HymnValue g = table_get(table, name);
            if (hymn_is_undefined(g)) {
                g.is = HYMN_VALUE_NONE;
            } else {
                hymn_reference(g);
            }
            push(H, g);
            hymn_dereference(H, i);
            hymn_dereference(H, v);
            break;
        }
        default: {
            const char *is = hymn_value_type(v.is);
            hymn_dereference(H, i);
            hymn_dereference(H, v);
            THROW("can't get value from %s (expected array, table, or string)", is)
        }
        }
        goto dispatch;
    }
    case OP_LEN: {
        HymnValue value = pop(H);
        switch (value.is) {
        case HYMN_VALUE_STRING: {
            HymnInt len = (HymnInt)hymn_string_len(hymn_as_string(value));
            push(H, hymn_new_int(len));
            break;
        }
        case HYMN_VALUE_ARRAY: {
            HymnInt len = hymn_as_array(value)->length;
            push(H, hymn_new_int(len));
            break;
        }
        case HYMN_VALUE_TABLE: {
            HymnInt len = (HymnInt)hymn_as_table(value)->size;
            push(H, hymn_new_int(len));
            break;
        }
        default: {
            const char *is = hymn_value_type(value.is);
            hymn_dereference(H, value);
            THROW("call to 'len' can't use %s (expected array, string, or table)", is)
        }
        }
        hymn_dereference(H, value);
        goto dispatch;
    }
    case OP_ARRAY_POP: {
        HymnValue a = pop(H);
        if (!hymn_is_array(a)) {
            const char *is = hymn_value_type(a.is);
            hymn_dereference(H, a);
            THROW("call to 'pop' can't use %s (expected array)", is)
        } else {
            HymnValue value = hymn_array_pop(hymn_as_array(a));
            push(H, value);
            hymn_dereference(H, a);
        }
        goto dispatch;
    }
    case OP_ARRAY_PUSH: {
        HymnValue value = pop(H);
        HymnValue array = pop(H);
        if (!hymn_is_array(array)) {
            const char *is = hymn_value_type(array.is);
            hymn_dereference(H, array);
            hymn_dereference(H, value);
            THROW("call to 'push' can't use %s for 1st argument (expected array)", is)
        } else {
            hymn_array_push(hymn_as_array(array), value);
            hymn_dereference(H, array);
        }
        goto dispatch;
    }
    case OP_ARRAY_PUSH_LOCALS: {
        HymnValue array = frame->stack[READ_BYTE(frame)];
        if (!hymn_is_array(array)) {
            const char *is = hymn_value_type(array.is);
            THROW("call to 'push' can't use %s for 1st argument (expected array)", is)
        } else {
            HymnValue value = frame->stack[READ_BYTE(frame)];
            hymn_array_push(hymn_as_array(array), value);
        }
        goto dispatch;
    }
    case OP_INSERT: {
        HymnValue p = pop(H);
        HymnValue i = pop(H);
        HymnValue v = pop(H);
        if (hymn_is_array(v)) {
            if (!hymn_is_int(i)) {
                const char *is = hymn_value_type(i.is);
                hymn_dereference(H, p);
                hymn_dereference(H, i);
                hymn_dereference(H, v);
                THROW("call to 'insert' can't use %s for 2nd argument (expected integer)", is)
            }
            HymnArray *array = hymn_as_array(v);
            HymnInt size = array->length;
            HymnInt index = hymn_as_int(i);
            if (index > size) {
                hymn_dereference(H, p);
                hymn_dereference(H, i);
                hymn_dereference(H, v);
                THROW("index out of bounds in call to 'insert': %d > %d", index, size)
            }
            if (index < 0) {
                index = size + index;
                if (index < 0) {
                    hymn_dereference(H, p);
                    hymn_dereference(H, i);
                    hymn_dereference(H, v);
                    THROW("negative index in 'insert' call: %d", index)
                }
            }
            if (index == size) {
                hymn_array_push(array, p);
            } else {
                hymn_array_insert(array, index, p);
            }
            hymn_dereference(H, v);
        } else {
            const char *is = hymn_value_type(v.is);
            hymn_dereference(H, p);
            hymn_dereference(H, i);
            hymn_dereference(H, v);
            THROW("call to 'insert' can't use %s for 1st argument (expected array)", is)
        }
        goto dispatch;
    }
    case OP_DELETE: {
        HymnValue i = pop(H);
        HymnValue v = pop(H);
        if (hymn_is_array(v)) {
            if (!hymn_is_int(i)) {
                const char *is = hymn_value_type(i.is);
                hymn_dereference(H, i);
                hymn_dereference(H, v);
                THROW("call to 'delete' can't use %s for 2nd argument (expected integer)", is)
            }
            HymnArray *array = hymn_as_array(v);
            HymnInt size = array->length;
            HymnInt index = hymn_as_int(i);
            if (index >= size) {
                hymn_dereference(H, i);
                hymn_dereference(H, v);
                THROW("index out of bounds in call to 'delete': %d >= %d.", index, size)
            }
            if (index < 0) {
                index = size + index;
                if (index < 0) {
                    hymn_dereference(H, i);
                    hymn_dereference(H, v);
                    THROW("negative index in 'delete' call: %d", index)
                }
            }
            HymnValue value = hymn_array_remove_index(array, index);
            push(H, value);
            hymn_dereference(H, v);
        } else if (hymn_is_table(v)) {
            if (!hymn_is_string(i)) {
                const char *is = hymn_value_type(i.is);
                hymn_dereference(H, i);
                hymn_dereference(H, v);
                THROW("call to 'delete' can't use %s for 2nd argument (expected string)", is)
            }
            HymnTable *table = hymn_as_table(v);
            HymnObjectString *name = hymn_as_hymn_string(i);
            HymnValue value = table_remove(table, name);
            if (hymn_is_undefined(value)) {
                value.is = HYMN_VALUE_NONE;
            } else {
                hymn_dereference_string(H, name);
            }
            push(H, value);
            hymn_dereference(H, v);
            hymn_dereference_string(H, name);
        } else {
            const char *is = hymn_value_type(v.is);
            hymn_dereference(H, i);
            hymn_dereference(H, v);
            THROW("call to 'delete' can't use %s for 1st argument (expected array or table)", is)
        }
        goto dispatch;
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
            hymn_reference(new);
            hymn_dereference(H, value);
            break;
        }
        case HYMN_VALUE_TABLE: {
            HymnTable *copy = new_table_copy(hymn_as_table(value));
            HymnValue new = hymn_new_table_value(copy);
            push(H, new);
            hymn_reference(new);
            hymn_dereference(H, value);
            break;
        }
        default:
            push(H, hymn_new_none());
        }
        goto dispatch;
    }
    case OP_SLICE: {
        HymnValue b = pop(H);
        HymnValue a = pop(H);
        HymnValue v = pop(H);
        if (!hymn_is_int(a)) {
            const char *is = hymn_value_type(a.is);
            hymn_dereference(H, a);
            hymn_dereference(H, b);
            hymn_dereference(H, v);
            THROW("slice can't use %s (expected integer)", is)
        }
        HymnInt start = hymn_as_int(a);
        if (start < 0) {
            hymn_dereference(H, a);
            hymn_dereference(H, b);
            hymn_dereference(H, v);
            THROW("negative slice start: %d", start)
        }
        if (hymn_is_string(v)) {
            HymnString *original = hymn_as_string(v);
            HymnInt size = (HymnInt)hymn_string_len(original);
            HymnInt end;
            if (hymn_is_int(b)) {
                end = hymn_as_int(b);
            } else if (hymn_is_none(b)) {
                end = size;
            } else {
                const char *is = hymn_value_type(b.is);
                hymn_dereference(H, a);
                hymn_dereference(H, b);
                hymn_dereference(H, v);
                THROW("slice can't use %s (expected integer)", is)
            }
            if (end > size) {
                hymn_dereference(H, a);
                hymn_dereference(H, b);
                hymn_dereference(H, v);
                THROW("slice out of bounds: %d > %d", end, size)
            }
            if (end < 0) {
                end = size + end;
                if (end < 0) {
                    hymn_dereference(H, a);
                    hymn_dereference(H, b);
                    hymn_dereference(H, v);
                    THROW("negative slice end: %d", end)
                }
            }
            if (start >= end) {
                hymn_dereference(H, a);
                hymn_dereference(H, b);
                hymn_dereference(H, v);
                THROW("slice out of range: %d >= %d", start, end)
            }
            HymnString *sub = hymn_substring(original, (size_t)start, (size_t)end);
            push_string(H, sub);
        } else if (hymn_is_array(v)) {
            HymnArray *array = hymn_as_array(v);
            HymnInt size = array->length;
            HymnInt end;
            if (hymn_is_int(b)) {
                end = hymn_as_int(b);
            } else if (hymn_is_none(b)) {
                end = size;
            } else {
                const char *is = hymn_value_type(b.is);
                hymn_dereference(H, a);
                hymn_dereference(H, b);
                hymn_dereference(H, v);
                THROW("slice can't use %s (expected integer)", is)
            }
            if (end > size) {
                hymn_dereference(H, a);
                hymn_dereference(H, b);
                hymn_dereference(H, v);
                THROW("slice out of bounds: %d > %d", end, size)
            }
            if (end < 0) {
                end = size + end;
                if (end < 0) {
                    hymn_dereference(H, a);
                    hymn_dereference(H, b);
                    hymn_dereference(H, v);
                    THROW("negative slice end: %d", end)
                }
            }
            if (start >= end) {
                hymn_dereference(H, a);
                hymn_dereference(H, b);
                hymn_dereference(H, v);
                THROW("slice out of range: %d >= %d", start, end)
            }
            HymnArray *copy = new_array_slice(array, start, end);
            HymnValue new = hymn_new_array_value(copy);
            hymn_reference(new);
            push(H, new);
        } else {
            const char *is = hymn_value_type(v.is);
            hymn_dereference(H, a);
            hymn_dereference(H, b);
            hymn_dereference(H, v);
            THROW("can't slice %s (expected string or array)", is)
        }
        hymn_dereference(H, v);
        goto dispatch;
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
            push(H, hymn_new_float(0.0));
            break;
        case HYMN_VALUE_STRING:
            push_string(H, hymn_new_string(""));
            break;
        case HYMN_VALUE_ARRAY: {
            HymnArray *array = hymn_as_array(value);
            hymn_array_clear(H, array);
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
        default:
            break;
        }
        goto dispatch;
    }
    case OP_KEYS: {
        HymnValue value = pop(H);
        if (!hymn_is_table(value)) {
            const char *is = hymn_value_type(value.is);
            hymn_dereference(H, value);
            THROW("call to 'keys' can't use %s (expected table)", is)
        } else {
            HymnTable *table = hymn_as_table(value);
            HymnArray *array = table_keys(table);
            HymnValue keys = hymn_new_array_value(array);
            hymn_reference(keys);
            push(H, keys);
            hymn_dereference(H, value);
        }
        goto dispatch;
    }
    case OP_INDEX: {
        HymnValue b = pop(H);
        HymnValue a = pop(H);
        switch (a.is) {
        case HYMN_VALUE_STRING: {
            if (!hymn_is_string(b)) {
                const char *is = hymn_value_type(b.is);
                hymn_dereference(H, a);
                hymn_dereference(H, b);
                THROW("call to 'index' can't use %s for 2nd argument (expected string)", is)
            }
            size_t index = 0;
            bool found = string_find(hymn_as_string(a), hymn_as_string(b), &index);
            if (found) {
                push(H, hymn_new_int((HymnInt)index));
            } else {
                push(H, hymn_new_int(-1));
            }
            hymn_dereference(H, a);
            hymn_dereference(H, b);
            break;
        }
        case HYMN_VALUE_ARRAY:
            push(H, hymn_new_int(hymn_array_index_of(hymn_as_array(a), b)));
            hymn_dereference(H, a);
            hymn_dereference(H, b);
            break;
        case HYMN_VALUE_TABLE: {
            HymnObjectString *key = table_key_of(hymn_as_table(a), b);
            if (key == NULL) {
                push(H, hymn_new_none());
            } else {
                push(H, hymn_new_string_value(key));
            }
            hymn_dereference(H, a);
            hymn_dereference(H, b);
            break;
        }
        default: {
            const char *is = hymn_value_type(a.is);
            hymn_dereference(H, a);
            hymn_dereference(H, b);
            THROW("call to 'index' can't use %s for 1st argument (expected string, array, or table)", is)
        }
        }
        goto dispatch;
    }
    case OP_TYPE: {
        HymnValue value = pop(H);
        const char *is = hymn_value_type(value.is);
        push_string(H, hymn_new_string(is));
        hymn_dereference(H, value);
        goto dispatch;
    }
    case OP_INT: {
        HymnValue value = pop(H);
        if (hymn_is_int(value)) {
            push(H, value);
        } else if (hymn_is_float(value)) {
            HymnInt number = (HymnInt)hymn_as_float(value);
            push(H, hymn_new_int(number));
        } else if (hymn_is_string(value)) {
            HymnString *string = hymn_as_string(value);
            char *end = NULL;
            double number = strtod(string, &end);
            if (string == end) {
                push(H, hymn_new_none());
            } else {
                push(H, hymn_new_int((HymnInt)number));
            }
            hymn_dereference(H, value);
        } else {
            const char *is = hymn_value_type(value.is);
            hymn_dereference(H, value);
            THROW("can't cast %s to integer", is)
        }
        goto dispatch;
    }
    case OP_FLOAT: {
        HymnValue value = pop(H);
        if (hymn_is_int(value)) {
            HymnFloat number = (HymnFloat)hymn_as_int(value);
            push(H, hymn_new_float(number));
        } else if (hymn_is_float(value)) {
            push(H, value);
        } else if (hymn_is_string(value)) {
            HymnString *string = hymn_as_string(value);
            char *end = NULL;
            double number = strtod(string, &end);
            if (string == end) {
                push(H, hymn_new_none());
            } else {
                push(H, hymn_new_float(number));
            }
            hymn_dereference(H, value);
        } else {
            const char *is = hymn_value_type(value.is);
            hymn_dereference(H, value);
            THROW("can't cast %s to float", is)
        }
        goto dispatch;
    }
    case OP_STRING: {
        HymnValue value = pop(H);
        push_string(H, hymn_value_to_string(value));
        hymn_dereference(H, value);
        goto dispatch;
    }
    case OP_ECHO: {
        HymnValue value = pop(H);
        HymnString *string = hymn_value_to_string(value);
        H->print("%s\n", string);
        hymn_string_delete(string);
        hymn_dereference(H, value);
        goto dispatch;
    }
    case OP_PRINT: {
        HymnValue value = pop(H);
        HymnValue route = pop(H);
        HymnString *string = hymn_value_to_string(value);
        if (hymn_value_false(route)) {
            H->print("%s", string);
        } else {
            H->print_error("%s", string);
        }
        hymn_string_delete(string);
        hymn_dereference(H, value);
        hymn_dereference(H, route);
        goto dispatch;
    }
    case OP_SOURCE: {
        HymnValue value = pop(H);
        HymnString *inspect = NULL;
        if (hymn_is_func(value)) {
            HymnFunction *func = hymn_as_func(value);
            if (func->source != NULL) inspect = hymn_string_copy(func->source);
        }
        if (inspect == NULL) inspect = hymn_value_to_string(value);
        push_string(H, inspect);
        hymn_dereference(H, value);
        goto dispatch;
    }
    case OP_CODES: {
        HymnValue value = pop(H);
        HymnString *debug = NULL;
        if (hymn_is_func(value)) {
            HymnFunction *func = hymn_as_func(value);
            debug = disassemble_byte_code(&func->code);
        }
        if (debug == NULL) debug = hymn_value_to_string(value);
        push_string(H, debug);
        hymn_dereference(H, value);
        goto dispatch;
    }
    case OP_STACK: {
        if (H->stack_top != H->stack) {
            HymnString *debug = hymn_new_string("");
            for (HymnValue *i = H->stack; i != H->stack_top; i++) {
                debug = hymn_string_append_char(debug, '[');
                HymnString *stack_debug = debug_value_to_string(*i);
                debug = hymn_string_append(debug, stack_debug);
                hymn_string_delete(stack_debug);
                debug = hymn_string_append(debug, "]\n");
            }
            push_string(H, debug);
        } else {
            push_string(H, hymn_new_string(""));
        }
        goto dispatch;
    }
    case OP_REFERENCE: {
        HymnValue value = pop(H);
        int count = 0;
        switch (value.is) {
        case HYMN_VALUE_STRING:
            count = ((HymnObjectString *)value.as.o)->count;
            break;
        case HYMN_VALUE_ARRAY:
            count = ((HymnArray *)value.as.o)->count;
            break;
        case HYMN_VALUE_TABLE:
            count = ((HymnTable *)value.as.o)->count;
            break;
        case HYMN_VALUE_FUNC:
            count = ((HymnFunction *)value.as.o)->count;
            break;
        case HYMN_VALUE_FUNC_NATIVE:
            count = ((HymnNativeFunction *)value.as.o)->count;
            break;
        default:
            break;
        }
        push(H, hymn_new_int(count));
        hymn_dereference(H, value);
        goto dispatch;
    }
    case OP_THROW: {
        frame = exception(H);
        if (frame == NULL) return;
        goto dispatch;
    }
    case OP_DUPLICATE: {
        HymnValue top = peek(H, 1);
        push(H, top);
        hymn_reference(top);
        goto dispatch;
    }
    case OP_USE: {
        HymnValue file = pop(H);
        if (hymn_is_string(file)) {
            frame = import(H, hymn_as_hymn_string(file));
            hymn_dereference(H, file);
            if (frame == NULL) return;
        } else {
            const char *is = hymn_value_type(file.is);
            hymn_dereference(H, file);
            THROW("import can't use %s (expected string)", is)
        }
        goto dispatch;
    }
    default:
        UNREACHABLE();
    }
}

static char *interpret(Hymn *H) {
    run(H);
    char *error = NULL;
    if (H->error) {
        error = string_to_chars(H->error);
        hymn_string_delete(H->error);
        H->error = NULL;
    }
    return error;
}

static void print_stdout(const char *format, ...) {
    va_list args;

    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

static void print_stderr(const char *format, ...) {
    va_list args;

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

Hymn *new_hymn(void) {
    Hymn *H = hymn_calloc(1, sizeof(Hymn));
    reset_stack(H);

    // STRINGS

    set_init(&H->strings);

    HymnObjectString *search_this = hymn_new_intern_string(H, "<parent>" PATH_SEP_STRING "<path>.hm");
    hymn_reference_string(search_this);

    HymnObjectString *search_relative = hymn_new_intern_string(H, "." PATH_SEP_STRING "<path>.hm");
    hymn_reference_string(search_relative);

    HymnObjectString *search_libs = hymn_new_intern_string(H, "." PATH_SEP_STRING "libs" PATH_SEP_STRING "<path>.hm");
    hymn_reference_string(search_libs);

    // GLOBALS

    table_init(&H->globals);

    HymnObjectString *globals = hymn_new_intern_string(H, "GLOBALS");
    hymn_reference_string(globals);

    HymnValue globals_value = hymn_new_table_value(&H->globals);
    table_put(&H->globals, globals, globals_value);
    hymn_reference_string(globals);
    hymn_reference(globals_value);
    hymn_reference(globals_value);

    // PATHS

    H->paths = hymn_new_array(3);

    HymnObjectString *paths = hymn_new_intern_string(H, "PATHS");
    hymn_reference_string(paths);

    H->paths->items[0] = hymn_new_string_value(search_this);
    H->paths->items[1] = hymn_new_string_value(search_relative);
    H->paths->items[2] = hymn_new_string_value(search_libs);

    HymnValue paths_value = hymn_new_array_value(H->paths);
    table_put(&H->globals, paths, paths_value);
    hymn_reference_string(paths);
    hymn_reference(paths_value);
    hymn_reference(paths_value);

    // IMPORTS

    H->imports = hymn_new_table();

    HymnObjectString *imports = hymn_new_intern_string(H, "IMPORTS");
    hymn_reference_string(imports);

    HymnValue imports_value = hymn_new_table_value(H->imports);
    table_put(&H->globals, imports, imports_value);
    hymn_reference_string(imports);
    hymn_reference(imports_value);
    hymn_reference(imports_value);

    H->print = print_stdout;
    H->print_error = print_stderr;

    return H;
}

void hymn_delete(Hymn *H) {
    {
        HymnTable *globals_table = &H->globals;
        HymnObjectString *globals = hymn_new_intern_string(H, "GLOBALS");
        table_remove(globals_table, globals);
        hymn_dereference_string(H, globals);

        table_release(H, globals_table);
        assert(globals_table->size == 0);
    }

    hymn_array_delete(H, H->paths);
    table_delete(H, H->imports);

    HymnSet *strings = &H->strings;
    {
        unsigned int bins = strings->bins;
        for (unsigned int i = 0; i < bins; i++) {
            HymnSetItem *item = strings->items[i];
            while (item != NULL) {
                HymnSetItem *next = item->next;
                hymn_dereference_string(H, item->string);
                item = next;
            }
        }
    }
    assert(strings->size == 0);
    free(strings->items);

    hymn_string_delete(H->error);

#ifndef HYMN_NO_DYNAMIC_LIBS
    HymnLibList *lib = H->libraries;
    while (lib != NULL) {
        hymn_close_dlib(lib->lib);
        HymnLibList *next = lib->next;
        free(lib);
        lib = next;
    }
#endif

    free(H);
}

HymnValue hymn_get(Hymn *H, const char *name) {
    return hymn_table_get(&H->globals, name);
}

void hymn_add(Hymn *H, const char *name, HymnValue value) {
    HymnObjectString *string = hymn_new_intern_string(H, name);
    HymnValue previous = table_put(&H->globals, string, value);
    if (hymn_is_undefined(previous)) {
        hymn_reference_string(string);
    } else {
        hymn_dereference(H, previous);
    }
    hymn_reference(value);
}

void hymn_add_string(Hymn *H, const char *name, const char *string) {
    HymnObjectString *object = hymn_new_intern_string(H, string);
    hymn_add(H, name, hymn_new_string_value(object));
}

void hymn_add_table(Hymn *H, const char *name, HymnTable *table) {
    hymn_add(H, name, hymn_new_table_value(table));
}

void hymn_add_pointer(Hymn *H, const char *name, void *pointer) {
    hymn_add(H, name, hymn_new_pointer(pointer));
}

void hymn_add_string_to_table(Hymn *H, HymnTable *table, const char *name, const char *string) {
    HymnObjectString *object = hymn_new_intern_string(H, string);
    hymn_set_property_const(H, table, name, hymn_new_string_value(object));
}

void hymn_add_function_to_table(Hymn *H, HymnTable *table, const char *name, HymnNativeCall func) {
    HymnObjectString *string = hymn_new_intern_string(H, name);
    HymnNativeFunction *native = new_native_function(string, func);
    HymnValue value = hymn_new_native(native);
    hymn_set_property(H, table, string, value);
}

void hymn_add_function(Hymn *H, const char *name, HymnNativeCall func) {
    hymn_add_function_to_table(H, &H->globals, name, func);
}

char *hymn_call(Hymn *H, const char *name, int arguments) {
    HymnValue function = hymn_table_get(&H->globals, name);
    if (hymn_is_undefined(function)) {
        return NULL;
    }
    hymn_reference(function);

    push(H, function);
    call_value(H, function, arguments);

    char *error = interpret(H);
    if (error != NULL) return error;

    assert(H->stack_top == H->stack);
    reset_stack(H);

    return NULL;
}

char *hymn_debug(Hymn *H, const char *script, const char *source) {
    HymnString *code = NULL;
    if (source == NULL) {
        code = hymn_read_file(script);
        if (code == NULL) {
            HymnString *format = hymn_string_format("file not found: %s\n", script);
            char *error = string_to_chars(format);
            hymn_string_delete(format);
            return error;
        }
    } else {
        code = hymn_new_string(source);
    }

    CompileResult result = compile(H, script, code, TYPE_SCRIPT);

    char *error = result.error;
    if (error != NULL) {
        hymn_string_delete(code);
        return error;
    }

    HymnFunction *main = result.func;

    HymnString *debug = disassemble_byte_code(&main->code);
    printf("\n-- %s --\n%s\n", script != NULL ? script : "script", debug);
    hymn_string_delete(debug);

    HymnValuePool *constants = &main->code.constants;
    int count = constants->count;
    HymnValue *values = constants->values;

    for (int i = 0; i < count; i++) {
        HymnValue value = values[i];
        if (hymn_is_func(value)) {
            HymnFunction *func = hymn_as_func(value);
            debug = disassemble_byte_code(&func->code);
            printf("\n-- %s --\n%s\n", func->name != NULL ? func->name : "script", debug);
            hymn_string_delete(debug);
        }
    }

    function_delete(main);
    hymn_string_delete(code);

    assert(H->stack_top == H->stack);
    reset_stack(H);

    return NULL;
}

static char *exec(Hymn *H, const char *script, const char *source, enum FunctionType type) {
    CompileResult result = compile(H, script, source, type);

    char *error = result.error;
    if (error != NULL) return error;

    HymnFunction *func = result.func;
    HymnValue function = hymn_new_func_value(func);
    func->count = 1;

    push(H, function);
    call(H, func, 0);

    error = interpret(H);
    if (error != NULL) return error;

    assert(H->stack_top == H->stack);
    reset_stack(H);

    return NULL;
}

char *hymn_run(Hymn *H, const char *script, const char *source) {
    return exec(H, script, source, TYPE_SCRIPT);
}

char *hymn_do(Hymn *H, const char *source) {
    return exec(H, NULL, source, TYPE_SCRIPT);
}

char *hymn_direct(Hymn *H, const char *source) {
    return exec(H, NULL, source, TYPE_DIRECT);
}

char *hymn_script(Hymn *H, const char *script) {
    HymnString *source = hymn_read_file(script);
    if (source == NULL) {
        HymnString *format = hymn_string_format("file not found: %s\n", script);
        char *error = string_to_chars(format);
        hymn_string_delete(format);
        return error;
    }
    char *error = exec(H, script, source, TYPE_SCRIPT);
    hymn_string_delete(source);
    return error;
}

#ifndef HYMN_NO_REPL

#include <ctype.h>

#define INPUT_LIMIT 256

typedef struct History History;

struct History {
    HymnString *input;
    History *previous;
    History *next;
};

#ifndef _MSC_VER
#include <termios.h>

#define CURSOR_RESET "\033[2K\r"

#define cursor_backspace() printf("\b \b")
#define cursor_forward() printf("\033[1C")
#define cursor_backward() printf("\033[1D")
#define cursor_clear() printf("\033[s\033[K")
#define cursor_erase() printf("\033[1D\033[s\033[K")
#define cursor_unsave() printf("\033[u")
#define cursor_reset() printf(CURSOR_RESET)

enum Keyboard {
    ARROW_UP = 1000,
    ARROW_DOWN,
    ARROW_LEFT,
    ARROW_RIGHT,
    PAGE_UP,
    PAGE_DOWN,
    HOME_KEY,
    END_KEY,
    DELETE_KEY
};

static char letters[] =
    "0123456789"
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static struct termios save_termios;

static int read_key(void) {
    do {
        char c;
        ssize_t e = read(STDIN_FILENO, &c, 1);
        if (e == 1) {
            if (c == '\x1b') {
                char sequence[3];
                if (read(STDIN_FILENO, &sequence[0], 1) != 1) {
                    return '\x1b';
                }
                if (read(STDIN_FILENO, &sequence[1], 1) != 1) {
                    return '\x1b';
                }
                if (sequence[0] == '[') {
                    if (sequence[1] >= '0' && sequence[1] <= '9') {
                        if (read(STDIN_FILENO, &sequence[2], 1) != 1) {
                            return '\x1b';
                        }
                        if (sequence[2] == '~') {
                            switch (sequence[1]) {
                            case '1': return HOME_KEY;
                            case '3': return DELETE_KEY;
                            case '4': return END_KEY;
                            case '5': return PAGE_UP;
                            case '6': return PAGE_DOWN;
                            case '7': return HOME_KEY;
                            case '8': return END_KEY;
                            default: break;
                            }
                        }
                    } else {
                        switch (sequence[1]) {
                        case 'A': return ARROW_UP;
                        case 'B': return ARROW_DOWN;
                        case 'D': return ARROW_LEFT;
                        case 'C': return ARROW_RIGHT;
                        case 'H': return HOME_KEY;
                        case 'F': return END_KEY;
                        default: break;
                        }
                    }
                } else if (sequence[0] == 'O') {
                    switch (sequence[1]) {
                    case 'H': return HOME_KEY;
                    case 'F': return END_KEY;
                    default: break;
                    }
                }
                return '\x1b';
            } else {
                return c;
            }
        } else if (e == -1 && errno != EAGAIN) {
            printf("\nuser interrupt");
            return EOF;
        }
    } while (true);
}

static void reset_terminal(void) {
    if (tcsetattr(STDIN_FILENO, TCSANOW, &save_termios) == -1) {
        perror("tcsetattr");
    }
}
#else
static void remove_newline(char *line) {
    size_t i = 0;
    while (line[i] != '\0') {
        i++;
    }
    while (i > 0) {
        i--;
        if (line[i] != '\n') return;
        line[i] = '\0';
    }
}
#endif

static void call_function(Hymn *H, HymnFunction *func) {
    HymnValue function = hymn_new_func_value(func);
    hymn_reference(function);
    push(H, function);
    call(H, func, 0);
    char *error = interpret(H);
    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
        fflush(stderr);
        free(error);
    }
    assert(H->stack_top == H->stack);
    reset_stack(H);
}

void hymn_repl(Hymn *H) {

    printf("welcome to hymn v" HYMN_VERSION "\ntype .help for more information\n");

#ifdef _MSC_VER
    char temp_dir[MAX_PATH];
    GetTempPath(MAX_PATH, temp_dir);
#else
    if (tcgetattr(STDIN_FILENO, &save_termios) == -1) {
        perror("tcgetattr");
        return;
    }
    struct termios new_term = save_termios;
    new_term.c_lflag &= ~(0U | ECHO | ICANON);
    new_term.c_cc[VMIN] = 1;
    new_term.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &new_term) == -1) {
        perror("tcsetattr");
        return;
    }
    atexit(reset_terminal);

    int index = 0;
    int count = 0;

    History *lines = NULL;
    History *cursor = NULL;
#endif

    char line[INPUT_LIMIT];
    HymnString *input = hymn_new_string_with_capacity(INPUT_LIMIT);

    History *history = NULL;
    bool open_editor = false;

    while (true) {
        printf(input[0] == '\0' ? "> " : "... ");
        fflush(stdout);

#ifdef _MSC_VER
        if (fgets(line, sizeof(line), stdin) == NULL) {
            break;
        }
        remove_newline(line);
#else
#define CONTROL_KEY(c) ((c)&0x1f)

        index = 0;
        count = 0;

        while (true) {
            int c = read_key();
            switch (c) {
            case CONTROL_KEY('d'):
                printf("\n");
                goto quit;
            case CONTROL_KEY('e'):
                open_editor = true;
                goto scan;
            case PAGE_UP:
            case ARROW_UP:
                if (lines != NULL) {
                    if (cursor == NULL) {
                        cursor = lines;
                    } else if (cursor->previous == NULL) {
                        continue;
                    } else {
                        cursor = cursor->previous;
                    }
                    size_t len = hymn_string_len(cursor->input);
                    memcpy(line, cursor->input, len + 1);
                    index = (int)len;
                    count = index;
                    printf(CURSOR_RESET "%s %s", input[0] == '\0' ? ">" : "...", line);
                    fflush(stdout);
                }
                continue;
            case PAGE_DOWN:
            case ARROW_DOWN:
                if (cursor != NULL) {
                    cursor = cursor->next;
                    if (cursor != NULL) {
                        size_t len = hymn_string_len(cursor->input);
                        memcpy(line, cursor->input, len + 1);
                        index = (int)len;
                        count = index;
                    } else {
                        line[0] = '\0';
                        index = 0;
                        count = 0;
                    }
                    printf(CURSOR_RESET "%s %s", input[0] == '\0' ? ">" : "...", line);
                    fflush(stdout);
                }
                continue;
            case HOME_KEY:
            case ARROW_LEFT:
                if (index > 0) {
                    index--;
                    cursor_backward();
                    fflush(stdout);
                }
                continue;
            case END_KEY:
            case ARROW_RIGHT:
                if (index < count) {
                    index++;
                    cursor_forward();
                    fflush(stdout);
                }
                continue;
            case DELETE_KEY:
                if (index < count) {
                    count--;
                    for (int i = index; i < count; i++) {
                        line[i] = line[i + 1];
                    }
                    cursor_clear();
                    line[count] = '\0';
                    printf("%s", &line[index]);
                    cursor_unsave();
                    fflush(stdout);
                }
                continue;
            case EOF:
                hymn_string_zero(input);
                count = 0;
                goto scan;
            case '\n':
            case '\r':
                goto scan;
            default:
                if (iscntrl(c)) {
                    if (c == 8 || c == 127) {
                        // delete or backspace
                        if (index > 0) {
                            index--;
                            for (int i = index; i < count - 1; i++) {
                                line[i] = line[i + 1];
                            }
                            count--;
                            if (index == count) {
                                cursor_backspace();
                            } else {
                                cursor_erase();
                                line[count] = '\0';
                                printf("%s", &line[index]);
                                cursor_unsave();
                            }
                            fflush(stdout);
                        }
                    } else {
                        printf("<%d>", c);
                        fflush(stdout);
                    }
                } else if (c != '\0') {
                    if (count < INPUT_LIMIT) {
                        count++;
                        for (int i = count - 1; i > index; i--) {
                            line[i] = line[i - 1];
                        }
                        line[index] = (char)c;
                        index++;
                        if (index == count) {
                            printf("%c", c);
                        } else {
                            cursor_clear();
                            line[count] = '\0';
                            printf("%s", &line[index - 1]);
                            cursor_unsave();
                            cursor_forward();
                        }
                        fflush(stdout);
                    }
                }
            }
        }

    scan:
        line[count] = '\0';
        printf("\n");
        cursor = NULL;
#endif

        if (line[0] == '.') {
            if (hymn_string_equal(line, ".exit") || hymn_string_equal(line, ".quit")) {
                goto quit;
            } else if (hymn_string_equal(line, ".edit")) {
                line[0] = '\0';
                open_editor = true;
                goto editing;
            } else if (hymn_string_equal(line, ".help")) {
                printf(".exit   Exit interactive mode\n"
                       ".quit   Alias for .exit\n"
                       ".edit   Edit input using $EDITOR\n"
                       ".save   Save history to [FILE]\n"
                       ".load   Read history from [FILE]\n"
                       ".help   Print this help message\n"
                       "press ^E to use $EDITOR\n"
                       "press ^C to cancel expression\n"
                       "press ^D to exit interactive mode\n");
            } else if (string_starts_with(line, ".save ")) {
                char path[PATH_MAX];
                strcpy(path, &line[6]);
                if (path[0] == '\0') {
                    printf("bad file path\n");
                } else if (history == NULL) {
                    printf("no history to save\n");
                } else if (hymn_file_exists(path)) {
                    printf("history can't overwrite an existing file: %s\n", path);
                } else {
                    FILE *open = fopen(path, "w");
                    if (open == NULL) {
                        printf("failed to write history: %s\n", path);
                    } else {
                        History *head = history;
                        while (head->previous != NULL) {
                            head = head->previous;
                        }
                        while (head != NULL) {
                            fprintf(open, "%s\n", head->input);
                            head = head->next;
                        }
                        fclose(open);
                        printf("history saved to: %s\n", path);
                    }
                }
            } else if (string_starts_with(line, ".load ")) {
                char path[PATH_MAX];
                strcpy(path, &line[6]);
                if (path[0] == '\0') {
                    printf("bad file path\n");
                } else if (hymn_file_exists(path)) {
                    HymnString *source = hymn_read_file(path);
                    if (source == NULL) {
                        printf("failed to read history: %s\n", path);
                    } else {
                        char *error = hymn_do(H, source);
                        hymn_string_delete(source);
                        if (error != NULL) {
                            fprintf(stderr, "%s\n", error);
                            fflush(stderr);
                            free(error);
                        }
                    }
                } else {
                    printf("history file not found: %s\n", path);
                }
            } else if (hymn_string_equal(line, ".save")) {
                printf("specify a path\n");
            } else if (hymn_string_equal(line, ".load")) {
                printf("specify a path\n");
            } else {
                printf("invalid interactive command\n");
            }
            continue;
        }

    editing:
        if (line[0] != '\0') {
#ifndef _MSC_VER
            History *save = hymn_calloc(1, sizeof(History));
            save->input = hymn_new_string(line);
            if (lines == NULL) {
                lines = save;
            } else {
                lines->next = save;
                save->previous = lines;
                lines = save;
            }
#endif
            if (input[0] != '\0') input = hymn_string_append_char(input, '\n');
            input = hymn_string_append(input, line);
            hymn_string_trim(input);
        }

        if (open_editor) {
            open_editor = false;

            char *editor = getenv("EDITOR");
            if (editor == NULL) {
#ifdef _MSC_VER
                editor = "notepad";
#else
                printf("no EDITOR set\n");
                continue;
#endif
            }

            FILE *temp = NULL;

#ifdef _MSC_VER
            char path[MAX_PATH];
            GetTempFileName(temp_dir, "hymn", 0, path);
            temp = fopen(path, "w");
#else
            char path[] = "/tmp/hymn.XXXXXX";
            for (int a = 0; a < 64; a++) {
                for (int x = 0; x < 6; x++) {
                    path[10 + x] = letters[(size_t)((double)rand() / RAND_MAX * (sizeof(letters) - 1))];
                }
                temp = fopen(path, "wx");
                if (temp != NULL) {
                    break;
                }
            }
#endif

            if (temp == NULL) {
                printf("failed to create temporary file\n");
                continue;
            }

            hymn_string_trim(input);
            if (hymn_string_len(input) > 0) {
                fprintf(temp, "%s", input);
                hymn_string_zero(input);
            }

            fclose(temp);

            HymnString *edit = hymn_string_format("%s %s", editor, path);
            int result = system(edit);
            if (result == -1) {
                printf("failed edit: %d\n", result);
            }
            hymn_string_delete(edit);

            HymnString *content = hymn_read_file(path);
            unlink(path);

            if (content == NULL) {
                printf("failed to read temporary file\n");
                continue;
            }

            input = hymn_string_append(input, content);
            hymn_string_delete(content);
            hymn_string_trim(input);
            if (input[0] == '\0') {
                continue;
            }
            printf("%s\n", input);
        }

        if (input[0] == '\0') continue;

        CompileResult result = compile(H, NULL, input, TYPE_REPL);
        char *error = result.error;
        if (error != NULL) {
            if (!hymn_string_equal(error, "<eof>")) {
                hymn_string_zero(input);
                fprintf(stderr, "%s\n", error);
                fflush(stderr);
                free(error);
            }
            continue;
        }
        HymnFunction *func = result.func;

        History *save = hymn_calloc(1, sizeof(History));
        save->input = hymn_new_string(input);
        if (history == NULL) {
            history = save;
        } else {
            history->next = save;
            save->previous = history;
            history = save;
        }
        hymn_string_zero(input);

        call_function(H, func);
    }

quit:
    hymn_string_delete(input);
#ifndef _MSC_VER
    while (lines != NULL) {
        hymn_string_delete(lines->input);
        History *previous = lines->previous;
        free(lines);
        lines = previous;
    }
#endif
    while (history != NULL) {
        hymn_string_delete(history->input);
        History *previous = history->previous;
        free(history);
        history = previous;
    }
}
#endif

#ifndef HYMN_NO_DYNAMIC_LIBS

void hymn_add_dlib(Hymn *H, void *library) {
    HymnLibList *tail = H->libraries;
    HymnLibList *head = hymn_calloc(1, sizeof(HymnLibList));
    head->lib = library;
    head->next = tail;
    H->libraries = head;
}

#ifdef _MSC_VER
typedef void (*HymnDynamicLib)(Hymn *H);

void hymn_close_dlib(void *library) {
    FreeLibrary(library);
}

HymnString *hymn_use_dlib(Hymn *H, const char *path, const char *func) {
    HINSTANCE lib = LoadLibrary(path);
    if (lib != NULL) {
        HymnDynamicLib proc = (HymnDynamicLib)GetProcAddress(lib, func);
        if (proc != NULL) {
            proc(H);
            hymn_add_dlib(H, lib);
            return NULL;
        }
    }

    HymnString *message = NULL;
    int error = GetLastError();
    char buffer[128];
    if (FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, 0, error, 0, buffer, sizeof(buffer), 0)) {
        message = hymn_new_string(buffer);
    } else {
        message = hymn_string_format("windows error: %d\n", error);
    }
    if (lib != NULL) {
        hymn_close_dlib(lib);
    }
    return message;
}

#else
#include <dlfcn.h>

void hymn_close_dlib(void *library) {
    dlclose(library);
}

HymnString *hymn_use_dlib(Hymn *H, const char *path, const char *func) {
    void *lib = dlopen(path, RTLD_NOW);
    if (lib != NULL) {
        void *(*proc)(Hymn *);
        *(void **)(&proc) = dlsym(lib, func);
        if (proc != NULL) {
            proc(H);
            hymn_add_dlib(H, lib);
            return NULL;
        }
    }

    HymnString *message = hymn_new_string(dlerror());
    if (lib != NULL) {
        hymn_close_dlib(lib);
    }
    return message;
}
#endif

#endif
