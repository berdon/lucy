/* lucy_lib.c - Core functionality for the Lucy annotation processor library */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/lucy_api.h"
#include "../include/parsing.h"

/* Static global state for annotation tracking */
static struct Annotation annotations[MAX_ANNOTATIONS];
int annotation_count = 0;

struct Annotation *get_annotations(void) {
    return annotations;
}

int get_annotation_count(void) {
    return annotation_count;
}

void sync_annotations(void) {
    /* Copy static annotations to __ANNOTATIONS */
    for (int i = 0; i < annotation_count && i < MAX_ANNOTATIONS; i++) {
        __ANNOTATIONS[i] = annotations[i];
    }
    __ANNOTATION_COUNT = annotation_count;
}

/* Non-static global state for extensions (exposed via lucy.h for testing) */
Extension extensions[MAX_ANNOTATIONS];
int extension_count = 0;

/* Weak symbols for annotation tracking, overridden by generated annotations.c */
__attribute__((weak)) int __ANNOTATION_COUNT = 0;
__attribute__((weak)) struct Annotation __ANNOTATIONS[MAX_ANNOTATIONS] = {};

/* Temporary structure to hold multiple annotations before a function */
typedef struct {
    char name[MAX_BUFFER_SIZE];
    char arg[MAX_BUFFER_SIZE];
} PendingAnnotation;

/* Looks up the base annotation for an extension */
const char *get_extension_base(const char *name) {
    for (int i = 0; i < extension_count; i++) {
        if (strcmp(extensions[i].name, name) == 0) {
            return extensions[i].base;
        }
    }
    return NULL;
}

/* Loads extension definitions from annotations.h */
void load_extensions(const char *base_annotations_path) {
    FILE *base_in = fopen(base_annotations_path, "r");
    if (!base_in) {
        perror("Error opening base annotations.h for extensions");
        return;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), base_in)) {
        line[strcspn(line, "\n")] = 0;
        if (is_extension_def(line)) {
            if (extension_count < MAX_ANNOTATIONS) {
                extract_extension(line, extensions[extension_count].name,
                                 extensions[extension_count].base_arg,
                                 extensions[extension_count].base,
                                 extensions[extension_count].base_arg);
                extension_count++;
            }
        }
    }
    fclose(base_in);
}

/* Processes an input C file with annotations */
int lucy_process_file(const char *input_path, const char *output_path) {
    FILE *in = fopen(input_path, "r");
    FILE *out = fopen(output_path, "w");
    if (!in || !out) {
        perror("File error");
        if (in) fclose(in);
        if (out) fclose(out);
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    PendingAnnotation pending_annotations[MAX_ANNOTATIONS];
    int pending_count = 0;
    int in_when_block = 0;
    int brace_count = 0;

    while (fgets(line, sizeof(line), in)) {
        line[strcspn(line, "\n")] = 0;

        if (is_extension_def(line)) {
            if (extension_count < MAX_ANNOTATIONS) {
                extract_extension(line, extensions[extension_count].name,
                                 extensions[extension_count].base_arg,
                                 extensions[extension_count].base,
                                 extensions[extension_count].base_arg);
                extension_count++;
            }
            fprintf(out, "%s\n", line);
            continue;
        }

        if (is_annotation(line)) {
            if (pending_count < MAX_ANNOTATIONS) {
                extract_annotation_name(line, pending_annotations[pending_count].name,
                                       pending_annotations[pending_count].arg);
                pending_count++;
            }
            continue;
        }

        if (pending_count > 0 && is_function_definition(line)) {
            char func_name[MAX_BUFFER_SIZE];
            extract_function_name(line, func_name);

            int has_when = 0;
            for (int i = 0; i < pending_count; i++) {
                const char *base = get_extension_base(pending_annotations[i].name);
                if (strcmp(pending_annotations[i].name, "When") == 0 ||
                    (base && strcmp(base, "When") == 0)) {
                    has_when = 1;
                }
            }

            /* Debugging 1: Log when we apply #ifdef */
            if (has_when) {
                fprintf(out, "#ifdef TARGET_TEST\n");
                in_when_block = 1;
                brace_count = 1;
            }

            /* Debugging 2: Log annotation tracking */
            for (int i = 0; i < pending_count && annotation_count < MAX_ANNOTATIONS; i++) {
                annotations[annotation_count].name = strdup(pending_annotations[i].name);
                annotations[annotation_count].target = NULL;
                annotations[annotation_count].target_name = strdup(func_name);
                annotations[annotation_count].type = strdup("function");
                split_args(pending_annotations[i].arg, annotations[annotation_count].args,
                          &annotations[annotation_count].arg_count);

                const char *base = get_extension_base(pending_annotations[i].name);
                if (strcmp(pending_annotations[i].name, "When") == 0 && pending_annotations[i].arg[0]) {
                    annotations[annotation_count].condition = strdup(pending_annotations[i].arg);
                } else if (base && strcmp(base, "When") == 0) {
                    if (strcmp(pending_annotations[i].name, "Disable") == 0) {
                        annotations[annotation_count].condition = strdup("__LUCY_TEST_DISABLE__");
                        annotations[annotation_count].isRemoved = 1;
                    } else {
                        annotations[annotation_count].condition = strdup("TARGET_TEST");
                        annotations[annotation_count].isRemoved = 0;
                    }
                } else {
                    annotations[annotation_count].condition = NULL;
                    annotations[annotation_count].isRemoved = 0;
                }
                annotation_count++;
            }

            fprintf(out, "%s\n", line);
            pending_count = 0;
        } else {
            fprintf(out, "%s\n", line);
            if (in_when_block) {
                int in_string = 0;
                char prev_char = '\0';
                for (const char *c = line; *c; c++) {
                    if (*c == '"') {
                        if (prev_char != '\\') in_string = !in_string;
                    }
                    if (!in_string) {
                        if (*c == '{') brace_count++;
                        else if (*c == '}') brace_count--;
                    }
                    prev_char = *c;
                }
                if (brace_count <= 0 && is_function_end(line)) {
                    fprintf(out, "#endif\n");
                    in_when_block = 0;
                    brace_count = 0;
                }
            }
        }
    }

    if (in_when_block) {
        fprintf(out, "#endif\n");
    }

    fclose(in);
    fclose(out);
    sync_annotations();
    return 0;
}

/* Generates annotations.h with function declarations */
int lucy_generate_annotations_header(const char *base_annotations_path, const char *output_path) {
    FILE *header_out = fopen(output_path, "w");
    if (!header_out) {
        perror("Error opening output annotations.h");
        return 1;
    }

    fprintf(header_out, "#ifndef ANNOTATIONS_H\n");
    fprintf(header_out, "#define ANNOTATIONS_H\n\n");
    fprintf(header_out, "#include \"lucy.h\"\n\n");
    fprintf(header_out, "// User-defined Annotation Extensions\n");

    FILE *base_in = fopen(base_annotations_path, "r");
    if (!base_in) {
        perror("Error opening base annotations.h");
        fclose(header_out);
        return 1;
    }
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), base_in)) {
        line[strcspn(line, "\n")] = 0;
        if (is_extension_def(line)) {
            fprintf(header_out, "%s\n", line);
        }
    }
    fclose(base_in);

    fprintf(header_out, "\n// Function Declarations\n");
    for (int i = 0; i < annotation_count; i++) {
        fprintf(header_out, "extern void %s(void);\n", annotations[i].target_name);
    }

    fprintf(header_out, "\n// Annotation Tracking Declarations\n");
    fprintf(header_out, "extern struct Annotation __ANNOTATIONS[];\n");
    fprintf(header_out, "extern int __ANNOTATION_COUNT;\n");
    fprintf(header_out, "extern struct Annotation *find_annotated_blocks(const char *name);\n");
    fprintf(header_out, "#endif // ANNOTATIONS_H\n");

    fclose(header_out);
    return 0;
}

/* Generates annotations.c with tracking data */
int lucy_generate_annotations_source(const char *output_path) {
    FILE *out = fopen(output_path, "w");
    if (!out) {
        perror("Error opening annotations.c");
        return 1;
    }

    fprintf(out, "#include \"annotations.h\"\n");
    fprintf(out, "#include <string.h>\n\n");
    fprintf(out, "// Generated Annotation Tracking\n");
    fprintf(out, "struct Annotation __ANNOTATIONS[%d] = {\n", annotation_count);
    for (int i = 0; i < annotation_count; i++) {
        if (annotations[i].condition) {
            fprintf(out, "#ifdef %s\n", annotations[i].condition);
            fprintf(out, "    {\"%s\", %s, \"%s\", %d, {", annotations[i].name,
                    annotations[i].target_name, annotations[i].type, annotations[i].isRemoved);
            for (int j = 0; j < MAX_ARGS; j++) {
                if (j < annotations[i].arg_count) {
                    fprintf(out, "\"%s\"", annotations[i].args[j]);
                } else {
                    fprintf(out, "NULL");
                }
                if (j < MAX_ARGS - 1) fprintf(out, ", ");
            }
            fprintf(out, "}, %d, \"%s\", \"%s\"}%s\n", annotations[i].arg_count,
                    annotations[i].condition,
                    annotations[i].target_name,
                    i < annotation_count - 1 ? "," : "");
            fprintf(out, "#else\n");
            fprintf(out, "    {\"%s\", NULL, \"%s\", 1, {", annotations[i].name,
                    annotations[i].type);
            for (int j = 0; j < MAX_ARGS; j++) {
                if (j < annotations[i].arg_count) {
                    fprintf(out, "\"%s\"", annotations[i].args[j]);
                } else {
                    fprintf(out, "NULL");
                }
                if (j < MAX_ARGS - 1) fprintf(out, ", ");
            }
            fprintf(out, "}, %d, \"%s\", \"%s\"}%s\n", annotations[i].arg_count,
                    annotations[i].condition,
                    annotations[i].target_name,
                    i < annotation_count - 1 ? "," : "");
            fprintf(out, "#endif\n");
        } else {
            fprintf(out, "    {\"%s\", %s, \"%s\", %d, {", annotations[i].name,
                    annotations[i].target_name, annotations[i].type, annotations[i].isRemoved);
            for (int j = 0; j < MAX_ARGS; j++) {
                if (j < annotations[i].arg_count) {
                    fprintf(out, "\"%s\"", annotations[i].args[j]);
                } else {
                    fprintf(out, "NULL");
                }
                if (j < MAX_ARGS - 1) fprintf(out, ", ");
            }
            fprintf(out, "}, %d, NULL, \"%s\"}%s\n", annotations[i].arg_count,
                    annotations[i].target_name,
                    i < annotation_count - 1 ? "," : "");
        }
    }
    fprintf(out, "};\n");
    fprintf(out, "int __ANNOTATION_COUNT = %d;\n", annotation_count);
    fclose(out);
    return 0;
}

/* Initializes library state */
void lucy_init(void) {
    annotation_count = 0;
    extension_count = 0;
    memset(annotations, 0, sizeof(annotations));
    memset(extensions, 0, sizeof(extensions));
    // No __ANNOTATIONS manipulation
}

/* Cleans up dynamically allocated memory */
void lucy_cleanup(void) {
    for (int i = 0; i < annotation_count; i++) {
        free((void *)annotations[i].name);
        // Don’t free target; it’s a function pointer from __ANNOTATIONS
        free((void *)annotations[i].type);
        free((void *)annotations[i].condition);
        free((void *)annotations[i].target_name);
        for (int j = 0; j < annotations[i].arg_count; j++) {
            free((void *)annotations[i].args[j]);
        }
    }
    annotation_count = 0;
    extension_count = 0;
    memset(annotations, 0, sizeof(annotations));
    memset(extensions, 0, sizeof(extensions));
    // No __ANNOTATIONS cleanup
}

/* Find annotated blocks by name (e.g., "Test") in the global __ANNOTATIONS array */
struct Annotation *find_annotated_blocks(const char *name) {
    struct Annotation *matches = calloc(MAX_ANNOTATIONS, sizeof(struct Annotation));
    if (!matches) {
        fprintf(stderr, "Memory allocation failed in find_annotated_blocks\n");
        return NULL;
    }
    int count = 0;
    for (int i = 0; i < __ANNOTATION_COUNT; i++) {
        if (strcmp(__ANNOTATIONS[i].name, name) == 0) {
            matches[count] = __ANNOTATIONS[i];
            count++;
            if (count >= MAX_ANNOTATIONS) break;
        }
    }
    return matches;
}