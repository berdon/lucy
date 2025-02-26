# Lucy: Annotation Processor and Testing Framework

## Overview

Lucy is a lightweight C annotation processor and testing framework designed to simplify code generation and unit testing. It includes `lucy`, a command-line tool for processing annotations in C source files, and `lucy-test`, a library for annotation-based unit testing. Lucy aims to provide a minimal, dependency-free solution for developers seeking to enhance their C projects with automated processing and testing capabilities.

## Features

- **Annotation Processing**: Parse custom annotations in C comments (e.g., `// @Annotation`) to generate code or metadata.
- **Unit Testing**: Use `lucy-test` to write and run tests with simple `// @Test` annotations.
- **Extensible**: Define custom annotation extensions for project-specific needs.
- **Sample Project**: Includes `toyvm`, a minimal interpreter demonstrating `lucy-test` usage.

## Directory Structure
```
./
├── src/
│   └── lucy.c              # Lucy source code
├── tests/
│   ├── simple.c           # Simple unit tests
│   ├── complex.c          # Complex unit tests
│   └── main.c             # Test runner
├── include/
│   ├── lucy.h             # Default annotation struct and externs
|   ├── lucy_api.h         # Shared library API
│   ├── test_utils.h       # Assertion macros
│   └── annotations.h      # User-defined annotation extensions
├── build/                 # Generated files (lucy, processed .c, annotations.h/c)
└── Makefile               # Build script
```

## Installation

### Prerequisites
- GCC or a compatible C compiler (e.g., Clang).
- Standard C library.

### Building
From the root `lucy/` directory:

```bash
make clean && make
```

This builds:
- `lucy`: The annotation processor binary.
- `liblucy-test.so`: The shared library for unit testing.

## Using lucy-test in Your Projects

`lucy-test` provides a simple way to add unit tests to C projects using annotations. Here’s how to integrate it:

### Setup
1. **Include the Header**: Add `#include "lucy_test.h"` to your test file, assuming it’s in your include path.
2. **Write Tests**: Use `// @Test` annotations above test functions. Optionally, use `// @Disable` to skip tests.

```c
#include "lucy_test.h"

// @Test("Example test")
void test_example() {
    assertEquals(1 + 1, 2, "1 + 1 should equal 2");
}

// @Disable("Not implemented yet")
// @Test("Disabled test")
void test_disabled() {
    assertTrue(0, "This should not run");
}
```

3. **Process with lucy**: Use the `lucy` tool to preprocess your test file and generate annotation metadata:

```bash
lucy include/annotations.h build/annotations.h build/annotations.c tests.c:build/tests_processed.c
```

4. **Compile and Link**:
   - Compile your source and test files with `-DTARGET_TEST=1` to enable test mode.
   - Link against `liblucy-test.so` to include the test runner.

```bash
gcc -Wall -O2 -DTARGET_TEST=1 -Iinclude -c src/mylib.c -o build/mylib.o
gcc -Wall -O2 -DTARGET_TEST=1 -Iinclude -c build/tests_processed.c -o build/tests_processed.o
gcc -Wall -O2 -DTARGET_TEST=1 -Iinclude -c build/annotations.c -o build/annotations.o
gcc build/tests_processed.o build/mylib.o build/annotations.o -L. -llucy-test -o test_runner
```

5. **Run Tests**:
   ```bash
   ./test_runner
   ```
   Output example:
   ```
   Running tests...
   Running: Example test ✔

   Disabled tests:
   Disabled: Disabled test (Not implemented yet)

   Test Summary: 1/1 passed (1 disabled)
   ✔ All enabled tests passed!
   ```

### Makefile Integration
Add rules to your Makefile to automate the process:

```make
CFLAGS = -Wall -O2 -Iinclude
TEST_CFLAGS = $(CFLAGS) -DTARGET_TEST=1
LDFLAGS = -L. -llucy-test

test: test_runner
    ./test_runner

test_runner: build/tests_processed.o build/mylib.o build/annotations.o
    $(CC) $^ $(LDFLAGS) -o $@

build/annotations.h build/annotations.c build/tests_processed.c: tests.c
    ./lucy include/annotations.h build/annotations.h build/annotations.c tests.c:build/tests_processed.c

build/tests_processed.o: build/tests_processed.c
    $(CC) $(TEST_CFLAGS) -c $< -o $@

build/mylib.o: src/mylib.c
    $(CC) $(CFLAGS) -c $< -o $@

build/annotations.o: build/annotations.c
    $(CC) $(TEST_CFLAGS) -c $< -o $@
```

### Notes
- Ensure `liblucy-test.so` is in the link path (e.g., `-L.` or adjust `LD_LIBRARY_PATH` on macOS).
- Use `lucy-test.h` assertions like `assertEquals`, `assertTrue`, and `assertStringEquals` for test conditions.

## Sample Project: ToyVM

The `samples/toyvm/` directory contains a sample project, `toyvm`, demonstrating `lucy-test` in action. ToyVM is a minimal interpreter for the "Toy" language, supporting:

- **Variables**: `x = 5` (single-letter variables `a-z`).
- **Arithmetic**: `+`, `-`, `*`, `/` (e.g., `x + 3`, `z = x + y`).
- **Print**: `print(x)` (outputs in grey text).
- **Debug Mode**: Run with `--debug` or `-d` for detailed execution logs.

### Building and Running ToyVM
```bash
cd samples/toyvm
make clean && make
./toyvm
```

Example session:
```toy
ToyVM Interactive Shell (Ctrl+D or Ctrl+C to exit)
x = 5
y = 6
z = x + y
print(z)  # Outputs 11 in grey
x - 2
print(x)  # Outputs 3 in grey
^D
```

### Running ToyVM Tests
```bash
make test
```
Outputs 10 passing tests covering tokenization, arithmetic, and VM execution.

### Exploring ToyVM
- Source: `toyvm.c`, `toyvm.h`
- Tests: `tests.c`
- Details: See `samples/toyvm/README.md` for a full breakdown.

ToyVM showcases how `lucy-test` simplifies testing a small interpreter, making it a great starting point for learning to use Lucy in your projects.

## Known Issues
- **macOS Library Loading**: `liblucy-test.so` may not load with `-rpath` on macOS. Workaround:
```bash
export DYLD_LIBRARY_PATH=.:$DYLD_LIBRARY_PATH
```

## Contributing
Contributions are welcome! Feel free to submit pull requests or open issues for bugs, features, or improvements.

## Future Improvements
- Expand annotation syntax for more complex use cases.
- Enhance `lucy-test` with test fixtures or setup/teardown hooks.
- Improve error reporting in the preprocessor and test runner.