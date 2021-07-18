#include "test_hymn.h"

static char *test(char *script) {
    Hymn *hymn = new_hymn();
    char *error = hymn_read(hymn, script);
    if (error != NULL) {
        return error;
    }
    hymn_delete(hymn);
    ASSERT(error, error != NULL);
    return NULL;
}

static char *test_if() {
    return test("test/scripts/if.hm");
}

static char *test_break() {
    return test("test/scripts/break.hm");
}

static char *test_switch() {
    return test("test/scripts/switch.hm");
}

char *test_hymn_all() {
    TEST(test_if);
    TEST(test_break);
    TEST(test_switch);
    return NULL;
}
