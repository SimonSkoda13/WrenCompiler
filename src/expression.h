/*
 * Prekladač jazyka IFJ25
 * VUT FIT
 *
 * Autori:
 *   - Martin Michálik (xmicham00)
 */

#ifndef WRENCOMPILER_EXPRESSION_H
#define WRENCOMPILER_EXPRESSION_H

#include "scanner.h"
#include "parser.h"
#include "ast.h"
#include "errors.h"
#include <stdbool.h>


/**
 * Enumy pre symboly v precedenčnej tabuľke.
 */
typedef enum prec_symbols {
    PREC_LPAR,      // (
    PREC_RPAR,      // )
    PREC_IDENT,     // i (identifier/literal)
    PREC_PLUS,      // +
    PREC_MINUS,     // -
    PREC_MUL,       // *
    PREC_DIV,       // /
    PREC_LESS,      // <
    PREC_LESSEQ,    // <=
    PREC_GREATER,   // >
    PREC_GREATEREQ, // >=
    PREC_EQ,        // ==
    PREC_NEQ,       // !=
    PREC_IS,        // is
    PREC_DOLLAR,    // $ (end of expression)
    S,              // < shift
    R,              // > reduce
    W,              // = wait/equal
    ERR,            // error
    PREC_EMPTY,     // empty cell
    PREC_E,         // E non-terminal
    PREC_INVALID,
    PREC_END_EXP
} prec_symbols;


/**
 * štruktúra pre element zásobníka. Je trochu špeciálna, lebo musí obsahovať aj AST a informácie o tokene.
 * @property data Symbol z precedenčnej tabuľky.
 * @property token Token asociovaný s týmto elementom zásobníka.
 * @property tree AST podstrom ktorý patrí k tomuto prvku
 * @property next_element Ukazovateľ na ďalší element v zásobníku.
 */
typedef struct stack_el {
    prec_symbols data;
    t_token token;
    t_ast_node *tree;
    struct stack_el* next_element;

} * t_stack_exp;

/**
 * Štruktúra pre zásobník používaný v precedenčnej analýze.
 * @property top Ukazovateľ na vrchol zásobníka.
 */
typedef struct {
    t_stack_exp top;
} t_stack;

/**
 * Inicializuje zásobník.
 */
void stack_init(t_stack* stack);

/**
 * Získa vrchný terminálový symbol zo zásobníka. Ignoruje neterminály. (kto nechápe viz. prednášky)
 * @param stack Ukazovateľ na zásobník.
 * @return Vrchný terminálový symbol.
 */
prec_symbols stack_top_terminal(t_stack* stack);

/**
 * Získa vrchný symbol zo zásobníka.
 * @param stack Ukazovateľ na zásobník.
 * @return Vrchný symbol.
 */
prec_symbols stack_top(t_stack *stack);

/**
 * Pushne nový symbol na vrchol zásobníka.
 * @param stack Ukazovateľ na zásobník.
 * @param data Symbol na pushnutie.
 */
void stack_push(t_stack *stack, prec_symbols data);

/**
 * Vloží shift symbol nad vrchný terminálový symbol v zásobníku. (kto nechápe viz. prednášky)
 * @param stack Ukazovateľ na zásobník.
 */
void stack_shift_push(t_stack *stack);

/**
 * Odstráni a vráti vrchný element zásobníka.
 * @param stack Ukazovateľ na zásobník.
 * @return Vrchný element zásobníka.
 */
t_stack_exp stack_pop(t_stack *stack);

/**
 * Skontroluje či je zásobník prázdny (obsahuje len PREC_EMPTY).
 * @param stack Ukazovateľ na zásobník.
 * @return 1 ak je zásobník prázdny, inak 0.
 */
int stack_empty(t_stack *stack);

/**
 * Aplikuje pravidlá redukcie na zásobník.
 * @param stack Ukazovateľ na zásobník.
 * @return 1 ak bola aplikovaná nejaká redukcia, inak 0.
 */
int rule_reduction(t_stack *stack);

/**
 * Mapuje token na príslušný symbol v precedenčnej tabuľke.
 * @param token Ukazovateľ na token.
 * @return Príslušný symbol v precedenčnej tabuľke.
 */
prec_symbols map_token_to_enum(t_token *token);

/**
 * Skontroluje či je syntax v zásobníku korektná (E $).
 * @param stack Ukazovateľ na zásobník.
 * @return 1 ak je syntax korektná, inak 0.
 * @todo Nie je hotové. Treba dorobiť kontroly
 */
int correct_syntax(t_stack *stack);

/**
 * Parsuje výraz pomocou precedenčnej analýzy a vytvorí AST.
 * @param token1 Prvý token výrazu. Ak sme hne začali parsovať výraz, môže byť NULL.
 * @param token2 Druhý token výrazu (môže byť NULL).
 * @param tree Ukazovateľ na výsledný AST.
 * @return 0 ak bol výraz úspešne parsovaný, inak chybový kód.
 */
int parse_expression(t_token *token1, t_token *token2, t_ast_node **tree);

#endif // WRENCOMPILER_EXPRESSION_H