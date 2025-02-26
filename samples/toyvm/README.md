# ToyVM: A Minimal Toy Interpreter Language

## Overview

ToyVM is a simple, educational toy interpreter for a minimal Python-like language called "Toy." It’s designed as a sample project to demonstrate the use of the lucy-test unit testing framework within the lucy ecosystem. ToyVM includes a tokenizer, a single-pass interpreter, and a basic virtual machine (VM) to execute simple programs, supporting variable assignments, basic arithmetic, and output.

### The Toy Language
Toy is a minimal, Python-esque language with the following features:
- **Variables**: `x = 5` (assigns a value to a single-letter variable `a-z`).
- **Arithmetic**: Supports `+`, `-`, `*`, `/` with variables or numbers:
  - `x + 3` (updates `x` with the result).
  - `z = x + y` (computes and stores the result in `z`).
- **Print**: `print(x)` (displays the value of `x` in grey text).
- **Comments**: `# This is a comment` (ignored by the interpreter).

Example program:

```toy
x = 5
y = 6
z = x + y
print(z)  # Outputs 11 in grey
x - 2
print(x)  # Outputs 3 in grey
```

## Development and Building

### Prerequisites
- **GCC**: C compiler (or compatible, e.g., Clang on macOS).
- **lucy**: The lucy annotation processor binary and `liblucy-test.so` must be built in the parent `lucy/` directory:

```bash
cd ../../
make
```

- **macOS Note**: See "Known Issues" for library loading workaround.

### Directory Structure
- `toyvm.c`: Core implementation (tokenizer, interpreter, VM).
- `toyvm.h`: Header file with types and function declarations.
- `tests.c`: Unit tests using lucy-test.
- `Makefile`: Build script for the ToyVM binary and tests.
- `README.md`: This documentation.

### Building
From `/samples/toyvm/`:

```bash
make clean && make
```

- Builds `toyvm` (standalone interpreter) and `test_runner` (unit test binary).

### Running ToyVM
- **Interactive Mode**:

```toy
./toyvm
ToyVM Interactive Shell (Ctrl+D or Ctrl+C to exit)
x = 5
y = 6
z = x + y
print(z)  # Outputs 11 in grey
^D
```
- **With Debug Logging**:

```toy
./toyvm --debug  # or ./toyvm -d
```

  - Displays detailed tokenization, bytecode generation, and VM execution logs.
- **File Mode**:

Create `test.toy`:

```toy
x = 5
x * 4
print(x)
```

Run:

```toy
./toyvm test.toy
20  # Grey
```

## Unit Tests

ToyVM uses lucy-test for unit testing, located in `tests.c`. The test suite covers:
- **Tokenizer**: Parsing numbers, identifiers, operators, and `print` statements.
- **Interpreter**: Variable assignment and arithmetic operations (`+`, `-`, `*`, `/`).
- **VM**: Execution of bytecode, including variable-to-variable operations and printing.

### Running Tests

```bash
make test
```

Expected output (after adding new tests):

```bash
Running tests...
Running: Tokenize a number ✔
Running: Tokenize an identifier ✔
Running: Tokenize print statement ✔
Running: Interpret assignment ✔
Running: Interpret addition ✔
Running: Interpret subtraction ✔
Running: Interpret multiplication ✔
Running: Interpret division ✔
Running: Interpret var-to-var addition ✔
Running: VM push and print ✔

Disabled tests:
Disabled: VM invalid opcode (Test VM failure case not implemented yet)

Test Summary: 10/10 passed (1 disabled)
✔ All enabled tests passed!
```

### Test Annotations
- `// @Test("description")`: Marks a test function to be run.
- `// @Disable("reason")`: Disables a test, logging it without execution (e.g., `test_vm_invalid_opcode`).

## Using lucy-test for Unit Tests

ToyVM integrates lucy-test from the lucy project for lightweight, annotation-based unit testing:
- **Setup**: Tests are written in `tests.c` with `// @Test` annotations above functions. The lucy preprocessor generates `build/annotations.h` and `build/annotations.c` to track these tests.
- **Building**: The Makefile uses lucy to preprocess `tests.c` into `build/tests_processed.c`, then links it with `liblucy-test.so` (from `../../`), which provides the test runner’s `main()` function.
- **Execution**: The runner (`test_runner`) executes all `@Test`-annotated functions, skipping those with `@Disable`, and reports results.
- **Benefits**: Simple syntax, no external test framework dependencies, and integration with the lucy ecosystem.

Example test:

```c
// @Test("Interpret addition")
void test_interpret_addition() {
    ToyVM *vm = vm_new();
    interpret(vm, "x = 5");  // Set initial value
    interpret(vm, "x + 3");  // Add 3 to x
    assertEquals(8, vm->vars['x' - 'a'], "x should be 8 after addition");
    vm_free(vm);
}
```

## Known Issues
- **macOS Library Loading**: On macOS, `test_runner` may fail to load `liblucy-test.so` despite the `-rpath` setting. Workaround:

```bash
export DYLD_LIBRARY_PATH=../../:$DYLD_LIBRARY_PATH
./test_runner
```

This is a known issue to be investigated further (e.g., rpath handling or library placement).

## Future Improvements
- Support multi-letter variables with a symbol table.
- Add loops or conditionals to the Toy language.
- Enhance error handling (e.g., syntax errors, overflow checks).
- Add more unit tests for edge cases (e.g., negative numbers, division by zero).