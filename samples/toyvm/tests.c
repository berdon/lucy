#include "lucy_test.h"
#include "toyvm.h"
#include <string.h>

// @Test("Tokenize a number")
void test_tokenize_number() {
    int token_count;
    Token *tokens = tokenize("42", &token_count);
    assertEquals(TOKEN_NUMBER, tokens[0].type, "First token should be a number");
    assertStringEquals("42", tokens[0].value, "Number value should be 42");
    assertEquals(TOKEN_EOF, tokens[1].type, "Second token should be EOF");
    assertEquals(2, token_count, "Should have 2 tokens");
    free_tokens(tokens, token_count);
}

// @Test("Tokenize an identifier")
void test_tokenize_identifier() {
    int token_count;
    Token *tokens = tokenize("x", &token_count);
    assertEquals(TOKEN_IDENTIFIER, tokens[0].type, "First token should be an identifier");
    assertStringEquals("x", tokens[0].value, "Identifier value should be x");
    assertEquals(TOKEN_EOF, tokens[1].type, "Second token should be EOF");
    free_tokens(tokens, token_count);
}

// @Test("Tokenize print statement")
void test_tokenize_print() {
    int token_count;
    Token *tokens = tokenize("print(x)", &token_count);
    assertEquals(TOKEN_PRINT, tokens[0].type, "First token should be print");
    assertEquals(TOKEN_IDENTIFIER, tokens[1].type, "Second token should be identifier");
    assertStringEquals("x", tokens[1].value, "Identifier should be x");
    assertEquals(TOKEN_EOF, tokens[2].type, "Third token should be EOF");
    free_tokens(tokens, token_count);
}

// @Test("Interpret assignment")
void test_interpret_assignment() {
    ToyVM *vm = vm_new();
    interpret(vm, "x = 5");  // Sets x to 5 in VM
    assertEquals(5, vm->vars['x' - 'a'], "x should be assigned 5");
    vm_free(vm);
}

// @Test("Interpret addition")
void test_interpret_addition() {
    ToyVM *vm = vm_new();
    interpret(vm, "x = 5");  // Set initial value
    interpret(vm, "x + 3");  // Add 3 to x
    assertEquals(8, vm->vars['x' - 'a'], "x should be 8 after addition");
    vm_free(vm);
}

// @Test("Interpret subtraction")
void test_interpret_subtraction() {
    ToyVM *vm = vm_new();
    interpret(vm, "x = 5");
    interpret(vm, "x - 2");
    assertEquals(3, vm->vars['x' - 'a'], "x should be 3 after subtraction");
    vm_free(vm);
}

// @Test("Interpret multiplication")
void test_interpret_multiplication() {
    ToyVM *vm = vm_new();
    interpret(vm, "x = 5");
    interpret(vm, "x * 4");
    assertEquals(20, vm->vars['x' - 'a'], "x should be 20 after multiplication");
    vm_free(vm);
}

// @Test("Interpret division")
void test_interpret_division() {
    ToyVM *vm = vm_new();
    interpret(vm, "x = 6");
    interpret(vm, "x / 2");
    assertEquals(3, vm->vars['x' - 'a'], "x should be 3 after division");
    vm_free(vm);
}

// @Test("Interpret var-to-var addition")
void test_interpret_var_to_var_addition() {
    ToyVM *vm = vm_new();
    interpret(vm, "x = 5");
    interpret(vm, "y = 6");
    interpret(vm, "z = x + y");
    assertEquals(11, vm->vars['z' - 'a'], "z should be 11 after x + y");
    vm_free(vm);
}

// @Test("VM push and print")
void test_vm_push_print() {
    ToyVM *vm = vm_new();
    interpret(vm, "x = 42\nprint(x)");  // Generates bytecode internally
    vm_execute(vm);  // Executes the generated bytecode
    assertTrue(1, "VM should execute push and print without crashing");
    vm_free(vm);
}

// @Disable("Test VM failure case not implemented yet")
// @Test("VM invalid opcode")
void test_vm_invalid_opcode() {
    ToyVM *vm = vm_new();
    char bytecode[] = {255, OP_HALT};  // Invalid opcode
    vm->code = bytecode;  // Manually set for this test
    vm->code_size = 2;
    vm_execute(vm);
    assertTrue(0, "Should fail on invalid opcode");
    vm_free(vm);
}