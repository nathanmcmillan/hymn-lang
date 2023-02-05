/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "hymn.h"
#include "hymn_libs.h"
#include "hymn_path.h"
#include "hymn_text.h"

#if !defined(HYMN_NO_TEST)

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

HymnString *indent(HymnString *text) {
    size_t len = hymn_string_len(text);
    size_t newlines = 1;
    for (size_t i = 0; i < len; i++) {
        if (text[i] == '\n') {
            newlines++;
        }
    }
    HymnString *indented = hymn_new_string_with_capacity(len + newlines * 4);
    size_t n = 0;
    for (size_t s = 0; s < 4; s++) {
        indented[n++] = ' ';
    }
    for (size_t i = 0; i < len; i++) {
        if (text[i] == '\n') {
            indented[n++] = '\n';
            for (size_t s = 0; s < 4; s++) {
                indented[n++] = ' ';
            }
        } else {
            indented[n++] = text[i];
        }
    }
    hymn_string_delete(text);
    HymnStringHead *head = hymn_string_head(indented);
    head->length = head->capacity;
    indented[n++] = '\0';
    return indented;
}

static HymnString *parse_expected(HymnString *source) {
    HymnString *expected = hymn_new_string("");
    size_t size = hymn_string_len(source);
    for (size_t pos = 0; pos < size; pos++) {
        char c = source[pos];
        if (c == '#' && pos + 1 < size) {
            if (source[pos + 1] == ' ') {
                pos += 2;
                while (pos < size) {
                    c = source[pos];
                    expected = hymn_string_append_char(expected, c);
                    if (c == '\n') {
                        break;
                    }
                    pos++;
                }
                continue;
            } else if (source[pos + 1] == '\n') {
                pos++;
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
        printf("TEST FILE NOT FOUND: %s\n", script);
        exit(1);
    }
    HymnString *expected = parse_expected(source);
    HymnString *result = NULL;
    if (!hymn_string_equal(expected, "")) {
        Hymn *hymn = new_hymn();
        hymn->print = console;
        hymn->print_error = console;
        hymn_string_zero(out);
        char *error = hymn_run(hymn, script, source);
        hymn_delete(hymn);
        if (error != NULL) {
            size_t len = strlen(error);
            if (len > 7 && memcmp(error, "error: ", 7) == 0) {
                for (size_t s = 7; s <= len; s++) {
                    error[s - 7] = error[s];
                }
            } else if (len > 10 && memcmp(error, "compiler: ", 10) == 0) {
                for (size_t s = 10; s <= len; s++) {
                    error[s - 10] = error[s];
                }
            }
        }
        if (hymn_string_starts_with(expected, "@exception")) {
            if (error == NULL) {
                hymn_string_trim(out);
                out = indent(out);
                result = hymn_string_format("EXPECTED AN ERROR\nBUT WAS:\n%s", out);
            } else {
                size_t slen = hymn_string_len(expected);
                if (slen > 11) {
                    HymnString *start = hymn_substring(expected, 11, slen);
                    size_t len = strlen(error);
                    if (len < slen - 11 || memcmp(error, start, slen - 11) != 0) {
                        start = indent(start);
                        HymnString *was = indent(hymn_new_string(error));
                        result = hymn_string_format("EXPECTED ERROR:\n%s\n\nBUT WAS:\n%s", start, was);
                        hymn_string_delete(was);
                    }
                    hymn_string_delete(start);
                }
                free(error);
            }
        } else {
            hymn_string_trim(out);
            if (error != NULL) {
                result = hymn_new_string(error);
                hymn_string_trim(result);
                free(error);
            } else if (hymn_string_starts_with(expected, "@starts")) {
                HymnString *start = hymn_substring(expected, 8, hymn_string_len(expected));
                if (!hymn_string_starts_with(out, start)) {
                    start = indent(start);
                    out = indent(out);
                    result = hymn_string_format("EXPECTED START:\n%s\n\nBUT WAS:\n%s", start, out);
                }
                hymn_string_delete(start);
            } else if (!hymn_string_equal(out, expected)) {
                expected = indent(expected);
                out = indent(out);
                result = hymn_string_format("EXPECTED:\n%s\n\nBUT WAS:\n%s", expected, out);
            }
        }
    }
    hymn_string_delete(source);
    hymn_string_delete(expected);
    return result != NULL ? indent(result) : NULL;
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
    if (!hymn_string_equal(out, "{ \"number\": 8 }")) {
        printf("incorrent output: %s\n\n", out);
        tests_fail++;
        goto end;
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

static void test_dynamic_library() {
#ifndef HYMN_NO_DYNAMIC_LIBS
    tests_count++;
    printf("dlib\n");
    Hymn *hymn = new_hymn();
    hymn->print = console;
    hymn_string_zero(out);

    HymnString *result = hymn_use_dlib(hymn, "test" PATH_SEP_STRING "dlib" HYMN_DLIB_EXTENSION, "hymn_use_test_dlib");

    if (result != NULL) {
        fprintf(stderr, "error: %s\n", result);
        hymn_string_delete(result);
        tests_fail++;
        goto end;
    }

    char *error = NULL;

    error = hymn_do(hymn, "echo dlib.fun()");
    if (error != NULL) {
        goto fail;
    }

    hymn_string_trim(out);
    if (!hymn_string_equal(out, "256")) {
        printf("incorrent output: %s\n\n", out);
        tests_fail++;
        goto end;
    }

    tests_success++;
    goto end;

fail:
    printf("%s\n\n", error);
    free(error);
    tests_fail++;

end:
    hymn_delete(hymn);
#endif
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
        size_t len = hymn_string_len(script);
        if (len > 2) {
            size_t index = len - 2;
            while (true) {
                if (index == 0 || script[index] == PATH_SEP) {
                    printf("%s\n", &script[index + 1]);
                    break;
                }
                index--;
            }
        } else {
            printf("%s\n", script);
        }
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

    if (filter == NULL || hymn_string_equal(filter, "api")) {
        test_api();
    }

    if (filter == NULL || hymn_string_equal(filter, "dlib")) {
        test_dynamic_library();
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

#endif
