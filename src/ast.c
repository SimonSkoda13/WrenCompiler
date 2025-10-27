/*
 * Prekladač jazyka IFJ25
 * VUT FIT
 *
 * Autori:
 *   - Martin Michálik (xmicham00)
 */
#include "ast.h"
#include "errors.h"
#include <stdlib.h>

/**
 * Všeobcná funkcia pre tvornu AST uzla.
 * @param token Ukazovateľ na token ktorý bude uložený v uzle.
 * @param left Ukazovateľ na ľavého potomka.
 * @param right Ukazovateľ na pravého potomka.
 */
t_ast_node* create_ast_node(t_token *token, t_ast_node *left, t_ast_node *right) {
    t_ast_node *node = malloc(sizeof(t_ast_node));
    if (node == NULL) {
      fprintf(stderr, "Internal error: Memory allocation failed in create_ast_node\n");
      exit(ERR_INTERNAL);
    }
    node->token = token;
    node->left = left;
    node->right = right;
    return node;
}

/**
 * Funkcia pre vytvorenie listového AST uzla (bez potomkov).
 * @param token Ukazovateľ na token ktorý bude uložený v uzle.
 */
t_ast_node* create_leaf_node(t_token *token) {
    return create_ast_node(token, NULL, NULL);
}

/**
 * Rekurzívna funkcia pre uvoľnenie AST.
 * @param node Ukazovateľ na koreň AST ktorý sa má uvoľniť.
 */
void free_ast(t_ast_node *node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}