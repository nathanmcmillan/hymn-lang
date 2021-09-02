#include "main.h"
#include "test.h"

int tests_success = 0;
int tests_fail = 0;
int tests_count = 0;

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    printf("\n");
    test_hymn();
    printf("\nSuccess: %d, Failed: %d, Total: %d\n\n", tests_success, tests_fail, tests_count);

    return 0;
}
