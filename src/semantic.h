/*
*
* Project: IFJ Compiler
*
* Autor: Jaroslav Vrbiniak (xvrbinj00)
*
*/

#ifndef WRENCOMPILER_SEMANTIC_H
#define WRENCOMPILER_SEMANTIC_H

#include <stdbool.h>
#include "ast.h"
#include "scanner.h"
#include "errors.h"
#include "symtable.h" 

//   Typy pre semantiku
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_BOOL,
    TYPE_VOID
} t_type_kind;

//   Deklarácie funkcií semantickej analýzy
void semantic_check(t_ast_node *root);

// Vyhodnotenie typu výrazu
t_type_kind eval_expr_type(t_ast_node *expr, t_symtable *tab);

// Kontrola volania funkcie
void check_function_call(const char *name, t_ast_node *args, t_symtable *tab);

// Error handling
void semantic_error(int code, int line, const char *msg);

#endif // SEMANTIC_H
