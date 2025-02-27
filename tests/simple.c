#include "../include/lucy_test.h"
#include <string.h>

// Global state for demonstration
static int setup_count = 0;
static int teardown_count = 0;

// @Setup
void setup_counter() {
    setup_count++;
}

// @Teardown
void teardown_counter() {
    teardown_count++;
}

// @Test("Test string equality")
void test_string_equality() {
    const char *expected = "hello";
    const char *actual = "hello";
    assertStringEquals(expected, actual, "Strings should match");
    assertEquals(1, setup_count, "Setup should have run once");
}

// @Test("Test basic arithmetic")
void test_arithmetic() {
    int expected = 4;
    int actual = 2 + 2;
    assertEquals(expected, actual, NULL);
}

// @Test("Verify setup and teardown counts")
void test_setup_teardown_counts() {
    assertEquals(setup_count - 1, teardown_count, "Setup and teardown counts should match");
    assertTrue(setup_count > 0, "Setup should have run at least once");
}

// @Test("Test string equality with both null")
void test_string_equality_both_null() {
    const char *expected = NULL;
    const char *actual = NULL;
    assertStringEquals(expected, actual, "Both null should be equal");
}

// @Test("Test string equality with expected null fails")
void test_string_equality_expected_null() {
    const char *expected = NULL;
    const char *actual = "hello";
    assertStringNotEquals(expected, actual, "Null and non-null should not be equal");
}

// @Test("Test string equality with actual null fails")
void test_string_equality_actual_null() {
    const char *expected = "hello";
    const char *actual = NULL;
    assertStringNotEquals(expected, actual, "Non-null and null should not be equal");
}

// @Test("Test string equality with non-null strings")
void test_string_equality_non_null() {
    const char *expected = "hello";
    const char *actual = "hello";
    assertStringEquals(expected, actual, "Same strings should pass");
}

// @Test("Test string inequality with both null fails")
void test_string_inequality_both_null() {
    const char *expected = NULL;
    const char *actual = NULL;
    assertStringEquals(expected, actual, "Both null should be equal (not unequal)");
}