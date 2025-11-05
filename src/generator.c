/*
 * Prekladač jazyka IFJ25
 * VUT FIT
 *
 * Autori:
 *   - Martin Michálik (xmicham00)
 */

#include "generator.h"

int must_escape(unsigned char c) {
    return (c <= 32 || c == 35 || c == 92);
}

size_t calculate_escaped_length(const char *input) {
    size_t len = 0;
    const unsigned char *str = (const unsigned char *)input;
    
    while (*str) {
        if (must_escape(*str)) {
            len += 4; // \xyz (4 znaky)
        } else {
            len += 1;
        }
        str++;
    }
    
    return len;
}

char* convert_to_escaped_string(const char *input) {
    if (input == NULL) {
        return NULL;
    }
    
    size_t escaped_len = calculate_escaped_length(input);
    char *output = (char *)malloc(escaped_len + 1); // +1 pro '\0'
    
    if (output == NULL) {
        return NULL; // Chyba alokace
    }
    
    const unsigned char *src = (const unsigned char *)input;
    char *dst = output;
    
    while (*src) {
        if (must_escape(*src)) {
            // Vytvoř escape sekvenci \xyz
            sprintf(dst, "\\%03d", *src);
            dst += 4;
        } else {
            // Zkopíruj znak bez změny
            *dst = *src;
            dst++;
        }
        src++;
    }
    
    *dst = '\0'; // Ukončovací nula
    return output;
}

void generate_header() {
    printf(".IFJcode25\n");
    printf("JUMP $$main\n");
}

void builtin_write_string_literal(char *string) {
    char *escaped_string = convert_to_escaped_string(string);
    if (escaped_string == NULL) {
        exit_with_error(ERR_INTERNAL, "Internal error: Memory allocation failed in builtin_write_literal");
    } else {
        printf("WRITE string@%s\n", escaped_string);
        free(escaped_string);
    }
}

void builtin_write_float_literal(double number) {
    printf("WRITE float@%a\n", number);
}

void builtin_write_integer_literal(long long number) {
    printf("WRITE int@%lld\n", number);
}

void builtin_write_var(char *var_id) {
    printf("WRITE LF@%s\n", var_id);
}

void generate_var_declaration(const char *var_name) {
    printf("DEFVAR LF@%s\n", var_name);
}

void generate_function_start(const char *func_name) {
    // Vygenerujeme label pre funkciu
    if (strcmp(func_name, "main") == 0) {
        printf("LABEL $$main\n");
    } else {
        printf("LABEL $$%s\n", func_name);
    }
    
    // Vytvoríme a aktivujeme lokálny rámec
    printf("CREATEFRAME\n");
    printf("PUSHFRAME\n");
}

void generate_function_end(const char *func_name) {
    printf("POPFRAME\n");
    if (strcmp(func_name, "main") != 0) {
        printf("RETURN\n");   
    }
}

// Pomocná premenná pre generovanie dočasných premenných
static int temp_var_counter = 0;

/**
 * @brief Pomocná funkcia pre generovanie názvu dočasnej premennej
 * @return Číslo dočasnej premennej
 */
int get_next_temp_var() {
    return temp_var_counter++;
}

/**
 * @brief Pomocná funkcia pre vytvorenie hodnoty (literálu alebo premennej) pre inštrukcie
 * @param node AST uzol
 * @param result Buffer pre výsledok (formát "int@123", "string@...", "LF@var")
 * @param result_size Veľkosť bufferu
 */
void get_value_string(t_ast_node *node, char *result, size_t result_size) {
    if (node == NULL || node->token == NULL) {
        result[0] = '\0';
        return;
    }
    
    switch (node->token->type) {
        case NUM_INT:
        case NUM_EXP_INT:
            snprintf(result, result_size, "int@%ld", node->token->value.number_int);
            break;
        case NUM_FLOAT:
        case NUM_EXP_FLOAT:
            snprintf(result, result_size, "float@%a", node->token->value.number_float);
            break;
        case STRING_LITERAL: {
            char *escaped = convert_to_escaped_string(node->token->value.string);
            if (escaped) {
                snprintf(result, result_size, "string@%s", escaped);
                free(escaped);
            } else {
                result[0] = '\0';
            }
            break;
        }
        case IDENTIFIER:
        case GLOBAL_VAR:
            snprintf(result, result_size, "LF@%s", node->token->value.string);
            break;
        default:
            result[0] = '\0';
            break;
    }
}

/**
 * @brief Rekurzívna funkcia pre generovanie kódu z AST
 * @param node Aktuálny AST uzol
 * @param result_var Názov premennej kde sa uloží výsledok (formát: "LF@__tmp0")
 * @return 0 pri úspechu, inak error kód
 */
int generate_expression_code(t_ast_node *node, char *result_var, size_t result_var_size) {
    if (node == NULL) {
        return ERR_INTERNAL;
    }
    
    // Ak je to list (literál alebo premenná), jednoducho skopírujeme hodnotu
    if (node->left == NULL && node->right == NULL) {
        get_value_string(node, result_var, result_var_size);
        return 0;
    }
    
    // Vytvoríme dočasné premenné pre operandy
    char left_var[256] = {0};
    char right_var[256] = {0};
    
    // Spracujeme ľavý podstrom
    if (node->left != NULL) {
        generate_expression_code(node->left, left_var, sizeof(left_var));
        
        // Ak ľavý operand nie je jednoduchá hodnota, musíme ho uložiť do temp premennej
        if (node->left->left != NULL || node->left->right != NULL) {
            int temp_num = get_next_temp_var();
            char temp_name[256];
            snprintf(temp_name, sizeof(temp_name), "LF@__tmp%d", temp_num);
            printf("DEFVAR %s\n", temp_name);
            printf("MOVE %s %s\n", temp_name, left_var);
            strncpy(left_var, temp_name, sizeof(left_var) - 1);
        }
    }
    
    // Spracujeme pravý podstrom
    if (node->right != NULL) {
        generate_expression_code(node->right, right_var, sizeof(right_var));
        
        // Ak pravý operand nie je jednoduchá hodnota, musíme ho uložiť do temp premennej
        if (node->right->left != NULL || node->right->right != NULL) {
            int temp_num = get_next_temp_var();
            char temp_name[256];
            snprintf(temp_name, sizeof(temp_name), "LF@__tmp%d", temp_num);
            printf("DEFVAR %s\n", temp_name);
            printf("MOVE %s %s\n", temp_name, right_var);
            strncpy(right_var, temp_name, sizeof(right_var) - 1);
        }
    }
    
    // Vytvoríme premennú pre výsledok tejto operácie
    int result_temp_num = get_next_temp_var();
    snprintf(result_var, result_var_size, "LF@__tmp%d", result_temp_num);
    printf("DEFVAR %s\n", result_var);
    
    // Vygenerujeme inštrukciu podľa typu operátora
    switch (node->token->type) {
        case OP_ADD:
            printf("ADD %s %s %s\n", result_var, left_var, right_var);
            break;
        case OP_SUB:
            printf("SUB %s %s %s\n", result_var, left_var, right_var);
            break;
        case OP_MUL:
            printf("MUL %s %s %s\n", result_var, left_var, right_var);
            break;
        case OP_DIV:
            printf("DIV %s %s %s\n", result_var, left_var, right_var);
            break;
        case OP_LESS_THAN:
            printf("LT %s %s %s\n", result_var, left_var, right_var);
            break;
        case OP_GREATER_THAN:
            printf("GT %s %s %s\n", result_var, left_var, right_var);
            break;
        case OP_EQUALS:
            printf("EQ %s %s %s\n", result_var, left_var, right_var);
            break;
        case OP_NOT_EQUALS:
            // NEQ = NOT(EQ)
            printf("EQ %s %s %s\n", result_var, left_var, right_var);
            printf("NOT %s %s\n", result_var, result_var);
            break;
        case OP_LESS_EQUAL:
            // LE = NOT(GT)
            printf("GT %s %s %s\n", result_var, left_var, right_var);
            printf("NOT %s %s\n", result_var, result_var);
            break;
        case OP_GREATER_THAN_EQUAL:
            // GE = NOT(LT)
            printf("LT %s %s %s\n", result_var, left_var, right_var);
            printf("NOT %s %s\n", result_var, result_var);
            break;
        default:
            return ERR_INTERNAL;
    }
    
    return 0;
}

void generate_assignment(const char *var_name, t_ast_node *ast) {
    if (ast == NULL) {
        exit_with_error(ERR_INTERNAL, "Internal error: NULL AST in generate_assignment");
    }
    
    // Ak je to jednoduchý literál alebo premenná, priamo priradíme
    if (ast->left == NULL && ast->right == NULL) {
        char value_str[512];
        get_value_string(ast, value_str, sizeof(value_str));
        printf("MOVE LF@%s %s\n", var_name, value_str);
    } else {
        // Komplexný výraz - použijeme generate_expression_code
        char result_var[256];
        int err = generate_expression_code(ast, result_var, sizeof(result_var));
        if (err) {
            exit_with_error(ERR_INTERNAL, "Internal error: Failed to generate expression code");
        }
        // Presunieme výsledok do cieľovej premennej
        printf("MOVE LF@%s %s\n", var_name, result_var);
    }
}
