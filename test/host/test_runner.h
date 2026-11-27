#ifndef TEST_RUNNER_H
#define TEST_RUNNER_H
#include <stdio.h>
#include <string.h>

static int tests_run = 0, tests_failed = 0;

#define CHECK(cond) do { \
    tests_run++; \
    if (!(cond)) { tests_failed++; \
        printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); } \
  } while (0)

#define CHECK_STR(a, b) do { \
    tests_run++; \
    if (strcmp((a),(b)) != 0) { tests_failed++; \
        printf("FAIL %s:%d: \"%s\" != \"%s\"\n", __FILE__, __LINE__, (a), (b)); } \
  } while (0)

#define TEST_SUMMARY() do { \
    printf("%d run, %d failed\n", tests_run, tests_failed); \
    return tests_failed ? 1 : 0; \
  } while (0)

#endif
