#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "toyvm.h"

// Global debug flag
static int debug = 0;

// ANSI color codes
#define COLOR_GREY "\033[90m"  // Bright black (grey-ish)
#define COLOR_RESET "\033[0m"

// Debugging helper to print tokens
static const char *token_type_str(TokenType type) {
    switch (type) {
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_EQUALS: return "EQUALS";
        case TOKEN_PLUS: return "PLUS";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_MULTIPLY: return "MULTIPLY";
        case TOKEN_DIVIDE: return "DIVIDE";
        case TOKEN_PRINT: return "PRINT";
        case TOKEN_EOF: return "EOF";
        case TOKEN_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

/* Tokenizer implementation */
Token *tokenize(const char *source, int *token_count) {
    Token *tokens = malloc(sizeof(Token) * 256);  // Max 256 tokens for simplicity
    int count = 0;
    const char *p = source;

    if (debug) printf("Tokenizing: '%s'\n", source);
    while (*p && count < 256) {
        while (*p == ' ' || *p == '\n' || *p == '\t') p++;  // Skip whitespace
        if (*p == '\0') break;

        tokens[count].value = NULL;
        if (*p >= '0' && *p <= '9') {
            const char *start = p;
            while (*p >= '0' && *p <= '9') p++;
            int len = p - start;
            tokens[count].type = TOKEN_NUMBER;
            tokens[count].value = strndup(start, len);
            tokens[count].length = len;
        } else if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z')) {
            const char *start = p;
            while ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z')) p++;
            int len = p - start;
            if (len == 5 && strncmp(start, "print", 5) == 0) {
                tokens[count].type = TOKEN_PRINT;
            } else {
                tokens[count].type = TOKEN_IDENTIFIER;
            }
            tokens[count].value = strndup(start, len);
            tokens[count].length = len;
            // Skip optional parentheses
            if (*p == '(' || *p == ')') p++;
        } else if (*p == '=') {
            tokens[count].type = TOKEN_EQUALS;
            tokens[count].value = strndup(p, 1);
            tokens[count].length = 1;
            p++;
        } else if (*p == '+') {
            tokens[count].type = TOKEN_PLUS;
            tokens[count].value = strndup(p, 1);
            tokens[count].length = 1;
            p++;
        } else if (*p == '-') {
            tokens[count].type = TOKEN_MINUS;
            tokens[count].value = strndup(p, 1);
            tokens[count].length = 1;
            p++;
        } else if (*p == '*') {
            tokens[count].type = TOKEN_MULTIPLY;
            tokens[count].value = strndup(p, 1);
            tokens[count].length = 1;
            p++;
        } else if (*p == '/') {
            tokens[count].type = TOKEN_DIVIDE;
            tokens[count].value = strndup(p, 1);
            tokens[count].length = 1;
            p++;
        } else if (*p == '#') {
            while (*p && *p != '\n') p++;  // Skip comments
            continue;
        } else {
            p++;  // Skip unrecognized characters
            continue;
        }
        if (debug) printf("Token %d: type=%s, value='%s'\n", count, token_type_str(tokens[count].type), tokens[count].value ? tokens[count].value : "");
        count++;
    }

    tokens[count].type = TOKEN_EOF;
    tokens[count].value = NULL;
    tokens[count].length = 0;
    if (debug) printf("Token %d: type=TOKEN_EOF\n", count);
    count++;

    *token_count = count;
    return tokens;
}

void free_tokens(Token *tokens, int token_count) {
    for (int i = 0; i < token_count; i++) {
        if (tokens[i].value) {
            free(tokens[i].value);
        }
    }
    free(tokens);
}

/* Interpreter implementation */
void interpret(ToyVM *vm, const char *source) {
    int token_count;
    Token *tokens = tokenize(source, &token_count);
    int i = 0;

    char *bytecode = malloc(256);  // Dynamically allocate bytecode
    int bytecode_pos = 0;

    if (debug) printf("Interpreting %d tokens:\n", token_count);
    while (i < token_count && tokens[i].type != TOKEN_EOF) {
        if (tokens[i].type == TOKEN_IDENTIFIER) {
            if (i + 1 < token_count && tokens[i + 1].type == TOKEN_EQUALS) {
                if (i + 2 < token_count && tokens[i + 2].type == TOKEN_NUMBER) {
                    // Case: x = 5
                    int value = atoi(tokens[i + 2].value);
                    char var = tokens[i].value[0];  // Assume single-letter variable
                    bytecode[bytecode_pos++] = OP_PUSH;
                    bytecode[bytecode_pos++] = value;
                    bytecode[bytecode_pos++] = OP_STORE;
                    bytecode[bytecode_pos++] = var - 'a';  // Index for a-z
                    if (debug) printf("Generated: PUSH %d, STORE %c\n", value, var);
                    i += 3;
                } else if (i + 4 < token_count && tokens[i + 2].type == TOKEN_IDENTIFIER &&
                           (tokens[i + 3].type == TOKEN_PLUS || tokens[i + 3].type == TOKEN_MINUS ||
                            tokens[i + 3].type == TOKEN_MULTIPLY || tokens[i + 3].type == TOKEN_DIVIDE) &&
                           tokens[i + 4].type == TOKEN_IDENTIFIER) {
                    // Case: z = x + y
                    char dest_var = tokens[i].value[0];
                    char var1 = tokens[i + 2].value[0];
                    char var2 = tokens[i + 4].value[0];
                    bytecode[bytecode_pos++] = OP_LOAD;
                    bytecode[bytecode_pos++] = var1 - 'a';
                    bytecode[bytecode_pos++] = OP_LOAD;
                    bytecode[bytecode_pos++] = var2 - 'a';
                    switch (tokens[i + 3].type) {
                        case TOKEN_PLUS:
                            bytecode[bytecode_pos++] = OP_ADD;
                            if (debug) printf("Generated: LOAD %c, LOAD %c, ADD, STORE %c\n", var1, var2, dest_var);
                            break;
                        case TOKEN_MINUS:
                            bytecode[bytecode_pos++] = OP_SUB;
                            if (debug) printf("Generated: LOAD %c, LOAD %c, SUB, STORE %c\n", var1, var2, dest_var);
                            break;
                        case TOKEN_MULTIPLY:
                            bytecode[bytecode_pos++] = OP_MUL;
                            if (debug) printf("Generated: LOAD %c, LOAD %c, MUL, STORE %c\n", var1, var2, dest_var);
                            break;
                        case TOKEN_DIVIDE:
                            bytecode[bytecode_pos++] = OP_DIV;
                            if (debug) printf("Generated: LOAD %c, LOAD %c, DIV, STORE %c\n", var1, var2, dest_var);
                            break;
                        default:
                            i += 5;
                            continue;
                    }
                    bytecode[bytecode_pos++] = OP_STORE;
                    bytecode[bytecode_pos++] = dest_var - 'a';
                    i += 5;
                } else {
                    i += 2;  // Skip past equals if no valid expression follows
                }
            } else if (i + 1 < token_count && i + 2 < token_count && tokens[i + 2].type == TOKEN_NUMBER) {
                // Case: x + 3
                char var = tokens[i].value[0];
                int value = atoi(tokens[i + 2].value);
                bytecode[bytecode_pos++] = OP_LOAD;
                bytecode[bytecode_pos++] = var - 'a';
                bytecode[bytecode_pos++] = OP_PUSH;
                bytecode[bytecode_pos++] = value;
                switch (tokens[i + 1].type) {
                    case TOKEN_PLUS:
                        bytecode[bytecode_pos++] = OP_ADD;
                        if (debug) printf("Generated: LOAD %c, PUSH %d, ADD, STORE %c\n", var, value, var);
                        break;
                    case TOKEN_MINUS:
                        bytecode[bytecode_pos++] = OP_SUB;
                        if (debug) printf("Generated: LOAD %c, PUSH %d, SUB, STORE %c\n", var, value, var);
                        break;
                    case TOKEN_MULTIPLY:
                        bytecode[bytecode_pos++] = OP_MUL;
                        if (debug) printf("Generated: LOAD %c, PUSH %d, MUL, STORE %c\n", var, value, var);
                        break;
                    case TOKEN_DIVIDE:
                        bytecode[bytecode_pos++] = OP_DIV;
                        if (debug) printf("Generated: LOAD %c, PUSH %d, DIV, STORE %c\n", var, value, var);
                        break;
                    default:
                        i += 3;
                        continue;
                }
                bytecode[bytecode_pos++] = OP_STORE;
                bytecode[bytecode_pos++] = var - 'a';
                i += 3;
            } else {
                i++;
            }
        } else if (tokens[i].type == TOKEN_PRINT && i + 1 < token_count &&
                   tokens[i + 1].type == TOKEN_IDENTIFIER) {
            char var = tokens[i + 1].value[0];
            bytecode[bytecode_pos++] = OP_LOAD;
            bytecode[bytecode_pos++] = var - 'a';
            bytecode[bytecode_pos++] = OP_PRINT;
            if (debug) printf("Generated: LOAD %c, PRINT\n", var);
            i += 2;
        } else {
            i++;
        }
    }

    bytecode[bytecode_pos++] = OP_HALT;
    if (debug) printf("Generated: HALT\n");
    if (vm->code) free(vm->code);  // Free previous bytecode if exists
    vm->code = bytecode;
    vm->code_size = bytecode_pos;
    vm_execute(vm);
}

/* VM implementation */
ToyVM *vm_new() {
    ToyVM *vm = malloc(sizeof(ToyVM));
    vm->sp = 0;
    vm->ip = 0;
    vm->code = NULL;
    vm->code_size = 0;
    memset(vm->vars, 0, sizeof(vm->vars));  // Initialize variables to 0
    return vm;
}

void vm_free(ToyVM *vm) {
    if (vm->code) free(vm->code);
    free(vm);
}

void vm_execute(ToyVM *vm) {
    vm->ip = 0;
    if (debug) printf("Executing %d bytes of bytecode:\n", vm->code_size);
    while (vm->ip < vm->code_size) {
        Opcode op = (Opcode)vm->code[vm->ip++];
        switch (op) {
            case OP_PUSH:
                vm->stack[vm->sp++] = (unsigned char)vm->code[vm->ip++];
                if (debug) printf("PUSH %d (sp=%d)\n", vm->stack[vm->sp - 1], vm->sp);
                break;
            case OP_STORE: {
                unsigned char var_idx = (unsigned char)vm->code[vm->ip++];
                vm->vars[var_idx] = vm->stack[--vm->sp];
                if (debug) printf("STORE %c = %d (sp=%d)\n", 'a' + var_idx, vm->vars[var_idx], vm->sp);
                break;
            }
            case OP_LOAD: {
                unsigned char var_idx = (unsigned char)vm->code[vm->ip++];
                vm->stack[vm->sp++] = vm->vars[var_idx];
                if (debug) printf("LOAD %c = %d (sp=%d)\n", 'a' + var_idx, vm->stack[vm->sp - 1], vm->sp);
                break;
            }
            case OP_ADD:
                vm->stack[vm->sp - 2] = vm->stack[vm->sp - 2] + vm->stack[vm->sp - 1];
                vm->sp--;
                if (debug) printf("ADD %d + %d = %d (sp=%d)\n", vm->stack[vm->sp], vm->stack[vm->sp + 1], vm->stack[vm->sp], vm->sp);
                break;
            case OP_SUB:
                vm->stack[vm->sp - 2] = vm->stack[vm->sp - 2] - vm->stack[vm->sp - 1];
                vm->sp--;
                if (debug) printf("SUB %d - %d = %d (sp=%d)\n", vm->stack[vm->sp], vm->stack[vm->sp + 1], vm->stack[vm->sp], vm->sp);
                break;
            case OP_MUL:
                vm->stack[vm->sp - 2] = vm->stack[vm->sp - 2] * vm->stack[vm->sp - 1];
                vm->sp--;
                if (debug) printf("MUL %d * %d = %d (sp=%d)\n", vm->stack[vm->sp], vm->stack[vm->sp + 1], vm->stack[vm->sp], vm->sp);
                break;
            case OP_DIV:
                if (vm->stack[vm->sp - 1] == 0) {
                    fprintf(stderr, "Error: Division by zero\n");
                    return;
                }
                vm->stack[vm->sp - 2] = vm->stack[vm->sp - 2] / vm->stack[vm->sp - 1];
                vm->sp--;
                if (debug) printf("DIV %d / %d = %d (sp=%d)\n", vm->stack[vm->sp], vm->stack[vm->sp + 1], vm->stack[vm->sp], vm->sp);
                break;
            case OP_PRINT:
                printf(COLOR_GREY "%d" COLOR_RESET "\n", vm->stack[--vm->sp]);
                if (debug) printf("PRINT %d (sp=%d)\n", vm->stack[vm->sp], vm->sp);
                break;
            case OP_HALT:
                if (debug) printf("HALT\n");
                return;
        }
    }
}

/* Main entry point for the toyvm executable */
#ifndef TARGET_TEST
int main(int argc, char *argv[]) {
    // Parse command-line arguments for --debug or -d
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "-d") == 0) {
            debug = 1;
            break;
        }
    }

    // Handle file input or interactive shell
    ToyVM *vm = vm_new();
    if (argc > 1 && !debug) {  // If debug is set, assume interactive mode unless file explicitly provided
        /* Read from a file if provided */
        FILE *file = fopen(argv[1], "r");
        if (!file) {
            perror("Error opening file");
            vm_free(vm);
            return 1;
        }
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fseek(file, 0, SEEK_SET);
        char *source = malloc(size + 1);
        fread(source, 1, size, file);
        source[size] = '\0';
        fclose(file);

        interpret(vm, source);
        free(source);
    } else {
        /* Read from stdin */
        printf("ToyVM Interactive Shell (Ctrl+D or Ctrl+C to exit)\n");
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), stdin)) {
            interpret(vm, buffer);
        }
    }
    vm_free(vm);
    return 0;
}
#endif /* TARGET_TEST */