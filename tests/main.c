/* Test runner for lucy-annotated unit tests.
 * This program finds and executes functions annotated with @Test from the generated
 * annotations.h, reporting results with colored ASCII icons.
 * 
 * Dependencies:
 * - annotations.h (generated by lucy): Provides __ANNOTATIONS, __ANNOTATION_COUNT, and find_annotated_blocks.
 * - test_utils.h: Provides assertion macros (assertTrue, assertFalse, assertEquals, assertNotEquals).
 *
 * Usage:
 * Compile with annotations.o, test files, and run:
 *   gcc -o test_runner build/*.<o>  (Note: Use .<o> to avoid nested comment warning)
 *   ./test_runner
 */

 #include "../include/annotations.h"  // Generated annotation tracking
 #include "../include/lucy_test.h"   // Assertion macros
 #include <string.h>
 #include <stdio.h>
 
 /* Forward declarations of test functions (defined in processed test files).
  * These are annotated with @Test and processed by lucy into annotations.h.
  */
 void test_string_equality(void);
 void test_arithmetic(void);
 void test_string_inequality(void);
 void test_false_condition(void);
 void test_pointers(void);
 
 int main() {
     /* Initialize counters for test results */
     int passed = 0;
     int total = 0;
 
     /* ANSI escape codes for colored output */
     const char *GREEN_CHECK = "\033[32m✔\033[0m";  // Green checkmark
     const char *RED_X = "\033[31m✘\033[0m";        // Red X
 
     printf("Running tests...\n");
 
     /* Find all @Test-annotated functions using lucy's generated tracking */
     struct Annotation *tests = find_annotated_blocks("Test");
 
     /* Iterate over all annotations */
     for (int i = 0; i < __ANNOTATION_COUNT; i++) {
         /* Check if the annotation is a @Test, has a valid target, and isn’t removed */
         if (tests[i].target && !tests[i].isRemoved && strcmp(tests[i].name, "Test") == 0) {
             total++;
             void (*test_func)(void) = (void (*)(void))tests[i].target;  // Cast target to function pointer
             const char *desc = tests[i].args[1];                        // Second arg is the description
 
             printf("Running: %s ", desc);
             fflush(stdout);  // Ensure "Running" prints before test execution
 
             test_func();  // Execute the test function
 
             /* If we reach here, the test passed (assertions exit on failure) */
             passed++;
             printf("%s\n", GREEN_CHECK);
         }
     }
 
     /* Print summary with pass/fail counts */
     printf("\nTest Summary: %d/%d passed\n", passed, total);
     if (passed == total && total > 0) {
         printf("%s All tests passed!\n", GREEN_CHECK);
         return 0;
     } else {
         printf("%s Some tests failed or no tests ran.\n", RED_X);
         return 1;
     }
 }