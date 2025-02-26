#ifndef TOYVM_H
#define TOYVM_H

/* Token types for the tokenizer */
typedef enum {
    TOKEN_NUMBER,
    TOKEN_IDENTIFIER,
    TOKEN_EQUALS,
    TOKEN_PLUS,
    TOKEN_MINUS,  // New
    TOKEN_MULTIPLY,  // New
    TOKEN_DIVIDE,  // New
    TOKEN_PRINT,
    TOKEN_EOF,
    TOKEN_ERROR
} TokenType;

/* Structure for a token */
typedef struct {
    TokenType type;
    char *value;  // Dynamically allocated string
    int length;
} Token;

/* VM instruction opcodes */
typedef enum {
    OP_PUSH,
    OP_STORE,  // Store value in variable
    OP_LOAD,   // Load value from variable
    OP_ADD,
    OP_SUB,    // New: Subtract
    OP_MUL,    // New: Multiply
    OP_DIV,    // New: Divide
    OP_PRINT,
    OP_HALT
} Opcode;

/* Structure for the VM */
typedef struct {
    int stack[256];        // Simple stack for values
    int sp;                // Stack pointer
    int ip;                // Instruction pointer
    char *code;            // Bytecode
    int code_size;
    int vars[26];          // Simple variable storage (a-z)
} ToyVM;

/* Tokenizer functions */
Token *tokenize(const char *source, int *token_count);
void free_tokens(Token *tokens, int token_count);

/* Interpreter functions */
void interpret(ToyVM *vm, const char *source);

/* VM functions */
ToyVM *vm_new();
void vm_free(ToyVM *vm);
void vm_execute(ToyVM *vm);

#endif // TOYVM_H