## michalik

## Task 3: Statement Parser (Recursive Descent)

**Files:** `src/parser.c`, `src/parser.h`

- ~~Parse prolog and class skeleton~~
- ~~Parse function definitions (static functions, getters, setters)~~
- ~~Parse statements: `var`, `if-else`, `while`, `return`, assignment, function calls~~
- ~~Handle blocks and scoping~~
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

## Task: Generovanie uživateľom vytvorených funkcii

- ~~Generácia uživateľských funkcii~~
- ~~Volanie funkcii~~
- ~~Preťažovanie funkcii~~
- ~~Bloky~~

---

## skoda

## magyjar

---

## vrbiniak

## Task 4: Semantic Analysis

**Files:** `src/semantic.c`, `src/semantic.h`

- Check undefined variables/functions (error 3)
- Check redefinitions (error 4)
- Check function argument counts (error 5)
- Check type compatibility (error 6)
- Handle function overloading

---

## Possible tasks

---

## Task 5: Code Generator

**Files:** `src/codegen.c`, `src/codegen.h`

- Generate IFJcode25 instructions
- Handle frames (GF, LF, TF)
- Generate jumps, labels, function calls
- Generate runtime type checks

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

### Čítať zadanie keď nie je jasné čo by to malo robiť!

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
- Môžme tu robiť sémanticku analýzu zložitejších výrazov `foo = "ab" * 3 + 2` -> chyba

### Semantická anylýza

- [x] Tu bude treba tabuľku symbolov
- Treba v parseri aj zisťovať akého typu sú premenné
- Proste ak budem mať niečo ako `foo = 2 + bar` tak musím dať pozor aby `bar` bol typu `Num`
- Tiež zisťovať použitie nedeklarovanej funkcie alebo premennej. S funkciami to bude trochu náročenjšie keďže sa môžu nachádzať
  hocikde a môže sa volať funkcia ktorá je deklarovaná až ďalej v kóde
- Treba dávať pozor aj na nesprávne volania funkcii (nesprávny počet prametrov alebo typ)
- [x] Kontrolovať redeklarácie
- Asi samostatná symtable pre globálne premenné ostatné musia byť podľa scope
- Treba pozrieť zadanie a prejsť si veci ktoré je treba detekovať semantickou analýzov (redekláracie, zlý počet parametrov, atď.)
- Tiež treba trochu upraviť a vyčistiť vracanie error kódov. Zatiaľ som to nekontroloval takže vraciam error kódy ako mi ich
  navrhovalo takže to nebude úplne podľa zadania

### Generovanie kódu

- [x] Mal by sa dať využiť AST

- Bude to jebačka lebo stack frames a podobné srandy
- Keď mám AST k výrazu tak viem vygenerovať správne inštrukcie v IFJcode25 tak aby hodnota výrazu bola správna
- V `parser.c` doplniť funkcie rekurzívneho zostupu o generovanie kódu (po tom čo ostané kontroly prebehnú v poriadku)
- ~~Zatiaľ je len veľmi hrubá implementácia ktorá umožňuje generovanie kódu pre jednoduché programy typu Hello World!~~
- ~~Generovanie funkcií nie je hotové zatiaľ sa vieme generovať len builtin funkciu Ifj.write a main funkciu (špecialny prípad)~~
- [x] Treba urobiť generovanie uživateľom deklarovaných funkcií. Pri tomto treba myslieť na preťažovanie takže funkcia myFunc(param1)
      a myFunc(param1, param2) sú dve úplne rozdielne funkcie
- [x] Statické gettery a settery nemám analyzované takže neviem povedať či tam bude nejkaký väčší ojeb alebo to bude zase len špecialny
      prípad funkcie
- Globálne premenné nie sú vyriešené. Pri tomto to odhadujem na dve tabuľky symbolov (globálnu a lokálnu) a globálne premenné
  riešiť v tej globálnej tabuľke
- [x] Bloky. Je to vysvetlené v democivku aký tu nastáva problém. Takže bude treba urobiť name-mangling pri vkladaní do tabuľky symbolov
      a potom získať aj správnu premennú z tabuľky podľa zanorenia
- [x] Built-in funkcie. Tá funkcia Ifj.write je len jeden z pár špeciálnych prípadov kedy sa to dá riešiť takto. Bude treba podporovať
      aj iné builtin funkcie ktoré si však budeme musieť vygenerovať sami
- Vyriešiť generáciu kodú pre operácie s reťazcami (+ \*)
- Typové kontroly za behu programu. Viz. zadanie
- [x] Urobiť generovanie kódu pre ostatné koštrukcie (if, while, ...)
