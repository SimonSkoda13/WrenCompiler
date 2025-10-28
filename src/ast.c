/*
 * AST uzly pre IFJ25
 * Autor: Matúš Magyar (xmagyam00)
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <ctype.h>

#include "ast.h"


/**
 * Vytvorí nový AST uzol s tokenom a dvoma potomkami.
 * @param token Ukazovateľ na token, ktorý sa uloží do uzla.
 * @param left Ľavý potomok uzla (môže byť NULL).
 * @param right Pravý potomok uzla (môže byť NULL).
 * @return Ukazovateľ na nový AST uzol, alebo NULL pri chybe alokácie.
 */
AstNode *ast_create(t_token* token, AstNode *left, AstNode *right) {
    AstNode *node = calloc(1, sizeof(AstNode));
    if (node != NULL) {
        node->token = token;
        node->left = left;
        node->right = right;
        return node;
    }
    return NULL;
}


/**
 * Vytvorí nový listový AST uzol (bez potomkov).
 * @param token Ukazovateľ na token, ktorý sa uloží do listového uzl.
 * @return Ukazovateľ na nový listový AST uzol, alebo NULL pri chybe alokácie.
 */

AstNode *ast_create_leaf(t_token* token) {
    AstNode *node = calloc(1, sizeof(AstNode));
    if (node != NULL) {
        node->token = token;
        node->left = NULL;
        node->right = NULL;
        return node;
    }
    return NULL;
}

/**
 * Uvoľní pamäť použitú AST uzlom a jeho potomkami rekurzívne.
 * @param node Ukazovateľ na koreň AST stromu na uvoľnenie (môže byť NULL).
 * @return void
 */
void ast_free(AstNode *node) {
    if (node == NULL) return;
    ast_free(node->left);
    ast_free(node->right);
    if (node->token != NULL) {
        if (node->token->type == IDENTIFIER && node->token->value.identifier != NULL) {
            free(node->token->value.identifier);
        }
        if (node->token->type == STRING_LITERAL && node->token->value.string != NULL) {
            free(node->token->value.string);
        }
        free(node->token);
    }
    free(node);
}
