# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -DTARGET_TEST=1 -I./include
LDFLAGS = -shared

# Directories
SRC_DIR = ./src
TEST_DIR = ./tests
INCLUDE_DIR = ./include
BUILD_DIR = ./build
BIN_DIR = ./

# Targets
LUCY_TARGET = $(BIN_DIR)/lucy
LIB_TARGET = $(BIN_DIR)/liblucy.so
TEST_TARGET = $(BIN_DIR)/test_runner

# Source and object files
LUCY_SRC = $(SRC_DIR)/lucy.c
LUCY_LIB_SRC = $(SRC_DIR)/lucy_lib.c
PARSING_SRC = $(SRC_DIR)/parsing.c
LUCY_OBJ = $(BUILD_DIR)/lucy.o
LUCY_LIB_OBJ = $(BUILD_DIR)/lucy_lib.o
PARSING_OBJ = $(BUILD_DIR)/parsing.o
LIB_OBJ = $(BUILD_DIR)/lucy_shared.o

TEST_SRCS = $(TEST_DIR)/simple.c $(TEST_DIR)/complex.c $(TEST_DIR)/lucy_tests.c $(TEST_DIR)/main.c
TEST_OBJS = $(BUILD_DIR)/simple_processed.o $(BUILD_DIR)/complex_processed.o $(BUILD_DIR)/lucy_tests_processed.o $(BUILD_DIR)/main.o $(BUILD_DIR)/annotations.o $(LUCY_LIB_OBJ) $(PARSING_OBJ)
TEST_PREPROCESSED = $(BUILD_DIR)/simple_processed.c $(BUILD_DIR)/complex_processed.c $(BUILD_DIR)/lucy_tests_processed.c

# Default target
all: $(LUCY_TARGET) $(LIB_TARGET) test

# Build lucy binary
$(LUCY_TARGET): $(LUCY_OBJ) $(LUCY_LIB_OBJ) $(PARSING_OBJ)
	$(CC) $(LUCY_OBJ) $(LUCY_LIB_OBJ) $(PARSING_OBJ) -o $@

# Build lucy shared library
$(LIB_TARGET): $(LIB_OBJ) $(PARSING_OBJ)  # Added PARSING_OBJ
	$(CC) $(LDFLAGS) -o $@ $(LIB_OBJ) $(PARSING_OBJ)  # Link both objects

# Compile lucy source for binary
$(LUCY_OBJ): $(LUCY_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile lucy library source
$(LUCY_LIB_OBJ): $(LUCY_LIB_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile parsing source
$(PARSING_OBJ): $(PARSING_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile lucy source for shared library
$(LIB_OBJ): $(LUCY_LIB_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

# Generate annotations and preprocess test files
$(BUILD_DIR)/annotations.h $(BUILD_DIR)/annotations.c $(TEST_PREPROCESSED): $(TEST_SRCS) $(INCLUDE_DIR)/annotations.h | $(LUCY_TARGET) $(BUILD_DIR)
	$(LUCY_TARGET) $(INCLUDE_DIR)/annotations.h $(BUILD_DIR)/annotations.h $(BUILD_DIR)/annotations.c \
		$(TEST_DIR)/simple.c:$(BUILD_DIR)/simple_processed.c \
		$(TEST_DIR)/complex.c:$(BUILD_DIR)/complex_processed.c \
		$(TEST_DIR)/lucy_tests.c:$(BUILD_DIR)/lucy_tests_processed.c

# Compile test objects
$(BUILD_DIR)/%_processed.o: $(BUILD_DIR)/%_processed.c $(BUILD_DIR)/annotations.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/annotations.o: $(BUILD_DIR)/annotations.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/main.o: $(TEST_DIR)/main.c $(BUILD_DIR)/annotations.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link test runner
$(BIN_DIR)/$(TEST_TARGET): $(TEST_OBJS)
	$(CC) $(TEST_OBJS) -o $@

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Run tests
test: $(TEST_TARGET)
	$(BIN_DIR)/$(TEST_TARGET)

# Clean up
clean:
	rm -rf $(BUILD_DIR) $(LUCY_TARGET) $(LIB_TARGET) $(TEST_TARGET)

# Phony targets
.PHONY: all test clean