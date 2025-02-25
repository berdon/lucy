#ifndef LUCY_H
#define LUCY_H

/* Annotation structure used by lucy for tracking annotated functions */
struct Annotation {
    const char *name;      // Annotation name (e.g., "Test")
    void *target;          // Function pointer or NULL if removed
    const char *type;      // Type of annotated block (e.g., "function")
    int isRemoved;         // 1 if block is conditionally removed, 0 if included
    const char *args[8];   // Array of argument strings (max 8)
    int arg_count;         // Number of arguments
    const char *condition; // Condition for @When or derived annotations (e.g., "TARGET_TEST")
    const char *target_name; // Name of the target function (e.g., "test_string_equality")
};

/* External declarations for annotation tracking */
extern struct Annotation __ANNOTATIONS[];
extern int __ANNOTATION_COUNT;
extern struct Annotation *find_annotated_blocks(const char *name);

/* Internal functions exposed for testing */
void extract_annotation_name(const char *line, char *name, char *arg);
void extract_extension(const char *line, char *name, char *args, char *base, char *base_arg);
const char *get_extension_base(const char *name);
void load_extensions(const char *base_annotations_path);

/* Internal state exposed for testing */
#define MAX_ANNOTATIONS 1000
typedef struct {
    char name[64];
    char base[64];
    char base_arg[64];
} Extension;
extern Extension extensions[MAX_ANNOTATIONS];
extern int extension_count;
extern int annotation_count;  // Added for test access

#endif // LUCY_H