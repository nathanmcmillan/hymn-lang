/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "hymn.h"
#include "hymn_path.h"
#include "hymn_string.h"

struct FilterList {
    int count;
    HymnString **filtered;
};

int tests_success = 0;
int tests_fail = 0;
int tests_count = 0;

static HymnString *out;

static struct FilterList string_filter(HymnString **input, int count, bool (*filter)(HymnString *string, const char *using), const char *using) {
    int size = 0;
    HymnString **filtered = hymn_calloc(count, sizeof(HymnString *));
    for (int i = 0; i < count; i++) {
        if (filter(input[i], using)) {
            filtered[size++] = hymn_string_copy(input[i]);
        }
    }
    return (struct FilterList){.count = size, .filtered = filtered};
}

static void delete_filter_list(struct FilterList *list) {
    for (int i = 0; i < list->count; i++) {
        hymn_string_delete(list->filtered[i]);
    }
    free(list->filtered);
}

static void console(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int len = vsnprintf(NULL, 0, format, args);
    va_end(args);
    char *chars = hymn_malloc((len + 1) * sizeof(char));
    va_start(args, format);
    vsnprintf(chars, len + 1, format, args);
    va_end(args);
    out = hymn_string_append(out, chars);
    free(chars);
}

static HymnString *parse_expected(HymnString *source) {
    HymnString *expected = hymn_new_string("");
    size_t size = hymn_string_len(source);
    for (size_t pos = 0; pos < size; pos++) {
        char c = source[pos];
        if (c == '-' && pos + 2 < size && source[pos + 1] == '-') {
            if (source[pos + 2] == ' ') {
                pos += 3;
                while (pos < size) {
                    c = source[pos];
                    expected = hymn_string_append_char(expected, c);
                    if (c == '\n') {
                        break;
                    }
                    pos++;
                }
                continue;
            } else if (source[pos + 2] == '\n') {
                pos += 2;
                expected = hymn_string_append_char(expected, '\n');
                continue;
            }
        }
        break;
    }
    hymn_string_trim(expected);
    return expected;
}

static HymnString *test_source(HymnString *script) {
    HymnString *source = hymn_read_file(script);
    if (source == NULL) {
        printf("Test file not found: %s\n", script);
        exit(1);
    }
    HymnString *expected = parse_expected(source);
    HymnString *result = NULL;
    if (strcmp(expected, "") != 0) {
        Hymn *hymn = new_hymn();
        hymn->print = console;
        hymn->print_error = console;
        hymn_string_zero(out);
        char *error = hymn_run(hymn, script, source);
        hymn_delete(hymn);
        if (strcmp(expected, "@Exception") == 0) {
            if (error == NULL) {
                result = hymn_new_string("Expected an error.\n");
            } else {
                free(error);
            }
        } else {
            hymn_string_trim(out);
            if (error != NULL) {
                result = hymn_new_string(error);
                free(error);
                hymn_string_trim(result);
            } else if (hymn_string_starts_with(expected, "@Starts")) {
                HymnString *start = hymn_substring(expected, 8, hymn_string_len(expected));
                if (!hymn_string_starts_with(out, start)) {
                    result = hymn_string_format("Expected start:\n%s\n\nBut was:\n%s", start, out);
                }
                hymn_string_delete(start);
            } else if (!hymn_string_equal(out, expected)) {
                result = hymn_string_format("Expected:\n%s\n\nBut was:\n%s", expected, out);
            }
        }
    }
    hymn_string_delete(source);
    hymn_string_delete(expected);
    return result;
}

HymnValue fun_for_vm(Hymn *vm, int count, HymnValue *arguments) {
    (void)vm;
    (void)count;
    (void)arguments;
    return hymn_new_none();
}

static void test_api() {
    tests_count++;
    printf("api\n");
    Hymn *hymn = new_hymn();
    hymn->print = console;
    hymn_string_zero(out);

    hymn_add_function(hymn, "fun", fun_for_vm);

    void *point = hymn_calloc(1, sizeof(void *));
    hymn_add_pointer(hymn, "point", point);

    char *error = NULL;

    error = hymn_do(hymn, "fun(point)");
    if (error != NULL) {
        goto fail;
    }

    hymn_do(hymn, "let table = {}");
    HymnTable *table = hymn_as_table(hymn_get(hymn, "table"));
    hymn_set_property_const(hymn, table, "number", hymn_new_int(8));
    error = hymn_do(hymn, "echo table");
    if (error != NULL) {
        goto fail;
    }

    hymn_string_trim(out);
    if (strcmp(out, "{ number: 8 }") != 0) {
        printf("Incorrent output: %s\n\n", out);
    }

    tests_success++;
    goto end;

fail:
    printf("%s\n\n", error);
    free(error);
    tests_fail++;

end:
    hymn_delete(hymn);
    free(point);
}

static void test_hymn(const char *filter) {
    out = hymn_new_string("");

    struct HymnPathFileList all = hymn_walk("test" PATH_SEP_STRING "language", true);

    struct FilterList scripts = string_filter(all.files, all.count, hymn_string_ends_with, ".hm");

    for (int i = 0; i < scripts.count; i++) {
        HymnString *script = scripts.filtered[i];
        if (filter != NULL && !hymn_string_contains(script, filter)) {
            continue;
        }
        tests_count++;
        printf("%s\n", script);
        HymnString *error = test_source(script);
        if (error != NULL) {
            printf("%s\n\n", error);
            tests_fail++;
        } else {
            tests_success++;
        }
        hymn_string_delete(error);
    }

    hymn_delete_file_list(&all);
    delete_filter_list(&scripts);

    if (filter == NULL || strcmp(filter, "api") == 0) {
        test_api();
    }

    hymn_string_delete(out);
}

int main(int argc, char **argv) {
    const char *filter = NULL;
    if (argc >= 2) {
        filter = argv[1];
    }

    printf("\n");
    clock_t start = clock();
    test_hymn(filter);
    double elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
    printf("\nSuccess: %d, Failed: %d, Total: %d, Time: %g s\n\n", tests_success, tests_fail, tests_count, elapsed);

    return 0;
}
