/* Simple unit tests for lucy annotation processing */
#include "../include/annotations.h"
#include "../include/test_utils.h"
#include <string.h>

// @Test(TARGET_TEST, "Test string equality")
void test_string_equality() {
    const char *expected = "hello";
    const char *actual = "hello";
    assertStringEquals(expected, actual, "Strings should match");
}

// @Test(TARGET_TEST, "Test basic arithmetic")
void test_arithmetic() {
    int expected = 4;
    int actual = 2 + 2;
    assertEquals(expected, actual, NULL);
}

// @Test(DISABLED_TEST, "Disabled test")
void test_disabled() {
    assertTrue(0, "This should not run");
}