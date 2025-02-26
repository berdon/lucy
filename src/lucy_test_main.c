/* lucy_test_main.c - Entry point for lucy-test library
 *
 * Provides a default main() that runs all @Test-annotated functions and logs
 * @Disable-annotated functions when linked into an executable compiled with
 * -DTARGET_TEST=1.
 *
 * Dependencies:
 * - lucy_api.h: For lucy_init(), lucy_cleanup(), find_annotated_blocks().
 * - test_utils.h: For assertion macros and reporting.
 */
#include <stdio.h>
#include <string.h>
#include "../include/lucy_api.h"
#include "../include/test_utils.h"

/* External symbols provided by the user's generated annotations.c */
extern int __ANNOTATION_COUNT;
extern struct Annotation __ANNOTATIONS[];

int main(int argc, char *argv[]) {
    lucy_init();

    /* Find all @Test and @Disable-annotated functions */
    struct Annotation *tests = find_annotated_blocks("Test");
    struct Annotation *disabled = find_annotated_blocks("Disable");

    /* Filter enabled tests (excluding those with @Disable) */
    struct Annotation enabled_tests[MAX_ANNOTATIONS];
    int enabled_test_count = 0;
    for (int i = 0; i < __ANNOTATION_COUNT; i++) {
        if (tests[i].name && strcmp(tests[i].name, "Test") == 0 && !tests[i].isRemoved) {
            int is_disabled = 0;
            for (int j = 0; j < __ANNOTATION_COUNT; j++) {
                if (disabled[j].name && strcmp(disabled[j].name, "Disable") == 0 &&
                    strcmp(disabled[j].target_name, tests[i].target_name) == 0) {
                    is_disabled = 1;
                    break;
                }
            }
            if (!is_disabled) {
                enabled_tests[enabled_test_count] = tests[i];
                enabled_test_count++;
            }
        }
    }

    /* Count disabled tests */
    int disabled_count = 0;
    for (int i = 0; i < __ANNOTATION_COUNT; i++) {
        if (disabled[i].name && strcmp(disabled[i].name, "Disable") == 0) {
            disabled_count++;
        }
    }

    printf("Running tests...\n");

    /* Run enabled @Test-annotated functions */
    int passed = 0;
    for (int i = 0; i < enabled_test_count; i++) {
        const char *desc = (enabled_tests[i].arg_count > 0 && enabled_tests[i].args[0]) ? enabled_tests[i].args[0] : enabled_tests[i].target_name;
        printf("Running: %s ", desc);
        void (*test_func)(void) = (void (*)(void))enabled_tests[i].target;
        test_func();
        printf("✔\n");
        passed++;
    }

    /* Log disabled @Disable-annotated functions */
    if (disabled_count > 0) {
        printf("\nDisabled tests:\n");
        for (int i = 0; i < __ANNOTATION_COUNT; i++) {
            if (disabled[i].name && strcmp(disabled[i].name, "Disable") == 0) {
                const char *reason = (disabled[i].arg_count > 0 && disabled[i].args[0]) ? disabled[i].args[0] : "No reason provided";
                /* Find the @Test description if available, otherwise use target_name */
                const char *test_desc = disabled[i].target_name;
                for (int j = 0; j < __ANNOTATION_COUNT; j++) {
                    if (tests[j].name && strcmp(tests[j].name, "Test") == 0 &&
                        strcmp(tests[j].target_name, disabled[i].target_name) == 0 &&
                        tests[j].arg_count > 0 && tests[j].args[0]) {
                        test_desc = tests[j].args[0];
                        break;
                    }
                }
                printf("Disabled: %s (%s)\n", test_desc, reason);
            }
        }
    }

    printf("\nTest Summary: %d/%d passed (%d disabled)\n", passed, enabled_test_count, disabled_count);
    if (passed == enabled_test_count && enabled_test_count > 0) {
        printf("✔ All enabled tests passed!\n");
    } else if (enabled_test_count == 0) {
        printf("✘ No enabled tests ran.\n");
        return 1;
    } else {
        printf("✘ Some enabled tests failed.\n");
        return 1;
    }

    lucy_cleanup();
    return 0;
}