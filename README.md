# Lucy

A lightweight C annotation processor and testing framework built with Grok from xAI.

## Overview

Lucy is a lightweight C annotation processor and testing framework designed to simplify code generation and unit testing. It includes `lucy`, a command-line tool for processing annotations in C source files, and `lucy-test`, a library for annotation-based unit testing. Lucy aims to provide a minimal, dependency-free solution for developers seeking to enhance their C projects with automated processing and testing capabilities.

## tl;dr

Framework for streamlining code generation and, in the future, AspectJ style point cuts for c.

Use `lucy-test` (built on `lucy`) to add unit tests:

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

Output example:

```
Running tests...
Running: Example test ✔

Disabled tests:
Disabled: Disabled test (Not implemented yet)

Test Summary: 1/1 passed (1 disabled)
✔ All enabled tests passed!
```

## AI Generation

This project has been built over a handful of hours using Grok. The _only_ hand coding/debugging was "helping" the LLM with either compilation and runtime errors.

### Thoughts

The building process was aking to holding a junior/middling developers hand. At times working through issues felt like going round and round in circles until I dug in, found the problem, and either pointed it out or the _actual solution_ to Grok. This isn't something a non-developer would be able to do. This might not even be something most developers can do - as you need to be able to jump into a 100% new codebase with zero context and root-cause issues.

It's vitally important to keep the amount of context the LLM needs to a minimum. Give singular tasks with guardrails and have ways to verify the output. The whole point of the lucy project stemmed from wanting to have an easy way for the LLM to add more unit tests to break apart code into smaller chunks.

## Features

- **Annotation Processing**: Supports custom annotations via `#annotation` definitions, with `@When` as the base annotation.
- **Testing Framework**: Lightweight test runner with support for `@Test`, `@Disable`, `@Setup`, and `@Teardown` annotations.
- **Extensible**: Easily extendable with new annotations and preprocessing logic.

## Building

To build Lucy, ensure you have `gcc` or a compatible C compiler installed, then run:

``` 
make
```

This generates:
- `lucy`: The annotation processor.
- `liblucy.so`: Shared library for annotation processing.
- `liblucy-test.so`: Shared library for the test runner.
- `test_runner`: The executable to run unit tests.

## Usage

### Annotation Processing
Run `lucy` to process source files and generate annotation headers/source:

``` 
./lucy include/annotations.h build/annotations.h build/annotations.c \
    src/source.c:build/source_processed.c
```

This processes `src/source.c` into `build/source_processed.c`, generating `build/annotations.h` and `build/annotations.c`.

### Writing Tests
Lucy’s testing framework uses annotations to define and run tests. Include `lucy_test.h` and link against `liblucy-test.so`.

#### Basic Test
Define a test with `@Test`:

``` 
#include "lucy_test.h"

// @Test("Check addition")
void test_addition() {
    assertEquals(4, 2 + 2, "2 + 2 should equal 4");
}
```

#### Disabling Tests
Use `@Disable` to skip a test:

``` 
// @Disable("Not implemented yet")
// @Test("Pending feature")
void test_pending() {
    assertTrue(0, "This won’t run");
}
```

#### Setup and Teardown
Use `@Setup` and `@Teardown` to run code before and after each test. All functions annotated with `@Setup` run before each test, and all `@Teardown` functions run after:

``` 
static int counter = 0;

// @Setup
void setup_counter() {
    counter = 1;
}

// @Teardown
void teardown_counter() {
    counter = 0;
}

// @Test("Counter is set before test")
void test_counter_setup() {
    assertEquals(1, counter, "Counter should be set by setup");
}
```

- **Multiple Setups/Teardowns**: If multiple `@Setup` or `@Teardown` functions exist, all are executed in the order they appear in `__ANNOTATIONS`.
- **No Cleanup**: Lucy doesn’t automatically reset state between tests; manage it manually in `@Setup` or `@Teardown` if needed.

### Running Tests
Compile and link your test files with `liblucy-test.so`, then run:

``` 
gcc -I./include -L./ -llucy-test build/annotations.o tests/*.c -o test_runner
./test_runner
```

Output:
``` 
Running tests...
Running: Check addition ✔
Running: Counter is set before test ✔

Disabled tests:
Disabled: Pending feature (Not implemented yet)

Test Summary: 2/2 passed, 0 failed (1 disabled)
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
- Improve error reporting in the preprocessor and test runner.