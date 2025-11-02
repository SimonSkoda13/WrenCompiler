## michalik

## Task 3: Statement Parser (Recursive Descent)

**Files:** `src/parser.c`, `src/parser.h`

- ~~Parse prolog and class skeleton~~
- ~~Parse function definitions (static functions, getters, setters)~~
- ~~Parse statements: `var`, `if-else`, `while`, `return`, assignment, function calls~~
- Handle blocks and scoping
- ~~Verify main() function exists~~

## Task 2: Expression Parser (Precedence Analysis)

**Files:** `src/expression.c`, `src/expression.h`

- ~~Build precedence table for operators~~
- ~~Parse arithmetic operators: `+`, `-`, `*`, `/`~~
- ~~Parse string operators: `+` (concat), `*` (iteration)~~
- ~~Parse relational operators: `<`, `>`, `<=`, `>=`, `==`, `!=`~~
- ~~Parse type test operator: `is`~~
- ~~Handle parentheses~~
- ~~build AST for parsed expression~~

---

## skoda

## magyjar

## Task: AST Implementation

**Files:** `src/ast.c`, `src/ast.h`

- Define AST node structures
- Implement functions to create and manipulate AST nodes

---

## vrbiniak

## Task 4: Stack implementation

---

## Possible tasks

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

## Non AI slop veci čo treba robiť

### Expression parsing

- [x] Treba vlastne urobiť Task 2
- [x] Potom v `parser.c` bude nutné parsovať všetky expression
- [x] To by malo byť štýlom že sa k danému expression vygeneruje AST
- [x] Aby sa to dalo bude musieť byť implemtovaný stack a precedenčná tabuľka
- ~~Zatiaľ funkcia `expression()` v `parser.c` je len niečo aby parser spracoval ten súbor. (Nefunguje)~~
- [ ] Parsovanie a vytváranie AST z výrazov už nejako funguje. Treba ale to skontrolovať a hlavne poriešiť ake error kódy
      by sa mali vraciať t parsovania výrazov (zatiaľ sa vracia 2 - syntaktická chyba) ale podľa zadanie bude treba
      kontrolovať aj typy vo výrazoch takže buď sa to bude robiť priamo pri parsovaní výrazu alebo sa skontroluje
      vygenerovaný AST
- [ ] Nie je to veľmi otestované na edge-case takže bolo by to trochu otestovať

### Semantická anylýza

- Tu bude treba tabuľku symbolov
- Treba v parseri aj zisťovať akého typu sú premenné
- Proste ak budem mať niečo ako `foo = 2 + bar` tak musím dať pozor aby `bar` bol typu `Num`
- Tiež zisťovať použitie nedeklarovanej funkcie alebo premennej
- Treba dávať pozor aj na nesprávne volania funkcii (nesprávny počet prametrov alebo typ)
- Kontrolovať redeklarácie
- Asi samostatná symtable pre globálne premenné ostatné musia byť podľa scope

### Generovanie kódu

- Mal by sa dať využiť AST
- Bude to jebačka lebo stack frames a podobné srandy
- Keď mám AST k výrazu tak viem vygenerovať správne inštrukcie v IFJcode25 tak aby hodnota výrazu bola správna
- V `parser.c` doplniť funkcie rekurzívneho zostupu o generovanie kódu (po tom čo ostané kontroly prebehnú v poriadku)
