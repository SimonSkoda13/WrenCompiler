/*
*
* Project: IFJ Compiler
*
* Autor: Jaroslav Vrbiniak (xvrbinj00)
*
*/
// nie je tam zaimplemetovane vypisovane errorov s cislom riadku 

#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Spracovanie errorov

void semantic_error(int code, int line, const char *msg) {
    fprintf(stderr, "Semantic Error %d at line %d: %s\n", code, line, msg);
    exit(code);
}

// Vyhodnotenie typu výrazu

t_type_kind eval_expr_type(t_ast_node *expr, t_symtable *tab) {
    if (!expr || !expr->token) return TYPE_UNKNOWN;

    switch (expr->token->type) {
        case NUM_INT: return TYPE_INT;
        case NUM_FLOAT: return TYPE_FLOAT;
        case STRING_LITERAL: return TYPE_STRING;

        case IDENTIFIER: {
            t_avl_node *node = symtable_search(tab, expr->token->value.identifier);
            if (!node)
                semantic_error(3, 0, "Undefined variable or function");

            if (node->ifj_sym_type == SYM_VAR_LOCAL || node->ifj_sym_type == SYM_VAR_GLOBAL) {
                switch (node->ifj_data.ifj_var.ifj_type) {
                    case TYPE_NUM: return TYPE_FLOAT;
                    case TYPE_STRING: return TYPE_STRING;
                    case TYPE_NULL: return TYPE_UNKNOWN;
                    default: return TYPE_UNKNOWN;
                }
            } else if (node->ifj_sym_type == SYM_GETTER) {
                return node->ifj_data.ifj_func.ifj_return_type;
            } else {
                return TYPE_UNKNOWN;
            }
        }

        case OP_ADD: case OP_SUB: case OP_MUL: case OP_DIV: {
            t_type_kind left = eval_expr_type(expr->left, tab);
            t_type_kind right = eval_expr_type(expr->right, tab);

            if (left != right)
                semantic_error(6, 0, "Type mismatch in arithmetic operation");

            return left;
        }

        default: return TYPE_UNKNOWN;
    }
}

// Kontrola volania funkcie

void check_function_call(const char *name, t_ast_node *args, t_symtable *tab) {
    t_avl_node *func = symtable_search(tab, name);
    if (!func)
        semantic_error(3, 0, "Call to undefined function");

    if (func->ifj_sym_type != SYM_FUNCTION && func->ifj_sym_type != SYM_GETTER)
        semantic_error(6, 0, "Trying to call a variable as a function");

    if (func->ifj_sym_type == SYM_GETTER && args != NULL)
        semantic_error(6, 0, "Getter called with arguments");

    if (func->ifj_sym_type == SYM_FUNCTION) {
        int count = 0;
        t_ast_node *arg_node = args;
        while (arg_node) {
            t_type_kind arg_type = eval_expr_type(arg_node->left, tab);
            if (count >= func->ifj_data.ifj_func.ifj_param_count)
                semantic_error(5, 0, "Too many arguments in function call");

            if (func->ifj_data.ifj_func.ifj_param_types &&
                func->ifj_data.ifj_func.ifj_param_types[count] != TYPE_UNKNOWN &&
                func->ifj_data.ifj_func.ifj_param_types[count] != arg_type)
                semantic_error(6, 0, "Argument type mismatch in function call");

            count++;
            arg_node = arg_node->right;
        }

        if (count < func->ifj_data.ifj_func.ifj_param_count)
            semantic_error(5, 0, "Too few arguments in function call");
    }
}

// Rekurzívna semantická kontrola AST

static void semantic_pass(t_ast_node *node, t_symtable *tab) {
    if (!node || !node->token) return;

    switch (node->token->type) {

        case KEYWORD:
            if (node->token->value.keyword == KW_STATIC) {
                // predpoklad: node->left obsahuje IDENTIFIER s menom funkcie
                if (node->left && node->left->token && node->left->token->type == IDENTIFIER) {
                    const char *func_name = node->left->token->value.identifier;

                    // vytvor symbol funkcie v symtable
                    symtable_insert_func(tab, func_name, 0, NULL, TYPE_UNKNOWN);
                }
            }
            break;

        case OP_ASSIGN: {
            if (!node->left || node->left->token->type != IDENTIFIER) return;

            const char *var_name = node->left->token->value.identifier;
            t_avl_node *lhs = symtable_search(tab, var_name);
            if (!lhs)
                semantic_error(3, 0, "Assignment to undefined variable");

            t_type_kind rhs_type = eval_expr_type(node->right, tab);

            if (lhs->ifj_sym_type == SYM_VAR_LOCAL || lhs->ifj_sym_type == SYM_VAR_GLOBAL) {
                if (lhs->ifj_data.ifj_var.ifj_type != TYPE_UNKNOWN &&
                    lhs->ifj_data.ifj_var.ifj_type != (e_data_type)rhs_type)
                    semantic_error(6, 0, "Type mismatch in assignment");

                lhs->ifj_data.ifj_var.ifj_type = (e_data_type)rhs_type;
            } else if (lhs->ifj_sym_type == SYM_SETTER) {
                // Setter can be assigned value of correct type
                if (lhs->ifj_data.ifj_func.ifj_param_count != 1 ||
                    lhs->ifj_data.ifj_func.ifj_param_types[0] != (e_data_type)rhs_type)
                    semantic_error(6, 0, "Setter type mismatch");
            } else {
                semantic_error(6, 0, "Cannot assign to this symbol");
            }

            break;
        }

        case IDENTIFIER: {
            if (node->right && node->right->token && node->right->token->type == LEFT_PAREN)
                check_function_call(node->token->value.identifier, node->right->left, tab);
            break;
        }

        default:
            semantic_pass(node->left, tab);
            semantic_pass(node->right, tab);
            break;
    }
}

// Hlavná funkcia semantickej kontroly

void semantic_check(t_ast_node *root) {
    t_symtable symtab;
    symtable_init(&symtab);

    semantic_pass(root, &symtab);

    symtable_destroy(&symtab);
}
