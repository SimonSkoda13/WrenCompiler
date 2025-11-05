/*
 * Prekladač jazyka IFJ25
 * VUT FIT
 *
 * Autori:
 *   - Matúš Magyar (xmagyam00)
 */

 /*Poznamka - mozeme pridat aj unarne a logicke operatory*/


#ifndef WRENCOMPILER_AST_H
#define WRENCOMPILER_AST_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "scanner.h"//pouzitie struct t_token(obsahuje value a type)
#include "errors.h"//pouzitie errors code z errors.h


//Štruktúra uzla AST
typedef struct AstNode {
    t_token* token;          
    struct AstNode* left;  
    struct AstNode* right;
} t_ast_node;

/**
 * Vytvorí nový AST uzol s tokenom a dvoma potomkami.
 * @param token Ukazovateľ na token, ktorý sa uloží do uzla (nekopíruje sa).
 * @param left Ľavý potomok uzla (môže byť NULL).
 * @param right Pravý potomok uzla (môže byť NULL).
 * @return Ukazovateľ na nový AST uzol, alebo NULL pri chybe alokácie.
 */
t_ast_node *ast_create(t_token *token, t_ast_node *left, t_ast_node *right);

/**
 * Vytvorí nový listový AST uzol (bez potomkov).
 * @param token Ukazovateľ na token, ktorý sa uloží do listového uzla.
 * @return Ukazovateľ na nový listový AST uzol, alebo NULL pri chybe alokácie.
 */
t_ast_node *ast_create_leaf(t_token* token);

/**
 * Uvoľní pamäť použitú AST uzlom a jeho potomkami rekurzívne.
 * @param node Ukazovateľ na koreň AST stromu na uvoľnenie (môže byť NULL).
 * @return void
 */
void ast_free(t_ast_node *node);

#endif // WRENCOMPILER_AST_H