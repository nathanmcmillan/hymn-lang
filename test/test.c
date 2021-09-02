#include "test.h"

static String *out;

static void console(const char *format, ...) {
    va_list args;

    va_start(args, format);
    int len = vsnprintf(NULL, 0, format, args);
    va_end(args);
    char *chars = safe_malloc((len + 1) * sizeof(char));
    va_start(args, format);
    vsnprintf(chars, len + 1, format, args);
    va_end(args);
    out = string_append(out, chars);
    free(chars);
}

static String *parse_expected(String *source) {
    String *expected = new_string("");
    usize size = string_len(source);
    for (usize pos = 0; pos < size; pos++) {
        char c = source[pos];
        if (c == '#' && pos + 1 < size) {
            if (source[pos + 1] == ' ') {
                pos += 2;
                while (pos < size) {
                    c = source[pos];
                    expected = string_append_char(expected, c);
                    if (c == '\n') {
                        break;
                    }
                    pos++;
                }
                continue;
            } else if (source[pos + 1] == '\n') {
                pos++;
                expected = string_append_char(expected, '\n');
                continue;
            }
        }
        break;
    }
    return expected;
}

static String *test_source(String *script) {
    String *source = cat(script);
    String *expected = parse_expected(source);
    Hymn *hymn = new_hymn();
    hymn->print = console;
    string_zero(out);
    char *error = hymn_do(hymn, source);
    hymn_delete(hymn);
    string_delete(source);
    String *result = NULL;
    if (strcmp(expected, "error\n") == 0) {
        result = new_string("Expected an error.\n");
    } else {
        if (error != NULL) {
            result = new_string(error);
            free(error);
        } else if (!string_equal(out, expected)) {
            result = string_format("Expected:\n%s\nBut was:\n%s", expected, out);
        }
    }
    string_delete(expected);
    return result;
}

void test_hymn() {
    out = new_string("");

    struct FileList all = directories("test/language");

    String *end = new_string(".hm");
    struct FilterList scripts = string_filter_ends_with(all.files, all.count, end);

    for (int i = 0; i < scripts.count; i++) {
        tests_count++;
        String *script = scripts.filtered[i];
        String *result = test_source(script);
        if (result != NULL) {
            printf("⨯ %s\n%s\n", script, result);
            tests_fail++;
        } else {
            printf("✓ %s\n", script);
            tests_success++;
        }
        string_delete(result);
    }

    delete_file_list(&all);
    delete_filter_list(&scripts);
    string_delete(end);
    // BENCHMARK(benchmark_fib);
}
