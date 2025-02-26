/* More complex unit tests for lucy annotation processing */
#include "../include/lucy_test.h"
#include <string.h>

// @Test("Test string inequality")
void test_string_inequality() {
    assertStringNotEquals("hello", "world", "Strings should not match");
}

// @Test("Test false condition")
void test_false_condition() {
    assertFalse(1 == 2, "One should not equal two");
}

// @Test("Test pointer comparison")
void test_pointer_comparison() {
    int x = 42;
    int *p = &x;
    int *q = &x;
    assertEquals(p, q, "Pointers to same address should match");
}