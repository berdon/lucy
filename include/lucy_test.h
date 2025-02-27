#ifndef LUCY_TEST_H
#define LUCY_TEST_H

/* Predefined @Test annotation for unit tests with optional description */
#define TEST_ANNOTATION "// #annotation @Test(description) : @When(TARGET_TEST)"

/* Predefined @Disable annotation to mark tests as disabled with an optional reason */
#define DISABLE_ANNOTATION "// #annotation @Disable(reason) : @When(__LUCY_TEST_DISABLE__)"

/* Predefined @Setup annotation for setup function before each test */
#define SETUP_ANNOTATION "// #annotation @Setup : @When(TARGET_TEST)"

/* Predefined @Teardown annotation for teardown function after each test */
#define TEARDOWN_ANNOTATION "// #annotation @Teardown : @When(TARGET_TEST)"

// Standard C includes for assertion macros
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global flag to track test failure within a single test function
extern int __test_failed;

// Assertion macros
#define assertTrue(actual, message) \
    do { \
        int _actual = (actual); \
        if (!_actual) { \
            printf("FAIL: %s:%d - %s (expected true, got false)\n", \
                   __FILE__, __LINE__, (message) ? (message) : "Assertion failed"); \
            __test_failed = 1; \
        } \
    } while (0)

#define assertFalse(actual, message) \
    do { \
        int _actual = (actual); \
        if (_actual) { \
            printf("FAIL: %s:%d - %s (expected false, got true)\n", \
                   __FILE__, __LINE__, (message) ? (message) : "Assertion failed"); \
            __test_failed = 1; \
        } \
    } while (0)

#define assertEquals(expected, actual, message) \
    do { \
        typeof(expected) _exp = (expected); \
        typeof(actual) _act = (actual); \
        if (_exp != _act) { \
            printf("FAIL: %s:%d - %s (expected %ld, got %ld)\n", \
                   __FILE__, __LINE__, (message) ? (message) : "Values not equal", \
                   (long)_exp, (long)_act); \
            __test_failed = 1; \
        } \
    } while (0)

#define assertNotEquals(expected, actual, message) \
    do { \
        typeof(expected) _exp = (expected); \
        typeof(actual) _act = (actual); \
        if (_exp == _act) { \
            printf("FAIL: %s:%d - %s (expected different values, both %ld)\n", \
                   __FILE__, __LINE__, (message) ? (message) : "Values equal", \
                   (long)_exp); \
            __test_failed = 1; \
        } \
    } while (0)

#define assertStringEquals(expected, actual, message) \
    do { \
        const char *_exp = (const char *)(expected); \
        const char *_act = (const char *)(actual); \
        if (_exp == NULL && _act == NULL) { \
            /* Both null, considered equal, do nothing */ \
        } else if (_exp == NULL || _act == NULL) { \
            printf("FAIL: %s:%d - %s (expected '%s', got '%s')\n", \
                   __FILE__, __LINE__, (message) ? (message) : "Strings not equal", \
                   _exp ? _exp : "(null)", _act ? _act : "(null)"); \
            __test_failed = 1; \
        } else if (strcmp(_exp, _act) != 0) { \
            printf("FAIL: %s:%d - %s (expected '%s', got '%s')\n", \
                   __FILE__, __LINE__, (message) ? (message) : "Strings not equal", \
                   _exp, _act); \
            __test_failed = 1; \
        } \
    } while (0)

#define assertStringNotEquals(expected, actual, message) \
    do { \
        const char *_exp = (const char *)(expected); \
        const char *_act = (const char *)(actual); \
        if (_exp == NULL && _act == NULL) { \
            printf("FAIL: %s:%d - %s (expected '%s', got '%s')\n", \
                   __FILE__, __LINE__, (message) ? (message) : "Strings equal", \
                   "(null)", "(null)"); \
            __test_failed = 1; \
        } else if (_exp != NULL && _act != NULL && strcmp(_exp, _act) == 0) { \
            printf("FAIL: %s:%d - %s (expected '%s', got '%s')\n", \
                   __FILE__, __LINE__, (message) ? (message) : "Strings equal", \
                   _exp, _act); \
            __test_failed = 1; \
        } \
    } while (0)

#endif // LUCY_TEST_H