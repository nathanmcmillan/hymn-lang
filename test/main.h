#ifndef MAIN_H
#define MAIN_H

#include <stdbool.h>
#include <stdio.h>

extern int tests_success;
extern int tests_fail;
extern int tests_count;

#define ASSERT(message, test) \
    do {                      \
        if (!(test)) {        \
            return message;   \
        }                     \
    } while (0)
#endif

#define TEST(test)                 \
    do {                           \
        printf("    %s\n", #test); \
        char *message = test();    \
        tests_count++;             \
        if (message) {             \
            tests_fail++;          \
            return message;        \
        }                          \
        tests_success++;           \
    } while (0)

#define TEST_SET(test_set)          \
    do {                            \
        printf("%s\n", #test_set);  \
        char *result = test_set();  \
        if (result != 0) {          \
            printf("%s\n", result); \
        }                           \
        printf("\n");               \
    } while (0);

#define BENCHMARK(test)                                              \
    do {                                                             \
        clock_t start = clock();                                     \
        char *message = test();                                      \
        double elapsed = (double)(clock() - start) / CLOCKS_PER_SEC; \
        printf("    %s: %f seconds\n", #test, elapsed);              \
        tests_count++;                                               \
        if (message) {                                               \
            tests_fail++;                                            \
            return message;                                          \
        }                                                            \
        tests_success++;                                             \
    } while (0)