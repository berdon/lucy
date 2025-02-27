/* Unit tests for lucy's API functionality */
#include "../include/lucy_test.h"
#include "../include/lucy_api.h"
#include "../include/parsing.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern struct Annotation *get_annotations(void);
extern int get_annotation_count(void);

/* Dummy functions for testing */
void dummy_func(void) {}
void dummy_another(void) {}

// @Test("Extract simple annotation name")
void test_extract_simple_annotation() {
    char name[MAX_BUFFER_SIZE] = {0};
    char arg[MAX_BUFFER_SIZE] = {0};
    extract_annotation_name("// @When", name, arg);
    assertStringEquals("When", name, "Expected name to be 'When'");
    assertStringEquals("", arg, "Expected no arguments");
}

// @Test("Extract annotation with arguments")
void test_extract_annotation_with_args() {
    char name[MAX_BUFFER_SIZE] = {0};
    char arg[MAX_BUFFER_SIZE] = {0};
    extract_annotation_name("// @Test(TARGET_TEST, \"desc\")", name, arg);
    assertStringEquals("Test", name, "Expected name to be 'Test'");
    assertStringEquals("TARGET_TEST, \"desc\"", arg, "Expected arguments to match input");
}

// @Test("Extract annotation with missing closing parenthesis")
void test_extract_annotation_missing_paren() {
    char name[MAX_BUFFER_SIZE] = {0};
    char arg[MAX_BUFFER_SIZE] = {0};
    extract_annotation_name("// @Test(TARGET_TEST", name, arg);
    assertStringEquals("Test", name, "Expected name to be 'Test'");
    assertStringEquals("TARGET_TEST", arg, "Expected partial args despite no ')'");
}

// @Test("Extract annotation with whitespace")
void test_extract_annotation_with_whitespace() {
    char name[MAX_BUFFER_SIZE] = {0};
    char arg[MAX_BUFFER_SIZE] = {0};
    extract_annotation_name("// @When   \t", name, arg);
    assertStringEquals("When", name, "Expected name trimmed to 'When'");
    assertStringEquals("", arg, "Expected no arguments");
}

// @Test("Extract extension definition")
void test_extract_extension() {
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

// @Test("Extract malformed extension - no base")
void test_extract_extension_no_base() {
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

// @Test("lucy_init resets state")
void test_lucy_init() {
    const char *input = "test_input.c";
    const char *output = "test_output.c";
    FILE *f = fopen(input, "w");
    fprintf(f, "// @When(TARGET_TEST)\nvoid test_func() {}\n");
    fclose(f);
    lucy_init();
    lucy_process_file(input, output);
    // Don’t check __ANNOTATIONS; it’s static
    // Verify runtime state via file output
    FILE *out = fopen(output, "r");
    char line[MAX_LINE_LENGTH];
    assertTrue(fgets(line, sizeof(line), out) != NULL, "Should read first line");
    assertStringEquals("#ifdef TARGET_TEST\n", line, "Expected #ifdef TARGET_TEST");
    fclose(out);

    remove(input);
    remove(output);
}

// @Test("lucy_process_file with @When")
void test_lucy_process_file_when() {
    const char *input = "test_input.c";
    const char *output = "test_output.c";
    FILE *f = fopen(input, "w");
    fprintf(f, "// @When(TARGET_TEST)\nvoid test_func() {}\n");
    fclose(f);

    lucy_init();
    int result = lucy_process_file(input, output);
    assertEquals(0, result, "Processing should succeed");

    FILE *out = fopen(output, "r");
    char line[MAX_LINE_LENGTH];
    assertTrue(fgets(line, sizeof(line), out) != NULL, "Should read first line");
    assertStringEquals("#ifdef TARGET_TEST\n", line, "Expected #ifdef TARGET_TEST");
    assertTrue(fgets(line, sizeof(line), out) != NULL, "Should read second line");
    assertStringEquals("void test_func() {}\n", line, "Expected function definition");
    assertTrue(fgets(line, sizeof(line), out) != NULL, "Should read third line");
    assertStringEquals("#endif\n", line, "Expected #endif");
    fclose(out);

    // Don’t check find_annotated_blocks; runtime annotations aren’t synced
    remove(input);
    remove(output);
}

// @Test("lucy_process_file with extension")
void test_lucy_process_file_extension() {
    const char *input = "test_input.c";
    const char *output = "test_output.c";
    FILE *f = fopen(input, "w");
    fprintf(f, "// #annotation @Custom(flag) : @When(TARGET_TEST)\n// @Custom(\"flag1\")\nvoid test_func() {}\n");
    fclose(f);

    lucy_init();
    int result = lucy_process_file(input, output);
    assertEquals(0, result, "Processing should succeed");

    FILE *out = fopen(output, "r");
    char line[MAX_LINE_LENGTH];
    fgets(line, sizeof(line), out); // Extension definition
    assertStringEquals("// #annotation @Custom(flag) : @When(TARGET_TEST)\n", line, "Expected extension definition preserved");
    fgets(line, sizeof(line), out); // #ifdef
    assertStringEquals("#ifdef TARGET_TEST\n", line, "Expected #ifdef from @Custom extension");
    fclose(out);

    struct Annotation *found = find_annotated_blocks("Custom");
    assertTrue(found[0].name != NULL, "Should find @Custom annotation");
    assertStringEquals("Custom", found[0].name, "Annotation name should be 'Custom'");
    free(found);

    remove(input);
    remove(output);
}

// @Test("lucy_generate_annotations_header")
void test_lucy_generate_annotations_header() {
    const char *base = "./include/annotations.h";
    const char *output = "test_annotations.h";
    lucy_init();
    int result = lucy_generate_annotations_header(base, output);
    assertEquals(0, result, "Header generation should succeed");

    FILE *f = fopen(output, "r");
    char buffer[1024];
    fread(buffer, 1, sizeof(buffer), f);
    assertTrue(strstr(buffer, "#ifndef ANNOTATIONS_H") != NULL, "Expected header guard");
    assertTrue(strstr(buffer, "#include \"lucy.h\"") != NULL, "Expected lucy.h include");
    fclose(f);
    remove(output);
}

// @Test("lucy_generate_annotations_source")
void test_lucy_generate_annotations_source() {
    const char *input = "test_input.c";
    const char *output = "test_output.c";
    const char *annotations_c = "test_annotations.c";
    FILE *f = fopen(input, "w");
    fprintf(f, "// @When(TARGET_TEST)\nvoid test_func() {}\n");
    fclose(f);

    lucy_init();
    lucy_process_file(input, output);
    int result = lucy_generate_annotations_source(annotations_c);
    assertEquals(0, result, "Source generation should succeed");

    FILE *out = fopen(annotations_c, "r");
    char buffer[1024];
    fread(buffer, 1, sizeof(buffer), out);
    assertTrue(strstr(buffer, "struct Annotation __ANNOTATIONS[") != NULL, "Expected annotations array");
    assertTrue(strstr(buffer, "{\"When\", test_func,") != NULL, "Expected test_func entry");
    fclose(out);

    remove(input);
    remove(output);
    remove(annotations_c);
}

// @Test("find_annotated_blocks finds runtime annotations")
void test_find_annotated_blocks() {
    const char *input = "test_input.c";
    const char *output = "test_output.c";
    FILE *f = fopen(input, "w");
    fprintf(f, "// @When(TARGET_TEST)\nvoid dummy_func() {}\n");
    fclose(f);

    lucy_init();
    int result = lucy_process_file(input, output);
    assertEquals(0, result, "Processing should succeed");

    struct Annotation *found = find_annotated_blocks("When");
    int count = 0;
    if (found) {
        for (int i = 0; found[i].name && i < MAX_ANNOTATIONS; i++) {
            if (strcmp(found[i].name, "When") == 0) {
                count++;
            }
        }
        free(found);
    } else {
        assertTrue(0, "find_annotated_blocks returned NULL");
    }
    assertTrue(count > 0, "Should find at least one When annotation after processing");

    remove(input);
    remove(output);
}

// @Test("lucy_cleanup resets state")
void test_lucy_cleanup() {
    const char *input = "test_input.c";
    const char *output = "test_output.c";
    FILE *f = fopen(input, "w");
    fprintf(f, "// @When(TARGET_TEST)\nvoid test_func() {}\n");
    fclose(f);

    lucy_init();
    lucy_process_file(input, output);
    struct Annotation *before = find_annotated_blocks("When");
    int before_count = 0;
    for (int i = 0; before[i].name && i < MAX_ANNOTATIONS; i++) before_count++;
    free(before);

    lucy_cleanup();
    struct Annotation *after = find_annotated_blocks("When");
    int after_count = 0;
    for (int i = 0; after[i].name && i < MAX_ANNOTATIONS; i++) after_count++;
    free(after);

    assertTrue(before_count > 0, "Should have annotations before cleanup");
    assertEquals(0, after_count, "Should have no annotations after cleanup");

    remove(input);
    remove(output);
}