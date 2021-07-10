#include "test_hymn.h"

static char *test() {

    Hymn *vm = new_hymn();

    char *error = hymn_read(vm, "test/scripts/temp.hm");
    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
        return 1;
    }

    ASSERT("example", true);

    return 0;
}

char *test_hymn_all() {
    TEST(test);
    return 0;
}
