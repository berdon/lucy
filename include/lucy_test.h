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

/* Include this header and link against liblucy-test.so to run tests */
#include "test_utils.h"  // For assert macros

#endif // LUCY_TEST_H