# Lucy: Precompiled Annotation Processor (AI Generated)

Lucy is a lightweight C preprocessor that adds annotation support to C code, enabling features like conditional compilation and metadata tracking without modifying the core language syntax. It processes `.c` files with annotations (e.g., `// @Annotation`), generates transformed outputs, and aggregates annotation tracking into separate files for linking. It can be used as a standalone binary or a shared library.

~99% of the code in this repository has been generated via AI.

## Features
- **Annotations**: Supports user-defined annotations (e.g., `// @Test`) with inheritance (e.g., extending `@When`).
- **Conditional Compilation**: Wraps annotated blocks in `#ifdef` directives based on conditions.
- **Tracking**: Generates an `annotations.h` and `annotations.c` with metadata for all annotated functions.
- **Library Support**: Provides `liblucy.so` for programmatic use in other applications.
- **Simple Integration**: Works with standard C build systems via Makefile.

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

## Usage (CLI)
Lucy can be used as a command-line tool integrated into a build pipeline via a Makefile.

### 1. Define Annotations
Edit `include/annotations.h` to define your annotations:

```c
#ifndef ANNOTATIONS_H
#define ANNOTATIONS_H

#include "lucy.h"

// Define custom annotations
// #annotation @Test(condition, description) : @When(condition)

#endif // ANNOTATIONS_H
```

### 2. Write Annotated Code

Add annotations to your .c files (e.g., tests/simple.c):

```c
#include "../include/annotations.h"
#include "../include/test_utils.h"
#include <string.h>

// @Test(TARGET_TEST, "Test string equality")
void test_string_equality() {
    const char *expected = "hello";
    const char *actual = "hello";
    assertEquals(expected, actual, "Strings should match");
}
```

### 3. Build with Makefile

The provided Makefile processes files with lucy and builds a test runner:

```bash
make
```

#### Steps:

- Compiles `lucy` from `src/lucy.c`.
- Runs `lucy` to process `tests/*.c` into `build/*_processed.c` and generate `build/annotations.h` and `build/annotations.c`.
- Compiles processed files and links into `test_runner`.

### 4. Run Tests

Execute the test runner:

```bash
./test_runner
```

#### Output:

Shows test results with green checkmarks (✔) for passes and red X’s (✘) for failures (via assertions).

### 5. Customize

- Conditions: Define macros like TARGET_TEST in CFLAGS (e.g., -DTARGET_TEST=1) to enable/disable tests.
- New Annotations: Add to include/annotations.h and use in your code.
- Clean: Run make clean to remove generated files.

## Usage (Shared Library)

Lucy can also be used as a shared library (liblucy.so) for programmatic annotation processing.

### 1. Include Headers

In your program:

```c
#include "lucy_api.h"
```

### 2. Process Files

Use the API:

```c
int main() {
    lucy_init();
    lucy_process_file("src/input.c", "build/input_processed.c");
    lucy_generate_annotations_header("include/annotations.h", "build/annotations.h");
    lucy_generate_annotations_source("build/annotations.c");
    lucy_cleanup();
    return 0;
}
```

### 3. Compile and Link

Compile your program with liblucy.so:

```bash
gcc -I./include -c my_program.c -o my_program.o
gcc my_program.o -L./ -llucy -o my_program
LD_LIBRARY_PATH=./ ./my_program
```

    Notes: Ensure liblucy.so is in the runtime library path (LD_LIBRARY_PATH).

## Example Makefile

Here’s a standalone Makefile example for using lucy with custom source files:

```makefile
# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -I./include
LDFLAGS = -shared

# Directories
SRC_DIR = ./src
INCLUDE_DIR = ./include
BUILD_DIR = ./build
BIN_DIR = ./

# Targets
LUCY = $(BIN_DIR)/lucy
LIB = $(BIN_DIR)/liblucy.so
PROGRAM = $(BIN_DIR)/my_program

# Sources and outputs
LUCY_SRC = $(SRC_DIR)/lucy.c
SOURCES = main.c util.c
PREPROCESSED = $(BUILD_DIR)/main_processed.c $(BUILD_DIR)/util_processed.c
OBJECTS = $(PREPROCESSED:.c=.o) $(BUILD_DIR)/annotations.o

# Default target
all: $(PROGRAM)

# Build lucy binary
$(LUCY): $(LUCY_SRC)
	$(CC) $(CFLAGS) -o $@ $<

# Build lucy shared library
$(LIB): $(LUCY_SRC)
	$(CC) $(CFLAGS) -fPIC -c $< -o $(BUILD_DIR)/lucy_shared.o
	$(CC) $(LDFLAGS) -o $@ $(BUILD_DIR)/lucy_shared.o

# Process sources and generate annotations
$(BUILD_DIR)/annotations.h $(BUILD_DIR)/annotations.c $(PREPROCESSED): $(SOURCES) $(INCLUDE_DIR)/annotations.h | $(LUCY) $(BUILD_DIR)
	$(LUCY) $(INCLUDE_DIR)/annotations.h $(BUILD_DIR)/annotations.h $(BUILD_DIR)/annotations.c $(SRC_DIR)/main.c:$(BUILD_DIR)/main_processed.c $(SRC_DIR)/util.c:$(BUILD_DIR)/util_processed.c

# Compile objects
$(BUILD_DIR)/%_processed.o: $(BUILD_DIR)/%_processed.c $(BUILD_DIR)/annotations.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/annotations.o: $(BUILD_DIR)/annotations.c
	$(CC) $(CFLAGS) -c $< -o $@

# Link program
$(PROGRAM): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean up
clean:
	rm -rf $(BUILD_DIR) $(LUCY) $(LIB) $(PROGRAM)

.PHONY: all clean
```

### Example Usage

Example Usage

1. Place your source files (`main.c`, `util.c`) in `src/`.
2. Define annotations in `include/annotations.h`.
3. Run:

```bash
make
./my_program
```

Optionally, clean:

```bash
make clean
```

This Makefile assumes lucy processes main.c and util.c, generating preprocessed files and annotation tracking, then links them into my_program.

## Example Build Command

```bash
gcc -Wall -Wextra -O2 -DTARGET_TEST=1 -I./include -c build/simple_processed.c -o build/simple_processed.o
gcc -Wall -Wextra -O2 -DTARGET_TEST=1 -I./include -c build/annotations.c -o build/annotations.o
gcc build/simple_processed.o build/annotations.o -o my_program
```

## Notes
- lucy.h: Provides default struct and externs, included by `annotations.h`.
- lucy_api.h: API for liblucy.so.
- test_utils.h: Defines assertion macros; extend as needed.
- Portability: Assumes a Unix-like environment with ANSI color support for the runner.

For issues or enhancements, modify `src/lucy.c` and rebuild.