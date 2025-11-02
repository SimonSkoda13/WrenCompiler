/*
 * Prekladač jazyka IFJ25
 * VUT FIT
 *
 * Autori:
 *   - Martin Michálik (xmicham00)
 */


#include "expression.h"

//DEBUG
void stack_print(t_stack *stack) {
    if (stack == NULL || stack->top == NULL) {
        printf("Stack: [empty]\n");
        return;
    }
    
    printf("Stack (top -> bottom): ");
    t_stack_exp current = stack->top;
    
    while (current != NULL && current->data != PREC_EMPTY) {
        switch (current->data) {
            case PREC_LPAR:      printf("( "); break;
            case PREC_RPAR:      printf(") "); break;
            case PREC_IDENT:     printf("i "); break;
            case PREC_PLUS:      printf("+ "); break;
            case PREC_MINUS:     printf("- "); break;
            case PREC_MUL:       printf("* "); break;
            case PREC_DIV:       printf("/ "); break;
            case PREC_LESS:      printf("< "); break;
            case PREC_LESSEQ:    printf("<= "); break;
            case PREC_GREATER:   printf("> "); break;
            case PREC_GREATEREQ: printf(">= "); break;
            case PREC_EQ:        printf("== "); break;
            case PREC_NEQ:       printf("!= "); break;
            case PREC_IS:        printf("is "); break;
            case PREC_DOLLAR:    printf("$ "); break;
            case S:              printf("<SHIFT> "); break;
            case PREC_E:         printf("E "); break;
            default:             printf("? "); break;
        }
        current = current->next_element;
    }
    
    printf("$bottom\n");
}


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
            return PREC_DOLLAR; // if statement if(expr) {...}
        case EOL:
            return PREC_DOLLAR; // Môžem jebať multiline výrazy v tomto farizejskom jazyku
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
    //
    return (stack->top != NULL && 
            stack->top->data == PREC_E &&
            stack->top->next_element != NULL &&
            stack->top->next_element->data == PREC_DOLLAR &&
            stack->top->next_element->next_element != NULL &&
            stack->top->next_element->next_element->data == PREC_EMPTY);
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
// E -> E op E, op are operators in precedence table
if (stack_data[0]->data == PREC_E && stack_data[2]->data == PREC_E) {
    t_ast_node *tree_ptr;
    t_token *operator = malloc(sizeof(t_token));
    if (operator == NULL) {
        exit_with_error(99, "Memory allocation failed in rule_reduction");
    }
    switch (stack_data[1]->data) {
        case PREC_PLUS:
            operator->type = OP_ADD;
            tree_ptr = ast_create(operator, stack_data[2]->tree,
                                    stack_data[0]->tree);
            break;
        case PREC_MUL:
            operator->type = OP_MUL;
            tree_ptr = ast_create(operator, stack_data[2]->tree,
                                    stack_data[0]->tree);
            break;
        case PREC_DIV:
            operator->type = OP_DIV;
            tree_ptr = ast_create(operator, stack_data[2]->tree,
                                    stack_data[0]->tree);
            break;
        case PREC_GREATER:
            operator->type = OP_GREATER_THAN;
            tree_ptr = ast_create(operator, stack_data[2]->tree,
                                    stack_data[0]->tree);
            break;
        case PREC_GREATEREQ:
            operator->type = OP_GREATER_THAN_EQUAL;
            tree_ptr = ast_create(operator, stack_data[2]->tree,
                                    stack_data[0]->tree);
            break;
        case PREC_LESS:
            operator->type = OP_LESS_THAN;
            tree_ptr = ast_create(operator, stack_data[2]->tree,
                                    stack_data[0]->tree);
            break;
        case PREC_LESSEQ:
            operator->type = OP_LESS_EQUAL;
            tree_ptr = ast_create(operator, stack_data[2]->tree,
                                    stack_data[0]->tree);
            break;
        case PREC_MINUS:
            operator->type = OP_SUB;
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

    t_token current_token_storage;  // Local storage for token
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
    // For debugging
    // stack_print(&stack);
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
                //TODO: Emtpy if statement edge-case
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
        //TODO: handle expression na multiple lines
        *tree = stack.top->tree;
        return 0;
    }
    return 2;
}