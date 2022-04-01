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
static size_t d = 0;

static bool digit(char c) {
    return '0' <= c && c <= '9';
}

static bool word(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

static bool brackets(char c) {
    return c == '(' || c == ')' || c == '[' || c == ']';
}

static bool math(char c) {
    return c == '+' || c == '-' || c == '=' || c == '<' || c == '>';
}

#define SIZE_CHECK   \
    if (s >= size) { \
        goto done;   \
    }

#define MEM_CHECK                         \
    if (new == NULL) {                    \
        fprintf(stderr, "out of memory"); \
        exit(EXIT_FAILURE);               \
    }

static void append(char c) {
    if (n + 1 >= capacity) {
        capacity += 256;
        new = realloc(new, (capacity + 1) * sizeof(char));
        MEM_CHECK
    }
    new[n++] = c;
}

static void skip() {
    if (s >= size) {
        return;
    }
    char c = source[s];
    while (c == '\n' || c == '\r' || c == ' ' || c == '\t') {
        s++;
        if (s >= size) {
            return;
        }
        c = source[s];
    }
}

static void indent() {
    for (size_t a = 0; a < d; a++) {
        append(' ');
        append(' ');
    }
}

static char peek() {
    if (s + 1 >= size) {
        return '\0';
    }
    return source[s + 1];
}

static char pre() {
    if (n >= 1) {
        return new[n - 1];
    }
    return '\0';
}

static void backspace() {
    if (pre() == ' ') {
        n--;
    }
}

static void space() {
    if (n >= 1 && new[n - 1] != ' ') {
        append(' ');
    }
}

static void clear() {
    while (true) {
        char c = pre();
        if (c == '\n' || c == '\r' || c == '\t' || c == ' ') {
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

static void format() {
    size = strlen(source);
    capacity = size;
    new = malloc((capacity + 1) * sizeof(char));
    MEM_CHECK
    // bool function = false;
    // bool if_statement = false;
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
            append(c);
            // size_t b = s;
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
            // size_t x = s - b;
            // if (x == 8 && match(b, "function")) {
            //     function = true;
            // } else if (x == 2 && match(b, "if")) {
            //     if_statement = true;
            // }
            append(' ');
        } else {
            switch (c) {
            case '{': {
                s++;
                space();
                append('{');
                if (peek() == '}') {
                    s++;
                    append('}');
                } else {
                    d++;
                }
                newline();
                // function = false;
                // if_statement = false;
                break;
            }
            case '}': {
                if (d >= 1) {
                    d--;
                }
                s++;
                clear();
                newline();
                append('}');
                newline();
                break;
            }
            case '+':
            case '-':
            case '=':
            case '<':
            case '>': {
                s++;
                if (math(pre())) {
                    backspace();
                } else {
                    space();
                }
                append(c);
                append(' ');
                break;
            }
            case '\'': {
                append(c);
                s++;
                char t = '\0';
                while (true) {
                    SIZE_CHECK
                    c = source[s];
                    append(c);
                    s++;
                    if (c == '\'' && t != '\\') {
                        break;
                    }
                    t = c;
                }
                break;
            }
            case '"': {
                append(c);
                s++;
                char t = '\0';
                while (true) {
                    SIZE_CHECK
                    c = source[s];
                    append(c);
                    s++;
                    if (c == '"' && t != '\\') {
                        break;
                    }
                    t = c;
                }
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
            case '\r':
            case '\t':
            case ' ': {
                s++;
                break;
            }
            case '(':
            case ')':
            case '.': {
                s++;
                backspace();
                append(c);
                break;
            }
            case ',': {
                s++;
                backspace();
                append(c);
                append(' ');
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
    new[capacity] = '\0';
    new[n] = '\0';
    while (n >= 1) {
        n--;
        if (new[n] == '\n' || new[n] == '\r' || new[n] == '\t' || new[n] == ' ') {
            new[n] = '\0';
            continue;
        }
        return;
    }
}

static void help() {
    printf("Hymn Script Formatter\n\n"
           "  -w  Write to file\n"
           "  -c  Format input command\n"
           "  -v  Print version information\n"
           "  -h  Print this help message\n");
}

static size_t file_size(const char *path) {
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

static void read_file(const char *path) {
    size_t size = file_size(path);
    FILE *open = fopen(path, "rb");
    if (open == NULL) {
        return;
    }
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
            // FILE *open = fopen(file, "w");
            // if (open == NULL) {
            //     fprintf(stderr, "Failed writing to file: %s\n", file);
            //     free(new);
            //     return EXIT_FAILURE;
            // }
            // fputs(new, open);
            // fclose(open);
            printf("%s", new);
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
