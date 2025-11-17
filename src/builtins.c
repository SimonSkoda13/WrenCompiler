/*
 * Prekladač jazyka IFJ25
 * VUT FIT
 *
 * Autori:
 *   - Matúš Magyar (xmagyam00)
 *   - Martin Michálik (xmicham00)
 */

#include "builtins.h"
#include "generator.h"
#include "symtable.h"
#include "errors.h"
#include "scanner.h"
#include <stdio.h>
#include <string.h>

/**
 * @brief Konvertuje AST uzol na IFJcode25 formát (int@42, string@hello, LF@var, atď.)
 * @param node AST uzol s hodnotou
 * @param result Buffer pre výsledok
 * @param result_size Veľkosť bufferu
 */
static void get_param_value(t_ast_node *node, char *result, size_t result_size)
{

    if (node == NULL || node->token == NULL)
    {
        result[0] = '\0';
        return;
    }

    e_token_type type = node->token->type;

    switch (type)
    {
    case NUM_INT:
    case NUM_EXP_INT:
        snprintf(result, result_size, "int@%ld", node->token->value.number_int);
        break;

    case NUM_FLOAT:
    case NUM_EXP_FLOAT:
        snprintf(result, result_size, "float@%a", node->token->value.number_float);
        break;

    case STRING_LITERAL:
    {
        char *escaped = convert_to_escaped_string(node->token->value.string);
        if (escaped)
        {
            snprintf(result, result_size, "string@%s", escaped);
            free(escaped);
        }
        else
        {
            result[0] = '\0';
        }
        break;
    }

    case IDENTIFIER:
    {
        // Najprv skontrolujeme, či ide o getter
        char *getter_mangled = mangle_getter_name(node->token->value.string);
        if (getter_mangled != NULL)
        {
            t_avl_node *getter_node = symtable_search(get_global_symtable(), getter_mangled);
            if (getter_node != NULL && getter_node->ifj_sym_type == SYM_GETTER)
            {
                // Je to getter - vygenerujeme volanie a výsledok uložíme do dočasnej premennej
                int temp_id = get_next_temp_var();
                char temp_var[64];
                snprintf(temp_var, sizeof(temp_var), "__getter_result_%d", temp_id);

                // Pridáme dočasnú premennú do zoznamu funkčných premenných
                symtable_add_function_var(get_global_symtable(), temp_var);

                // Zavoláme getter
                printf("CALL $%s\n", getter_mangled);

                // Getter vráti hodnotu na zásobník, popneme ju do dočasnej premennej
                printf("POPS LF@%s\n", temp_var);

                snprintf(result, result_size, "LF@%s", temp_var);
                free(getter_mangled);
                break;
            }
            free(getter_mangled);
        }

        // Nie je to getter - hľadáme lokálnu premennú
        t_avl_node *var_node = symtable_search_var_scoped(get_global_symtable(), node->token->value.string);
        if (var_node != NULL && var_node->ifj_sym_type == SYM_VAR_LOCAL)
        {
            int block_id = var_node->ifj_data.ifj_var.ifj_block_id;
            const char *unique_name = get_var_name_with_nesting(node->token->value.string, block_id);
            snprintf(result, result_size, "LF@%s", unique_name);
        }
        else
        {
            // Parameter or function-scoped variable
            snprintf(result, result_size, "LF@%s", node->token->value.string);
        }
        break;
    }

    case GLOBAL_VAR:
        // Global variables start with __ and use GF@ frame
        snprintf(result, result_size, "GF@__%s", node->token->value.string);
        break;

    case KEYWORD:
        if (node->token->value.keyword == KW_NULL_INST)
        {
            snprintf(result, result_size, "nil@nil");
        }
        else
        {
            result[0] = '\0';
        }
        break;

    default:
        result[0] = '\0';
    }
}

// Function to generate code for expressions (stub, to be implemented)
void generate_builtin_write(t_ast_node *param_node)
{
    if (param_node == NULL)
    {
        exit_with_error(ERR_INTERNAL, "NULL parameter in Ifj.write()");
    }

    char param_var[256];
    if (param_node->left == NULL && param_node->right == NULL)
    {
        get_param_value(param_node, param_var, sizeof(param_var));
    }
    else
    {
        generate_expression_code(param_node, param_var, sizeof(param_var));
    }

    if (param_var[0] == '\0')
    {
        exit_with_error(ERR_INTERNAL, "Unsupported type in Ifj.write()");
    }

    // If it's a variable (starts with LF@ or GF@), check if it's float and if it's a whole number
    if (strncmp(param_var, "LF@", 3) == 0 || strncmp(param_var, "GF@", 3) == 0 || strncmp(param_var, "TF@", 3) == 0)
    {
        static int write_counter = 0;
        int label_id = write_counter++;

        t_symtable *symtable = get_global_symtable();
        char type_var[64], temp_var[64], isint_var[64];
        snprintf(type_var, sizeof(type_var), "__write_type_%d", label_id);
        snprintf(temp_var, sizeof(temp_var), "__write_temp_%d", label_id);
        snprintf(isint_var, sizeof(isint_var), "__write_isint_%d", label_id);

        if (symtable)
        {
            symtable_add_function_var(symtable, type_var);
            symtable_add_function_var(symtable, temp_var);
            symtable_add_function_var(symtable, isint_var);
        }

        // Check type
        printf("TYPE LF@%s %s\n", type_var, param_var);
        printf("JUMPIFEQ $$write_is_float_%d LF@%s string@float\n", label_id, type_var);
        // Not float, just write it
        printf("WRITE %s\n", param_var);
        printf("JUMP $$write_end_%d\n", label_id);

        // It's float - check if it's a whole number using ISINT
        printf("LABEL $$write_is_float_%d\n", label_id);
        printf("ISINT LF@%s %s\n", isint_var, param_var);
        printf("JUMPIFEQ $$write_convert_to_int_%d LF@%s bool@true\n", label_id, isint_var);
        // Not a whole number, write as float
        printf("WRITE %s\n", param_var);
        printf("JUMP $$write_end_%d\n", label_id);

        // It's a whole number, convert to int and write
        printf("LABEL $$write_convert_to_int_%d\n", label_id);
        printf("FLOAT2INT LF@%s %s\n", temp_var, param_var);
        printf("WRITE LF@%s\n", temp_var);

        printf("LABEL $$write_end_%d\n", label_id);
    }
    else
    {
        // It's a literal, just write it
        printf("WRITE %s\n", param_var);
    }
}

// Function to generate code for expressions (stub, to be implemented)
void generate_builtin_read_str(const char *result_var)
{
    if (result_var == NULL)
    {
        exit_with_error(ERR_INTERNAL, "Internal error: NULL result variable in Ifj.read_str()");
    }
    printf("READ %s string\n", result_var);
}

// Function to generate code for expressions (stub, to be implemented)
void generate_builtin_read_num(const char *result_var)
{
    if (result_var == NULL)
    {
        exit_with_error(ERR_INTERNAL, "Internal error: NULL result variable in Ifj.read_num()");
    }
    printf("READ %s float\n", result_var);
}

// Function to generate code for expressions (stub, to be implemented)
void generate_builtin_floor(const char *result_var, t_ast_node *param_node)
{
    if (result_var == NULL || param_node == NULL)
    {
        exit_with_error(ERR_INTERNAL, "Internal error: NULL argument in Ifj.floor()");
    }

    // Generate code to evaluate the parameter expression
    char param_var[256];
    int err = generate_expression_code(param_node, param_var, sizeof(param_var));
    if (err)
    {
        exit_with_error(ERR_INTERNAL, "Failed to generate expression for Ifj.floor()");
    }

    // Call the builtin function via stack
    printf("PUSHS %s\n", param_var);
    printf("CALL $$__builtin_floor\n");
    printf("POPS %s\n", result_var);
}

void generate_builtin_str(const char *result_var, t_ast_node *param_node)
{
    if (result_var == NULL || param_node == NULL)
    {
        exit_with_error(ERR_INTERNAL, "Internal error: NULL argument in Ifj.str()");
    }

    // Generate code to evaluate the parameter expression
    char param_var[256];

    if (param_node->left == NULL && param_node->right == NULL)
    {
        get_param_value(param_node, param_var, sizeof(param_var));
    }
    else
    {
        int err = generate_expression_code(param_node, param_var, sizeof(param_var));
        if (err)
        {
            exit_with_error(ERR_INTERNAL, "Failed to generate expression for Ifj.str()");
        }
    }

    // Call the builtin function via stack
    printf("PUSHS %s\n", param_var);
    printf("CALL $$__builtin_str\n");
    printf("POPS %s\n", result_var);
}

void generate_builtin_length(const char *result_var, t_ast_node *param_node)
{
    if (result_var == NULL || param_node == NULL)
    {
        exit_with_error(ERR_INTERNAL, "Internal error: NULL argument in Ifj.length()");
    }

    char param_var[256];
    if (param_node->left == NULL && param_node->right == NULL)
    {
        get_param_value(param_node, param_var, sizeof(param_var));
    }
    else
    {
        generate_expression_code(param_node, param_var, sizeof(param_var));
    }

    // Call the builtin function via stack
    printf("PUSHS %s\n", param_var);
    printf("CALL $$__builtin_length\n");
    printf("POPS %s\n", result_var);
}

void generate_builtin_chr(const char *result_var, t_ast_node *param_node)
{
    if (result_var == NULL || param_node == NULL)
    {
        exit_with_error(ERR_INTERNAL, "Internal error: NULL argument in Ifj.chr()");
    }

    char param_var[256];
    if (param_node->left == NULL && param_node->right == NULL)
    {
        get_param_value(param_node, param_var, sizeof(param_var));
    }
    else
    {
        generate_expression_code(param_node, param_var, sizeof(param_var));
    }

    // Call the builtin function via stack
    printf("PUSHS %s\n", param_var);
    printf("CALL $$__builtin_chr\n");
    printf("POPS %s\n", result_var);
}

void generate_builtin_substring(const char *result_var, t_ast_node *param_node, t_ast_node *i_node, t_ast_node *j_node)
{
    if (result_var == NULL || param_node == NULL || i_node == NULL || j_node == NULL)
    {
        exit_with_error(ERR_INTERNAL, "Internal error: NULL argument in Ifj.substring()");
    }

    char param_var[256];
    if (param_node->left == NULL && param_node->right == NULL)
    {
        get_param_value(param_node, param_var, sizeof(param_var));
    }
    else
    {
        generate_expression_code(param_node, param_var, sizeof(param_var));
    }

    char i_var[256];
    if (i_node->left == NULL && i_node->right == NULL)
    {
        get_param_value(i_node, i_var, sizeof(i_var));
    }
    else
    {
        generate_expression_code(i_node, i_var, sizeof(i_var));
    }

    char j_var[256];
    if (j_node->left == NULL && j_node->right == NULL)
    {
        get_param_value(j_node, j_var, sizeof(j_var));
    }
    else
    {
        generate_expression_code(j_node, j_var, sizeof(j_var));
    }

    // Call the builtin function via stack (push in order, popped in reverse)
    printf("PUSHS %s\n", param_var);
    printf("PUSHS %s\n", i_var);
    printf("PUSHS %s\n", j_var);
    printf("CALL $$__builtin_substring\n");
    printf("POPS %s\n", result_var);
}

void generate_builtin_strcmp(const char *result_var, t_ast_node *str1_node, t_ast_node *str2_node)
{
    if (result_var == NULL || str1_node == NULL || str2_node == NULL)
    {
        exit_with_error(ERR_INTERNAL, "Internal error: NULL argument in Ifj.strcmp()");
    }

    char str1_var[256];
    if (str1_node->left == NULL && str1_node->right == NULL)
    {
        get_param_value(str1_node, str1_var, sizeof(str1_var));
    }
    else
    {
        generate_expression_code(str1_node, str1_var, sizeof(str1_var));
    }

    char str2_var[256];
    if (str2_node->left == NULL && str2_node->right == NULL)
    {
        get_param_value(str2_node, str2_var, sizeof(str2_var));
    }
    else
    {
        generate_expression_code(str2_node, str2_var, sizeof(str2_var));
    }

    // Call the builtin function via stack (push in order, popped in reverse)
    printf("PUSHS %s\n", str1_var);
    printf("PUSHS %s\n", str2_var);
    printf("CALL $$__builtin_strcmp\n");
    printf("POPS %s\n", result_var);
}

void generate_builtin_ord(const char *result_var, t_ast_node *param_node, t_ast_node *i_node)
{
    if (result_var == NULL || param_node == NULL || i_node == NULL)
    {
        exit_with_error(ERR_INTERNAL, "Internal error: NULL argument in Ifj.ord()");
    }

    char str_var[256];
    if (param_node->left == NULL && param_node->right == NULL)
    {
        get_param_value(param_node, str_var, sizeof(str_var));
    }
    else
    {
        generate_expression_code(param_node, str_var, sizeof(str_var));
    }

    char i_var[256];
    if (i_node->left == NULL && i_node->right == NULL)
    {
        get_param_value(i_node, i_var, sizeof(i_var));
    }
    else
    {
        generate_expression_code(i_node, i_var, sizeof(i_var));
    }

    // Call the builtin function via stack (push in order, popped in reverse)
    printf("PUSHS %s\n", str_var);
    printf("PUSHS %s\n", i_var);
    printf("CALL $$__builtin_ord\n");
    printf("POPS %s\n", result_var);
}