#ifndef LUCY_TEST_H
#define LUCY_TEST_H

/* Predefined @Test annotation for unit tests with optional description
 * Usage:
 *   // @Test
 *   void my_test() { ... }
 *   // @Test("My test description")
 *   void my_test() { ... }
 * Compile with -DTARGET_TEST=1 to run tests. Description is optional.
 */
#define TEST_ANNOTATION "// #annotation @Test(description) : @When(TARGET_TEST)"

/* Predefined @Disable annotation to mark tests as disabled with an optional reason
 * Usage:
 *   // @Disable
 *   void my_test() { ... }
 *   // @Disable("Reason for disabling")
 *   void my_test() { ... }
 * These tests will be logged as disabled and not executed (__LUCY_TEST_DISABLE__ is never defined).
 */
#define DISABLE_ANNOTATION "// #annotation @Disable(reason) : @When(__LUCY_TEST_DISABLE__)"

/* Include this header and link against liblucy-test.so to run tests */
#include "test_utils.h"  // For assert macros

#endif // LUCY_TEST_H