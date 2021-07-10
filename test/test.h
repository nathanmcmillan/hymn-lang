#ifndef TEST_H
#define TEST_H

#include <stdbool.h>
#include <stdio.h>

extern int tests_success;
extern int tests_fail;
extern int tests_count;

char *run_test(char *message, void(test)());

// #define VERBOSE

#ifdef VERBOSE
#define ASSERT(message, test)                                                                                                                                                                          \
    do {                                                                                                                                                                                               \
        printf("%s ... ", #test);                                                                                                                                                                      \
        fflush(stdout);                                                                                                                                                                                \
        if (!(test)) {                                                                                                                                                                                 \
            return message;                                                                                                                                                                            \
        }                                                                                                                                                                                              \
        printf("done!\n");                                                                                                                                                                             \
    } while (0)
#else
#define ASSERT(message, test)                                                                                                                                                                          \
    do {                                                                                                                                                                                               \
        if (!(test)) {                                                                                                                                                                                 \
            return message;                                                                                                                                                                            \
        }                                                                                                                                                                                              \
    } while (0)
#endif

#define TEST(test)                                                                                                                                                                                     \
    do {                                                                                                                                                                                               \
        printf("    %s\n", #test);                                                                                                                                                                     \
        char *message = test();                                                                                                                                                                        \
        tests_count++;                                                                                                                                                                                 \
        if (message) {                                                                                                                                                                                 \
            tests_fail++;                                                                                                                                                                              \
            return message;                                                                                                                                                                            \
        }                                                                                                                                                                                              \
        tests_success++;                                                                                                                                                                               \
    } while (0)

#define TEST_SET(test_set)                                                                                                                                                                             \
    do {                                                                                                                                                                                               \
        printf("%s\n", #test_set);                                                                                                                                                                     \
        char *result = test_set();                                                                                                                                                                     \
        if (result != 0) {                                                                                                                                                                             \
            printf("    ERROR: %s\n", result);                                                                                                                                                         \
        }                                                                                                                                                                                              \
        printf("\n");                                                                                                                                                                                  \
    } while (0);

#endif
