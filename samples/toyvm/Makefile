CC = gcc
BASE_CFLAGS = -Wall -Wextra -O2 -I../../include -I ./
TEST_CFLAGS = $(BASE_CFLAGS) -DTARGET_TEST=1
LDFLAGS = -L../../ -llucy-test -Wl,-rpath,../../

SRC_DIR = .
BUILD_DIR = ./build
BIN_DIR = .

TOYVM_SRC = $(SRC_DIR)/toyvm.c
TEST_SRC = $(SRC_DIR)/tests.c
TOYVM_OBJ = $(BUILD_DIR)/toyvm.o
TEST_OBJ = $(BUILD_DIR)/tests.o
TEST_PREPROCESSED = $(BUILD_DIR)/tests_processed.c
TEST_PROCESSED_OBJ = $(BUILD_DIR)/tests_processed.o
ANNOTATIONS_OBJ = $(BUILD_DIR)/annotations.o

all: toyvm test

toyvm: $(TOYVM_OBJ)
	$(CC) $(TOYVM_OBJ) -o $(BIN_DIR)/$@
	@echo "Built toyvm binary: $(BIN_DIR)/toyvm"

test: $(BIN_DIR)/test_runner
	@echo "Running test_runner..."
	DYLD_LIBRARY_PATH=../../:$$DYLD_LIBRARY_PATH $(BIN_DIR)/test_runner

$(BIN_DIR)/test_runner: $(TEST_PROCESSED_OBJ) $(BUILD_DIR)/toyvm_test.o $(ANNOTATIONS_OBJ)
	$(CC) $(TEST_PROCESSED_OBJ) $(BUILD_DIR)/toyvm_test.o $(ANNOTATIONS_OBJ) $(LDFLAGS) -o $@
	@echo "Built test_runner binary: $(BIN_DIR)/test_runner"

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TOYVM_OBJ): $(TOYVM_SRC) | $(BUILD_DIR)
	$(CC) $(BASE_CFLAGS) -c $< -o $@

$(BUILD_DIR)/toyvm_test.o: $(TOYVM_SRC) | $(BUILD_DIR)
	$(CC) $(TEST_CFLAGS) -c $< -o $@

$(TEST_OBJ): $(TEST_SRC) | $(BUILD_DIR)
	$(CC) $(TEST_CFLAGS) -c $< -o $@

$(BUILD_DIR)/annotations.h $(BUILD_DIR)/annotations.c $(TEST_PREPROCESSED): $(TEST_SRC) | $(BUILD_DIR)
	../../lucy ../../include/annotations.h $(BUILD_DIR)/annotations.h $(BUILD_DIR)/annotations.c $(TEST_SRC):$(TEST_PREPROCESSED)

$(TEST_PROCESSED_OBJ): $(TEST_PREPROCESSED) $(BUILD_DIR)/annotations.h | $(BUILD_DIR)
	$(CC) $(TEST_CFLAGS) -c $< -o $@

$(ANNOTATIONS_OBJ): $(BUILD_DIR)/annotations.c | $(BUILD_DIR)
	$(CC) $(TEST_CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)/toyvm $(BIN_DIR)/test_runner

.PHONY: all test clean