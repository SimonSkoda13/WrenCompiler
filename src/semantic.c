/*
 *
 * Prekladač jazyka IFJ2ERR_SEM_PARAMS
 * VUT FIT
 *
 * Autor: Jaroslav Vrbiniak (xvrbinj00)
 *
 */

#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Pomocná funkcia ktorá prevedie typ e_data_type na vnútorný typ t_type_kind.
 */
static t_type_kind from_e_data_type(e_data_type d) {
    switch (d) {
        case TYPE_NUM:      return TYPE_FLOAT;
        case TYPE_STRING:   return TYPE_STRING;
        case TYPE_NULL:     return TYPE_NULL;
        case TYPE_UNKNOWN: 
        default:            return TYPE_UNKNOWN;
    }
}

/*
 * Opačná konverzia ako hore.
 * Používa sa pri porovnávaní typov v semantickej analýze.
 * Zjednocuje INT + FLOAT => NUM (TYPE_NUM).
 */
static e_data_type to_e_data_type(t_type_kind t) {
    switch (t) {
        case TYPE_INT:      return TYPE_NUM;
        case TYPE_FLOAT:    return TYPE_NUM;
        case TYPE_STRING:   return TYPE_STRING;
        case TYPE_NULL:     return TYPE_NULL;
        default:            return TYPE_UNKNOWN;
    }
}

/*
 * Rekurzívne určuje typ výrazu.
 *   - Literály majú svoj typ
 *   - Identifikátor si typ berie zo symtable
 *   - Binárne operácie (+ - * /) kontrolujú kompatibilitu operandov
 */
t_type_kind eval_expr_type(t_ast_node *expr, t_symtable *tab) {
    if (!expr || !expr->token) return TYPE_UNKNOWN;

    switch (expr->token->type) {

        case NUM_INT:           return TYPE_INT;
        case NUM_FLOAT:         return TYPE_FLOAT;
        case STRING_LITERAL:    return TYPE_STRING;

        case IDENTIFIER: {
            if (!tab) return TYPE_UNKNOWN;

            t_avl_node *node = symtable_search(tab, expr->token->value.identifier);
            if (!node) {
                exit_with_error(ERR_SEM_UNDEF, "Undefined variable or function on line %d", semantic_scanner->line);
                return TYPE_UNKNOWN;
            }

            if (node->ifj_sym_type == SYM_VAR_LOCAL || node->ifj_sym_type == SYM_VAR_GLOBAL) {
                return from_e_data_type(node->ifj_data.ifj_var.ifj_type);
            }
            if (node->ifj_sym_type == SYM_FUNCTION || node->ifj_sym_type == SYM_GETTER) {
                return from_e_data_type(node->ifj_data.ifj_func.ifj_return_type);
            }

            return TYPE_UNKNOWN;
        }

        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV: {
            // rekurzia 
            t_type_kind left = eval_expr_type(expr->left, tab);
            t_type_kind right = eval_expr_type(expr->right, tab);

            // neznámy typ = už predtým nahlásená chyba
            if (left == TYPE_UNKNOWN || right == TYPE_UNKNOWN) return TYPE_UNKNOWN;

            // implicitná konverzia INT <-> FLOAT -> NUM
            if ((left == TYPE_INT && right == TYPE_FLOAT) || (left == TYPE_FLOAT && right == TYPE_INT))
                return TYPE_FLOAT;

            // obidva rovnaké číselné typy
            if (left == right && (left == TYPE_INT || left == TYPE_FLOAT))
                return left;

            // nepovolené typy – napr. String + Num
            exit_with_error(ERR_SEM_TYPE_COMPAT, "Type mismatch in arithmetic operation %d", semantic_scanner->line);
            return TYPE_UNKNOWN;
        }

        default:
            return TYPE_UNKNOWN;
    }
}

/*
 * Kontrola volania funkcie.
 * Berie:
 *   - názov funkcie
 *   - zoznam argumentov 
 *   - symtable
 *
 * Ošetruje:
 *   - existenciu funkcie
 *   - volanie getterov
 *   - počet parametrov
 *   - typy parametrov
 */
void check_function_call(const char *name, t_ast_node *args, t_symtable *tab) {
    if (!tab) return;

    t_avl_node *func = symtable_search(tab, name);

    if (!func) {
        exit_with_error(ERR_SEM_UNDEF, "Call to undefined function %d", semantic_scanner->line);
        return;
    }

    if (func->ifj_sym_type != SYM_FUNCTION && func->ifj_sym_type != SYM_GETTER) {
        exit_with_error(ERR_SEM_TYPE_COMPAT, "Trying to call a non-function symbol %d", semantic_scanner->line);
        return;
    }

    if (func->ifj_sym_type == SYM_GETTER && args != NULL) {
        exit_with_error(ERR_SEM_TYPE_COMPAT, "Getter called with arguments %d", semantic_scanner->line);
        return;
    }

    if (func->ifj_sym_type == SYM_FUNCTION) {
        int count = 0;
        t_ast_node *arg_node = args;

        while (arg_node) {
            t_type_kind arg_type = eval_expr_type(arg_node->left, tab);

            if (count >= func->ifj_data.ifj_func.ifj_param_count) {
                exit_with_error(ERR_SEM_PARAMS, "Too many arguments in function call %d", semantic_scanner->line);
                return;
            }

            if (func->ifj_data.ifj_func.ifj_param_types) {
                e_data_type expected = func->ifj_data.ifj_func.ifj_param_types[count];

                if (expected != TYPE_UNKNOWN &&
                    expected != to_e_data_type(arg_type))
                {
                    exit_with_error(ERR_SEM_TYPE_COMPAT, "Argument type mismatch in function call %d", semantic_scanner->line);
                    return;
                }
            }

            count++;
            arg_node = arg_node->right;
        }

        if (count < func->ifj_data.ifj_func.ifj_param_count) {
            exit_with_error(ERR_SEM_PARAMS, "Too few arguments in function call %d", semantic_scanner->line);
            return;
        }
    }
}

/*
 * Rekurzívna semantická kontrola AST.
 * Patroluje celý strom a kontroluje príkazy:
 *   - priradenie
 *   - používanie identifikátorov
 *   - volanie funkcií
 */
static void semantic_pass(t_ast_node *node, t_symtable *tab) {
    if (!node || !node->token) return;

    switch (node->token->type) {

        case KEYWORD:
            if (node->token->value.keyword == KW_VAR) {
                const char *var_name = node->left->token->value.identifier;
                t_avl_node *existing = symtable_search(tab, var_name);
                if (existing != NULL) {
                    exit_with_error(ERR_SEM_REDEF, "Variable '%s' redefined on line %d", var_name, semantic_scanner->line);
                }
            }
            break;

        case IDENTIFIER: {
            // Redefinícia funkcie (ak deklarácia)
            if (node->left && node->left->token && node->left->token->type == LEFT_PAREN) {
                const char *func_name = node->token->value.identifier;
                t_avl_node *existing_func = symtable_search(tab, func_name);
                if (existing_func != NULL) {
                    exit_with_error(ERR_SEM_REDEF, "Function '%s' redefined on line %d", func_name, semantic_scanner->line);
                }
            }

            // Volanie funkcie alebo čítanie premennej
            if (node->right && node->right->token && node->right->token->type == LEFT_PAREN) {
                check_function_call(node->token->value.identifier, node->right->left, tab);
            } else {
                t_avl_node *sym = symtable_search(tab, node->token->value.identifier);
                if (!sym)
                    exit_with_error(ERR_SEM_UNDEF, "Use of undefined variable or function %d", semantic_scanner->line);
            }
            break;
        }

        case OP_ASSIGN: {

            if (!node->left || !node->left->token ||
                node->left->token->type != IDENTIFIER)
            {
                exit_with_error(ERR_SEM_TYPE_COMPAT, "Left side of assignment must be identifier %d", semantic_scanner->line);
                break;
            }

            const char *var_name = node->left->token->value.identifier;
            t_avl_node *lhs = symtable_search(tab, var_name);

            if (!lhs) {
                exit_with_error(ERR_SEM_UNDEF, "Assignment to undefined variable %d", semantic_scanner->line);
                break;
            }

            if (lhs->ifj_sym_type == SYM_VAR_LOCAL || lhs->ifj_sym_type == SYM_VAR_GLOBAL) {

                t_type_kind rhs_type = eval_expr_type(node->right, tab);
                if (rhs_type == TYPE_UNKNOWN) break;

                e_data_type lhs_type = lhs->ifj_data.ifj_var.ifj_type;

                if (lhs_type != TYPE_UNKNOWN) {
                    e_data_type rhs_e = to_e_data_type(rhs_type);

                    if (lhs_type == TYPE_NUM && rhs_e == TYPE_NUM) {
                        // OK – NUM si rozumie s INT aj FLOAT
                    } else if (lhs_type == rhs_e) {
                        // presná zhoda takze OK
                    } else {
                        exit_with_error(ERR_SEM_TYPE_COMPAT, "Type mismatch in assignment %d", semantic_scanner->line);
                    }
                } else {
                    // určujeme typ premennej
                    lhs->ifj_data.ifj_var.ifj_type = to_e_data_type(rhs_type);
                }

            } else if (lhs->ifj_sym_type == SYM_SETTER) {

                if (lhs->ifj_data.ifj_func.ifj_param_count != 1) {
                    exit_with_error(ERR_SEM_TYPE_COMPAT, "Setter has invalid arity %d", semantic_scanner->line);
                } else {
                    t_type_kind rhs_type = eval_expr_type(node->right, tab);
                    e_data_type expected = lhs->ifj_data.ifj_func.ifj_param_types[0];

                    if (expected != TYPE_UNKNOWN && expected != to_e_data_type(rhs_type)) {
                        exit_with_error(ERR_SEM_TYPE_COMPAT, "Setter type mismatch %d", semantic_scanner->line);
                    }
                }

            } else {
                exit_with_error(ERR_SEM_TYPE_COMPAT, "Cannot assign to this symbol %d", semantic_scanner->line);
            }

            break;
        }

        default:
            // rekurzívne prejdeme obidva podstromy
            semantic_pass(node->left, tab);
            semantic_pass(node->right, tab);
            break;
    }
}

/*
 * Hlavný vstup do semantiky.
 * Požaduje už inicializovanú symtable.
 * Ak by tam nebola, radšej vypíšeme upozornenie, ako spôsobiť segfault 
 */
void semantic_check(t_ast_node *root, t_symtable *tab) {
    if (!tab) {
        fprintf(stderr, "semantic_check: symbol table is NULL. Call symtable_init before.\n");
        return;
    }
    semantic_pass(root, tab);
}