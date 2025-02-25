#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define assertTrue(actual, message) \
    do { \
        int _actual = (actual); \
        if (!_actual) { \
            printf("FAIL: %s:%d - %s (expected true, got false)\n", \
                   __FILE__, __LINE__, (message) ? (message) : "Assertion failed"); \
            exit(1); \
        } \
    } while (0)

#define assertFalse(actual, message) \
    do { \
        int _actual = (actual); \
        if (_actual) { \
            printf("FAIL: %s:%d - %s (expected false, got true)\n", \
                   __FILE__, __LINE__, (message) ? (message) : "Assertion failed"); \
            exit(1); \
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
            exit(1); \
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
            exit(1); \
        } \
    } while (0)

#define assertStringEquals(expected, actual, message) \
    do { \
        if (strcmp((const char *)(expected), (const char *)(actual)) != 0) { \
            printf("FAIL: %s:%d - %s (expected '%s', got '%s')\n", \
                   __FILE__, __LINE__, (message) ? (message) : "Strings not equal", \
                   (const char *)(expected), (const char *)(actual)); \
            exit(1); \
        } \
    } while (0)

#define assertStringNotEquals(expected, actual, message) \
    do { \
        if (strcmp((const char *)(expected), (const char *)(actual)) == 0) { \
            printf("FAIL: %s:%d - %s (expected different strings, both '%s')\n", \
                   __FILE__, __LINE__, (message) ? (message) : "Strings equal", \
                   (const char *)(expected)); \
            exit(1); \
        } \
    } while (0)

#endif // TEST_UTILS_H