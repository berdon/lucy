#include "../include/annotations.h"
#include "../include/test_utils.h"
#include <string.h>

// @Test(TARGET_TEST, "Test string inequality")
void test_string_inequality() {
    const char *expected = "hello";
    const char *actual = "world";
    assertNotEquals(expected, actual, "Strings should differ");
}

// @Test(TARGET_TEST, "Test false condition")
void test_false_condition() {
    assertFalse(0, "Zero should be false");
    assertTrue(1, "One should be true");
}

// @Test(TARGET_TEST, "Test pointer comparison")
void test_pointers() {
    int x = 42;
    int *expected = &x;
    int *actual = &x;
    assertEquals(expected, actual, "Pointers should match");
}

// @Test(DISABLED_TEST, "Another disabled test")
void test_disabled_complex() {
    assertEquals(1, 2, "This should not run either");
}