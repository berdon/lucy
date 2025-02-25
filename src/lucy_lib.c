/* lucy_lib.c - Core functionality for the Lucy annotation processor library
 *
 * This file manages the high-level processing of annotated C code, relying on
 * parsing.c for token matching and string extraction. It’s used by both the
 * standalone Lucy CLI tool and the test runner to handle annotation processing,
 * extension resolution, conditional compilation, and metadata generation.
 *
 * Key Components:
 * - File Processing: Transforms input .c files with #ifdef directives.
 * - Annotation Tracking: Generates header/source files with metadata.
 * - State Management: Initializes and cleans up internal state.
 *
 * Dependencies:
 * - lucy_api.h: Public API (process_file, generate_annotations_*, init, cleanup) and defines.
 * - lucy.h: Internal structs (Annotation, Extension) and test-exposed functions.
 * - parsing.h: Parsing utilities (is_annotation, extract_annotation_name, etc.).
 * - Standard C libraries: stdio.h, stdlib.h, string.h for file I/O and string ops.
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include "../include/lucy_api.h"
 #include "../include/parsing.h"
 
 /* Static global state for annotation tracking */
 static struct Annotation annotations[MAX_ANNOTATIONS];
 int annotation_count = 0;
 
 /* Non-static global state for extensions (exposed via lucy.h for testing) */
 Extension extensions[MAX_ANNOTATIONS];
 int extension_count = 0;
 
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
     char pending_annotation[MAX_BUFFER_SIZE] = {0};
     char pending_arg[MAX_BUFFER_SIZE] = {0};
     int in_when_block = 0;
 
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
             extract_annotation_name(line, pending_annotation, pending_arg);
             continue;
         }
 
         if (pending_annotation[0] && is_function_definition(line)) {
             char func_name[MAX_BUFFER_SIZE];
             extract_function_name(line, func_name);
 
             if (annotation_count < MAX_ANNOTATIONS) {
                 annotations[annotation_count].name = strdup(pending_annotation);
                 annotations[annotation_count].target = NULL;
                 annotations[annotation_count].target_name = strdup(func_name);
                 annotations[annotation_count].type = strdup("function");
                 split_args(pending_arg, annotations[annotation_count].args,
                           &annotations[annotation_count].arg_count);
 
                 const char *base = get_extension_base(pending_annotation);
                 if ((strcmp(pending_annotation, "When") == 0 && pending_arg[0]) ||
                     (base && strcmp(base, "When") == 0 && annotations[annotation_count].arg_count > 0)) {
                     annotations[annotation_count].condition = strdup(annotations[annotation_count].args[0]);
                     fprintf(out, "#ifdef %s\n", annotations[annotation_count].condition);
                     in_when_block = 1;
                 } else {
                     annotations[annotation_count].condition = NULL;
                 }
                 annotations[annotation_count].isRemoved = 0;
 
                 annotation_count++;
             }
             fprintf(out, "%s\n", line);
 
             pending_annotation[0] = 0;
             pending_arg[0] = 0;
         } else {
             fprintf(out, "%s\n", line);
             if (in_when_block && is_function_end(line)) {
                 fprintf(out, "#endif\n");
                 in_when_block = 0;
             }
         }
     }
 
     fclose(in);
     fclose(out);
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
             fprintf(out, "    {\"%s\", %s, \"%s\", 0, {", annotations[i].name,
                     annotations[i].target_name, annotations[i].type);
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
             fprintf(out, "    {\"%s\", %s, \"%s\", 0, {", annotations[i].name,
                     annotations[i].target_name, annotations[i].type);
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
     fprintf(out, "struct Annotation *find_annotated_blocks(const char *name) {\n");
     fprintf(out, "    static struct Annotation matches[%d];\n", MAX_ANNOTATIONS);
     fprintf(out, "    int count = 0;\n");
     fprintf(out, "    for (int i = 0; i < __ANNOTATION_COUNT; i++) {\n");
     fprintf(out, "        if (strcmp(__ANNOTATIONS[i].name, name) == 0) {\n");
     fprintf(out, "            matches[count++] = __ANNOTATIONS[i];\n");
     fprintf(out, "        }\n");
     fprintf(out, "    }\n");
     fprintf(out, "    return matches;\n");
     fprintf(out, "}\n");
 
     fclose(out);
     return 0;
 }
 
 /* Initializes library state */
 void lucy_init(void) {
     annotation_count = 0;
     extension_count = 0;
     memset(annotations, 0, sizeof(annotations));
     memset(extensions, 0, sizeof(extensions));
 }
 
 /* Cleans up dynamically allocated memory */
 void lucy_cleanup(void) {
     for (int i = 0; i < annotation_count; i++) {
         free((void *)annotations[i].name);
         free((void *)annotations[i].target);
         free((void *)annotations[i].type);
         free((void *)annotations[i].condition);
         free((void *)annotations[i].target_name);
         for (int j = 0; j < annotations[i].arg_count; j++) {
             free((void *)annotations[i].args[j]);
         }
     }
 }