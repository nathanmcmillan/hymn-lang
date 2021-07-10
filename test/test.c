#include "test.h"
#include "test_hymn.h"

int tests_success = 0;
int tests_fail = 0;
int tests_count = 0;

int main() {
    printf("\n");
    TEST_SET(test_hymn_all);
    printf("Success: %d, Failed: %d, Total: %d\n\n", tests_success, tests_fail, tests_count);
    return 0;
}
