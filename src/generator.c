/*
 * Prekladač jazyka IFJ25
 * VUT FIT
 *
 * Autori:
 *   - Martin Michálik (xmicham00)
 */

#define _POSIX_C_SOURCE 200809L
#include "generator.h"
#include "symtable.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// Globálna premenná pre prístup k tabuľke symbolov pri generovaní kódu
static t_symtable *global_symtable = NULL;

// Tabuľka pre globálne premenné (aby sme vedeli, ktoré už boli deklarované)
static t_symtable *global_vars_symtable = NULL;

// Počítadlo pre unikátne labely pre if/while konštrukcie
static int ifj_label_counter = 0;

// Buffer for function body code generation
static char *function_body_buffer = NULL;
static size_t function_body_buffer_size = 0;
static FILE *function_body_stream = NULL;
static FILE *original_stdout = NULL;

// Buffer for helper variables (if_cond, while_cond, etc.)
#define MAX_HELPER_VARS 100
static char *helper_vars[MAX_HELPER_VARS];
static int helper_vars_count = 0;

// Aktuálna funkcia (pre sledovanie, či sme v main)
static const char *current_function_name = NULL;

void generator_set_symtable(t_symtable *ifj_symtable)
{
    global_symtable = ifj_symtable;
}

t_symtable *get_global_symtable()
{
    return global_symtable;
}

void generator_set_global_symtable(t_symtable *global_table)
{
    global_vars_symtable = global_table;
}

void generate_global_defvar_once(const char *var_name)
{
    if (global_vars_symtable == NULL)
    {
        return;
    }

    // Skontrolujeme či DEFVAR už bol vygenerovaný (nie či premenná existuje v symtable)
    t_avl_node *node = symtable_search(global_vars_symtable, var_name);
    
    // Generujeme DEFVAR len ak:
    // 1. Premenná ešte neexistuje v symtable (node == NULL), alebo
    // 2. Premenná existuje, ale DEFVAR ešte nebol vygenerovaný
    if (node == NULL || !node->ifj_data.ifj_var.ifj_defvar_generated)
    {
        // Deklarujeme s hodnotou null
        printf("DEFVAR GF@__%s\n", var_name);
        printf("MOVE GF@__%s nil@nil\n", var_name);
        
        // Označíme že DEFVAR bol vygenerovaný
        if (node == NULL)
        {
            // Pridáme do tabuľky ak ešte neexistuje
            symtable_insert_global_var(global_vars_symtable, var_name, TYPE_UNKNOWN);
            node = symtable_search(global_vars_symtable, var_name);
        }
        
        if (node != NULL)
        {
            node->ifj_data.ifj_var.ifj_defvar_generated = true;
        }
    }
}

int get_next_label_id()
{
    return ifj_label_counter++;
}

void register_helper_var(const char *var_name)
{
    if (helper_vars_count >= MAX_HELPER_VARS)
    {
        exit_with_error(ERR_INTERNAL, "Too many helper variables");
    }
    
    // Check if already registered
    for (int i = 0; i < helper_vars_count; i++)
    {
        if (strcmp(helper_vars[i], var_name) == 0)
        {
            return; // Already registered
        }
    }
    
    helper_vars[helper_vars_count] = strdup(var_name);
    helper_vars_count++;
}

void generate_helper_defvars()
{
    for (int i = 0; i < helper_vars_count; i++)
    {
        printf("DEFVAR %s\n", helper_vars[i]);
    }
}

void clear_helper_vars()
{
    for (int i = 0; i < helper_vars_count; i++)
    {
        free(helper_vars[i]);
        helper_vars[i] = NULL;
    }
    helper_vars_count = 0;
}

// Prejdeme všetky uzly v globálnej symtable a vygenerujeme DEFVAR pre globálne premenné
void traverse_and_generate(t_avl_node *node)
{
    if (node == NULL)
    {
        return;
    }
    
    traverse_and_generate(node->ifj_left);
    
    // Ak je to globálna premenná, vygenerujeme DEFVAR
    if (node->ifj_sym_type == SYM_VAR_GLOBAL)
    {
        printf("DEFVAR GF@__%s\n", node->ifj_key);
        printf("MOVE GF@__%s nil@nil\n", node->ifj_key);
    }
    
    traverse_and_generate(node->ifj_right);
}

void generate_global_variables_declarations()
{
    if (global_vars_symtable == NULL)
    {
        return;
    }
    
    if (global_vars_symtable->ifj_root == NULL)
    {
        return;
    }
    
    traverse_and_generate(global_vars_symtable->ifj_root);
}

void start_function_body_buffering()
{
    // Save original stdout
    original_stdout = stdout;

    // Open memory stream for buffering
    function_body_stream = open_memstream(&function_body_buffer, &function_body_buffer_size);
    if (function_body_stream == NULL)
    {
        exit_with_error(ERR_INTERNAL, "Failed to create memory stream for function body");
    }

    // Redirect stdout to buffer
    stdout = function_body_stream;
}

void end_function_body_buffering()
{
    if (function_body_stream == NULL)
    {
        return;
    }

    // Flush and close the memory stream
    fflush(function_body_stream);
    fclose(function_body_stream);
    function_body_stream = NULL;

    // Restore original stdout
    stdout = original_stdout;

    // Now generate all DEFVARs first
    generate_all_function_defvars();
    
    // Generate helper variables (if_cond, etc.)
    generate_helper_defvars();
    
    // Pre main funkciu vygenerujeme deklarácie globálnych premenných
    // pred výpisom tela funkcie (po LABEL $$main, pred CREATEFRAME)
    if (current_function_name != NULL && strcmp(current_function_name, "main") == 0)
    {
        generate_global_variables_declarations();
    }

    // Then output the buffered function body
    if (function_body_buffer != NULL && function_body_buffer_size > 0)
    {
        fwrite(function_body_buffer, 1, function_body_buffer_size, stdout);
        free(function_body_buffer);
        function_body_buffer = NULL;
        function_body_buffer_size = 0;
    }
}

int must_escape(unsigned char c)
{
    return (c <= 32 || c == 35 || c == 92);
}

size_t calculate_escaped_length(const char *input)
{
    size_t len = 0;
    const unsigned char *str = (const unsigned char *)input;

    while (*str)
    {
        if (must_escape(*str))
        {
            len += 4; // \xyz (4 znaky)
        }
        else
        {
            len += 1;
        }
        str++;
    }

    return len;
}

char *convert_to_escaped_string(const char *input)
{
    if (input == NULL)
    {
        return NULL;
    }

    size_t escaped_len = calculate_escaped_length(input);
    char *output = (char *)malloc(escaped_len + 1); // +1 pro '\0'

    if (output == NULL)
    {
        return NULL; // Chyba alokace
    }

    const unsigned char *src = (const unsigned char *)input;
    char *dst = output;

    while (*src)
    {
        if (must_escape(*src))
        {
            // Vytvoř escape sekvenci \xyz
            sprintf(dst, "\\%03d", *src);
            dst += 4;
        }
        else
        {
            // Zkopíruj znak bez změny
            *dst = *src;
            dst++;
        }
        src++;
    }

    *dst = '\0'; // Ukončovací nula
    return output;
}

char *mangle_function_name(const char *func_name, int param_count)
{
    if (func_name == NULL)
    {
        return NULL;
    }

    // Špeciálny prípad pre main - nemanglujeme
    if (strcmp(func_name, "main") == 0)
    {
        char *result = malloc(strlen(func_name) + 1);
        if (result == NULL)
        {
            return NULL;
        }
        strcpy(result, func_name);
        return result;
    }

    // Vypočítame potrebnú veľkosť: "functionName$arityN\0"
    // max 10 číslic pre int + "$arity" (6 znakov) + pôvodné meno + '\0'
    size_t len = strlen(func_name) + 6 + 10 + 1;
    char *mangled = malloc(len);

    if (mangled == NULL)
    {
        return NULL;
    }

    snprintf(mangled, len, "%s$arity%d", func_name, param_count);
    return mangled;
}

char *mangle_getter_name(const char *getter_name)
{
    if (getter_name == NULL)
    {
        return NULL;
    }

    // Vypočítame potrebnú veľkosť: "getterName$get\0"
    // "$get" (4 znaky) + pôvodné meno + '\0'
    size_t len = strlen(getter_name) + 4 + 1;
    char *mangled = malloc(len);

    if (mangled == NULL)
    {
        return NULL;
    }

    snprintf(mangled, len, "%s$get", getter_name);
    return mangled;
}

char *mangle_setter_name(const char *setter_name)
{
    if (setter_name == NULL)
    {
        return NULL;
    }

    // Vypočítame potrebnú veľkosť: "setterName$set\0"
    // "$set" (4 znaky) + pôvodné meno + '\0'
    size_t len = strlen(setter_name) + 4 + 1;
    char *mangled = malloc(len);

    if (mangled == NULL)
    {
        return NULL;
    }

    snprintf(mangled, len, "%s$set", setter_name);
    return mangled;
}

// Generate unique variable name with nesting level
// Returns pointer to static buffer - not thread safe but OK for single-threaded compiler
const char *get_var_name_with_nesting(const char *var_name, int block_id)
{
    static char buffer[256];
    if (block_id == 0)
    {
        // Parameters (block_id=0) have no suffix
        snprintf(buffer, sizeof(buffer), "%s", var_name);
    }
    else
    {
        // Block variables get suffix with block ID
        snprintf(buffer, sizeof(buffer), "%s$b%d", var_name, block_id);
    }
    return buffer;
}

void generate_header()
{
    printf(".IFJcode25\n");
    printf("JUMP $$main\n");
}

void builtin_write_string_literal(char *string)
{
    char *escaped_string = convert_to_escaped_string(string);
    if (escaped_string == NULL)
    {
        exit_with_error(ERR_INTERNAL, "Internal error: Memory allocation failed in builtin_write_literal");
    }
    else
    {
        printf("WRITE string@%s\n", escaped_string);
        free(escaped_string);
    }
}

void builtin_write_float_literal(double number)
{
    printf("WRITE float@%a\n", number);
}

void builtin_write_integer_literal(long long number)
{
    printf("WRITE int@%lld\n", number);
}

void builtin_write_var(char *var_id)
{
    t_avl_node *var_node = symtable_search_var_scoped(global_symtable, var_id);
    if (var_node != NULL && var_node->ifj_sym_type == SYM_VAR_LOCAL)
    {
        int block_id = var_node->ifj_data.ifj_var.ifj_block_id;
        const char *unique_name = get_var_name_with_nesting(var_id, block_id);
        printf("WRITE LF@%s\n", unique_name);
    }
    else if (var_node != NULL && var_node->ifj_sym_type == SYM_VAR_GLOBAL)
    {
        // Global variables use GF@ frame
        printf("WRITE GF@__%s\n", var_id);
    }
    else
    {
        // Parameter or unknown - use as is
        printf("WRITE LF@%s\n", var_id);
    }
}

void generate_var_declaration(const char *var_name)
{
    // Get block ID from current block on stack (or 0 if no block)
    int block_id = 0;
    if (global_symtable->ifj_block_stack_top > 0)
    {
        block_id = global_symtable->ifj_block_stack[global_symtable->ifj_block_stack_top - 1];
    }
    const char *unique_name = get_var_name_with_nesting(var_name, block_id);

    // Add to function's variable list instead of generating DEFVAR immediately
    symtable_add_function_var(global_symtable, unique_name);
}

void generate_function_start(const char *func_name, const char *mangled_name, t_param_list *params)
{
    // Nastavíme aktuálnu funkciu
    current_function_name = func_name;
    
    // Vygenerujeme label pre funkciu
    if (strcmp(func_name, "main") == 0)
    {
        printf("LABEL $$main\n");
        // Pre main funkciu NEBUDE generovať deklarácie tu, ale až na konci po naparsovaní
        // Ponecháme prázdny marker, kam sa vrátia deklarácie
    }
    else
    {
        printf("LABEL $%s\n", mangled_name);
    }

    // Vytvoríme a aktivujeme lokálny rámec
    printf("CREATEFRAME\n");
    printf("PUSHFRAME\n");

    // Najprv definujeme všetky premenné pre parametre
    for (int i = 0; i < params->count; i++)
    {
        printf("DEFVAR LF@%s\n", params->names[i]);
    }

    // Potom popneme parametre zo zásobníka v opačnom poradí
    // (posledný parameter bol pushnutý ako posledný, takže je na vrchu zásobníka)
    for (int i = params->count - 1; i >= 0; i--)
    {
        printf("POPS LF@%s\n", params->names[i]);
    }
}

void generate_all_function_defvars()
{
    if (global_symtable == NULL)
    {
        return;
    }

    // Generate DEFVAR for all collected function variables
    for (int i = 0; i < global_symtable->ifj_function_vars_count; i++)
    {
        if (global_symtable->ifj_function_vars[i] != NULL)
        {
            printf("DEFVAR LF@%s\n", global_symtable->ifj_function_vars[i]);
        }
    }
}

void generate_function_end(const char *func_name)
{
    if (strcmp(func_name, "main") == 0)
    {
        // Main funkcia končí POPFRAME a EXIT
        printf("POPFRAME\n");
        printf("EXIT int@0\n");
    }
    else
    {
        // Bežná funkcia - implicitný return nil
        printf("PUSHS nil@nil\n");
        printf("POPFRAME\n");
        printf("RETURN\n");
    }
    
    // Clear helper variables for next function
    clear_helper_vars();
}

void generate_return_value(t_ast_node *ast)
{
    if (ast == NULL)
    {
        // Prázdny return - vrátime null na zásobník
        printf("PUSHS nil@nil\n");
    }
    else if (ast->left == NULL && ast->right == NULL)
    {
        // Jednoduchý literál alebo premenná - pushneme priamo na zásobník
        char value_str[512];
        get_value_string(ast, value_str, sizeof(value_str));
        printf("PUSHS %s\n", value_str);
    }
    else
    {
        // Komplexný výraz - vygenerujeme ho a pushneme výsledok na zásobník
        char result_var[256];
        int err = generate_expression_code(ast, result_var, sizeof(result_var));
        if (err)
        {
            exit_with_error(ERR_INTERNAL, "Internal error: Failed to generate return expression code");
        }
        printf("PUSHS %s\n", result_var);
    }
}

void generate_return_statement(t_ast_node *ast)
{
    // V main funkcii používame EXIT namiesto RETURN
    if (current_function_name != NULL && strcmp(current_function_name, "main") == 0)
    {
        printf("EXIT int@0\n");
        return;
    }
    
    // Pushni návratovú hodnotu na zásobník
    generate_return_value(ast);
    
    // Ukončenie funkcie
    printf("POPFRAME\n");
    printf("RETURN\n");
}

void generate_push_argument(t_ast_node *ast)
{
    if (ast == NULL)
    {
        exit_with_error(ERR_INTERNAL, "Internal error: NULL AST in generate_push_argument");
    }

    if (ast->left == NULL && ast->right == NULL)
    {
        // Jednoduchý literál alebo premenná - pushneme priamo na zásobník
        char value_str[512];
        get_value_string(ast, value_str, sizeof(value_str));
        printf("PUSHS %s\n", value_str);
    }
    else
    {
        // Komplexný výraz - vygenerujeme ho a pushneme výsledok na zásobník
        char result_var[256];
        int err = generate_expression_code(ast, result_var, sizeof(result_var));
        if (err)
        {
            exit_with_error(ERR_INTERNAL, "Internal error: Failed to generate argument expression code");
        }
        printf("PUSHS %s\n", result_var);
    }
}

void generate_push_string_literal(const char *str)
{
    char *escaped = convert_to_escaped_string(str);
    if (escaped == NULL)
    {
        exit_with_error(ERR_INTERNAL, "Internal error: Failed to convert string to escaped format");
    }
    printf("PUSHS string@%s\n", escaped);
    free(escaped);
}

void generate_push_int_literal(long long value)
{
    printf("PUSHS int@%lld\n", value);
}

void generate_push_float_literal(double value)
{
    printf("PUSHS float@%a\n", value);
}

void generate_push_variable(const char *var_name)
{
    // Nájdeme premennú viditeľnú z aktuálneho bloku
    t_avl_node *var_node = symtable_search_var_scoped(global_symtable, var_name);
    if (var_node != NULL && var_node->ifj_sym_type == SYM_VAR_LOCAL)
    {
        int block_id = var_node->ifj_data.ifj_var.ifj_block_id;
        const char *unique_name = get_var_name_with_nesting(var_name, block_id);
        printf("PUSHS LF@%s\n", unique_name);
    }
    else
    {
        // Global variable or parameter - use as is
        printf("PUSHS LF@%s\n", var_name);
    }
}

void generate_push_null()
{
    printf("PUSHS nil@nil\n");
}

void generate_function_call(const char *func_name, int arg_count)
{
    // Vytvoríme manglované meno funkcie podľa počtu argumentov
    char *mangled_name = mangle_function_name(func_name, arg_count);
    if (mangled_name == NULL)
    {
        exit_with_error(ERR_INTERNAL, "Internal error: Failed to mangle function name");
    }

    // Argumenty sú už na zásobníku (pushnuté arg_list)
    // Zavoláme funkciu s manglovaným menom
    printf("CALL $%s\n", mangled_name);

    free(mangled_name);
}

void generate_setter_call(const char *setter_name)
{
    // Vytvoríme manglované meno pre setter
    char *mangled_name = mangle_setter_name(setter_name);
    if (mangled_name == NULL)
    {
        exit_with_error(ERR_INTERNAL, "Internal error: Failed to mangle setter name");
    }

    // Argument je už na zásobníku
    // Zavoláme setter
    printf("CALL $%s\n", mangled_name);

    free(mangled_name);
}

void generate_move_retval_to_var(const char *var_name)
{
    // Po návrate z funkcie je návratová hodnota na zásobníku
    // Nájdeme premennú viditeľnú z aktuálneho bloku
    t_avl_node *var_node = symtable_search_var_scoped(global_symtable, var_name);
    if (var_node != NULL && var_node->ifj_sym_type == SYM_VAR_LOCAL)
    {
        int block_id = var_node->ifj_data.ifj_var.ifj_block_id;
        const char *unique_name = get_var_name_with_nesting(var_name, block_id);
        printf("POPS LF@%s\n", unique_name);
    }
    else
    {
        // Global variable or parameter - use as is
        printf("POPS LF@%s\n", var_name);
    }
}

void generate_move_retval_to_global_var(const char *var_name)
{
    // Po návrate z funkcie je návratová hodnota na zásobníku
    // Globálna premenná má prefix __
    printf("POPS GF@__%s\n", var_name);
}

// Pomocná premenná pre generovanie dočasných premenných
static int temp_var_counter = 0;

/**
 * @brief Pomocná funkcia pre generovanie názvu dočasnej premennej
 * @return Číslo dočasnej premennej
 */
int get_next_temp_var()
{
    return temp_var_counter++;
}

/**
 * @brief Skontroluje či identifier je getter a ak áno, vygeneruje jeho volanie
 * @param identifier_name Názov identifieru
 * @param ifj_symtable Ukazovateľ na tabuľku symbolov
 * @return 1 ak je getter, 0 ak nie je getter
 */
int generate_getter_call_if_needed(const char *identifier_name, t_symtable *ifj_symtable)
{
    // Vytvoríme manglované meno pre getter
    char *mangled_name = mangle_getter_name(identifier_name);
    if (mangled_name == NULL)
    {
        return 0;
    }

    // Skontrolujeme či existuje getter s týmto menom
    t_avl_node *node = symtable_search(ifj_symtable, mangled_name);

    if (node != NULL && node->ifj_sym_type == SYM_GETTER)
    {
        // Je to getter - vygenerujeme volanie
        // Getter je bezparametrická funkcia ktorá vráti hodnotu na zásobník
        printf("CALL $%s\n", mangled_name);
        free(mangled_name);
        return 1;
    }

    free(mangled_name);
    return 0;
}

void get_value_string(t_ast_node *node, char *result, size_t result_size)
{
    if (node == NULL || node->token == NULL)
    {
        result[0] = '\0';
        return;
    }

    switch (node->token->type)
    {
    case NUM_INT:
    case NUM_EXP_INT:
    case NUM_HEX:
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
    case GLOBAL_VAR:
    {
        // Globálna premenná - použijeme GF@
        // Už je deklarovaná na začiatku programu
        snprintf(result, result_size, "GF@__%s", node->token->value.string);
        break;
    }
    case IDENTIFIER:
    {
        // Najprv skontrolujeme, či ide o getter
        char *getter_mangled = mangle_getter_name(node->token->value.string);
        if (getter_mangled != NULL)
        {
            t_avl_node *getter_node = symtable_search(global_symtable, getter_mangled);
            if (getter_node != NULL && getter_node->ifj_sym_type == SYM_GETTER)
            {
                // Je to getter - vygenerujeme volanie a výsledok uložíme do dočasnej premennej
                int temp_id = get_next_temp_var();
                char temp_var[64];
                snprintf(temp_var, sizeof(temp_var), "__getter_result_%d", temp_id);
                
                // Pridáme dočasnú premennú do zoznamu funkčných premenných
                symtable_add_function_var(global_symtable, temp_var);
                
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
        t_avl_node *var_node = symtable_search_var_scoped(global_symtable, node->token->value.string);
        if (var_node != NULL && var_node->ifj_sym_type == SYM_VAR_LOCAL)
        {
            int block_id = var_node->ifj_data.ifj_var.ifj_block_id;
            const char *unique_name = get_var_name_with_nesting(node->token->value.string, block_id);
            snprintf(result, result_size, "LF@%s", unique_name);
        }
        else
        {
            // Not found - use as is (should not happen with proper semantic checking)
            snprintf(result, result_size, "LF@%s", node->token->value.string);
        }
        break;
    }
    case KEYWORD:
        // Handle null keyword
        if (node->token->value.keyword == KW_NULL_TYPE ||
            node->token->value.keyword == KW_NULL_INST)
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
        break;
    }
}

/**
 * @brief Helper function to generate code for binary operations that support both string and numeric types
 * @param operator The operator type (OP_ADD or OP_MUL)
 * @param left_var Left operand variable
 * @param right_var Right operand variable
 * @param result_var Result variable
 * @param tmp_id Unique temporary ID for labels
 */
void generate_polymorphic_operation(e_token_type operator, const char *left_var, const char *right_var, const char *result_var, int tmp_id)
{
    char left_type[64], right_type[64];
    snprintf(left_type, sizeof(left_type), "__type_left_%d", tmp_id);
    snprintf(right_type, sizeof(right_type), "__type_right_%d", tmp_id);
    symtable_add_function_var(global_symtable, left_type);
    symtable_add_function_var(global_symtable, right_type);

    // Get types of both operands
    printf("TYPE LF@%s %s\n", left_type, left_var);
    printf("TYPE LF@%s %s\n", right_type, right_var);

    if (operator == OP_ADD)
    {
        // Check for null operands first
        printf("JUMPIFEQ $$add_error_null_%d LF@%s string@nil\n", tmp_id, left_type);
        printf("JUMPIFEQ $$add_error_null_%d LF@%s string@nil\n", tmp_id, right_type);
        
        // OP_ADD: String + String = concatenation, Num + Num = addition
        printf("JUMPIFEQ $$add_is_string_%d LF@%s string@string\n", tmp_id, left_type);
        
        // Numeric addition path with INT2FLOAT conversion
        char left_conv[64], right_conv[64];
        snprintf(left_conv, sizeof(left_conv), "__conv_left_%d", tmp_id);
        snprintf(right_conv, sizeof(right_conv), "__conv_right_%d", tmp_id);
        symtable_add_function_var(global_symtable, left_conv);
        symtable_add_function_var(global_symtable, right_conv);

        // Check if left operand is numeric (int or float)
        printf("JUMPIFEQ $$add_left_is_numeric_%d LF@%s string@float\n", tmp_id, left_type);
        printf("JUMPIFEQ $$add_left_is_numeric_%d LF@%s string@int\n", tmp_id, left_type);
        printf("EXIT int@26\n");  // Left operand is not numeric or string
        printf("LABEL $$add_left_is_numeric_%d\n", tmp_id);
        
        printf("MOVE LF@%s %s\n", left_conv, left_var);
        printf("JUMPIFEQ $$conv_left_done_%d LF@%s string@float\n", tmp_id, left_type);
        printf("JUMPIFEQ $$conv_left_is_int_%d LF@%s string@int\n", tmp_id, left_type);
        printf("JUMP $$conv_left_done_%d\n", tmp_id);
        printf("LABEL $$conv_left_is_int_%d\n", tmp_id);
        printf("INT2FLOAT LF@%s LF@%s\n", left_conv, left_conv);
        printf("LABEL $$conv_left_done_%d\n", tmp_id);

        // Check if right operand is numeric (int or float)
        printf("JUMPIFEQ $$add_right_is_numeric_%d LF@%s string@float\n", tmp_id, right_type);
        printf("JUMPIFEQ $$add_right_is_numeric_%d LF@%s string@int\n", tmp_id, right_type);
        printf("EXIT int@26\n");  // Right operand is not numeric
        printf("LABEL $$add_right_is_numeric_%d\n", tmp_id);
        
        printf("MOVE LF@%s %s\n", right_conv, right_var);
        printf("JUMPIFEQ $$conv_right_done_%d LF@%s string@float\n", tmp_id, right_type);
        printf("JUMPIFEQ $$conv_right_is_int_%d LF@%s string@int\n", tmp_id, right_type);
        printf("JUMP $$conv_right_done_%d\n", tmp_id);
        printf("LABEL $$conv_right_is_int_%d\n", tmp_id);
        printf("INT2FLOAT LF@%s LF@%s\n", right_conv, right_conv);
        printf("LABEL $$conv_right_done_%d\n", tmp_id);

        printf("ADD %s LF@%s LF@%s\n", result_var, left_conv, right_conv);
        printf("JUMP $$add_end_%d\n", tmp_id);

        // String concatenation path
        printf("LABEL $$add_is_string_%d\n", tmp_id);
        printf("JUMPIFEQ $$add_concat_%d LF@%s string@string\n", tmp_id, right_type);
        printf("EXIT int@26\n");  // Type mismatch error (string + non-string)
        printf("LABEL $$add_concat_%d\n", tmp_id);
        printf("CONCAT %s %s %s\n", result_var, left_var, right_var);
        printf("JUMP $$add_end_%d\n", tmp_id);
        
        // Error label for null operands
        printf("LABEL $$add_error_null_%d\n", tmp_id);
        printf("EXIT int@26\n");
        printf("LABEL $$add_end_%d\n", tmp_id);
    }
    else if (operator == OP_MUL)
    {
        // Check for null operands first
        printf("JUMPIFEQ $$mul_error_null_%d LF@%s string@nil\n", tmp_id, left_type);
        printf("JUMPIFEQ $$mul_error_null_%d LF@%s string@nil\n", tmp_id, right_type);
        
        // OP_MUL: String * Num = repetition, Num * Num = multiplication
        printf("JUMPIFEQ $$mul_is_string_%d LF@%s string@string\n", tmp_id, left_type);
        
        // Numeric multiplication path with INT2FLOAT conversion
        char left_conv[64], right_conv[64];
        snprintf(left_conv, sizeof(left_conv), "__conv_left_%d", tmp_id);
        snprintf(right_conv, sizeof(right_conv), "__conv_right_%d", tmp_id);
        symtable_add_function_var(global_symtable, left_conv);
        symtable_add_function_var(global_symtable, right_conv);

        // Check if left operand is numeric (int or float)
        printf("JUMPIFEQ $$mul_left_is_numeric_%d LF@%s string@float\n", tmp_id, left_type);
        printf("JUMPIFEQ $$mul_left_is_numeric_%d LF@%s string@int\n", tmp_id, left_type);
        printf("EXIT int@26\n");  // Left operand is not numeric or string
        printf("LABEL $$mul_left_is_numeric_%d\n", tmp_id);
        
        printf("MOVE LF@%s %s\n", left_conv, left_var);
        printf("JUMPIFEQ $$conv_left_done_%d LF@%s string@float\n", tmp_id, left_type);
        printf("JUMPIFEQ $$conv_left_is_int_%d LF@%s string@int\n", tmp_id, left_type);
        printf("JUMP $$conv_left_done_%d\n", tmp_id);
        printf("LABEL $$conv_left_is_int_%d\n", tmp_id);
        printf("INT2FLOAT LF@%s LF@%s\n", left_conv, left_conv);
        printf("LABEL $$conv_left_done_%d\n", tmp_id);

        // Check if right operand is numeric (int or float)
        printf("JUMPIFEQ $$mul_right_is_numeric_%d LF@%s string@float\n", tmp_id, right_type);
        printf("JUMPIFEQ $$mul_right_is_numeric_%d LF@%s string@int\n", tmp_id, right_type);
        printf("EXIT int@26\n");  // Right operand is not numeric
        printf("LABEL $$mul_right_is_numeric_%d\n", tmp_id);
        
        printf("MOVE LF@%s %s\n", right_conv, right_var);
        printf("JUMPIFEQ $$conv_right_done_%d LF@%s string@float\n", tmp_id, right_type);
        printf("JUMPIFEQ $$conv_right_is_int_%d LF@%s string@int\n", tmp_id, right_type);
        printf("JUMP $$conv_right_done_%d\n", tmp_id);
        printf("LABEL $$conv_right_is_int_%d\n", tmp_id);
        printf("INT2FLOAT LF@%s LF@%s\n", right_conv, right_conv);
        printf("LABEL $$conv_right_done_%d\n", tmp_id);

        printf("MUL %s LF@%s LF@%s\n", result_var, left_conv, right_conv);
        printf("JUMP $$mul_end_%d\n", tmp_id);

        // String repetition path: String * Num
        printf("LABEL $$mul_is_string_%d\n", tmp_id);
        
        // Right operand must be Num (int or float)
        printf("JUMPIFEQ $$mul_right_is_int_%d LF@%s string@int\n", tmp_id, right_type);
        printf("JUMPIFEQ $$mul_right_is_float_%d LF@%s string@float\n", tmp_id, right_type);
        printf("EXIT int@26\n");  // Type mismatch error
        
        // Convert float to int and check if it's whole number
        printf("LABEL $$mul_right_is_float_%d\n", tmp_id);
        char is_whole[64], count_int[64];
        snprintf(is_whole, sizeof(is_whole), "__mul_is_whole_%d", tmp_id);
        snprintf(count_int, sizeof(count_int), "__mul_count_int_%d", tmp_id);
        symtable_add_function_var(global_symtable, is_whole);
        symtable_add_function_var(global_symtable, count_int);
        
        printf("ISINT LF@%s %s\n", is_whole, right_var);
        printf("JUMPIFEQ $$mul_can_convert_%d LF@%s bool@true\n", tmp_id, is_whole);
        printf("EXIT int@26\n");  // Float is not a whole number
        printf("LABEL $$mul_can_convert_%d\n", tmp_id);
        printf("FLOAT2INT LF@%s %s\n", count_int, right_var);
        printf("JUMP $$mul_repeat_%d\n", tmp_id);
        
        printf("LABEL $$mul_right_is_int_%d\n", tmp_id);
        printf("MOVE LF@%s %s\n", count_int, right_var);
        
        // String repetition loop
        printf("LABEL $$mul_repeat_%d\n", tmp_id);
        char counter[64], temp_result[64];
        snprintf(counter, sizeof(counter), "__mul_counter_%d", tmp_id);
        snprintf(temp_result, sizeof(temp_result), "__mul_result_%d", tmp_id);
        symtable_add_function_var(global_symtable, counter);
        symtable_add_function_var(global_symtable, temp_result);
        
        printf("MOVE LF@%s string@\n", temp_result);  // Empty string
        printf("MOVE LF@%s int@0\n", counter);
        printf("LABEL $$mul_loop_%d\n", tmp_id);
        printf("LT LF@%s LF@%s LF@%s\n", is_whole, counter, count_int);
        printf("JUMPIFEQ $$mul_loop_end_%d LF@%s bool@false\n", tmp_id, is_whole);
        printf("CONCAT LF@%s LF@%s %s\n", temp_result, temp_result, left_var);
        printf("ADD LF@%s LF@%s int@1\n", counter, counter);
        printf("JUMP $$mul_loop_%d\n", tmp_id);
        printf("LABEL $$mul_loop_end_%d\n", tmp_id);
        printf("MOVE %s LF@%s\n", result_var, temp_result);
        printf("JUMP $$mul_end_%d\n", tmp_id);
        
        // Error label for null operands
        printf("LABEL $$mul_error_null_%d\n", tmp_id);
        printf("EXIT int@26\n");
        printf("LABEL $$mul_end_%d\n", tmp_id);
    }
}

int generate_expression_code(t_ast_node *node, char *result_var, size_t result_var_size)
{
    if (node == NULL)
    {
        return ERR_INTERNAL;
    }

    // Ak je to list (literál alebo premenná), jednoducho skopírujeme hodnotu
    if (node->left == NULL && node->right == NULL)
    {
        // Ak je to IDENTIFIER, môže to byť getter
        if (node->token != NULL && node->token->type == IDENTIFIER && global_symtable != NULL)
        {
            // Skontrolujeme či je to getter
            if (generate_getter_call_if_needed(node->token->value.string, global_symtable))
            {
                // Bol to getter - vygeneroval sa CALL, výsledok je na zásobníku
                // Musíme ho popnúť do dočasnej premennej
                int tmp_num = get_next_temp_var();
                char temp_name[32];
                snprintf(temp_name, sizeof(temp_name), "__tmp%d", tmp_num);
                symtable_add_function_var(global_symtable, temp_name);
                snprintf(result_var, result_var_size, "LF@__tmp%d", tmp_num);
                printf("POPS %s\n", result_var);
                return 0;
            }
        }

        // Nie je to getter, použijeme štandardné spracovanie
        get_value_string(node, result_var, result_var_size);
        return 0;
    }

    // Vytvoríme dočasné premenné pre operandy
    char left_var[256] = {0};
    char right_var[256] = {0};

    // Spracujeme ľavý podstrom
    if (node->left != NULL)
    {
        int err = generate_expression_code(node->left, left_var, sizeof(left_var));
        if (err) return err;

        // Ak ľavý operand nie je jednoduchá hodnota, musíme ho uložiť do temp premennej
        if (node->left->left != NULL || node->left->right != NULL)
        {
            int temp_num = get_next_temp_var();
            char temp_name[256];
            snprintf(temp_name, sizeof(temp_name), "LF@__tmp%d", temp_num);
            // Add to function vars list instead of immediate DEFVAR
            char temp_name_no_prefix[32];
            snprintf(temp_name_no_prefix, sizeof(temp_name_no_prefix), "__tmp%d", temp_num);
            symtable_add_function_var(global_symtable, temp_name_no_prefix);
            printf("MOVE %s %s\n", temp_name, left_var);
            strncpy(left_var, temp_name, sizeof(left_var) - 1);
        }
    }

    // Spracujeme pravý podstrom (ale nie pre OP_IS, kde pravá strana je typ, nie výraz)
    if (node->right != NULL && node->token->type != OP_IS)
    {
        int err = generate_expression_code(node->right, right_var, sizeof(right_var));
        if (err) return err;

        // Ak pravý operand nie je jednoduchá hodnota, musíme ho uložiť do temp premennej
        if (node->right->left != NULL || node->right->right != NULL)
        {
            int temp_num = get_next_temp_var();
            char temp_name[256];
            snprintf(temp_name, sizeof(temp_name), "LF@__tmp%d", temp_num);
            // Add to function vars list instead of immediate DEFVAR
            char temp_name_no_prefix[32];
            snprintf(temp_name_no_prefix, sizeof(temp_name_no_prefix), "__tmp%d", temp_num);
            symtable_add_function_var(global_symtable, temp_name_no_prefix);
            printf("MOVE %s %s\n", temp_name, right_var);
            strncpy(right_var, temp_name, sizeof(right_var) - 1);
        }
    }

    // Vytvoríme premennú pre výsledok tejto operácie
    int result_temp_num = get_next_temp_var();
    snprintf(result_var, result_var_size, "LF@__tmp%d", result_temp_num);
    // Add to function vars list instead of immediate DEFVAR
    char result_temp_name_no_prefix[32];
    snprintf(result_temp_name_no_prefix, sizeof(result_temp_name_no_prefix), "__tmp%d", result_temp_num);
    symtable_add_function_var(global_symtable, result_temp_name_no_prefix);

    // Vygenerujeme inštrukciu podľa typu operátora
    switch (node->token->type)
    {
    case OP_ADD:
    {
        // OP_ADD: String + String = concatenation, Num + Num = addition
        int type_tmp = get_next_temp_var();
        generate_polymorphic_operation(OP_ADD, left_var, right_var, result_var, type_tmp);
        break;
    }
    case OP_MUL:
    {
        // OP_MUL: String * Num = repetition, Num * Num = multiplication
        int type_tmp = get_next_temp_var();
        generate_polymorphic_operation(OP_MUL, left_var, right_var, result_var, type_tmp);
        break;
    }
    case OP_EQUALS:
    case OP_NOT_EQUALS:
    {
        // Pre == a != musíme skonvertovať operandy na rovnaký typ
        // IFJcode25 EQ vyžaduje rovnaké typy operandov
        int type_tmp = get_next_temp_var();
        char left_conv[64], right_conv[64], left_type[64], right_type[64];
        snprintf(left_conv, sizeof(left_conv), "__conv_left_%d", type_tmp);
        snprintf(right_conv, sizeof(right_conv), "__conv_right_%d", type_tmp);
        snprintf(left_type, sizeof(left_type), "__type_left_%d", type_tmp);
        snprintf(right_type, sizeof(right_type), "__type_right_%d", type_tmp);
        symtable_add_function_var(global_symtable, left_conv);
        symtable_add_function_var(global_symtable, right_conv);
        symtable_add_function_var(global_symtable, left_type);
        symtable_add_function_var(global_symtable, right_type);

        // Convert left operand to float if it's int
        printf("MOVE LF@%s %s\n", left_conv, left_var);
        printf("TYPE LF@%s LF@%s\n", left_type, left_conv);
        printf("JUMPIFEQ $$conv_left_done_%d LF@%s string@float\n", type_tmp, left_type);
        printf("JUMPIFEQ $$conv_left_is_int_%d LF@%s string@int\n", type_tmp, left_type);
        printf("JUMP $$conv_left_done_%d\n", type_tmp);
        printf("LABEL $$conv_left_is_int_%d\n", type_tmp);
        printf("INT2FLOAT LF@%s LF@%s\n", left_conv, left_conv);
        printf("LABEL $$conv_left_done_%d\n", type_tmp);

        // Convert right operand to float if it's int
        printf("MOVE LF@%s %s\n", right_conv, right_var);
        printf("TYPE LF@%s LF@%s\n", right_type, right_conv);
        printf("JUMPIFEQ $$conv_right_done_%d LF@%s string@float\n", type_tmp, right_type);
        printf("JUMPIFEQ $$conv_right_is_int_%d LF@%s string@int\n", type_tmp, right_type);
        printf("JUMP $$conv_right_done_%d\n", type_tmp);
        printf("LABEL $$conv_right_is_int_%d\n", type_tmp);
        printf("INT2FLOAT LF@%s LF@%s\n", right_conv, right_conv);
        printf("LABEL $$conv_right_done_%d\n", type_tmp);

        // Now perform EQ with converted operands
        printf("EQ %s LF@%s LF@%s\n", result_var, left_conv, right_conv);
        if (node->token->type == OP_NOT_EQUALS)
        {
            printf("NOT %s %s\n", result_var, result_var);
        }
        break;
    }
    case OP_SUB:
    case OP_DIV:
    case OP_LESS_THAN:
    case OP_GREATER_THAN:
    case OP_LESS_EQUAL:
    case OP_GREATER_THAN_EQUAL:
    {
        // For arithmetic and comparison operations, always convert both operands to float
        int type_tmp = get_next_temp_var();
        char left_conv[64], right_conv[64], left_type[64], right_type[64];
        snprintf(left_conv, sizeof(left_conv), "__conv_left_%d", type_tmp);
        snprintf(right_conv, sizeof(right_conv), "__conv_right_%d", type_tmp);
        snprintf(left_type, sizeof(left_type), "__type_left_%d", type_tmp);
        snprintf(right_type, sizeof(right_type), "__type_right_%d", type_tmp);
        symtable_add_function_var(global_symtable, left_conv);
        symtable_add_function_var(global_symtable, right_conv);
        symtable_add_function_var(global_symtable, left_type);
        symtable_add_function_var(global_symtable, right_type);

        // Check types and convert left operand to float if it's int
        printf("MOVE LF@%s %s\n", left_conv, left_var);
        printf("TYPE LF@%s LF@%s\n", left_type, left_conv);
        // Check for null
        printf("JUMPIFEQ $$arith_error_null_%d LF@%s string@nil\n", type_tmp, left_type);
        // Check for valid numeric types
        printf("JUMPIFEQ $$conv_left_done_%d LF@%s string@float\n", type_tmp, left_type);
        printf("JUMPIFEQ $$conv_left_is_int_%d LF@%s string@int\n", type_tmp, left_type);
        // If not numeric or null, error
        printf("EXIT int@26\n");
        printf("LABEL $$conv_left_is_int_%d\n", type_tmp);
        printf("INT2FLOAT LF@%s LF@%s\n", left_conv, left_conv);
        printf("LABEL $$conv_left_done_%d\n", type_tmp);

        // Check types and convert right operand to float if it's int
        printf("MOVE LF@%s %s\n", right_conv, right_var);
        printf("TYPE LF@%s LF@%s\n", right_type, right_conv);
        // Check for null
        printf("JUMPIFEQ $$arith_error_null_%d LF@%s string@nil\n", type_tmp, right_type);
        // Check for valid numeric types
        printf("JUMPIFEQ $$conv_right_done_%d LF@%s string@float\n", type_tmp, right_type);
        printf("JUMPIFEQ $$conv_right_is_int_%d LF@%s string@int\n", type_tmp, right_type);
        // If not numeric or null, error
        printf("EXIT int@26\n");
        printf("LABEL $$conv_right_is_int_%d\n", type_tmp);
        printf("INT2FLOAT LF@%s LF@%s\n", right_conv, right_conv);
        printf("LABEL $$conv_right_done_%d\n", type_tmp);

        // Now perform the operation with converted operands
        if (node->token->type == OP_SUB)
        {
            printf("SUB %s LF@%s LF@%s\n", result_var, left_conv, right_conv);
        }
        else if (node->token->type == OP_DIV)
        {
            printf("DIV %s LF@%s LF@%s\n", result_var, left_conv, right_conv);
        }
        else if (node->token->type == OP_LESS_THAN)
        {
            printf("LT %s LF@%s LF@%s\n", result_var, left_conv, right_conv);
        }
        else if (node->token->type == OP_GREATER_THAN)
        {
            printf("GT %s LF@%s LF@%s\n", result_var, left_conv, right_conv);
        }
        else if (node->token->type == OP_LESS_EQUAL)
        {
            printf("GT %s LF@%s LF@%s\n", result_var, left_conv, right_conv);
            printf("NOT %s %s\n", result_var, result_var);
        }
        else if (node->token->type == OP_GREATER_THAN_EQUAL)
        {
            printf("LT %s LF@%s LF@%s\n", result_var, left_conv, right_conv);
            printf("NOT %s %s\n", result_var, result_var);
        }

        printf("JUMP $$arith_end_%d\n", type_tmp);
        // Error label for null operands
        printf("LABEL $$arith_error_null_%d\n", type_tmp);
        printf("EXIT int@26\n");
        printf("LABEL $$arith_end_%d\n", type_tmp);

        break;
    }
    case OP_IS:
    {
        if (node->right == NULL || node->right->token == NULL || node->right->token->type != KEYWORD)
        {
            return ERR_INTERNAL;
        }

        e_keyword expected_type = node->right->token->value.keyword;
        const char *type_string = NULL;

        // Mapovanie typov
        switch (expected_type)
        {
        case KW_NUM:
            type_string = "float"; // Preferujeme float lebo IFJ25 Num je float
            break;
        case KW_STRING:
            type_string = "string";
            break;
        case KW_NULL_TYPE:
            type_string = "nil";
            break;
        default:
            return ERR_INTERNAL; // Neplatný typ pre 'is'
        }

        // Vytvoríme dočasné premenné pre type check
        int is_tmp = get_next_temp_var();
        char type_var[64];
        snprintf(type_var, sizeof(type_var), "__is_type_%d", is_tmp);
        symtable_add_function_var(global_symtable, type_var);

        // Získame typ ľavého operandu
        printf("TYPE LF@%s %s\n", type_var, left_var);

        // Porovnáme typ
        if (expected_type == KW_NUM)
        {
            // Num môže byť int ALEBO float
            printf("JUMPIFEQ $$is_true_%d LF@%s string@float\n", is_tmp, type_var);
            printf("JUMPIFEQ $$is_true_%d LF@%s string@int\n", is_tmp, type_var);
        }
        else
        {
            // String alebo Null - presná zhoda
            printf("JUMPIFEQ $$is_true_%d LF@%s string@%s\n", is_tmp, type_var, type_string);
        }

        // Typ nesedí - false
        printf("MOVE %s bool@false\n", result_var);
        printf("JUMP $$is_end_%d\n", is_tmp);

        // Typ sedí - true
        printf("LABEL $$is_true_%d\n", is_tmp);
        printf("MOVE %s bool@true\n", result_var);

        // Koniec
        printf("LABEL $$is_end_%d\n", is_tmp);

        break;
    }
    default:
        return ERR_INTERNAL;
    }

    return 0;
}

void generate_assignment(const char *var_name, t_ast_node *ast)
{
    if (ast == NULL)
    {
        exit_with_error(ERR_INTERNAL, "Internal error: NULL AST in generate_assignment");
    }

    // Nájdeme premennú viditeľnú z aktuálneho bloku
    t_avl_node *var_node = symtable_search_var_scoped(global_symtable, var_name);
    if (var_node == NULL)
    {
        exit_with_error(ERR_INTERNAL, "Internal error: Variable '%s' not found in symbol table", var_name);
    }

    int block_id = var_node->ifj_data.ifj_var.ifj_block_id;
    const char *temp_name = get_var_name_with_nesting(var_name, block_id);

    // IMPORTANT: Copy to local buffer because get_var_name_with_nesting uses static buffer
    char unique_name[256];
    strncpy(unique_name, temp_name, sizeof(unique_name) - 1);
    unique_name[sizeof(unique_name) - 1] = '\0';

    // Ak je to jednoduchý literál alebo premenná, priamo priradíme
    if (ast->left == NULL && ast->right == NULL)
    {
        // Skontrolujeme či to nie je getter
        if (ast->token != NULL && ast->token->type == IDENTIFIER && global_symtable != NULL)
        {
            if (generate_getter_call_if_needed(ast->token->value.string, global_symtable))
            {
                // Je to getter - zavolali sme ho, výsledok je na zásobníku
                // Popneme ho do cieľovej premennej
                printf("POPS LF@%s\n", unique_name);
                return;
            }
        }

        // Nie je to getter - štandardné spracovanie
        char value_str[512];
        get_value_string(ast, value_str, sizeof(value_str));
        printf("MOVE LF@%s %s\n", unique_name, value_str);
    }
    else
    {
        // Komplexný výraz - použijeme generate_expression_code
        char result_var[256];
        int err = generate_expression_code(ast, result_var, sizeof(result_var));
        if (err)
        {
            exit_with_error(ERR_INTERNAL, "Internal error: Failed to generate expression code");
        }
        // Presunieme výsledok do cieľovej premennej
        printf("MOVE LF@%s %s\n", unique_name, result_var);
    }
}

void generate_global_assignment(const char *var_name, t_ast_node *ast)
{
    if (ast == NULL)
    {
        exit_with_error(ERR_INTERNAL, "Internal error: NULL AST in generate_global_assignment");
    }

    // Ak je to jednoduchý literál alebo premenná, priamo priradíme
    if (ast->left == NULL && ast->right == NULL)
    {
        char value_str[512];
        get_value_string(ast, value_str, sizeof(value_str));
        printf("MOVE GF@__%s %s\n", var_name, value_str);
    }
    else
    {
        // Komplexný výraz - použijeme generate_expression_code
        char result_var[256];
        int err = generate_expression_code(ast, result_var, sizeof(result_var));
        if (err)
        {
            exit_with_error(ERR_INTERNAL, "Internal error: Failed to generate expression code");
        }
        // Presunieme výsledok do globálnej premennej
        printf("MOVE GF@__%s %s\n", var_name, result_var);
    }
}

void generate_push_global_variable(const char *var_name)
{
    // Globálna premenná už je deklarovaná na začiatku programu
    printf("PUSHS GF@__%s\n", var_name);
}

/**
 * @brief Generuje začiatok if-else konštrukcie (vyhodnotenie podmienky)
 * @param condition_ast AST uzol s podmienkou
 * @param label_id Unikátne ID pre labely
 */
void generate_if_start(t_ast_node *condition_ast, int label_id)
{
    // Generujeme kód pre vyhodnotenie podmienky
    char result_var[256];
    int err = generate_expression_code(condition_ast, result_var, sizeof(result_var));
    if (err)
    {
        exit_with_error(ERR_INTERNAL, "Internal error: Failed to generate condition expression");
    }

    // Vytvoríme pomocnú premennú pre podmienku (pridáme do zoznamu function vars)
    char cond_var_name[64];
    snprintf(cond_var_name, sizeof(cond_var_name), "__if_cond_%d", label_id);
    symtable_add_function_var(global_symtable, cond_var_name);
    
    // Uložíme výsledok podmienky do lokálnej premennej
    printf("MOVE LF@%s %s\n", cond_var_name, result_var);

    // Rozlíšime dva prípady:
    // 1. Jednoduchá premenná/literál (if (__a)): iba nil je falsy
    // 2. Výraz s operátormi (if (a > b)): nil a false sú falsy
    
    bool is_simple_value = (condition_ast->left == NULL && condition_ast->right == NULL);
    
    if (is_simple_value)
    {
        // Pre jednoduchú premennú: iba nil je falsy
        printf("JUMPIFEQ $$if_else_%d LF@%s nil@nil\n", label_id, cond_var_name);
    }
    else
    {
        // Pre výrazy s operátormi: nil a false sú falsy
        printf("JUMPIFEQ $$if_else_%d LF@%s nil@nil\n", label_id, cond_var_name);
        printf("JUMPIFEQ $$if_else_%d LF@%s bool@false\n", label_id, cond_var_name);
    }
    
    // Ak podmienka je true, pokračujeme do then bloku (kód nasleduje)
}

/**
 * @brief Generuje prechod medzi then a else vetvou
 * @param label_id Unikátne ID pre labely
 */
void generate_if_else_start(int label_id)
{
    // Skočíme na koniec (preskočíme else vetvu)
    printf("JUMP $$if_end_%d\n", label_id);

    // Label pre else vetvu
    printf("LABEL $$if_else_%d\n", label_id);
}

/**
 * @brief Generuje koniec if-else konštrukcie
 * @param label_id Unikátne ID pre labely
 */
void generate_if_end(int label_id)
{
    printf("LABEL $$if_end_%d\n", label_id);
}

/**
 * @brief Generuje začiatok while cyklu
 * @param label_id Unikátne ID pre labely
 */
void generate_while_start(int label_id)
{
    // Label pre začiatok cyklu (sem sa vrátime po každej iterácii)
    printf("LABEL $$while_start_%d\n", label_id);
}

/**
 * @brief Generuje vyhodnotenie podmienky while cyklu
 * @param condition_ast AST uzol s podmienkou
 * @param label_id Unikátne ID pre labely
 */
void generate_while_condition(t_ast_node *condition_ast, int label_id)
{
    // Generujeme kód pre vyhodnotenie podmienky
    char result_var[256];
    int err = generate_expression_code(condition_ast, result_var, sizeof(result_var));
    if (err)
    {
        exit_with_error(ERR_INTERNAL, "Internal error: Failed to generate while condition");
    }

    // Create temporary variable for condition (will be added to function vars list)
    char cond_var_name[64];
    snprintf(cond_var_name, sizeof(cond_var_name), "__while_cond_%d", label_id);
    symtable_add_function_var(global_symtable, cond_var_name);

    // Uložíme výsledok podmienky
    printf("MOVE LF@%s %s\n", cond_var_name, result_var);

    // Kontrola pravdivosti - ak je false alebo null, opustime cyklus
    printf("JUMPIFEQ $$while_end_%d LF@%s nil@nil\n", label_id, cond_var_name);
    printf("JUMPIFEQ $$while_end_%d LF@%s bool@false\n", label_id, cond_var_name);

    // Ak podmienka je true, pokračujeme do tela cyklu (kód nasleduje)
}

/**
 * @brief Generuje koniec while cyklu
 * @param label_id Unikátne ID pre labely
 */
void generate_while_end(int label_id)
{
    // Skočíme späť na začiatok (kontrola podmienky)
    printf("JUMP $$while_start_%d\n", label_id);

    // Label pre koniec cyklu
    printf("LABEL $$while_end_%d\n", label_id);
}

/**
 * @brief Generuje definície všetkých builtin funkcií ako callable subroutines
 */
void generate_builtin_function_definitions()
{
    // floor(x) - int or float -> int
    printf("\n# Builtin function: floor\n");
    printf("LABEL $$__builtin_floor\n");
    printf("CREATEFRAME\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%param\n");
    printf("DEFVAR LF@%%result\n");
    printf("DEFVAR LF@%%type\n");
    printf("POPS LF@%%param\n");
    printf("TYPE LF@%%type LF@%%param\n");
    printf("JUMPIFEQ $$floor_is_int LF@%%type string@int\n");
    printf("JUMPIFEQ $$floor_is_float LF@%%type string@float\n");
    printf("EXIT int@25\n"); // Type error
    printf("LABEL $$floor_is_int\n");
    printf("MOVE LF@%%result LF@%%param\n");
    printf("JUMP $$floor_end\n");
    printf("LABEL $$floor_is_float\n");
    printf("FLOAT2INT LF@%%result LF@%%param\n");
    printf("LABEL $$floor_end\n");
    printf("PUSHS LF@%%result\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    // str(x) - any -> string
    printf("\n# Builtin function: str\n");
    printf("LABEL $$__builtin_str\n");
    printf("CREATEFRAME\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%param\n");
    printf("DEFVAR LF@%%result\n");
    printf("DEFVAR LF@%%type\n");
    printf("POPS LF@%%param\n");
    printf("TYPE LF@%%type LF@%%param\n");
    printf("JUMPIFEQ $$str_is_string LF@%%type string@string\n");
    printf("JUMPIFEQ $$str_is_int LF@%%type string@int\n");
    printf("JUMPIFEQ $$str_is_float LF@%%type string@float\n");
    printf("JUMPIFEQ $$str_is_bool LF@%%type string@bool\n");
    printf("JUMPIFEQ $$str_is_nil LF@%%type string@nil\n");
    printf("EXIT int@99\n"); // Unknown type
    printf("LABEL $$str_is_string\n");
    printf("MOVE LF@%%result LF@%%param\n");
    printf("JUMP $$str_end\n");
    printf("LABEL $$str_is_int\n");
    printf("INT2STR LF@%%result LF@%%param\n");
    printf("JUMP $$str_end\n");
    printf("LABEL $$str_is_float\n");
    // Check if float is whole number
    printf("DEFVAR LF@%%isint\n");
    printf("DEFVAR LF@%%temp\n");
    printf("ISINT LF@%%isint LF@%%param\n");
    printf("JUMPIFEQ $$str_float_is_int LF@%%isint bool@true\n");
    // Not whole number, use float to string
    printf("FLOAT2STR LF@%%result LF@%%param\n");
    printf("JUMP $$str_end\n");
    printf("LABEL $$str_float_is_int\n");
    // Whole number, convert to int then to string
    printf("FLOAT2INT LF@%%temp LF@%%param\n");
    printf("INT2STR LF@%%result LF@%%temp\n");
    printf("JUMP $$str_end\n");
    printf("LABEL $$str_is_bool\n");
    printf("JUMPIFEQ $$str_bool_true LF@%%param bool@true\n");
    printf("MOVE LF@%%result string@false\n");
    printf("JUMP $$str_end\n");
    printf("LABEL $$str_bool_true\n");
    printf("MOVE LF@%%result string@true\n");
    printf("JUMP $$str_end\n");
    printf("LABEL $$str_is_nil\n");
    printf("MOVE LF@%%result string@null\n");
    printf("LABEL $$str_end\n");
    printf("PUSHS LF@%%result\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    // length(s) - string -> int
    printf("\n# Builtin function: length\n");
    printf("LABEL $$__builtin_length\n");
    printf("CREATEFRAME\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%param\n");
    printf("DEFVAR LF@%%result\n");
    printf("DEFVAR LF@%%type\n");
    printf("POPS LF@%%param\n");
    printf("TYPE LF@%%type LF@%%param\n");
    printf("JUMPIFEQ $$length_ok LF@%%type string@string\n");
    printf("EXIT int@25\n"); // Type error
    printf("LABEL $$length_ok\n");
    printf("STRLEN LF@%%result LF@%%param\n");
    printf("PUSHS LF@%%result\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    // chr(i) - int -> string
    printf("\n# Builtin function: chr\n");
    printf("LABEL $$__builtin_chr\n");
    printf("CREATEFRAME\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%param\n");
    printf("DEFVAR LF@%%result\n");
    printf("DEFVAR LF@%%type\n");
    printf("DEFVAR LF@%%param_int\n");
    printf("DEFVAR LF@%%isint\n");
    printf("POPS LF@%%param\n");
    printf("TYPE LF@%%type LF@%%param\n");
    // Check if parameter is int or convertible float
    printf("JUMPIFEQ $$chr_param_is_int LF@%%type string@int\n");
    printf("JUMPIFEQ $$chr_param_is_float LF@%%type string@float\n");
    printf("EXIT int@25\n"); // Type error - not int or float
    printf("LABEL $$chr_param_is_float\n");
    printf("ISINT LF@%%isint LF@%%param\n");
    printf("JUMPIFEQ $$chr_can_convert LF@%%isint bool@true\n");
    printf("EXIT int@26\n"); // Operand type error - float is not whole number
    printf("LABEL $$chr_can_convert\n");
    printf("FLOAT2INT LF@%%param_int LF@%%param\n");
    printf("JUMP $$chr_ok\n");
    printf("LABEL $$chr_param_is_int\n");
    printf("MOVE LF@%%param_int LF@%%param\n");
    printf("LABEL $$chr_ok\n");
    printf("INT2CHAR LF@%%result LF@%%param_int\n");
    printf("PUSHS LF@%%result\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    // strcmp(s1, s2) - string, string -> int
    printf("\n# Builtin function: strcmp\n");
    printf("LABEL $$__builtin_strcmp\n");
    printf("CREATEFRAME\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%param1\n");
    printf("DEFVAR LF@%%param2\n");
    printf("DEFVAR LF@%%result\n");
    printf("DEFVAR LF@%%type1\n");
    printf("DEFVAR LF@%%type2\n");
    printf("POPS LF@%%param2\n"); // Pop in reverse order
    printf("POPS LF@%%param1\n");
    printf("TYPE LF@%%type1 LF@%%param1\n");
    printf("TYPE LF@%%type2 LF@%%param2\n");
    printf("JUMPIFEQ $$strcmp_type1_ok LF@%%type1 string@string\n");
    printf("EXIT int@25\n"); // Type error
    printf("LABEL $$strcmp_type1_ok\n");
    printf("JUMPIFEQ $$strcmp_type2_ok LF@%%type2 string@string\n");
    printf("EXIT int@25\n"); // Type error
    printf("LABEL $$strcmp_type2_ok\n");
    printf("LT LF@%%result LF@%%param1 LF@%%param2\n");
    printf("JUMPIFEQ $$strcmp_less LF@%%result bool@true\n");
    printf("GT LF@%%result LF@%%param1 LF@%%param2\n");
    printf("JUMPIFEQ $$strcmp_greater LF@%%result bool@true\n");
    printf("MOVE LF@%%result int@0\n");
    printf("JUMP $$strcmp_end\n");
    printf("LABEL $$strcmp_less\n");
    printf("MOVE LF@%%result int@-1\n");
    printf("JUMP $$strcmp_end\n");
    printf("LABEL $$strcmp_greater\n");
    printf("MOVE LF@%%result int@1\n");
    printf("LABEL $$strcmp_end\n");
    printf("PUSHS LF@%%result\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    // ord(s, i) - string, int -> int or null
    printf("\n# Builtin function: ord\n");
    printf("LABEL $$__builtin_ord\n");
    printf("CREATEFRAME\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%param1\n");
    printf("DEFVAR LF@%%param2\n");
    printf("DEFVAR LF@%%result\n");
    printf("DEFVAR LF@%%type1\n");
    printf("DEFVAR LF@%%type2\n");
    printf("DEFVAR LF@%%len\n");
    printf("DEFVAR LF@%%cmp\n");
    printf("DEFVAR LF@%%param2_int\n");
    printf("DEFVAR LF@%%isint\n");
    printf("POPS LF@%%param2\n"); // Pop in reverse order
    printf("POPS LF@%%param1\n");
    printf("TYPE LF@%%type1 LF@%%param1\n");
    printf("TYPE LF@%%type2 LF@%%param2\n");
    
    // Check first parameter is string
    printf("JUMPIFEQ $$ord_type1_ok LF@%%type1 string@string\n");
    printf("EXIT int@25\n"); // Type error
    printf("LABEL $$ord_type1_ok\n");
    
    // Check second parameter is int or float
    printf("JUMPIFEQ $$ord_param2_is_int LF@%%type2 string@int\n");
    printf("JUMPIFEQ $$ord_param2_is_float LF@%%type2 string@float\n");
    printf("EXIT int@25\n"); // Type error - not int or float
    
    // Convert float to int if it's a whole number
    printf("LABEL $$ord_param2_is_float\n");
    printf("ISINT LF@%%isint LF@%%param2\n");
    printf("JUMPIFEQ $$ord_can_convert LF@%%isint bool@true\n");
    printf("EXIT int@26\n"); // Operand type error - float is not a whole number
    printf("LABEL $$ord_can_convert\n");
    printf("FLOAT2INT LF@%%param2_int LF@%%param2\n");
    printf("JUMP $$ord_type2_ok\n");
    
    // Parameter is already int
    printf("LABEL $$ord_param2_is_int\n");
    printf("MOVE LF@%%param2_int LF@%%param2\n");
    
    printf("LABEL $$ord_type2_ok\n");
    printf("STRLEN LF@%%len LF@%%param1\n");
    printf("LT LF@%%cmp LF@%%param2_int int@0\n");
    printf("JUMPIFEQ $$ord_null LF@%%cmp bool@true\n");
    printf("LT LF@%%cmp LF@%%param2_int LF@%%len\n");
    printf("JUMPIFEQ $$ord_ok LF@%%cmp bool@true\n");
    printf("LABEL $$ord_null\n");
    printf("PUSHS nil@nil\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("LABEL $$ord_ok\n");
    printf("STRI2INT LF@%%result LF@%%param1 LF@%%param2_int\n");
    printf("PUSHS LF@%%result\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    // substring(s, i, j) - string, int, int -> string or null
    printf("\n# Builtin function: substring\n");
    printf("LABEL $$__builtin_substring\n");
    printf("CREATEFRAME\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%param1\n");
    printf("DEFVAR LF@%%param2\n");
    printf("DEFVAR LF@%%param3\n");
    printf("DEFVAR LF@%%result\n");
    printf("DEFVAR LF@%%type1\n");
    printf("DEFVAR LF@%%type2\n");
    printf("DEFVAR LF@%%type3\n");
    printf("DEFVAR LF@%%len\n");
    printf("DEFVAR LF@%%cmp\n");
    printf("DEFVAR LF@%%idx\n");
    printf("DEFVAR LF@%%char\n");
    printf("DEFVAR LF@%%param2_int\n");
    printf("DEFVAR LF@%%param3_int\n");
    printf("DEFVAR LF@%%isint2\n");
    printf("DEFVAR LF@%%isint3\n");
    printf("POPS LF@%%param3\n"); // Pop in reverse order
    printf("POPS LF@%%param2\n");
    printf("POPS LF@%%param1\n");
    printf("TYPE LF@%%type1 LF@%%param1\n");
    printf("TYPE LF@%%type2 LF@%%param2\n");
    printf("TYPE LF@%%type3 LF@%%param3\n");
    // Check param1 is string
    printf("JUMPIFEQ $$substring_type1_ok LF@%%type1 string@string\n");
    printf("EXIT int@25\n"); // Type error
    printf("LABEL $$substring_type1_ok\n");
    // Check param2 (i) is int or convertible float
    printf("JUMPIFEQ $$substring_param2_is_int LF@%%type2 string@int\n");
    printf("JUMPIFEQ $$substring_param2_is_float LF@%%type2 string@float\n");
    printf("EXIT int@25\n"); // Type error - not int or float
    printf("LABEL $$substring_param2_is_float\n");
    printf("ISINT LF@%%isint2 LF@%%param2\n");
    printf("JUMPIFEQ $$substring_param2_convert LF@%%isint2 bool@true\n");
    printf("EXIT int@26\n"); // Operand type error - float is not whole number
    printf("LABEL $$substring_param2_convert\n");
    printf("FLOAT2INT LF@%%param2_int LF@%%param2\n");
    printf("MOVE LF@%%param2 LF@%%param2_int\n");
    printf("LABEL $$substring_param2_is_int\n");
    // Check param3 (j) is int or convertible float
    printf("JUMPIFEQ $$substring_param3_is_int LF@%%type3 string@int\n");
    printf("JUMPIFEQ $$substring_param3_is_float LF@%%type3 string@float\n");
    printf("EXIT int@25\n"); // Type error - not int or float
    printf("LABEL $$substring_param3_is_float\n");
    printf("ISINT LF@%%isint3 LF@%%param3\n");
    printf("JUMPIFEQ $$substring_param3_convert LF@%%isint3 bool@true\n");
    printf("EXIT int@26\n"); // Operand type error - float is not whole number
    printf("LABEL $$substring_param3_convert\n");
    printf("FLOAT2INT LF@%%param3_int LF@%%param3\n");
    printf("MOVE LF@%%param3 LF@%%param3_int\n");
    printf("LABEL $$substring_param3_is_int\n");
    printf("STRLEN LF@%%len LF@%%param1\n");
    // Check if i < 0 or j < 0 or i > j or i >= len
    printf("LT LF@%%cmp LF@%%param2 int@0\n");
    printf("JUMPIFEQ $$substring_null LF@%%cmp bool@true\n");
    printf("LT LF@%%cmp LF@%%param3 int@0\n");
    printf("JUMPIFEQ $$substring_null LF@%%cmp bool@true\n");
    printf("GT LF@%%cmp LF@%%param2 LF@%%param3\n");
    printf("JUMPIFEQ $$substring_null LF@%%cmp bool@true\n");
    printf("GT LF@%%cmp LF@%%param2 LF@%%len\n");
    printf("JUMPIFEQ $$substring_null LF@%%cmp bool@true\n");
    printf("EQ LF@%%cmp LF@%%param2 LF@%%len\n");
    printf("JUMPIFEQ $$substring_null LF@%%cmp bool@true\n");
    // Adjust j if j > len
    printf("GT LF@%%cmp LF@%%param3 LF@%%len\n");
    printf("JUMPIFEQ $$substring_adjust_j LF@%%cmp bool@true\n");
    printf("JUMP $$substring_loop_init\n");
    printf("LABEL $$substring_adjust_j\n");
    printf("MOVE LF@%%param3 LF@%%len\n");
    printf("LABEL $$substring_loop_init\n");
    printf("MOVE LF@%%result string@\n");
    printf("MOVE LF@%%idx LF@%%param2\n");
    printf("LABEL $$substring_loop\n");
    printf("LT LF@%%cmp LF@%%idx LF@%%param3\n");
    printf("JUMPIFEQ $$substring_loop_body LF@%%cmp bool@true\n");
    printf("JUMP $$substring_end\n");
    printf("LABEL $$substring_loop_body\n");
    printf("GETCHAR LF@%%char LF@%%param1 LF@%%idx\n");
    printf("CONCAT LF@%%result LF@%%result LF@%%char\n");
    printf("ADD LF@%%idx LF@%%idx int@1\n");
    printf("JUMP $$substring_loop\n");
    printf("LABEL $$substring_null\n");
    printf("PUSHS nil@nil\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("LABEL $$substring_end\n");
    printf("PUSHS LF@%%result\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
}
