/*
 * Prekladač jazyka IFJ25
 * VUT FIT
 *
 * Autori:
 *   - Martin Michálik (xmicham00)
 */

#ifndef WRENCOMPILER_PARSER_H
#define WRENCOMPILER_PARSER_H

#include "scanner.h"
#include "errors.h"
#include "expression.h"
#include "generator.h"
#include "symtable.h"
#include "ast.h"
#include "builtins.h"

/**
 * @struct Štruktúra pre parser.
 */
typedef struct
{
    t_scanner *scanner;
    t_token *current_token;
    t_token *putback_token;
    bool has_putback;
    t_symtable *symtable; // Tabuľka symbolov
} t_parser;

/**
 * Globálna inštancia parsera.
 */
t_parser parser;

/**
 * Buffer pre odloženie generovania funkcie main
 */
extern FILE *main_output_buffer;
extern bool is_generating_main;

/**
 * Deklarácie funkcií parsera.
 */

/**
 * Získa nasledujúci token zo scanneru.
 * Ak je has_putback true, vráti uložený putback token namiesto získania nového a nastaví has_putback na false.
 */
void next_token();
/**
 * Uloží aktuálny token na putback a nastaví has_putback = true. Ďalšie volanie next_token() vráti tento token.
 */
void putback_token();
/**
 * Skontroluje, či je aktuálny token očakávaného typu. Ak nie, vyvolá chybu.
 * @param expected_type Očakávaný typ tokenu.
 */
void check_token(e_token_type expected_type);
/**
 * Získa nasledujúci token a skontroluje, či je očakávaného typu.
 * @param expected_type Očakávaný typ tokenu.
 */
void consume_token(e_token_type expected_type);
/**
 * Skontroluje, či je prolog valídny. import "ifj25" for Ifj
 */
void prolog();
/**
 * Skontroluje či je správna kostra programu. class Program { ... }
 */
void program();
/**
 * Postupne spracuje všetky definované funkcie v programe.
 */
void func_list();
/**
 * Spracuje definíciu jednej funkcie.
 */
void func();
/**
 * Spracuje zoznam parametrov funkcie.
 * @param params Ukazovateľ na štruktúru kde sa uložia názvy parametrov
 */
void param_list(t_param_list *params);
/**
 * Spracuje zvyšok zoznamu parametrov funkcie po prvom parametri.
 * @param params Ukazovateľ na štruktúru kde sa uložia názvy parametrov
 */
void param_list_tail(t_param_list *params);
/**
 * Spracuje blok kódu uzavretý v zložených zátvorkách. { ... }
 */
void block();
/**
 * Spracuje zoznam príkazov v bloku.
 */
void statement_list();
/**
 * Spracuje jeden príkaz.
 */
void statement();
/**
 * Spracuje deklaráciu premennej.
 */
void var_decl();
/**
 * Spracuje priradenie hodnoty premennej.
 */
void assign();
/**
 * Spracuje if-else príkaz.
 */
void if_statement();
/**
 * Spracuje while príkaz.
 */
void while_statement();
/**
 * Spracuje volanie funkcie.
 */
void func_call();
/**
 * Spracuje return príkaz.
 */
void return_statement();
/**
 * Spracuje zoznam argumentov funkcie.
 * @return Počet argumentov
 */
int arg_list();
/**
 * Spracuje zvyšok zoznamu argumentov funkcie po prvom argumente.
 * @return Počet argumentov v tomto volaní (0 alebo viac)
 */
int arg_list_tail();
/**
 * Spracuje výraz.
 * @param token1 Prvý token výrazu
 * @param token2 Druhý token výrazu (môže byť NULL)
 * @return Ukazovateľ na AST koreň výrazu
 */
t_ast_node *expression(t_token *token1, t_token *token2);
void expression_continue();
/**
 * Spracuje terminál vo výraze.
 */
void term();
/**
 * Spracuje operátor vo výraze.
 */
void op();
bool is_operator();
/**
 * Spracuje viacnásobné EOL tokeny.
 */
void eols();
/**
 * Hlavná funkcia pre spustenie parsera.
 */
void parse_program();

#endif // WRENCOMPILER_PARSER_H