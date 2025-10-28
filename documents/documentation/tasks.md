## michalik

## skoda

## magyjar

## Task: AST Implementation

**Files:** `src/ast.c`, `src/ast.h`

- Define AST node structures
- Implement functions to create and manipulate AST nodes

---

## vrbiniak

## Task 4: Stack implementation

**Files:** `src/stack.c`, `src/stack.h`

- Implement stack data structure for various uses
- Functions: push, pop, peek, is_empty
- Implement pop so it returns .data of the top element

---

## Possible tasks

## Task 2: Expression Parser (Precedence Analysis)

**Files:** `src/expression.c`, `src/expression.h`

- Build precedence table for operators
- Parse arithmetic operators: `+`, `-`, `*`, `/`
- Parse string operators: `+` (concat), `*` (iteration)
- Parse relational operators: `<`, `>`, `<=`, `>=`, `==`, `!=`
- Parse type test operator: `is`
- Handle parentheses
- evaluate expressions using AST

---

## Task 3: Statement Parser (Recursive Descent)

**Files:** `src/parser.c`, `src/parser.h`

- Parse prolog and class skeleton
- Parse function definitions (static functions, getters, setters)
- Parse statements: `var`, `if-else`, `while`, `return`, assignment, function calls
- Handle blocks and scoping
- Verify main() function exists
- Just scope of it

---

## Task 4: Semantic Analysis

**Files:** `src/semantic.c`, `src/semantic.h`

- Check undefined variables/functions (error 3)
- Check redefinitions (error 4)
- Check function argument counts (error 5)
- Check type compatibility (error 6)
- Handle function overloading

---

## Task 5: Code Generator

**Files:** `src/codegen.c`, `src/codegen.h`

- Generate IFJcode25 instructions
- Handle frames (GF, LF, TF)
- Generate jumps, labels, function calls
- Generate runtime type checks

---

## Task 6: Built-in Functions

**Files:** `src/builtins.c`, `src/builtins.h`

- Implement: `Ifj.read_str()`, `Ifj.read_num()`, `Ifj.write()`
- Implement: `Ifj.floor()`, `Ifj.str()`
- Implement: `Ifj.length()`, `Ifj.substring()`, `Ifj.strcmp()`, `Ifj.ord()`, `Ifj.chr()`

---

## Task 7: Error Handling

**Files:** `src/error.c`, `src/error.h`

- Create error reporting system for all error codes (1-6, 10, 25-26, 99)
- Print errors to stderr with line numbers

---

## Task 8: Main Integration

**Files:** `src/main.c`

- Connect: scanner → parser → semantic → codegen
- Handle stdin/stdout
- Return proper error codes

---

## Task 9: Testing

**Files:** `tests/`

- Create test programs
- Test with ic25int interpreter
- Run valgrind for memory leaks

---

## Task 10: Documentation

**Files:** `dokumentace.pdf`

- Describe AVL tree implementation
- Describe parser and codegen
- Document team contributions
