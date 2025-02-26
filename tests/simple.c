/* Simple unit tests for lucy annotation processing */
#include "../include/lucy_test.h"
#include <string.h>

// @Test("Test string equality")
void test_string_equality() {
    const char *expected = "hello";
    const char *actual = "hello";
    assertStringEquals(expected, actual, "Strings should match");
}

// @Test("Test basic arithmetic")
void test_arithmetic() {
    int expected = 4;
    int actual = 2 + 2;
    assertEquals(expected, actual, NULL);
}

// @Disable("Test disabled for demonstration")
// @Test("Disabled test expected to not run")
void test_disabled() {
    assertTrue(0, "This should not run");
}