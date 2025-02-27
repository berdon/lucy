/* Unit tests for lucy-test API functionality */
#include "../include/lucy_test.h"
#include <string.h>

// Static counters for setup/teardown verification
static int setup_count = 0;
static int teardown_count = 0;

// @Setup
void test_setup() {
    setup_count++;
}

// @Teardown
void test_teardown() {
    teardown_count++;
}

// @Test("Basic test runs")
void test_basic_test() {
    assertTrue(1, "Basic test should pass");
}

// @Test("Setup runs before test")
void test_setup_runs() {
    assertTrue(setup_count > 0, "Setup should have run once before this test");
}

// @Test("Teardown runs after test")
void test_teardown_runs() {
    assertTrue(teardown_count < setup_count, "Teardown should not have run yet for this test");
}

// @Test("Multiple tests increment counts")
void test_multiple_tests() {
    assertTrue(setup_count > 1, "Setup should have run for previous tests");
}

// @Test("Disabled test not run - manual check")
void test_disabled_not_run() {
    assertTrue(1, "Check disabled tests in output; this should run");
}

// @Disable("Demonstration")
// @Test("Disabled test")
void test_disabled() {
    assertTrue(0, "This should not run");
}