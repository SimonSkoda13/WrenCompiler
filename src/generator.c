/*
 * Prekladač jazyka IFJ25
 * VUT FIT
 *
 * Autori:
 *   - Martin Michálik (xmicham00)
 */

#include "generator.h"
#include "symtable.h"

// Globálna premenná pre prístup k tabuľke symbolov pri generovaní kódu
static t_symtable *global_symtable = NULL;

// Počítadlo pre unikátne labely pre if/while konštrukcie
static int ifj_label_counter = 0;

// Sada už definovaných temp premenných (aby sme ich nedefinovali dvakrát v while loope)
static bool temp_vars_defined[1000] = {false}; // podporíme až 1000 temp vars

void generator_set_symtable(t_symtable *ifj_symtable)
{
    global_symtable = ifj_symtable;
}

t_symtable *get_global_symtable()
{
    return global_symtable;
}

// Helper function to search for variable (local or global)
static t_avl_node *search_variable(const char *var_name)
{
    // First try scoped search for local variables
    t_avl_node *node = symtable_search_var_scoped(global_symtable, var_name);

    // If not found, try global search
    if (node == NULL)
    {
        node = symtable_search(global_symtable, var_name);
    }

    return node;
}

int get_next_label_id()
{
    return ifj_label_counter++;
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
const char *get_var_name_with_nesting(const char *var_name, int nesting_level)
{
    static char buffer[256];
    if (nesting_level == 0)
    {
        // Parameters have no suffix
        snprintf(buffer, sizeof(buffer), "%s", var_name);
    }
    else
    {
        // Nested variables get suffix with nesting level
        snprintf(buffer, sizeof(buffer), "%s$n%d", var_name, nesting_level);
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
    t_avl_node *var_node = search_variable(var_id);
    if (var_node != NULL && var_node->ifj_sym_type == SYM_VAR_GLOBAL)
    {
        // Global variables use GF@ frame
        printf("WRITE GF@%s\n", var_id);
    }
    else if (var_node != NULL && var_node->ifj_sym_type == SYM_VAR_LOCAL)
    {
        int nesting = var_node->ifj_data.ifj_var.ifj_nesting_level;
        const char *unique_name = get_var_name_with_nesting(var_id, nesting);
        printf("WRITE LF@%s\n", unique_name);
    }
    else
    {
        // Parameter or unknown - use as is
        printf("WRITE LF@%s\n", var_id);
    }
}

void generate_var_declaration(const char *var_name)
{
    // Get nesting level from global symtable
    int nesting = global_symtable->ifj_current_nesting;
    const char *unique_name = get_var_name_with_nesting(var_name, nesting);
    printf("DEFVAR LF@%s\n", unique_name);
}

void generate_function_start(const char *func_name, const char *mangled_name, t_param_list *params)
{
    // Vygenerujeme label pre funkciu
    if (strcmp(func_name, "main") == 0)
    {
        printf("LABEL $$main\n");
    }
    else
    {
        printf("LABEL $%s\n", mangled_name);
    }

    // Vytvoríme a aktivujeme lokálny rámec
    printf("CREATEFRAME\n");
    printf("PUSHFRAME\n");

    // Preddeklarujeme globálne temp premenné (pre while/if výrazy)
    // (aby sme ich nemuseli deklarovať vo vnútri while loopu)
    printf("# Temp variables for expressions\n");
    for (int i = 0; i < 50; i++)
    {
        printf("DEFVAR LF@__tmp%d\n", i);
    }
    printf("\n");

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

void generate_function_end(const char *func_name)
{
    printf("POPFRAME\n");
    if (strcmp(func_name, "main") != 0)
    {
        printf("RETURN\n");
    }
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
    // Nájdeme premennú
    t_avl_node *var_node = search_variable(var_name);
    if (var_node != NULL && var_node->ifj_sym_type == SYM_VAR_GLOBAL)
    {
        // Global variable - use GF@ frame
        printf("PUSHS GF@%s\n", var_name);
    }
    else if (var_node != NULL && var_node->ifj_sym_type == SYM_VAR_LOCAL)
    {
        // Local variable - use LF@ frame with nesting suffix
        int nesting = var_node->ifj_data.ifj_var.ifj_nesting_level;
        const char *unique_name = get_var_name_with_nesting(var_name, nesting);
        printf("PUSHS LF@%s\n", unique_name);
    }
    else
    {
        // Parameter - use LF@ frame as is
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
    // Nájdeme premennú
    t_avl_node *var_node = search_variable(var_name);
    if (var_node != NULL && var_node->ifj_sym_type == SYM_VAR_GLOBAL)
    {
        // Global variable - use GF@ frame
        printf("POPS GF@%s\n", var_name);
    }
    else if (var_node != NULL && var_node->ifj_sym_type == SYM_VAR_LOCAL)
    {
        // Local variable - use LF@ frame with nesting suffix
        int nesting = var_node->ifj_data.ifj_var.ifj_nesting_level;
        const char *unique_name = get_var_name_with_nesting(var_name, nesting);
        printf("POPS LF@%s\n", unique_name);
    }
    else
    {
        // Parameter - use LF@ frame as is
        printf("POPS LF@%s\n", var_name);
    }
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
    case KEYWORD:
        // Handle null literal
        if (node->token->value.keyword == KW_NULL_INST || node->token->value.keyword == KW_NULL_TYPE)
        {
            snprintf(result, result_size, "nil@nil");
        }
        else
        {
            result[0] = '\0';
        }
        break;
    case IDENTIFIER:
    case GLOBAL_VAR:
    {
        // Nájdeme premennú
        t_avl_node *var_node = search_variable(node->token->value.string);
        if (var_node != NULL && var_node->ifj_sym_type == SYM_VAR_GLOBAL)
        {
            // Global variable - use GF@ frame
            snprintf(result, result_size, "GF@%s", node->token->value.string);
        }
        else if (var_node != NULL && var_node->ifj_sym_type == SYM_VAR_LOCAL)
        {
            // Local variable - use LF@ frame with nesting suffix
            int nesting = var_node->ifj_data.ifj_var.ifj_nesting_level;
            const char *unique_name = get_var_name_with_nesting(node->token->value.string, nesting);
            snprintf(result, result_size, "LF@%s", unique_name);
        }
        else
        {
            // Parameter or not found - use as is with LF@
            snprintf(result, result_size, "LF@%s", node->token->value.string);
        }
        break;
    }
    default:
        result[0] = '\0';
        break;
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
                snprintf(result_var, result_var_size, "LF@__tmp%d", tmp_num);

                // DEFVAR už je globálne preddeklarovaný

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
        generate_expression_code(node->left, left_var, sizeof(left_var));

        // Ak ľavý operand nie je jednoduchá hodnota, musíme ho uložiť do temp premennej
        if (node->left->left != NULL || node->left->right != NULL)
        {
            int temp_num = get_next_temp_var();
            char temp_name[256];
            snprintf(temp_name, sizeof(temp_name), "LF@__tmp%d", temp_num);

            // DEFVAR už je globálne preddeklarovaný

            printf("MOVE %s %s\n", temp_name, left_var);
            strncpy(left_var, temp_name, sizeof(left_var) - 1);
        }
    }

    // Spracujeme pravý podstrom
    if (node->right != NULL)
    {
        generate_expression_code(node->right, right_var, sizeof(right_var));

        // Ak pravý operand nie je jednoduchá hodnota, musíme ho uložiť do temp premennej
        if (node->right->left != NULL || node->right->right != NULL)
        {
            int temp_num = get_next_temp_var();
            char temp_name[256];
            snprintf(temp_name, sizeof(temp_name), "LF@__tmp%d", temp_num);

            // DEFVAR už je globálne preddeklarovaný

            printf("MOVE %s %s\n", temp_name, right_var);
            strncpy(right_var, temp_name, sizeof(right_var) - 1);
        }
    }

    // Vytvoríme premennú pre výsledok tejto operácie
    int result_temp_num = get_next_temp_var();
    snprintf(result_var, result_var_size, "LF@__tmp%d", result_temp_num);

    // DEFVAR už je globálne preddeklarovaný v parse_program(), takže ho skipneme

    // Vygenerujeme inštrukciu podľa typu operátora
    switch (node->token->type)
    {
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

void generate_assignment(const char *var_name, t_ast_node *ast)
{
    if (ast == NULL)
    {
        exit_with_error(ERR_INTERNAL, "Internal error: NULL AST in generate_assignment");
    }

    // Find variable (local or global)
    t_avl_node *var_node = search_variable(var_name);

    if (var_node == NULL)
    {
        exit_with_error(ERR_INTERNAL, "Internal error: Variable '%s' not found in symbol table", var_name);
    }

    // Check if it's a global variable
    bool is_global = (var_node->ifj_sym_type == SYM_VAR_GLOBAL);
    char frame_prefix[32];
    char unique_name[256];

    if (is_global)
    {
        // Global variables use GF@ frame and no nesting suffix
        strcpy(frame_prefix, "GF@");
        strncpy(unique_name, var_name, sizeof(unique_name) - 1);
        unique_name[sizeof(unique_name) - 1] = '\0';
    }
    else
    {
        // Local variables use LF@ frame and nesting suffix
        strcpy(frame_prefix, "LF@");
        int nesting = var_node->ifj_data.ifj_var.ifj_nesting_level;
        const char *temp_unique_name = get_var_name_with_nesting(var_name, nesting);

        // MUST COPY because get_var_name_with_nesting uses static buffer!
        strncpy(unique_name, temp_unique_name, sizeof(unique_name) - 1);
        unique_name[sizeof(unique_name) - 1] = '\0';
    }

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
                printf("POPS %s%s\n", frame_prefix, unique_name);
                return;
            }
        }

        // Nie je to getter - štandardné spracovanie
        char value_str[512];
        get_value_string(ast, value_str, sizeof(value_str));
        printf("MOVE %s%s %s\n", frame_prefix, unique_name, value_str);
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
        printf("MOVE %s%s %s\n", frame_prefix, unique_name, result_var);
    }
}

/**
 * @brief Generuje začiatok if-else konštrukcie (vyhodnotenie podmienky)
 * @param condition_ast AST uzol s podmienkou
 * @param label_id Unikátne ID pre labely
 */
void generate_if_start(t_ast_node *condition_ast, int label_id, bool skip_defvar)
{
    // Generujeme kód pre vyhodnotenie podmienky
    char result_var[256];
    int err = generate_expression_code(condition_ast, result_var, sizeof(result_var));
    if (err)
    {
        exit_with_error(ERR_INTERNAL, "Internal error: Failed to generate condition expression");
    }

    // Uložíme výsledok podmienky do temporary premennej
    // Skip DEFVAR if we're inside while loop (already predeclared)
    if (!skip_defvar)
    {
        printf("DEFVAR LF@%%if_cond_%d\n", label_id);
    }
    printf("MOVE LF@%%if_cond_%d %s\n", label_id, result_var);

    // Kontrola pravdivosti podľa zadania:
    // - null = false -> skok na else
    // - bool@false = false -> skok na else
    // - všetko ostatné = true -> pokračujeme do then vetvy

    printf("JUMPIFEQ $$if_else_%d LF@%%if_cond_%d nil@nil\n", label_id, label_id);
    printf("JUMPIFEQ $$if_else_%d LF@%%if_cond_%d bool@false\n", label_id, label_id);

    // Ak podmienka je true, pokračujeme do then bloku (LABEL nasleduje)
}

/**
 * @brief Generuje label pre then vetvu
 * @param label_id Unikátne ID pre labely
 */
void generate_if_then(int label_id)
{
    // Then blok začína tu (kód then bloku bude vygenerovaný parserom)
    // Na konci then bloku musíme preskočiť else vetvu
    (void)label_id; // Then blok je default flow, nepotrebujeme label
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

    // Uložíme výsledok podmienky (DEFVAR už je urobený v parseri)
    printf("MOVE LF@%%while_cond_%d %s\n", label_id, result_var);

    // Kontrola pravdivosti - ak je false alebo null, opustime cyklus
    printf("JUMPIFEQ $$while_end_%d LF@%%while_cond_%d nil@nil\n", label_id, label_id);
    printf("JUMPIFEQ $$while_end_%d LF@%%while_cond_%d bool@false\n", label_id, label_id);

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
