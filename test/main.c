#include "main.h"
#include "test.h"

int tests_success = 0;
int tests_fail = 0;
int tests_count = 0;

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
