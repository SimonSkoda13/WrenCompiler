/*
 * Prekladač jazyka IFJ25
 * VUT FIT
 *
 * Autori:
 *   - Martin Michálik (xmicham00)
 *   - Šimon Škoda (xskodas00)
 */


#include "expression.h"

prec_symbols precedence_table[15][15] = {
    // (    )    i    +    -    *    /    <    <=   >    >=   ==   !=   is   $
    {  S,   W,   S,   S,   S,   S,   S,   S,   S,   S,   S,   S,   S,   S,   ERR}, // (
    {ERR,   R, ERR,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,     R}, // )
    {ERR,   R, ERR,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,     R}, // i
    {  S,   R,   S,   R,   R,   S,   S,   R,   R,   R,   R,   R,   R,   R,     R}, // +
    {  S,   R,   S,   R,   R,   S,   S,   R,   R,   R,   R,   R,   R,   R,     R}, // -
    {  S,   R,   S,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,     R}, // *
    {  S,   R,   S,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,   R,     R}, // /
    {  S,   R,   S,   S,   S,   S,   S, ERR, ERR, ERR, ERR, ERR, ERR, ERR,     R}, // <
    {  S,   R,   S,   S,   S,   S,   S, ERR, ERR, ERR, ERR, ERR, ERR, ERR,     R}, // <=
    {  S,   R,   S,   S,   S,   S,   S, ERR, ERR, ERR, ERR, ERR, ERR, ERR,     R}, // >
    {  S,   R,   S,   S,   S,   S,   S, ERR, ERR, ERR, ERR, ERR, ERR, ERR,     R}, // >=
    {  S,   R,   S,   S,   S,   S,   S, ERR, ERR, ERR, ERR, ERR, ERR, ERR,     R}, // ==
    {  S,   R,   S,   S,   S,   S,   S, ERR, ERR, ERR, ERR, ERR, ERR, ERR,     R}, // !=
    {  S,   R,   S,   S,   S,   S,   S, ERR, ERR, ERR, ERR, ERR, ERR, ERR,     R}, // is
    {  S, ERR,   S,   S,   S,   S,   S,   S,   S,   S,   S,   S,   S,   S,   ERR}  // $
};

prec_symbols map_token_to_enum(t_token *token) {
    switch (token->type) {
        case LEFT_PAREN:
            return PREC_LPAR;
        case RIGHT_PAREN:
            return PREC_RPAR;
        case GLOBAL_VAR:
            return PREC_IDENT;
        case IDENTIFIER:
            return PREC_IDENT;
        case NUM_INT:
            return PREC_IDENT;
        case NUM_FLOAT:
            return PREC_IDENT;
        case NUM_HEX:
            return PREC_IDENT;
        case NUM_EXP_FLOAT:
            return PREC_IDENT;
        case NUM_EXP_INT:
            return PREC_IDENT;
        case STRING_OR_MULTILINE:
            return PREC_IDENT;
        case STRING_LITERAL:
            return PREC_IDENT;
        case OP_ADD:
            return PREC_PLUS;
        case OP_SUB:
            return PREC_MINUS;
        case OP_MUL:
            return PREC_MUL;
        case OP_DIV:
            return PREC_DIV;
        case OP_LESS_THAN:
            return PREC_LESS;
        case OP_LESS_EQUAL:
            return PREC_LESSEQ;
        case OP_GREATER_THAN:
            return PREC_GREATER;
        case OP_GREATER_THAN_EQUAL:
            return PREC_GREATEREQ;
        case OP_EQUALS:
            return PREC_EQ;
        case OP_NOT_EQUALS:
            return PREC_NEQ;
        case KEYWORD: {
            if (token->value.keyword == KW_IS) {
                return PREC_IS;
            } else if (token->value.keyword == KW_NULL_TYPE ||
                       token->value.keyword == KW_NULL_INST ||
                       token->value.keyword == KW_NUM ||
                       token->value.keyword == KW_STRING) {
                return PREC_IDENT;
            } else {
                return PREC_INVALID;
            }
        }
        case LEFT_BRACE:
            return PREC_DOLLAR;
        case EOL:
            return PREC_DOLLAR;
        default:
            // Neznámy token
            return PREC_INVALID;
    }
}

void stack_init(t_stack* stack) {
    stack->top = NULL;
}

prec_symbols stack_top_terminal(t_stack* stack) {
    t_stack_exp current = stack->top;
    // Ignorujeme všetky neterminály na vrchu zásobníka
    while (current->data != PREC_EMPTY) {
        if (current->data >= PREC_LPAR && current->data <= PREC_DOLLAR) {
            return current->data;
        }
        current = current->next_element;
    }
    return PREC_EMPTY;
}

prec_symbols stack_top(t_stack *stack) {
    return stack->top->data;
}

void stack_push(t_stack *stack, prec_symbols data) {
    t_stack_exp new_element = malloc(sizeof(struct stack_el));
    if (new_element == NULL) {
    exit_with_error(ERR_INTERNAL, "Memory allocation failed in stack_push");
    }
    
    new_element->data = data;
    if (stack->top != NULL) {
    new_element->next_element = stack->top;
    } else {
    new_element->next_element = NULL;
    }
    
    stack->top = new_element;
}

void stack_shift_push(t_stack *stack) {
    t_stack_exp tmp;
    t_stack_exp prev;

    tmp = stack->top;
    prev = NULL;

    while (tmp->data != PREC_EMPTY) {
        if (tmp->data >= PREC_LPAR && tmp->data <= PREC_DOLLAR) {
            t_stack_exp new_element = malloc(sizeof(struct stack_el));
            if (new_element == NULL) {
            exit_with_error(ERR_INTERNAL, "Memory allocation failed in stack_push");
            }
            new_element->data = S;
            new_element->tree = NULL;
            if (prev != NULL) {
                new_element->next_element = prev->next_element;
                prev->next_element = new_element;
                return;
            } else {
                new_element->next_element = tmp;
                stack->top = new_element;
                return;
            }
            return;
        }
        prev = tmp;
        tmp = tmp->next_element;
    }
}

t_stack_exp stack_pop(t_stack *stack) {
    if (stack->top->data != PREC_EMPTY) {
        t_stack_exp popped_element = stack->top;
        stack->top = stack->top->next_element;
        return popped_element;
    }
    return NULL;
}

int stack_empty(t_stack *stack) {
    return stack->top->data == PREC_EMPTY;
}

int correct_syntax(t_stack *stack) {
    return (stack->top != NULL && 
            stack->top->data == PREC_E &&
            stack->top->next_element != NULL &&
            stack->top->next_element->data == PREC_DOLLAR &&
            stack->top->next_element->next_element != NULL &&
            stack->top->next_element->next_element->data == PREC_EMPTY);
}

// Pomocná funkcia na kontrolu, či je uzol literál
int is_literal(t_ast_node *node) {
    if (node == NULL || node->token == NULL) {
        return 0;
    }
    // Literál je listový uzol (bez detí) a je typu literálu
    return (node->left == NULL && node->right == NULL &&
            (node->token->type == NUM_INT || 
             node->token->type == NUM_EXP_INT ||
             node->token->type == NUM_HEX ||
             node->token->type == NUM_FLOAT ||
             node->token->type == NUM_EXP_FLOAT ||
             node->token->type == STRING_LITERAL ||
             (node->token->type == KEYWORD && node->token->value.keyword == KW_NULL_INST)));
}

// Pomocná funkcia na získanie kategórie typu literálu: 0=neznámy, 1=číslo, 2=reťazec, 3=null
int get_literal_type_category(t_ast_node *node) {
    if (!is_literal(node)) {
        return 0; // Nie je literál
    }
    
    if (node->token->type == NUM_INT || 
             node->token->type == NUM_EXP_INT ||
             node->token->type == NUM_HEX ||
             node->token->type == NUM_FLOAT ||
             node->token->type == NUM_EXP_FLOAT) {
        return 1; // Number
    } else if (node->token->type == STRING_LITERAL) {
        return 2; // String
    } else if (node->token->type == KEYWORD && node->token->value.keyword == KW_NULL_INST) {
        return 3; // Null
    }
    return 0;
}

// Pomocná funkcia na kontrolu, či je uzol platným typovým kľúčovým slovom pre operátor 'is' (Num, String alebo Null)
int is_valid_type_keyword(t_ast_node *node) {
    if (node == NULL || node->token == NULL) {
        return 0;
    }
    
    // Musí byť listový uzol (bez detí) a kľúčové slovo
    if (node->left != NULL || node->right != NULL || node->token->type != KEYWORD) {
        return 0;
    }
    
    // Musí byť jedno z typových kľúčových slov
    e_keyword kw = node->token->value.keyword;
    return (kw == KW_NUM || kw == KW_STRING || kw == KW_NULL_TYPE);
}

// Pomocná funkcia na kontrolu typov literálov pre binárne operácie
void check_binary_literal_types(e_token_type operator, t_ast_node *left, t_ast_node *right) {
    int left_type = get_literal_type_category(left);
    int right_type = get_literal_type_category(right);
    
    // Špeciálna kontrola pre operátor 'is' - pravá strana musí byť typové kľúčové slovo
    if (operator == OP_IS) {
        if (!is_valid_type_keyword(right)) {
            exit_with_error(ERR_SEM_TYPE_COMPAT, "Type error: Right operand of 'is' must be Num, String, or Null");
        }
        return;
    }
    
    // Ak ani jeden nie je literál, nemôžeme staticky kontrolovať
    if (left_type == 0 && right_type == 0) {
        return;
    }
    
    // Kontrola typov na základe operátora
    if (operator == OP_ADD) {
        // Add: string + string OK, number + number OK, string + number ERROR
        if ((left_type == 2 && right_type == 1) || (left_type == 1 && right_type == 2)) {
            exit_with_error(ERR_SEM_TYPE_COMPAT, "Type error: Cannot add string and number");
        }
    } else if (operator == OP_MUL) {
        // Multiply: number * number OK, string * number OK, number * string ERROR, string * string ERROR
        if ((left_type == 1 && right_type == 2) || (left_type == 2 && right_type == 2)) {
            exit_with_error(ERR_SEM_TYPE_COMPAT, "Type error: Invalid types for multiplication");
        }
    } else if (operator == OP_SUB || operator == OP_DIV) {
        // Subtraction and division: only number - number or number / number
        if ((left_type != 0 && left_type != 1) || (right_type != 0 && right_type != 1)) {
            exit_with_error(ERR_SEM_TYPE_COMPAT, "Type error: Subtraction and division require numeric operands");
        }
    } else if (operator == OP_LESS_THAN || operator == OP_LESS_EQUAL || 
               operator == OP_GREATER_THAN || operator == OP_GREATER_THAN_EQUAL) {
        // Relačné operátory (okrem == a !=): obe strany musia byť čísla
        if ((left_type != 0 && left_type != 1) || (right_type != 0 && right_type != 1)) {
            exit_with_error(ERR_SEM_TYPE_COMPAT, "Type error: Relational operators require numeric operands");
        }
    }
    // Pre operátory == a != povoľujeme zmiešané typy (runtime ich ošetrí)
}

int rule_reduction(t_stack *stack) {
    t_stack_exp stack_data[4];
    for (int j = 0; j < 4; j++) {
        stack_data[j] = malloc(sizeof(t_stack_exp));
        if (stack_data[j] == NULL) {
            exit_with_error(ERR_INTERNAL, "Memory allocation failed in rule_reduction");
        }
    }

    for (int i = 0; i < 4; i++) {
        stack_data[i] = stack_pop(stack);
        if ((stack_data[i])->data == S) {
            break;
        }
    }
    if (stack_data[0]->data == S) {
        return 0;
    }
// E -> i
if (stack_data[1]->data == S && stack_data[0]->data == PREC_IDENT) {
    stack_push(stack, PREC_E);
    stack->top->tree = ast_create_leaf(&stack_data[0]->token);
    stack->top->token = stack_data[0]->token;
    return 1;
}
// E -> (E)
if (stack_data[0]->data == PREC_RPAR && stack_data[1]->data == PREC_E &&
    stack_data[2]->data == PREC_LPAR) {
    stack_push(stack, PREC_E);
    stack->top->token = stack_data[1]->token;
    stack->top->tree = stack_data[1]->tree;
    return 1;
}
// E -> E op E
if (stack_data[0]->data == PREC_E && stack_data[2]->data == PREC_E) {
    t_ast_node *tree_ptr;
    t_token *operator = malloc(sizeof(t_token));
    if (operator == NULL) {
        exit_with_error(99, "Memory allocation failed in rule_reduction");
    }
    switch (stack_data[1]->data) {
        case PREC_PLUS:
            operator->type = OP_ADD;
            check_binary_literal_types(OP_ADD, stack_data[2]->tree, stack_data[0]->tree);
            tree_ptr = ast_create(operator, stack_data[2]->tree,
                                    stack_data[0]->tree);
            break;
        case PREC_MUL:
            operator->type = OP_MUL;
            check_binary_literal_types(OP_MUL, stack_data[2]->tree, stack_data[0]->tree);
            tree_ptr = ast_create(operator, stack_data[2]->tree,
                                    stack_data[0]->tree);
            break;
        case PREC_DIV:
            operator->type = OP_DIV;
            check_binary_literal_types(OP_DIV, stack_data[2]->tree, stack_data[0]->tree);
            tree_ptr = ast_create(operator, stack_data[2]->tree,
                                    stack_data[0]->tree);
            break;
        case PREC_GREATER:
            operator->type = OP_GREATER_THAN;
            check_binary_literal_types(OP_GREATER_THAN, stack_data[2]->tree, stack_data[0]->tree);
            tree_ptr = ast_create(operator, stack_data[2]->tree,
                                    stack_data[0]->tree);
            break;
        case PREC_GREATEREQ:
            operator->type = OP_GREATER_THAN_EQUAL;
            check_binary_literal_types(OP_GREATER_THAN_EQUAL, stack_data[2]->tree, stack_data[0]->tree);
            tree_ptr = ast_create(operator, stack_data[2]->tree,
                                    stack_data[0]->tree);
            break;
        case PREC_LESS:
            operator->type = OP_LESS_THAN;
            check_binary_literal_types(OP_LESS_THAN, stack_data[2]->tree, stack_data[0]->tree);
            tree_ptr = ast_create(operator, stack_data[2]->tree,
                                    stack_data[0]->tree);
            break;
        case PREC_LESSEQ:
            operator->type = OP_LESS_EQUAL;
            check_binary_literal_types(OP_LESS_EQUAL, stack_data[2]->tree, stack_data[0]->tree);
            tree_ptr = ast_create(operator, stack_data[2]->tree,
                                    stack_data[0]->tree);
            break;
        case PREC_MINUS:
            operator->type = OP_SUB;
            check_binary_literal_types(OP_SUB, stack_data[2]->tree, stack_data[0]->tree);
            tree_ptr = ast_create(operator, stack_data[2]->tree,
                                    stack_data[0]->tree);
            break;
        case PREC_EQ:
            operator->type = OP_EQUALS;
            tree_ptr = ast_create(operator, stack_data[2]->tree,
                                    stack_data[0]->tree);
            break;
        case PREC_NEQ:
            operator->type = OP_NOT_EQUALS;
            tree_ptr = ast_create(operator, stack_data[2]->tree,
                                    stack_data[0]->tree);
            break;
        case PREC_IS:
            operator->type = OP_IS;
            check_binary_literal_types(OP_IS, stack_data[2]->tree, stack_data[0]->tree);
            tree_ptr = ast_create(operator, stack_data[2]->tree,
                                    stack_data[0]->tree);
            break;

        default:
            break;
    }
    stack_push(stack, PREC_E);
    stack->top->tree = tree_ptr;
    return 1;
} 
return 0;
}

int parse_expression(t_token *token1, t_token *token2, t_ast_node **tree) {
    t_stack stack;
    stack_init(&stack);
    stack_push(&stack, PREC_EMPTY);
    int first_token_used = 0;
    int second_token = 0;
    if (token2 != NULL) {
        second_token = 1;
    }

    t_token current_token_storage;  // Local storage pre token
    t_token *current_token = &current_token_storage;
    prec_symbols current_token_enum;
    // Prvý token z parsera
    if (token1 != NULL) {
        current_token_storage = *token1;
        current_token_enum = map_token_to_enum(token1);
        if (current_token_enum == PREC_INVALID) {
            return 2;
        }
        current_token = token1;

    } else {
        get_next_token(parser.scanner, current_token);
        current_token_enum = map_token_to_enum(current_token);
        if (current_token_enum == PREC_INVALID) {
            return 2;
        }
    }
    prec_symbols top;

    stack_push(&stack, PREC_DOLLAR);

    do {
    top = stack_top_terminal(&stack);
    switch (precedence_table[top][current_token_enum]) {
        case W:  // Wait -> získame ďalší token a pushnem ten ktorý máme
            stack_push(&stack, current_token_enum);
            stack.top->token = *current_token;
            if (token2 != NULL && second_token) {
                current_token = token2;
                second_token = 0;
            } else {
                get_next_token(parser.scanner, current_token);
            }
            current_token_enum = map_token_to_enum(current_token);
            if (current_token_enum == PREC_INVALID) {
                return 2;
            }
            break;
        case S:  // shift
            stack_shift_push(&stack);
            stack_push(&stack, current_token_enum);
            stack.top->token = *current_token;
            if (token2 != NULL && second_token) {
                current_token = token2;
                second_token = 0;
            } else {
                get_next_token(parser.scanner, current_token);
            }
            if (!first_token_used) {
                // Ak sme práve začali parsovať expression
                first_token_used = 1;
            }
            current_token_enum = map_token_to_enum(current_token);
            if (current_token_enum == PREC_INVALID) {
                return 2;
            }
            break;
        case R:  // reduce
            if (!rule_reduction(&stack)) {
                return 2;
            }
            break;
        case ERR:
            return 2;
            break;
        default:
            return 2;
            break;
        }
    } while ((current_token_enum != PREC_DOLLAR) || (correct_syntax(&stack) == 0));
    if (correct_syntax(&stack)) {
        *tree = stack.top->tree;
        return 0;
    }
    return 2;
}