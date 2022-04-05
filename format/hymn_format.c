/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERSION "0.1.0"

static char *source = NULL;
static size_t size = 0;
static size_t s = 0;
static char *new = NULL;
static size_t capacity = 0;
static size_t n = 0;
static size_t deep = 0;
static bool *stack = NULL;
static size_t limit = 0;
static size_t f = 0;

static bool digit(char c) {
    return '0' <= c && c <= '9';
}

static bool word(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

static bool other(char c) {
    return !digit(c) && !word(c);
}

static bool math(char c) {
    return c == '+' || c == '-' || c == '=' || c == '<' || c == '>' || c == '!';
}

static bool left(char c) {
    return c == '(' || c == '[' || c == '{';
}

static bool brackets(char c) {
    return c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}';
}

#define SIZE_CHECK   \
    if (s >= size) { \
        goto done;   \
    }

#define MEM_CHECK(M)                      \
    if (M == NULL) {                      \
        fprintf(stderr, "out of memory"); \
        exit(EXIT_FAILURE);               \
    }

static void nest(bool v) {
    if (f + 1 >= limit) {
        limit += 16;
        stack = (stack == NULL) ? malloc(limit * sizeof(bool)) : realloc(stack, limit * sizeof(bool));
        MEM_CHECK(stack)
    }
    stack[f++] = v;
}

static bool compact() {
    if (f > 0) {
        f--;
        return stack[f];
    }
    return false;
}

static void append(char c) {
    if (n + 1 >= capacity) {
        capacity += 256;
        new = realloc(new, (capacity + 1) * sizeof(char));
        MEM_CHECK(new)
    }
    new[n++] = c;
}

static void skip() {
    if (s >= size) {
        return;
    }
    char c = source[s];
    while (c == ' ' || c == '\t') {
        s++;
        if (s >= size) {
            return;
        }
        c = source[s];
    }
}

static void indent() {
    for (size_t a = 0; a < deep; a++) {
        append(' ');
        append(' ');
    }
}

static char pre() {
    if (n >= 1) {
        return new[n - 1];
    }
    return '\0';
}

static char pre2() {
    char p = pre();
    if (p != ' ') {
        return p;
    } else if (n >= 2) {
        return new[n - 2];
    }
    return '\0';
}

static void backspace() {
    if (pre() == ' ') {
        n--;
    }
}

static void space() {
    if (n >= 1) {
        char p = new[n - 1];
        if (p != ' ' && p != '\n') {
            append(' ');
        }
    }
}

static void fix(size_t p) {
    size_t b = p;
    while (true) {
        if (p == 0) {
            return;
        }
        p--;
        char c = new[p];
        if (c == '}') {
            p++;
            if (p < n) {
                new[p++] = ' ';
                size_t d = b - p;
                if (d > 0) {
                    while (true) {
                        if (p + d >= n) {
                            break;
                        }
                        new[p] = new[p + d];
                        p++;
                    }
                    n -= d;
                }
            }
            return;
        } else if (c == '\n' || c == ' ') {
            continue;
        }
        return;
    }
}

static void clear() {
    while (true) {
        char c = pre();
        if (c == '\n' || c == ' ') {
            n--;
            continue;
        }
        return;
    }
}

static void newline() {
    if (n == 0) {
        return;
    }
    size_t b = n - 1;
    while (true) {
        if (new[b] == '\n') {
            return;
        } else if (new[b] == ' ' && b >= 1) {
            b--;
            continue;
        }
        append('\n');
        indent();
        return;
    }
}

static bool match(size_t b, const char *word) {
    int a = 0;
    do {
        if (source[b + a] != word[a]) {
            return false;
        }
        a++;
    } while (word[a] != '\0');
    return true;
}

static void spacing(char c) {
    // TODO
    char p = pre();
    if (c == '!') {
        if (word(p)) {
            append(' ');
        }
    }
}

static void format() {
    capacity = size;
    new = malloc((capacity + 1) * sizeof(char));
    MEM_CHECK(new)
    bool spacing = false;
    while (true) {
        SIZE_CHECK
        char c = source[s];
        if (digit(c)) {
            append(c);
            while (true) {
                s++;
                SIZE_CHECK
                c = source[s];
                if (digit(c)) {
                    append(c);
                } else {
                    break;
                }
            }
            append(' ');
        } else if (word(c)) {
            size_t p = n;
            append(c);
            size_t b = s;
            while (true) {
                s++;
                SIZE_CHECK
                c = source[s];
                if (word(c)) {
                    append(c);
                } else {
                    break;
                }
            }
            size_t x = s - b;
            if ((x == 2 && match(b, "if")) || (x == 3 && match(b, "for")) || (x == 5 && match(b, "while"))) {
                spacing = true;
            } else if (x == 4 && (match(b, "elif") || match(b, "else"))) {
                spacing = true;
                fix(p);
            } else {
                spacing = false;
            }
            append(' ');
        } else {
            switch (c) {
            case '{': {
                s++;
                if (left(pre2())) {
                    backspace();
                } else {
                    space();
                }
                char p = pre2();
                append(c);
                skip();
                SIZE_CHECK
                c = source[s];
                if (c == '}') {
                    s++;
                    append('}');
                    newline();
                } else {
                    deep++;
                    if (p == '=' || p == ':' || p == '(' || p == ',') {
                        append(' ');
                        nest(c != '\n');
                    } else {
                        newline();
                    }
                }
                break;
            }
            case '}': {
                if (deep >= 1) {
                    deep--;
                }
                s++;
                if (compact()) {
                    space();
                    append(c);
                } else {
                    clear();
                    newline();
                    append(c);
                    newline();
                }
                break;
            }
            case '(':
            case '[': {
                s++;
                if (brackets(pre2()) || (!spacing && word(pre2()))) {
                    backspace();
                }
                deep++;
                append(c);
                skip();
                SIZE_CHECK
                c = source[s];
                nest(c != '\n');
                break;
            }
            case ')': {
                if (deep >= 1) {
                    deep--;
                }
                s++;
                if (compact()) {
                    backspace();
                    append(c);
                } else {
                    clear();
                    newline();
                    append(c);
                    newline();
                }
                break;
            }
            case ']': {
                if (deep >= 1) {
                    deep--;
                }
                s++;
                if (compact()) {
                    backspace();
                    append(c);
                    append(' ');
                } else {
                    clear();
                    newline();
                    append(c);
                    newline();
                }
                break;
            }
            case '+':
            case '-':
            case '=':
            case '<':
            case '>': {
                s++;
                if (math(pre2())) {
                    backspace();
                } else {
                    space();
                }
                append(c);
                append(' ');
                break;
            }
            case '!': {
                s++;
                if (math(pre2())) {
                    backspace();
                } else {
                    space();
                }
                append(c);
                break;
            }
            case '\'': {
                append(c);
                s++;
                while (true) {
                    SIZE_CHECK
                    c = source[s];
                    append(c);
                    s++;
                    if (c == '\\') {
                        SIZE_CHECK
                        append(source[s++]);
                    } else if (c == '\'') {
                        break;
                    }
                }
                append(' ');
                break;
            }
            case '"': {
                append(c);
                s++;
                while (true) {
                    SIZE_CHECK
                    c = source[s];
                    append(c);
                    s++;
                    if (c == '\\') {
                        SIZE_CHECK
                        append(source[s++]);
                    } else if (c == '"') {
                        break;
                    }
                }
                append(' ');
                break;
            }
            case '#': {
                append(c);
                s++;
                while (true) {
                    SIZE_CHECK
                    c = source[s];
                    append(c);
                    s++;
                    if (c == '\n') {
                        break;
                    }
                }
                break;
            }
            case '\n': {
                s++;
                bool two = false;
                while (true) {
                    SIZE_CHECK
                    c = source[s];
                    if (c == '\n') {
                        s++;
                        two = true;
                    } else if (c == '\r' || c == '\t' || c == ' ') {
                        s++;
                    } else {
                        break;
                    }
                }
                newline();
                if (two) {
                    append('\n');
                    indent();
                }
                break;
            }
            case '.': {
                s++;
                backspace();
                append(c);
                break;
            }
            case ',':
            case ':': {
                s++;
                backspace();
                append(c);
                append(' ');
                break;
            }
            case '\r':
            case '\t':
            case ' ': {
                s++;
                break;
            }
            default: {
                s++;
                append(c);
            }
            }
        }
    }
done:
    newline();
    new[capacity] = '\0';
    new[n] = '\0';
}

static void help() {
    printf("Hymn Script Formatter\n\n"
           "  -w  Write to file\n"
           "  -c  Format input command\n"
           "  -v  Print version information\n"
           "  -h  Print this help message\n");
}

static void read_file(const char *path) {
    size = 0;
    FILE *open = fopen(path, "rb");
    if (open == NULL) {
        return;
    }
    int ch;
    while ((ch = fgetc(open)) != EOF) {
        size++;
    }
    fseek(open, 0, SEEK_SET);
    source = malloc((size + 1) * sizeof(char));
    if (source == NULL) {
        fprintf(stderr, "out of memory");
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; i < size; i++) {
        source[i] = (char)fgetc(open);
    }
    source[size] = '\0';
    fclose(open);
}

int main(int argc, char **argv) {

    char *file = NULL;
    bool write = false;

    if (argc >= 2) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
                help();
                return 2;
            } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
                printf("Hymn Script Formatter " VERSION "\n");
                return EXIT_SUCCESS;
            } else if (strcmp(argv[i], "-c") == 0) {
                if (i + 1 < argc) {
                    source = argv[i + 1];
                    i++;
                } else {
                    help();
                    return EXIT_FAILURE;
                }
            } else if (strcmp(argv[i], "-w") == 0) {
                write = true;
            } else {
                file = argv[i];
            }
        }
    }

    if (source != NULL) {
        format();
        if (new == NULL) {
            return EXIT_FAILURE;
        } else {
            printf("%s", new);
            free(new);
        }
    } else if (file != NULL) {
        read_file(file);
        if (source == NULL) {
            fprintf(stderr, "File does not exist: %s\n", file);
            return EXIT_FAILURE;
        }
        format();
        free(source);
        if (new == NULL) {
            return EXIT_FAILURE;
        } else if (write) {
            FILE *open = fopen(file, "wb");
            if (open == NULL) {
                fprintf(stderr, "Failed writing to file: %s\n", file);
                free(new);
                return EXIT_FAILURE;
            }
            fputs(new, open);
            fclose(open);
        } else {
            printf("%s", new);
        }
        free(new);
    } else {
        help();
        return 2;
    }
    return EXIT_SUCCESS;
}
