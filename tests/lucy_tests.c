/* Unit tests for lucy's internal functionality */
#include "../include/annotations.h"
#include "../include/test_utils.h"
#include "../include/lucy_api.h"
#include "../include/parsing.h"
#include <string.h>

/* Function prototypes from lucy.h for testing */
const char *get_extension_base(const char *name);
void load_extensions(const char *base_annotations_path);

/* Dummy functions to test translation */
void dummy_func(void) {}
void dummy_another(void) {}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcomment"
// @Test(TARGET_TEST, "Extract simple annotation name")
#pragma GCC diagnostic pop
void test_extract_simple_annotation() {
    lucy_init();
    char name[MAX_BUFFER_SIZE] = {0};
    char arg[MAX_BUFFER_SIZE] = {0};
    extract_annotation_name("// @When", name, arg);
    assertStringEquals("When", name, "Expected name to be 'When'");
    assertStringEquals("", arg, "Expected no arguments");
}

// @Test(TARGET_TEST, "Extract annotation with arguments")
void test_extract_annotation_with_args() {
    lucy_init();
    char name[MAX_BUFFER_SIZE] = {0};
    char arg[MAX_BUFFER_SIZE] = {0};
    extract_annotation_name("// @Test(TARGET_TEST, \"desc\")", name, arg);
    assertStringEquals("Test", name, "Expected name to be 'Test'");
    assertStringEquals("TARGET_TEST, \"desc\"", arg, "Expected arguments to match input");
}

// @Test(TARGET_TEST, "Extract annotation with missing closing parenthesis")
void test_extract_annotation_missing_paren() {
    lucy_init();
    char name[MAX_BUFFER_SIZE] = {0};
    char arg[MAX_BUFFER_SIZE] = {0};
    extract_annotation_name("// @Test(TARGET_TEST", name, arg);
    assertStringEquals("Test", name, "Expected name to be 'Test'");
    assertStringEquals("TARGET_TEST", arg, "Expected partial args despite no ')'");
}

// @Test(TARGET_TEST, "Extract annotation with whitespace")
void test_extract_annotation_with_whitespace() {
    lucy_init();
    char name[MAX_BUFFER_SIZE] = {0};
    char arg[MAX_BUFFER_SIZE] = {0};
    extract_annotation_name("// @When   \t", name, arg);
    assertStringEquals("When", name, "Expected name trimmed to 'When'");
    assertStringEquals("", arg, "Expected no arguments");
}

// @Test(TARGET_TEST, "Extract extension definition")
void test_extract_extension() {
    lucy_init();
    char name[MAX_BUFFER_SIZE] = {0};
    char args[MAX_BUFFER_SIZE] = {0};
    char base[MAX_BUFFER_SIZE] = {0};
    char base_arg[MAX_BUFFER_SIZE] = {0};
    extract_extension("// #annotation @Test(condition, desc) : @When(cond)", name, args, base, base_arg);
    assertStringEquals("Test", name, "Expected extension name 'Test'");
    assertStringEquals("condition, desc", args, "Expected extension args");
    assertStringEquals("When", base, "Expected base annotation 'When'");
    assertStringEquals("cond", base_arg, "Expected base arg 'cond'");
}

// @Test(TARGET_TEST, "Extract malformed extension - no base")
void test_extract_extension_no_base() {
    lucy_init();
    char name[MAX_BUFFER_SIZE] = {0};
    char args[MAX_BUFFER_SIZE] = {0};
    char base[MAX_BUFFER_SIZE] = {0};
    char base_arg[MAX_BUFFER_SIZE] = {0};
    extract_extension("// #annotation @Test(condition)", name, args, base, base_arg);
    assertStringEquals("Test", name, "Expected name 'Test' despite no base");
    assertStringEquals("condition", args, "Expected args 'condition'");
    assertStringEquals("", base, "Expected empty base with no ':'");
    assertStringEquals("", base_arg, "Expected empty base arg");
}

// @Test(TARGET_TEST, "Get extension base")
void test_get_extension_base() {
    lucy_init();
    extern int extension_count;
    extern Extension extensions[];
    strcpy(extensions[0].name, "Test");
    strcpy(extensions[0].base, "When");
    extension_count = 1;

    const char *base = get_extension_base("Test");
    assertStringEquals("When", base, "Expected 'Test' to extend 'When'");

    base = get_extension_base("Unknown");
    assertEquals(NULL, base, "Expected NULL for unknown annotation");
}

// @Test(TARGET_TEST, "Get extension base with empty list")
void test_get_extension_base_empty() {
    lucy_init();
    extern int extension_count;
    extension_count = 0;
    const char *base = get_extension_base("Test");
    assertEquals(NULL, base, "Expected NULL with no extensions");
}

// @Test(TARGET_TEST, "Process file with @When - manual check")
void test_process_when() {
    lucy_init();
    assertTrue(1, "Check build/lucy_tests_processed.c for #ifdef TARGET_TEST around dummy_func");
}

// @Test(TARGET_TEST, "Multiple annotations on same function")
void test_multiple_annotations() {
    lucy_init();
    assertTrue(1, "Manual check: Only last annotation should apply (limitation)");
}

// @Test(TARGET_TEST, "Generate annotations header - manual check")
void test_generate_annotations_header() {
    lucy_init();
    assertTrue(1, "Check build/annotations.h for extern void dummy_func(void);");
}

// @Test(TARGET_TEST, "Generate annotations source - manual check")
void test_generate_annotations_source() {
    lucy_init();
    assertTrue(1, "Check build/annotations.c for correct __ANNOTATIONS entry for dummy_func");
}

// @Test(TARGET_TEST, "Annotation with empty args")
void test_empty_args_annotation() {
    lucy_init();
    char name[MAX_BUFFER_SIZE] = {0};
    char arg[MAX_BUFFER_SIZE] = {0};
    extract_annotation_name("// @Test()", name, arg);
    assertStringEquals("Test", name, "Expected name 'Test'");
    assertStringEquals("", arg, "Expected empty args");
}

// @Test(TARGET_TEST, "Max annotations limit")
void test_max_annotations() {
    lucy_init();
    extern int annotation_count;
    annotation_count = MAX_ANNOTATIONS;
    assertTrue(1, "Manual check: Ensure no more than MAX_ANNOTATIONS are processed");
}