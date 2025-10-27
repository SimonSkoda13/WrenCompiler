/*
 * Prekladač jazyka IFJ25
 * VUT FIT
 *
 * Autori:
 *   - Martin Michálik (xmicham00)
 */


#ifndef WRENCOMPILER_AST_H
#define WRENCOMPILER_AST_H

#include "scanner.h"
/**
 * Štruktúra AST uzla.
 */
typedef struct ast_node {
    t_token *token;
    struct ast_node *left;
    struct ast_node *right;
} t_ast_node;

/**
 * Všeobcná funkcia pre tvornu AST uzla.
 * @param token Ukazovateľ na token ktorý bude uložený v uzle.
 * @param left Ukazovateľ na ľavého potomka.
 * @param right Ukazovateľ na pravého potomka.
 */
t_ast_node* create_ast_node(t_token *token, t_ast_node *left, t_ast_node *right);
/**
 * Funkcia pre vytvorenie listového AST uzla (bez potomkov).
 * @param token Ukazovateľ na token ktorý bude uložený v uzle.
 */
t_ast_node* create_leaf_node(t_token *token);
/**
 * Rekurzívna funkcia pre uvoľnenie AST.
 * @param node Ukazovateľ na koreň AST ktorý sa má uvoľniť.
 */
void free_ast(t_ast_node *node);

#endif // WRENCOMPILER_AST_H