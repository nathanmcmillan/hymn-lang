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
        if (c == '#' && pos + 1 < size && source[pos + 1] == ' ') {
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
        }
        break;
    }
    return expected;
}

static char *test(char *script) {
    String *source = cat(script);
    String *expected = parse_expected(source);
    Hymn *hymn = new_hymn();
    hymn->print = console;
    string_zero(out);
    char *error = hymn_do(hymn, source);
    hymn_delete(hymn);
    string_delete(source);
    ASSERT(error, error == NULL);
    ASSERT(out, string_equal(out, expected));
    string_delete(expected);
    return NULL;
}

static char *test_exception(char *script) {
    string_zero(out);
    Hymn *hymn = new_hymn();
    hymn->print = console;
    char *error = hymn_read(hymn, script);
    hymn_delete(hymn);
    ASSERT("Expected an exception.", error != NULL);
    return NULL;
}

static char *test_arithmetic() {
    return test("test/language/arithmetic.hm");
}

static char *test_runtime_error() {
    return test_exception("test/language/runtime_error.hm");
}

static char *test_if() {
    return test("test/language/if.hm");
}

static char *test_break() {
    return test("test/language/break.hm");
}

static char *test_switch() {
    return test("test/language/switch.hm");
}

static char *test_functions() {
    return test("test/language/functions.hm");
}

static char *benchmark_fib() {
    return test("test/language/fib.hm");
}

char *test_hymn_all() {
    out = new_string("");
    TEST(test_arithmetic);
    TEST(test_runtime_error);
    TEST(test_if);
    TEST(test_break);
    TEST(test_switch);
    TEST(test_functions);
    BENCHMARK(benchmark_fib);
    return NULL;
}
