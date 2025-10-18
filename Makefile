# Makefile for IFJ Compiler Project
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic -g
TARGET = compiler
SRC_DIR = src
BUILD_DIR = build
TEST_DIR = tests
LDLIBS = -lm

SCAN = scanner

# Source files (will be added as you implement)
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Default target
all: $(BUILD_DIR) $(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build the compiler
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build and run scanner test binary
scanner-test: 
		$(CC) $(CFLAGS) -o $(TEST_DIR)/$@.o $(SRC_DIR)/$(SCAN).c $(SRC_DIR)/$(SCAN).h $(SRC_DIR)/$(SCAN)_test.c $(LDLIBS)

# Debug build: no optimizations, debug symbols, optional sanitizers
DEBUG_CFLAGS = $(CFLAGS) -O0 -ggdb -DDEBUG -fsanitize=address,undefined
scanner-test-debug:
		$(CC) $(DEBUG_CFLAGS) -o $(TEST_DIR)/scanner_test_dbg.o $(SRC_DIR)/$(SCAN).c $(SRC_DIR)/$(SCAN)_test.c $(LDLIBS)

# Run with valgrind for memory checks
valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET)

# Clean build artifacts
clean:
	rm -f $(TARGET)
	rm -f $(BUILD_DIR)/*.o

# Run tests (placeholder for now)
test: $(TARGET)
	@echo "Tests will be implemented here"
	@echo "Compiler built successfully!"

# Format code with clang-format
format:
	find $(SRC_DIR) -name "*.c" -o -name "*.h" | xargs clang-format -i

# Run static analysis
lint:
	clang-tidy $(SOURCES) -- $(CFLAGS)

.PHONY: all clean test valgrind format lint
