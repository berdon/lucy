#ifndef LUCY_API_H
#define LUCY_API_H

#include "lucy.h"  // For struct Annotation definition

/* Maximum length of a line in source files; limits buffer size for parsing */
#define MAX_LINE_LENGTH 1024
/* Maximum number of arguments an annotation can have; matches struct Annotation */
#define MAX_ARGS 8
/* Maximum size of name and arg buffers; matches MAX_LINE_LENGTH for safety */
#define MAX_BUFFER_SIZE 1024
/* Maximum number of annotations that can be tracked */
#define MAX_ANNOTATIONS 1000

/* Process a single input file and write to an output file */
int lucy_process_file(const char *input_path, const char *output_path);

/* Generate the annotations header from a base file and collected annotations */
int lucy_generate_annotations_header(const char *base_annotations_path, const char *output_path);

/* Generate the annotations source file with tracking data */
int lucy_generate_annotations_source(const char *output_path);

/* Initialize the internal annotation state */
void lucy_init(void);

/* Free internal annotation memory */
void lucy_cleanup(void);

/* Find annotated blocks by name (e.g., "Test") in the global __ANNOTATIONS array */
struct Annotation *find_annotated_blocks(const char *name);

#endif // LUCY_API_H