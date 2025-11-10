/*
 * Prekladač jazyka IFJ25
 * VUT FIT
 *
 * Autori:
 *   - Matúš Magyar (xmagyam00)
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
        // Need to check nesting level for local variables
        // Use the generator function that handles this
        t_avl_node *var_node = symtable_search_var_scoped(get_global_symtable(), node->token->value.string);
        if (var_node != NULL && var_node->ifj_sym_type == SYM_VAR_LOCAL)
        {
            int nesting = var_node->ifj_data.ifj_var.ifj_nesting_level;
            const char *unique_name = get_var_name_with_nesting(node->token->value.string, nesting);
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
        snprintf(result, result_size, "GF@%s", node->token->value.string);
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

    printf("WRITE %s\n", param_var);
}

// Function to generate code for expressions (stub, to be implemented)
void generate_builtin_read_str(const char *result_var)
{
    if (result_var == NULL)
    {
        exit_with_error(ERR_INTERNAL, "Internal error: NULL result variable in Ifj.read_str()");
    }
    // result_var already contains "LF@" prefix
    printf("READ %s string\n", result_var);
}

// Function to generate code for expressions (stub, to be implemented)
void generate_builtin_read_num(const char *result_var)
{
    if (result_var == NULL)
    {
        exit_with_error(ERR_INTERNAL, "Internal error: NULL result variable in Ifj.read_num()");
    }
    // result_var already contains "LF@" prefix
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

    static int floor_counter = 0;
    int label_id = floor_counter++;

    // Runtime type check - parameter must be int or float
    printf("DEFVAR LF@__floor_type_%d\n", label_id);
    printf("TYPE LF@__floor_type_%d %s\n", label_id, param_var);
    printf("JUMPIFEQ $$floor_type_int_%d LF@__floor_type_%d string@int\n", label_id, label_id);
    printf("JUMPIFEQ $$floor_type_float_%d LF@__floor_type_%d string@float\n", label_id, label_id);
    printf("EXIT int@25\n");

    // If int, just copy the value
    printf("LABEL $$floor_type_int_%d\n", label_id);
    printf("MOVE %s %s\n", result_var, param_var);
    printf("JUMP $$floor_end_%d\n", label_id);

    // If float, convert to int
    printf("LABEL $$floor_type_float_%d\n", label_id);
    printf("FLOAT2INT %s %s\n", result_var, param_var);

    printf("LABEL $$floor_end_%d\n", label_id);
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

    // Unikátne labely
    static int str_counter = 0;
    int label_id = str_counter++;

    // Type check za behu
    printf("# Ifj.str() - type conversion\n");
    printf("DEFVAR LF@__str_type_%d\n", label_id);
    printf("TYPE LF@__str_type_%d %s\n", label_id, param_var);

    // String - len skopíruj
    printf("JUMPIFEQ $$str_is_string_%d LF@__str_type_%d string@string\n",
           label_id, label_id);

    // Int - konvertuj
    printf("JUMPIFEQ $$str_is_int_%d LF@__str_type_%d string@int\n",
           label_id, label_id);

    // Float - konvertuj
    printf("JUMPIFEQ $$str_is_float_%d LF@__str_type_%d string@float\n",
           label_id, label_id);

    // Null - "null"
    printf("JUMPIFEQ $$str_is_nil_%d LF@__str_type_%d string@nil\n",
           label_id, label_id);

    // === STRING PRÍPAD ===
    printf("LABEL $$str_is_string_%d\n", label_id);
    printf("MOVE %s %s\n", result_var, param_var);
    printf("JUMP $$str_end_%d\n", label_id);

    // === INT PRÍPAD ===
    printf("LABEL $$str_is_int_%d\n", label_id);
    // INT2STRING doesn't exist in IFJcode25 - convert to float first, then to string
    printf("DEFVAR LF@__str_tmp_float_%d\n", label_id);
    printf("INT2FLOAT LF@__str_tmp_float_%d %s\n", label_id, param_var);
    printf("FLOAT2STR %s LF@__str_tmp_float_%d\n", result_var, label_id);
    printf("JUMP $$str_end_%d\n", label_id);

    // === FLOAT PRÍPAD - formát %.2f ===
    printf("LABEL $$str_is_float_%d\n", label_id);
    printf("FLOAT2STR %s %s\n", result_var, param_var);
    printf("JUMP $$str_end_%d\n", label_id);

    // === NULL PRÍPAD ===
    printf("LABEL $$str_is_nil_%d\n", label_id);
    printf("MOVE %s string@null\n", result_var);

    printf("LABEL $$str_end_%d\n", label_id);
}

// Function to generate code for expressions (stub, to be implemented)
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

    static int length_counter = 0;
    int label_id = length_counter++;

    printf("DEFVAR LF@__length_type_%d\n", label_id);
    printf("TYPE LF@__length_type_%d %s\n", label_id, param_var);
    printf("JUMPIFEQ $$length_type_ok_%d LF@__length_type_%d string@string\n",
           label_id, label_id);
    printf("EXIT int@25\n");
    printf("LABEL $$length_type_ok_%d\n", label_id);

    printf("STRLEN %s %s\n", result_var, param_var);
}

// Function to generate code for expressions (stub, to be implemented)
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

    static int chr_counter = 0;
    int label_id = chr_counter++;

    printf("DEFVAR LF@__chr_type_%d\n", label_id);
    printf("TYPE LF@__chr_type_%d %s\n", label_id, param_var);
    printf("JUMPIFEQ $$chr_type_ok_%d LF@__chr_type_%d string@int\n", label_id, label_id);
    printf("EXIT int@25\n");
    printf("LABEL $$chr_type_ok_%d\n", label_id);
    printf("INT2CHAR %s %s\n", result_var, param_var);
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

    static int substring_counter = 0;
    int label_id = substring_counter++;

    printf("DEFVAR LF@__substr_s_type_%d\n", label_id);
    // Kontrola typu s
    printf("TYPE LF@__substr_s_type_%d %s\n", label_id, param_var);
    printf("JUMPIFEQ $$substr_s_ok_%d LF@__substr_s_type_%d string@string\n", label_id, label_id);
    printf("EXIT int@25\n");
    printf("LABEL $$substr_s_ok_%d\n", label_id);

    // Kontrola typu i
    printf("DEFVAR LF@__substr_cmp_%d\n", label_id);
    printf("TYPE LF@__substr_cmp_%d %s\n", label_id, i_var);
    printf("JUMPIFEQ $$substr_i_ok_%d LF@__substr_cmp_%d string@int\n", label_id, label_id);
    printf("EXIT int@26\n");
    printf("LABEL $$substr_i_ok_%d\n", label_id);

    // Kontrola typu j
    printf("DEFVAR LF@__substr_j_type_%d\n", label_id);
    printf("TYPE LF@__substr_j_type_%d %s\n", label_id, j_var);
    printf("JUMPIFEQ $$substr_j_ok_%d LF@__substr_j_type_%d string@int\n", label_id, label_id);
    printf("EXIT int@26\n");
    printf("LABEL $$substr_j_ok_%d\n", label_id);

    // POMOCNÉ PREMENNÉ
    printf("DEFVAR LF@__substr_len_%d\n", label_id);
    printf("STRLEN LF@__substr_len_%d %s\n", label_id, param_var);

    // VALIDÁCIA INDEXOV PODLA ZADANIA
    // i < 0 -> null
    printf("LT LF@__substr_cmp_%d %s int@0\n", label_id, i_var);
    printf("JUMPIFEQ $$substr_return_null_%d LF@__substr_cmp_%d bool@true\n", label_id, label_id);

    // j < 0 -> null
    printf("LT LF@__substr_cmp_%d %s int@0\n", label_id, j_var);
    printf("JUMPIFEQ $$substr_return_null_%d LF@__substr_cmp_%d bool@true\n", label_id, label_id);

    // i > j -> null
    printf("GT LF@__substr_cmp_%d %s %s\n", label_id, i_var, j_var);
    printf("JUMPIFEQ $$substr_return_null_%d LF@__substr_cmp_%d bool@true\n", label_id, label_id);

    // i >= length(s) -> null (check if NOT i < length)
    printf("LT LF@__substr_cmp_%d %s LF@__substr_len_%d\n", label_id, i_var, label_id);
    printf("JUMPIFEQ $$substr_return_null_%d LF@__substr_cmp_%d bool@false\n", label_id, label_id);

    // j > length(s) -> null
    printf("GT LF@__substr_cmp_%d %s LF@__substr_len_%d\n", label_id, j_var, label_id);
    printf("JUMPIFEQ $$substr_return_null_%d LF@__substr_cmp_%d bool@true\n", label_id, label_id);

    // VYTVORENIE SUBSTRING
    printf("MOVE %s string@\n", result_var); // Inicializuj na prázdny string
    printf("DEFVAR LF@__substr_idx_%d\n", label_id);
    printf("MOVE LF@__substr_idx_%d %s\n", label_id, i_var);
    printf("DEFVAR LF@__substr_char_%d\n", label_id);

    // Loop: od i po j-1
    printf("LABEL $$substr_loop_%d\n", label_id);
    printf("LT LF@__substr_cmp_%d LF@__substr_idx_%d %s\n", label_id, label_id, j_var);
    printf("JUMPIFEQ $$substr_end_%d LF@__substr_cmp_%d bool@false\n", label_id, label_id);

    // Získaj znak na indexe
    printf("GETCHAR LF@__substr_char_%d %s LF@__substr_idx_%d\n", label_id, param_var, label_id);
    // Pridaj znak k výsledku
    printf("CONCAT %s %s LF@__substr_char_%d\n", result_var, result_var, label_id);
    // Inkrementuj index
    printf("ADD LF@__substr_idx_%d LF@__substr_idx_%d int@1\n", label_id, label_id);
    printf("JUMP $$substr_loop_%d\n", label_id);

    printf("JUMP $$substr_end_%d\n", label_id);

    // RETURN NULL
    printf("LABEL $$substr_return_null_%d\n", label_id);
    printf("MOVE %s nil@nil\n", result_var);

    printf("LABEL $$substr_end_%d\n", label_id);
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

    static int strcmp_counter = 0;
    int label_id = strcmp_counter++;

    // Type checking
    printf("DEFVAR LF@__strcmp_type_%d\n", label_id);
    printf("TYPE LF@__strcmp_type_%d %s\n", label_id, str1_var);
    printf("JUMPIFEQ $$strcmp_s1_ok_%d LF@__strcmp_type_%d string@string\n", label_id, label_id);
    printf("EXIT int@25\n");
    printf("LABEL $$strcmp_s1_ok_%d\n", label_id);

    printf("TYPE LF@__strcmp_type_%d %s\n", label_id, str2_var);
    printf("JUMPIFEQ $$strcmp_s2_ok_%d LF@__strcmp_type_%d string@string\n", label_id, label_id);
    printf("EXIT int@25\n");
    printf("LABEL $$strcmp_s2_ok_%d\n", label_id);

    // Porovnanie - LT pre s1 < s2
    printf("LT LF@__strcmp_type_%d %s %s\n", label_id, str1_var, str2_var);
    printf("JUMPIFEQ $$strcmp_less_%d LF@__strcmp_type_%d bool@true\n", label_id, label_id);

    // GT pre s1 > s2
    printf("GT LF@__strcmp_type_%d %s %s\n", label_id, str1_var, str2_var);
    printf("JUMPIFEQ $$strcmp_greater_%d LF@__strcmp_type_%d bool@true\n", label_id, label_id);

    // s1 == s2
    printf("MOVE %s int@0\n", result_var);
    printf("JUMP $$strcmp_end_%d\n", label_id);

    // s1 < s2
    printf("LABEL $$strcmp_less_%d\n", label_id);
    printf("MOVE %s int@-1\n", result_var);
    printf("JUMP $$strcmp_end_%d\n", label_id);

    // s1 > s2
    printf("LABEL $$strcmp_greater_%d\n", label_id);
    printf("MOVE %s int@1\n", result_var);

    printf("LABEL $$strcmp_end_%d\n", label_id);
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

    static int ord_counter = 0;
    int label_id = ord_counter++;

    // Type checking
    printf("DEFVAR LF@__ord_type_%d\n", label_id);
    printf("TYPE LF@__ord_type_%d %s\n", label_id, str_var);
    printf("JUMPIFEQ $$ord_s_ok_%d LF@__ord_type_%d string@string\n", label_id, label_id);
    printf("EXIT int@25\n");
    printf("LABEL $$ord_s_ok_%d\n", label_id);

    // Type checking
    printf("TYPE LF@__ord_type_%d %s\n", label_id, i_var);
    printf("JUMPIFEQ $$ord_i_ok_%d LF@__ord_type_%d string@int\n", label_id, label_id);
    printf("EXIT int@26\n");
    printf("LABEL $$ord_i_ok_%d\n", label_id);

    // Získaj dĺžku stringu
    printf("DEFVAR LF@__ord_len_%d\n", label_id);
    printf("STRLEN LF@__ord_len_%d %s\n", label_id, str_var);

    // Kontrola či je string prázdny
    printf("DEFVAR LF@__ord_cmp_%d\n", label_id);
    printf("EQ LF@__ord_cmp_%d LF@__ord_len_%d int@0\n", label_id, label_id);
    printf("JUMPIFEQ $$ord_return_zero_%d LF@__ord_cmp_%d bool@true\n", label_id, label_id);

    // Kontrola či i < 0
    printf("LT LF@__ord_cmp_%d %s int@0\n", label_id, i_var);
    printf("JUMPIFEQ $$ord_return_zero_%d LF@__ord_cmp_%d bool@true\n", label_id, label_id);

    // Kontrola či i >= length(s) -
    printf("LT LF@__ord_cmp_%d %s LF@__ord_len_%d\n", label_id, i_var, label_id);
    printf("JUMPIFEQ $$ord_return_zero_%d LF@__ord_cmp_%d bool@false\n", label_id, label_id);

    printf("STRI2INT %s %s %s\n", result_var, str_var, i_var);
    printf("JUMP $$ord_end_%d\n", label_id);

    printf("LABEL $$ord_return_zero_%d\n", label_id);
    printf("MOVE %s int@0\n", result_var);

    printf("LABEL $$ord_end_%d\n", label_id);
}