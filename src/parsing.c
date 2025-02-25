/* parsing.c - Token matching and parsing utilities for Lucy annotation processor
 *
 * This file encapsulates the low-level parsing logic for identifying and
 * extracting tokens from C source code comments and function definitions.
 * It’s used by lucy_lib.c to process annotations and extensions.
 *
 * Dependencies:
 * - parsing.h: Declarations for parsing functions.
 * - lucy_api.h: Defines MAX_LINE_LENGTH, MAX_ARGS, MAX_BUFFER_SIZE.
 * - Standard C libraries: stdio.h, stdlib.h, string.h for string ops.
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include "../include/parsing.h"
 #include "../include/lucy_api.h"
 
 int is_annotation(const char *line) {
     return strncmp(line, "// @", 4) == 0;
 }
 
 int is_extension_def(const char *line) {
     return strncmp(line, "// #annotation ", 15) == 0;
 }
 
 void extract_annotation_name(const char *line, char *name, char *arg) {
     const char *at = strchr(line, '@');  // Find the '@' explicitly
     if (!at) {
         name[0] = 0;  // No '@' found, invalid annotation
         arg[0] = 0;
         return;
     }
 
     const char *start = at + 1;  // Skip '@' to get just the annotation name
     const char *paren = strchr(start, '(');
     const char *space = strchr(start, ' ');
     const char *end;
 
     // Determine end: either '(', space, or end of line
     if (paren && (!space || paren < space)) {
         end = paren;
     } else if (space) {
         end = space;
     } else {
         end = start + strlen(start);
     }
 
     int name_len = end - start;
     if (name_len >= MAX_BUFFER_SIZE) name_len = MAX_BUFFER_SIZE - 1;
     strncpy(name, start, name_len);
     name[name_len] = 0;
 
     if (paren) {
         const char *arg_start = paren + 1;
         const char *arg_end = strchr(paren, ')');
         if (arg_end) {
             int arg_len = arg_end - arg_start;
             if (arg_len >= MAX_BUFFER_SIZE) arg_len = MAX_BUFFER_SIZE - 1;
             strncpy(arg, arg_start, arg_len);
             arg[arg_len] = 0;
         } else {
             int arg_len = strlen(arg_start);
             if (arg_len >= MAX_BUFFER_SIZE) arg_len = MAX_BUFFER_SIZE - 1;
             strncpy(arg, arg_start, arg_len);
             arg[arg_len] = 0;
         }
     } else {
         arg[0] = 0;
     }
 }
 
 void extract_extension(const char *line, char *name, char *args, char *base, char *base_arg) {
     const char *start = line + 15;
     const char *at = strchr(start, '@');
     if (!at) return;
 
     const char *paren = strchr(at, '(');
     if (!paren) return;
 
     int name_len = paren - at - 1;
     if (name_len >= MAX_BUFFER_SIZE) name_len = MAX_BUFFER_SIZE - 1;
     strncpy(name, at + 1, name_len);
     name[name_len] = 0;
 
     const char *arg_start = paren + 1;
     const char *arg_end = strchr(paren, ')');
     if (!arg_end) return;
     int arg_len = arg_end - arg_start;
     if (arg_len >= MAX_BUFFER_SIZE) arg_len = MAX_BUFFER_SIZE - 1;
     strncpy(args, arg_start, arg_len);
     args[arg_len] = 0;
 
     const char *colon = strstr(line, " : @");
     if (!colon) return;
     const char *base_start = colon + 4;
     const char *base_paren = strchr(base_start, '(');
     if (!base_paren) return;
 
     int base_len = base_paren - base_start;
     if (base_len >= MAX_BUFFER_SIZE) base_len = MAX_BUFFER_SIZE - 1;
     strncpy(base, base_start, base_len);
     base[base_len] = 0;
 
     const char *base_arg_start = base_paren + 1;
     const char *base_arg_end = strchr(base_paren, ')');
     if (!base_arg_end) return;
     int base_arg_len = base_arg_end - base_arg_start;
     if (base_arg_len >= MAX_BUFFER_SIZE) base_arg_len = MAX_BUFFER_SIZE - 1;
     strncpy(base_arg, base_arg_start, base_arg_len);
     base_arg[base_arg_len] = 0;
 }
 
 int is_function_definition(const char *line) {
     return strchr(line, '(') && strchr(line, ')') && strchr(line, '{');
 }
 
 void extract_function_name(const char *line, char *name) {
     const char *start = strchr(line, ' ') + 1;
     const char *end = strchr(line, '(');
     int len = end - start;
     if (len >= MAX_BUFFER_SIZE) len = MAX_BUFFER_SIZE - 1;
     strncpy(name, start, len);
     name[len] = 0;
 }
 
 int is_function_end(const char *line) {
     return strchr(line, '}') != NULL;
 }
 
 void split_args(const char *arg_str, const char **args, int *arg_count) {
     char temp[MAX_BUFFER_SIZE];
     strncpy(temp, arg_str, MAX_BUFFER_SIZE - 1);
     temp[MAX_BUFFER_SIZE - 1] = 0;
     *arg_count = 0;
 
     char *token = strtok(temp, ",");
     while (token && *arg_count < MAX_ARGS) {
         while (*token == ' ') token++;
         int len = strlen(token);
         while (len > 0 && token[len - 1] == ' ') len--;
         if (len > 1 && token[0] == '"' && token[len - 1] == '"') {
             token++;
             len -= 2;
         }
         if (len >= MAX_BUFFER_SIZE) len = MAX_BUFFER_SIZE - 1;
 
         char *arg = malloc(len + 1);
         if (!arg) {
             perror("Memory allocation failed in split_args");
             return;
         }
         strncpy(arg, token, len);
         arg[len] = 0;
         args[*arg_count] = arg;
         (*arg_count)++;
         token = strtok(NULL, ",");
     }
 }