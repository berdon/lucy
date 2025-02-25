#ifndef PARSING_H
#define PARSING_H

/* Checks if a line starts with an annotation comment (e.g., "// @")
 * - line: Input line to check
 * Returns: 1 if it’s an annotation, 0 otherwise
 */
int is_annotation(const char *line);

/* Checks if a line defines an annotation extension (e.g., "// #annotation")
 * - line: Input line to check
 * Returns: 1 if it’s an extension definition, 0 otherwise
 */
int is_extension_def(const char *line);

/* Extracts the name and arguments from an annotation comment
 * - line: Input line (e.g., "// @Test(TARGET_TEST, \"desc\")")
 * - name: Output buffer for annotation name (e.g., "Test")
 * - arg: Output buffer for arguments (e.g., "TARGET_TEST, \"desc\"")
 * Note: Assumes name and arg buffers are at least MAX_BUFFER_SIZE chars
 */
void extract_annotation_name(const char *line, char *name, char *arg);

/* Extracts components from an extension definition (e.g., "// #annotation @Test(condition, desc) : @When(cond)")
 * - line: Input line with extension definition
 * - name: Output buffer for extension name (e.g., "Test")
 * - args: Output buffer for extension args (e.g., "condition, desc")
 * - base: Output buffer for base annotation (e.g., "When")
 * - base_arg: Output buffer for base args (e.g., "cond")
 * Note: Assumes buffers are at least MAX_BUFFER_SIZE chars
 */
void extract_extension(const char *line, char *name, char *args, char *base, char *base_arg);

/* Checks if a line is a function definition (contains '(', ')', and '{')
 * - line: Input line to check
 * Returns: 1 if it’s a function definition, 0 otherwise
 */
int is_function_definition(const char *line);

/* Extracts the function name from a definition line (e.g., "void func() {")
 * - line: Input line with function definition
 * - name: Output buffer for function name
 * Note: Assumes name buffer is at least MAX_BUFFER_SIZE chars
 */
void extract_function_name(const char *line, char *name);

/* Checks if a line ends a function block (contains '}')
 * - line: Input line to check
 * Returns: 1 if it’s a function end, 0 otherwise
 */
int is_function_end(const char *line);

/* Splits a comma-separated argument string into an array
 * - arg_str: Input string (e.g., "TARGET_TEST, \"desc\"")
 * - args: Output array of pointers to argument strings
 * - arg_count: Output number of arguments parsed
 * Note: Allocates memory for each arg; caller must free
 */
void split_args(const char *arg_str, const char **args, int *arg_count);

#endif // PARSING_H