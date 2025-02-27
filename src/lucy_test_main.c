#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/lucy_api.h"    // For lucy_init, lucy_cleanup, etc.
#include "../include/lucy_test.h"   // For assertions and test annotations
#include "annotations.h"            // Generated header with embedded lucy.h

extern int __ANNOTATION_COUNT;
extern struct Annotation __ANNOTATIONS[];
extern void sync_annotations(void);

int __test_failed = 0;

int main(int argc, char *argv[]) {
    int debug = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "-d") == 0) {
            debug = 1;
            break;
        }
    }

    lucy_init();

    if (debug) {
        printf("Total annotations: %d\n", __ANNOTATION_COUNT);
        for (int i = 0; i < __ANNOTATION_COUNT; i++) {
            printf("Annotation %d: name=%s, target_name=%s, isRemoved=%d\n",
                   i, __ANNOTATIONS[i].name, __ANNOTATIONS[i].target_name, __ANNOTATIONS[i].isRemoved);
        }
    }

    struct Annotation *tests = find_annotated_blocks("Test");
    struct Annotation *disabled = find_annotated_blocks("Disable");
    struct Annotation *setups = find_annotated_blocks("Setup");
    struct Annotation *teardowns = find_annotated_blocks("Teardown");

    if (debug) {
        printf("tests=%p, disabled=%p, setups=%p, teardowns=%p\n", 
               (void*)tests, (void*)disabled, (void*)setups, (void*)teardowns);
    }

    if (!tests || !disabled || !setups || !teardowns) {
        printf("Failed to allocate annotation arrays\n");
        if (tests) free(tests);
        if (disabled) free(disabled);
        if (setups) free(setups);
        if (teardowns) free(teardowns);
        lucy_cleanup();
        return 1;
    }

    if (debug) {
        printf("Tests found: %d\n", __ANNOTATION_COUNT);
        for (int i = 0; i < __ANNOTATION_COUNT; i++) {
            printf("  %d: name=%s, target_name=%s, isRemoved=%d\n", 
                   i, tests[i].name ? tests[i].name : "(null)", 
                   tests[i].target_name ? tests[i].target_name : "(null)", 
                   tests[i].isRemoved);
        }
    }

    struct Annotation enabled_tests[MAX_ANNOTATIONS];
    int enabled_test_count = 0;
    int test_count = 0;

    for (int i = 0; i < __ANNOTATION_COUNT && tests[i].name; i++) {
        if (strcmp(tests[i].name, "Test") == 0 && !tests[i].isRemoved) {
            test_count++;
        }
    }

    if (debug) printf("test_count: %d\n", test_count);

    int test_index = 0;
    for (int i = 0; i < __ANNOTATION_COUNT && tests[i].name && test_index < test_count; i++) {
        if (strcmp(tests[i].name, "Test") == 0 && !tests[i].isRemoved) {
            int is_disabled = 0;
            for (int j = 0; j < __ANNOTATION_COUNT && disabled[j].name; j++) {
                if (strcmp(disabled[j].name, "Disable") == 0 &&
                    strcmp(disabled[j].target_name, tests[i].target_name) == 0) {
                    is_disabled = 1;
                    break;
                }
            }
            if (!is_disabled) {
                if (debug) printf("Adding %s to enabled_tests\n", tests[i].target_name);
                enabled_tests[enabled_test_count] = tests[i];
                enabled_test_count++;
            }
            test_index++;
        }
    }

    int disabled_count = 0;
    for (int i = 0; i < __ANNOTATION_COUNT && disabled[i].name; i++) {
        if (strcmp(disabled[i].name, "Disable") == 0) {
            disabled_count++;
        }
    }

    if (debug) printf("disabled_count: %d, enabled_test_count: %d\n", disabled_count, enabled_test_count);

    int passed = 0;
    int failed = 0;
    for (int i = 0; i < enabled_test_count; i++) {
        const char *desc = (enabled_tests[i].arg_count > 0 && enabled_tests[i].args[0]) ? enabled_tests[i].args[0] : enabled_tests[i].target_name;
        printf("Running: %s ", desc);
        fflush(stdout);

        __test_failed = 0;
        for (int j = 0; j < __ANNOTATION_COUNT && setups[j].name; j++) {
            if (strcmp(setups[j].name, "Setup") == 0 && !setups[j].isRemoved) {
                if (debug) printf("Running setup: %s\n", setups[j].target_name);
                void (*setup_func)(void) = (void (*)(void))setups[j].target;
                setup_func();
            }
        }

        if (debug) printf("Running test: %s\n", desc);
        void (*test_func)(void) = (void (*)(void))enabled_tests[i].target;
        test_func();

        for (int j = 0; j < __ANNOTATION_COUNT && teardowns[j].name; j++) {
            if (strcmp(teardowns[j].name, "Teardown") == 0 && !teardowns[j].isRemoved) {
                if (debug) printf("Running teardown: %s\n", teardowns[j].target_name);
                void (*teardown_func)(void) = (void (*)(void))teardowns[j].target;
                teardown_func();
            }
        }

        if (__test_failed) {
            printf("✘\n");
            failed++;
        } else {
            printf("✔\n");
            passed++;
        }
    }

    /* ... disabled tests output ... */

    printf("\nTest Summary: %d/%d passed, %d failed (%d disabled)\n", passed, enabled_test_count, failed, disabled_count);
    if (failed == 0 && enabled_test_count > 0) {
        printf("✔ All enabled tests passed!\n");
    } else if (enabled_test_count == 0) {
        printf("✘ No enabled tests ran.\n");
    } else {
        printf("✘ Some enabled tests failed.\n");
    }

    if (debug) {
        printf("Freeing tests=%p\n", (void*)tests);
        printf("Freeing disabled=%p\n", (void*)disabled);
        printf("Freeing setups=%p\n", (void*)setups);
        printf("Freeing teardowns=%p\n", (void*)teardowns);
    }
    free(tests);
    free(disabled);
    free(setups);
    free(teardowns);
    lucy_cleanup();
    return failed > 0 ? 1 : 0;
}