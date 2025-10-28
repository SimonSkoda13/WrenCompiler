# Makefile for IFJ Compiler Project
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic -g
TARGET = compiler
SRC_DIR = src
BUILD_DIR = build
TEST_DIR = tests
LDLIBS = -lm

CUR_TEST=_test_cur_
REF_TEST=_test_ref_

SCAN = scanner
ERRORS = errors
LEX = lex
EXPL_DIR=examples
EXPL_PATH=$(TEST_DIR)/$(EXPL_DIR)/
SCAN_PATH=$(TEST_DIR)/scanner/

# Názvy testovacích programov
EX0 = ex0-vsechny-konstrukce
EX1 = ex1-faktorial-iterativne
EX2 = ex2-faktorial-rekurzivne
EX3 = ex3-prace-s-retezci
EX4 = ex4-ciselne-literaly
EX5 = ex5-multiline-strings

# Source files (exclude *_test.c files)
SOURCES = $(filter-out %_test.c, $(wildcard $(SRC_DIR)/*.c))
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Default target
all: $(BUILD_DIR) $(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build the compiler
$(TARGET): $(OBJECTS)
		$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LDLIBS)

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build and run scanner test binary
$(SCAN)-test: 
		$(CC) $(CFLAGS) -o $(SCAN_PATH)/$@.o $(SRC_DIR)/$(SCAN).c $(SRC_DIR)/$(SCAN).h $(SRC_DIR)/$(ERRORS).h $(SRC_DIR)/$(SCAN)_test.c $(LDLIBS)

	@echo "\n------------------------------------ 'vsechny_konstrukce' ------------------------------------\n"
	@./$(SCAN_PATH)$(SCAN)-test.o < $(EXPL_PATH)$(EX0).wren > $(SCAN_PATH)$(SCAN)$(CUR_TEST)$(EX0).output	
	@echo "\nTest case 'vsechny_konstrukce' output differences:"
	@diff -su $(SCAN_PATH)$(SCAN)$(REF_TEST)$(EX0).output $(SCAN_PATH)$(SCAN)$(CUR_TEST)$(EX0).output || exit 0

	@echo "\n------------------------------------ 'faktorial_iterativne' ------------------------------------\n"
	@./$(SCAN_PATH)$(SCAN)-test.o < $(EXPL_PATH)$(EX1).wren > $(SCAN_PATH)$(SCAN)$(CUR_TEST)$(EX1).output	
	@echo "\nTest case 'faktorial_iterativne' output differences:"
	@diff -su $(SCAN_PATH)$(SCAN)$(REF_TEST)$(EX1).output $(SCAN_PATH)$(SCAN)$(CUR_TEST)$(EX1).output || exit 0

	@echo "\n------------------------------------ 'faktorial_rekurzivne' ------------------------------------\n"
	@./$(SCAN_PATH)$(SCAN)-test.o < $(EXPL_PATH)$(EX2).wren > $(SCAN_PATH)$(SCAN)$(CUR_TEST)$(EX2).output	
	@echo "\nTest case 'faktorial_rekurzivne' output differences:"
	@diff -su $(SCAN_PATH)$(SCAN)$(REF_TEST)$(EX2).output $(SCAN_PATH)$(SCAN)$(CUR_TEST)$(EX2).output || exit 0

	@echo "\n------------------------------------ 'prace_s_retezci' ------------------------------------\n"
	@./$(SCAN_PATH)$(SCAN)-test.o < $(EXPL_PATH)$(EX3).wren > $(SCAN_PATH)$(SCAN)$(CUR_TEST)$(EX3).output	
	@echo "\nTest case 'prace_s_retezci' output differences:"
	@diff -su $(SCAN_PATH)$(SCAN)$(REF_TEST)$(EX3).output $(SCAN_PATH)$(SCAN)$(CUR_TEST)$(EX3).output || exit 0

	@echo "\n------------------------------------ 'ciselne-literaly' ------------------------------------\n"
	@./$(SCAN_PATH)$(SCAN)-test.o < $(EXPL_PATH)$(EX4).wren > $(SCAN_PATH)$(SCAN)$(CUR_TEST)$(EX4).output	
	@echo "\nTest case 'ciselne-literaly' output differences:"
	@diff -su $(SCAN_PATH)$(SCAN)$(REF_TEST)$(EX4).output $(SCAN_PATH)$(SCAN)$(CUR_TEST)$(EX4).output || exit 0

	@echo "\n------------------------------------ 'multiline-string' ------------------------------------\n"
	@./$(SCAN_PATH)$(SCAN)-test.o < $(EXPL_PATH)$(EX5).wren > $(SCAN_PATH)$(SCAN)$(CUR_TEST)$(EX5).output	
	@echo "\nTest case 'multiline-string' output differences:"
	@diff -su $(SCAN_PATH)$(SCAN)$(REF_TEST)$(EX5).output $(SCAN_PATH)$(SCAN)$(CUR_TEST)$(EX5).output || exit 0
	

$(SCAN)-clean:
		rm -f $(SCAN_PATH)/$(SCAN)-test.o
		rm -f $(SCAN_PATH)/$(SCAN)$(CUR_TEST)*.output

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
