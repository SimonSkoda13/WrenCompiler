/*
 * Prekladač jazyka IFJ25
 * VUT FIT
 *
 * Autori:
 *   - Martin Michálik (xmicham00)
 */

#include "parser.h"
#include <string.h>

// Buffer pre odložené generovanie funkcie main
FILE *main_output_buffer = NULL;
bool is_generating_main = false;

// Tracking variables declared inside loops (for predeclaration)
#define MAX_LOOP_VARS 100
static char *loop_vars[MAX_LOOP_VARS];
static int loop_vars_count = 0;
static bool collecting_loop_vars = false;

// Tracking if statement labels inside loops (for predeclaration of %if_cond_X)
#define MAX_LOOP_IFS 50
static int loop_if_labels[MAX_LOOP_IFS];
static int loop_if_labels_count = 0;

// DEBUG
void ast_print_tree(t_ast_node *node, int depth)
{
    if (node == NULL)
    {
        return;
    }
    // Print indentation
    for (int i = 0; i < depth; i++)
    {
        printf("  ");
    }

    // Print node information
    switch (node->token->type)
    {
    case OP_ADD:
        printf("OP_ADD (+)\n");
        break;
    case OP_SUB:
        printf("OP_SUB (-)\n");
        break;
    case OP_MUL:
        printf("OP_MUL (*)\n");
        break;
    case OP_DIV:
        printf("OP_DIV (/)\n");
        break;
    case OP_LESS_THAN:
        printf("OP_LESS_THAN (<)\n");
        break;
    case OP_LESS_EQUAL:
        printf("OP_LESS_EQUAL (<=)\n");
        break;
    case OP_GREATER_THAN:
        printf("OP_GREATER_THAN (>)\n");
        break;
    case OP_GREATER_THAN_EQUAL:
        printf("OP_GREATER_THAN_EQUAL (>=)\n");
        break;
    case OP_EQUALS:
        printf("OP_EQUALS (==)\n");
        break;
    case OP_NOT_EQUALS:
        printf("OP_NOT_EQUALS (!=)\n");
        break;
    case OP_IS:
        printf("OP_IS (is)\n");
        break;
    case NUM_INT:
        printf("NUM_INT (%ld)\n", node->token->value.number_int);
        break;
    case NUM_FLOAT:
        printf("NUM_FLOAT (%g)\n", node->token->value.number_float);
        break;
    case NUM_EXP_INT:
        printf("NUM_EXP_INT (%ld)\n", node->token->value.number_int);
        break;
    case NUM_EXP_FLOAT:
        printf("NUM_EXP_FLOAT (%g)\n", node->token->value.number_float);
        break;
    case STRING_LITERAL:
        printf("STRING_LITERAL (\"%s\")\n", node->token->value.string);
        break;
    case IDENTIFIER:
        printf("IDENTIFIER (%s)\n", node->token->value.string);
        break;
    case GLOBAL_VAR:
        printf("GLOBAL_VAR (%s)\n", node->token->value.string);
        break;
    case KEYWORD:
        if (node->token->value.keyword == KW_NULL_TYPE ||
            node->token->value.keyword == KW_NULL_INST)
        {
            printf("KEYWORD (%s)\n", "null");
        }
        else if (node->token->value.keyword == KW_NUM)
        {
            printf("KEYWORD (%s)\n", "Num");
        }
        else if (node->token->value.keyword == KW_STRING)
        {
            printf("KEYWORD (%s)\n", "String");
        }
        else
            printf("KEYWORD (%s)\n", "unknown");
        break;
    default:
        printf("UNKNOWN_TYPE (%d)\n", node->token->type);
        break;
    }

    // Recursively print left and right children
    if (node->left != NULL)
    {
        ast_print_tree(node->left, depth + 1);
    }
    if (node->right != NULL)
    {
        ast_print_tree(node->right, depth + 1);
    }
}

void next_token()
{
    // Check if we have a putback token first
    if (parser.has_putback)
    {
        *parser.current_token = *parser.putback_token;
        parser.has_putback = false;
        return;
    }

    int lex_result = get_next_token(parser.scanner, parser.current_token);
    if (lex_result == 1)
    {
        exit_with_error(ERR_LEXICAL,
                        "Lexical error encountered while getting next token at line %d",
                        parser.scanner->line);
    }
}

void putback_token()
{
    if (parser.has_putback)
    {
        exit_with_error(ERR_INTERNAL,
                        "Internal error: Cannot putback token twice at line %d",
                        parser.scanner->line);
    }

    // Copy current token to putback storage
    *parser.putback_token = *parser.current_token;
    parser.has_putback = true;
}

void check_token(e_token_type expected_type)
{
    if (parser.current_token->type != expected_type)
    {
        exit_with_error(ERR_SYNTAX,
                        "Syntax error: Expected token type %d but got %d at line %d",
                        expected_type,
                        parser.current_token->type,
                        parser.scanner->line);
    }
}

void consume_token(e_token_type expected_type)
{
    next_token();
    check_token(expected_type);
}

bool check_token_type(e_token_type expected_type)
{
    return parser.current_token->type == expected_type;
}

void prolog()
{
    // Consume any leading whitespace or EOLs
    next_token();
    if (parser.current_token->type == EOL)
    {
        eols();
    }
    check_token(KEYWORD); // 'import'
    if (parser.current_token->value.keyword != KW_IMPORT)
    {
        exit_with_error(ERR_SYNTAX, "Syntax error: Expected 'import' keyword at line %d", parser.scanner->line);
    }
    consume_token(STRING_LITERAL); // "ifj25"
    if (parser.current_token->value.string == NULL || strcmp(parser.current_token->value.string, "ifj25") != 0)
    {
        exit_with_error(ERR_SYNTAX, "Syntax error: Expected 'ifj25' string literal at line %d", parser.scanner->line);
    }
    consume_token(KEYWORD); // 'for'
    if (parser.current_token->value.keyword != KW_FOR)
    {
        exit_with_error(ERR_SYNTAX, "Syntax error: Expected 'for' keyword at line %d", parser.scanner->line);
    }
    consume_token(KEYWORD); // Ifj
    if (parser.current_token->value.keyword != KW_IFJ)
    {
        exit_with_error(ERR_SYNTAX, "Syntax error: Expected 'Ifj' keyword at line %d", parser.scanner->line);
    }
    next_token();
    eols();
}

void program()
{
    check_token(KEYWORD); // 'class'
    if (parser.current_token->value.keyword != KW_CLASS)
    {
        exit_with_error(ERR_SYNTAX, "Syntax error: Expected 'class' keyword at line %d", parser.scanner->line);
    }
    consume_token(IDENTIFIER); // Program
    if (parser.current_token->value.string == NULL || strcmp(parser.current_token->value.string, "Program") != 0)
    {
        exit_with_error(ERR_SYNTAX, "Syntax error: Expected 'Program' identifier at line %d", parser.scanner->line);
    }
    consume_token(LEFT_BRACE); // '{'
    consume_token(EOL);
    func_list();
    check_token(RIGHT_BRACE); // '}'
}

void func_list()
{
    next_token();
    if (parser.current_token->type == EOL)
    {
        eols(); // consume any EOLs between functions
    }

    // Base case: end of class
    if (parser.current_token->type == RIGHT_BRACE)
    {
        return; // ✓ Stop recursion
    }

    if (parser.current_token->type != KEYWORD)
    {
        exit_with_error(ERR_SYNTAX, "Syntax error: Expected function declaration %d", parser.scanner->line);
    }
    else if (parser.current_token->type == KEYWORD && parser.current_token->value.keyword == KW_STATIC)
    {
        func();
    }
    else
    {
        exit_with_error(ERR_SYNTAX, "Unknown Syntax error at line %d", parser.scanner->line);
    }
    func_list();
}

void func()
{
    check_token(KEYWORD); // 'static'
    if (parser.current_token->value.keyword != KW_STATIC)
    {
        exit_with_error(ERR_SYNTAX, "Syntax error: Expected 'static' keyword at line %d", parser.scanner->line);
    }
    consume_token(IDENTIFIER); // function IDENTIFIER

    // Uložíme názov funkcie
    char *func_name = parser.current_token->value.string;

    next_token();
    // static getter
    if (parser.current_token->type == LEFT_BRACE)
    {
        // Getter - 0 parametrov, TYPE_UNKNOWN návratový typ
        // Vytvoríme manglované meno pre getter
        char *mangled_name = mangle_getter_name(func_name);
        if (mangled_name == NULL)
        {
            exit_with_error(ERR_INTERNAL, "Internal error: Failed to mangle getter name");
        }

        // Skontrolujeme či getter už neexistuje
        t_avl_node *existing = symtable_search(parser.symtable, mangled_name);
        if (existing != NULL)
        {
            free(mangled_name);
            exit_with_error(ERR_SEM_REDEF,
                            "Semantic error: Getter '%s' already declared at line %d",
                            func_name, parser.scanner->line);
        }

        bool success = symtable_insert_func(parser.symtable, mangled_name,
                                            0, NULL, TYPE_UNKNOWN, SYM_GETTER);

        if (!success)
        {
            exit_with_error(ERR_INTERNAL,
                            "Internal error: Failed to insert getter '%s' into symbol table at line %d",
                            func_name, parser.scanner->line);
        }
        t_param_list empty_params = {.count = 0};
        generate_function_start(func_name, mangled_name, &empty_params); // true = getter

        // Vstúpime do nového scope pre lokálne premenné gettera
        symtable_enter_scope(parser.symtable);

        // Nastavíme nesting level na 0 (getter nemá parametre)
        parser.symtable->ifj_current_nesting = 0;

        putback_token();
        block();

        // Opustíme scope
        symtable_exit_scope(parser.symtable);

        generate_function_end(func_name);
        free(mangled_name);
        return;
    }
    else if (parser.current_token->type == OP_ASSIGN)
    { // static setter
        // Setter - 1 parameter, TYPE_UNKNOWN typ
        // Vytvoríme manglované meno pre setter
        char *mangled_name = mangle_setter_name(func_name);
        if (mangled_name == NULL)
        {
            exit_with_error(ERR_INTERNAL, "Internal error: Failed to mangle setter name");
        }

        // Skontrolujeme či setter už neexistuje
        t_avl_node *existing = symtable_search(parser.symtable, mangled_name);
        if (existing != NULL)
        {
            free(mangled_name);
            exit_with_error(ERR_SEM_REDEF,
                            "Semantic error: Setter '%s' already declared at line %d",
                            func_name, parser.scanner->line);
        }

        e_data_type param_type = TYPE_UNKNOWN;
        bool success = symtable_insert_func(parser.symtable, mangled_name,
                                            1, &param_type, TYPE_UNKNOWN, SYM_SETTER);
        if (!success)
        {
            exit_with_error(ERR_INTERNAL,
                            "Internal error: Failed to insert setter '%s' into symbol table at line %d",
                            func_name, parser.scanner->line);
        }
        consume_token(LEFT_PAREN);
        consume_token(IDENTIFIER);
        char *param_name = parser.current_token->value.string;
        consume_token(RIGHT_PAREN);

        t_param_list setter_params = {.count = 1};
        setter_params.names[0] = param_name;
        generate_function_start(func_name, mangled_name, &setter_params); // false = nie getter
        free(mangled_name);

        // Vstúpime do nového scope pre lokálne premenné settera
        symtable_enter_scope(parser.symtable);

        // Nastavíme nesting level na 0 pre parametre
        parser.symtable->ifj_current_nesting = 0;

        // Vložíme parameter settera do symboltabuľky ako lokálnu premennú (nesting=0)
        bool param_success = symtable_insert_var(parser.symtable, param_name,
                                                 SYM_VAR_LOCAL, TYPE_UNKNOWN);
        if (!param_success)
        {
            exit_with_error(ERR_INTERNAL,
                            "Internal error: Failed to insert setter parameter '%s' into symbol table at line %d",
                            param_name, parser.scanner->line);
        }

        block();

        // Opustíme scope
        symtable_exit_scope(parser.symtable);

        generate_function_end(func_name);
        return;
    }

    check_token(LEFT_PAREN); // '('

    // Parsujeme zoznam parametrov
    t_param_list params;
    param_list(&params);
    check_token(RIGHT_PAREN); // ')'

    // Vytvoríme manglované meno pre vloženie do symtable
    char *mangled_name = mangle_function_name(func_name, params.count);
    if (mangled_name == NULL)
    {
        exit_with_error(ERR_INTERNAL, "Internal error: Failed to mangle function name");
    }

    // Skontrolujeme či funkcia s touto aritou už neexistuje
    t_avl_node *existing = symtable_search(parser.symtable, mangled_name);
    if (existing != NULL)
    {
        free(mangled_name);
        exit_with_error(ERR_SEM_REDEF,
                        "Semantic error: Function '%s' with %d parameters already declared at line %d",
                        func_name, params.count, parser.scanner->line);
    }

    // Vložíme funkciu do symbol table s počtom parametrov
    // TODO: Momentálne používame TYPE_UNKNOWN pre typy parametrov
    e_data_type *param_types = NULL;
    if (params.count > 0)
    {
        param_types = malloc(params.count * sizeof(e_data_type));
        for (int i = 0; i < params.count; i++)
        {
            param_types[i] = TYPE_UNKNOWN;
        }
    }

    bool success = symtable_insert_func(parser.symtable, mangled_name,
                                        params.count, param_types, TYPE_UNKNOWN, SYM_FUNCTION);
    if (param_types)
    {
        free(param_types);
    }

    if (!success)
    {
        exit_with_error(ERR_INTERNAL,
                        "Internal error: Failed to insert function '%s' into symbol table at line %d",
                        func_name, parser.scanner->line);
    }

    // Ak je to funkcia main, presmerujeme výstup do bufferu
    if (strcmp(func_name, "main") == 0)
    {
        main_output_buffer = tmpfile();
        if (main_output_buffer == NULL)
        {
            exit_with_error(ERR_INTERNAL,
                            "Internal error: Failed to create temporary buffer for main function");
        }

        // Uložíme pôvodný stdout a presmerujeme na buffer
        FILE *original_stdout = stdout;
        stdout = main_output_buffer;
        is_generating_main = true;

        // Vygenerujeme začiatok funkcie (LABEL, CREATEFRAME, PUSHFRAME)
        generate_function_start(func_name, mangled_name, &params); // false = nie getter

        // Vstúpime do nového scope pre lokálne premenné funkcie
        symtable_enter_scope(parser.symtable);

        // Nastavíme nesting level na 0 pre parametre
        parser.symtable->ifj_current_nesting = 0;

        // Vložíme parametre funkcie do symboltabuľky ako lokálne premenné (nesting=0)
        for (int i = 0; i < params.count; i++)
        {
            bool param_success = symtable_insert_var(parser.symtable, params.names[i],
                                                     SYM_VAR_LOCAL, TYPE_UNKNOWN);
            if (!param_success)
            {
                exit_with_error(ERR_INTERNAL,
                                "Internal error: Failed to insert parameter '%s' into symbol table at line %d",
                                params.names[i], parser.scanner->line);
            }
        }

        block();

        // Opustíme scope a vyčistíme lokálne premenné
        symtable_exit_scope(parser.symtable);

        // Vygenerujeme koniec funkcie (POPFRAME, RETURN)
        generate_function_end(func_name);

        // Obnovíme stdout
        stdout = original_stdout;
        is_generating_main = false;

        free(mangled_name);
    }
    else
    {
        // Pre ostatné funkcie generujeme priamo
        generate_function_start(func_name, mangled_name, &params);

        // Vstúpime do nového scope pre lokálne premenné funkcie
        symtable_enter_scope(parser.symtable);

        // Nastavíme nesting level na 0 pre parametre
        parser.symtable->ifj_current_nesting = 0;

        // Vložíme parametre funkcie do symboltabuľky ako lokálne premenné (nesting=0)
        for (int i = 0; i < params.count; i++)
        {
            bool param_success = symtable_insert_var(parser.symtable, params.names[i],
                                                     SYM_VAR_LOCAL, TYPE_UNKNOWN);
            if (!param_success)
            {
                exit_with_error(ERR_INTERNAL,
                                "Internal error: Failed to insert parameter '%s' into symbol table at line %d",
                                params.names[i], parser.scanner->line);
            }
        }

        block();

        // Opustíme scope a vyčistíme lokálne premenné
        symtable_exit_scope(parser.symtable);

        generate_function_end(func_name);
        free(mangled_name);
    }
}

void param_list(t_param_list *params)
{
    params->count = 0;

    next_token();
    if (parser.current_token->type == RIGHT_PAREN)
    {
        return;
    }
    check_token(IDENTIFIER);

    // Pridáme prvý parameter
    if (params->count < MAX_PARAMS)
    {
        params->names[params->count++] = parser.current_token->value.string;
    }
    else
    {
        exit_with_error(ERR_INTERNAL, "Internal error: Too many parameters at line %d", parser.scanner->line);
    }

    param_list_tail(params);
}

void param_list_tail(t_param_list *params)
{
    next_token();
    if (parser.current_token->type == COMMA)
    {
        consume_token(IDENTIFIER);

        // Pridáme ďalší parameter
        if (params->count < MAX_PARAMS)
        {
            params->names[params->count++] = parser.current_token->value.string;
        }
        else
        {
            exit_with_error(ERR_INTERNAL, "Internal error: Too many parameters at line %d", parser.scanner->line);
        }

        param_list_tail(params);
    }
    else if (parser.current_token->type == RIGHT_PAREN)
    {
        return;
    }
    else
    {
        exit_with_error(ERR_SYNTAX, "Syntax error: Expected ',' or ')' at line %d", parser.scanner->line);
    }
}

void block()
{
    consume_token(LEFT_BRACE);
    consume_token(EOL); // '{ EOL'

    // Enter new nesting level
    symtable_enter_nesting(parser.symtable);

    statement_list();

    // Exit nesting level and clean up variables
    symtable_exit_nesting(parser.symtable);

    check_token(RIGHT_BRACE); // '}'
}
void statement_list()
{
    next_token();
    if (parser.current_token->type == RIGHT_BRACE)
    {
        return;
    }
    statement();
    statement_list();
}
void statement()
{
    if (parser.current_token->type == KEYWORD)
    {
        if (parser.current_token->value.keyword == KW_VAR)
        {
            var_decl();
        }
        else if (parser.current_token->value.keyword == KW_IF)
        {
            if_statement();
        }
        else if (parser.current_token->value.keyword == KW_WHILE)
        {
            while_statement();
        }
        else if (parser.current_token->value.keyword == KW_RETURN)
        {
            return_statement();
        }
        else if (parser.current_token->value.keyword == KW_IFJ)
        {
            func_call();
        }
        else
        {
            exit_with_error(ERR_SYNTAX, "Syntax error: Unexpected keyword in statement at line %d", parser.scanner->line);
        }
    }
    else if (parser.current_token->type == IDENTIFIER)
    {
        assign();
    }
    else if (parser.current_token->type == GLOBAL_VAR)
    {
        assign();
    }
    else if (parser.current_token->type == LEFT_BRACE)
    {
        putback_token();
        block();
    }
}

void var_decl()
{
    check_token(KEYWORD);
    next_token();

    // Check if it's a global variable (starts with __)
    bool is_global = (parser.current_token->type == GLOBAL_VAR);
    e_token_type var_token_type = parser.current_token->type;

    if (!is_global && parser.current_token->type != IDENTIFIER)
    {
        exit_with_error(ERR_SYNTAX,
                        "Syntax error: Expected variable identifier at line %d",
                        parser.scanner->line);
    }

    // Získame meno premennej
    char *var_name = parser.current_token->value.string;

    // Skontrolujeme či premenná už existuje
    t_avl_node *existing = is_global ? symtable_search(parser.symtable, var_name) : symtable_search_var_current_nesting(parser.symtable, var_name);

    if (existing != NULL)
    {
        // Premenná už existuje - sémantická chyba (redefinícia)
        exit_with_error(ERR_SEM_REDEF,
                        "Semantic error: Variable '%s' already declared at line %d",
                        var_name, parser.scanner->line);
    }

    // Pridáme premennú do tabuľky symbolov
    e_symbol_type sym_type = is_global ? SYM_VAR_GLOBAL : SYM_VAR_LOCAL;
    bool success = symtable_insert_var(parser.symtable, var_name, sym_type, TYPE_UNKNOWN);
    if (!success)
    {
        exit_with_error(ERR_INTERNAL,
                        "Internal error: Failed to insert variable '%s' into symbol table at line %d",
                        var_name, parser.scanner->line);
    }

    // Generate DEFVAR
    if (is_global)
    {
        // Global variables use GF@ frame
        printf("DEFVAR GF@%s\n", var_name);
    }
    else if (collecting_loop_vars)
    {
        // Local variable in loop - add to list and skip DEFVAR
        if (loop_vars_count < MAX_LOOP_VARS)
        {
            // Store the FULL variable name with nesting suffix
            int nesting = parser.symtable->ifj_current_nesting;
            const char *full_name = get_var_name_with_nesting(var_name, nesting);
            loop_vars[loop_vars_count++] = strdup(full_name);
        }
        else
        {
            exit_with_error(ERR_INTERNAL,
                            "Internal error: Too many variables in loop at line %d",
                            parser.scanner->line);
        }
    }
    else
    {
        // Regular local variable - generate DEFVAR
        generate_var_declaration(var_name);
    }

    consume_token(EOL);
}

// Helper function to parse AST node from current token (for built-in parameters)
static t_ast_node *parse_builtin_param()
{
    t_ast_node *param_ast = NULL;

    if (parser.current_token->type == STRING_LITERAL ||
        parser.current_token->type == NUM_INT ||
        parser.current_token->type == NUM_EXP_INT ||
        parser.current_token->type == NUM_FLOAT ||
        parser.current_token->type == NUM_EXP_FLOAT ||
        parser.current_token->type == IDENTIFIER ||
        parser.current_token->type == GLOBAL_VAR ||
        (parser.current_token->type == KEYWORD &&
         parser.current_token->value.keyword == KW_NULL_INST))
    {
        // Create a copy of the current token to preserve its value
        t_token *token_copy = malloc(sizeof(t_token));
        if (token_copy == NULL)
        {
            exit_with_error(ERR_INTERNAL, "Internal error: Memory allocation failed");
        }

        // Deep copy the token
        token_copy->type = parser.current_token->type;
        token_copy->value = parser.current_token->value;

        // For strings, make a copy of the string data
        if (parser.current_token->type == STRING_LITERAL ||
            parser.current_token->type == IDENTIFIER ||
            parser.current_token->type == GLOBAL_VAR)
        {
            if (parser.current_token->value.string != NULL)
            {
                token_copy->value.string = strdup(parser.current_token->value.string);
                if (token_copy->value.string == NULL)
                {
                    free(token_copy);
                    exit_with_error(ERR_INTERNAL, "Internal error: Memory allocation failed");
                }
            }
        }

        param_ast = ast_create_leaf(token_copy);
        if (param_ast == NULL)
        {
            if (token_copy->type == STRING_LITERAL ||
                token_copy->type == IDENTIFIER ||
                token_copy->type == GLOBAL_VAR)
            {
                free(token_copy->value.string);
            }
            free(token_copy);
            exit_with_error(ERR_INTERNAL, "Internal error: Failed to create AST node");
        }
    }
    else
    {
        exit_with_error(ERR_SYNTAX, "Syntax error: Invalid parameter type at line %d",
                        parser.scanner->line);
    }

    return param_ast;
}

void assign()
{
    // Remember if this is a global variable
    bool is_global_var = (parser.current_token->type == GLOBAL_VAR);

    // Uložíme názov identifieru pred consume_token - MUST COPY because tokens will be overwritten!
    char identifier_copy[256];
    strncpy(identifier_copy, parser.current_token->value.string, sizeof(identifier_copy) - 1);
    identifier_copy[sizeof(identifier_copy) - 1] = '\0';
    char *identifier = identifier_copy;

    // Skontrolujeme či to môže byť setter (only for regular identifiers, not globals)
    char *setter_mangled = NULL;
    t_avl_node *setter_node = NULL;

    if (!is_global_var)
    {
        setter_mangled = mangle_setter_name(identifier);
        if (setter_mangled != NULL)
        {
            setter_node = symtable_search(parser.symtable, setter_mangled);
            free(setter_mangled);
        }
    }

    // Ak je to setter, spracujeme ho
    if (setter_node != NULL && setter_node->ifj_sym_type == SYM_SETTER)
    {
        // Je to setter - spracujeme ako volanie funkcie s 1 parametrom
        consume_token(OP_ASSIGN); // '='
        next_token();

        // Spracujeme argument (literál alebo identifikátor)
        if (parser.current_token->type == STRING_LITERAL)
        {
            generate_push_string_literal(parser.current_token->value.string);
        }
        else if (parser.current_token->type == NUM_INT || parser.current_token->type == NUM_EXP_INT || parser.current_token->type == NUM_HEX)
        {
            generate_push_int_literal(parser.current_token->value.number_int);
        }
        else if (parser.current_token->type == NUM_FLOAT || parser.current_token->type == NUM_EXP_FLOAT)
        {
            generate_push_float_literal(parser.current_token->value.number_float);
        }
        else if (parser.current_token->type == IDENTIFIER)
        {
            // Skontrolujeme či premenná existuje (hľadáme s najvyšším nesting levelom)
            t_avl_node *var_node = symtable_search_var_scoped(parser.symtable, parser.current_token->value.string);
            if (var_node == NULL)
            {
                exit_with_error(ERR_SEM_UNDEF,
                                "Semantic error: Variable '%s' is not defined at line %d",
                                parser.current_token->value.string, parser.scanner->line);
            }
            generate_push_variable(parser.current_token->value.string);
        }
        else if (parser.current_token->type == KEYWORD &&
                 (parser.current_token->value.keyword == KW_NULL_TYPE ||
                  parser.current_token->value.keyword == KW_NULL_INST))
        {
            generate_push_null();
        }
        else
        {
            exit_with_error(ERR_SYNTAX,
                            "Syntax error: Setter argument must be a literal or identifier at line %d",
                            parser.scanner->line);
        }

        // Zavoláme setter (argument je už na zásobníku)
        generate_setter_call(identifier);

        consume_token(EOL);
        return;
    }

    // Nie je to setter - musí to byť premenná
    t_avl_node *var_node;
    if (is_global_var)
    {
        // Global variable - use standard search
        var_node = symtable_search(parser.symtable, identifier);
    }
    else
    {
        // Local variable - use scoped search (finds highest nesting level)
        var_node = symtable_search_var_scoped(parser.symtable, identifier);
    }

    if (var_node == NULL)
    {
        exit_with_error(ERR_SEM_UNDEF,
                        "Semantic error: Variable '%s' is not defined at line %d",
                        identifier, parser.scanner->line);
    }

    char *var_name = identifier;
    consume_token(OP_ASSIGN); // '='
    next_token();

    t_ast_node *ast = NULL;

    // Kontrola či ide o volanie funkcie alebo výraz
    if (parser.current_token->type == KEYWORD &&
        parser.current_token->value.keyword == KW_IFJ)
    {
        // Built-in function call: Ifj.functionName(...)
        consume_token(DOT);
        consume_token(IDENTIFIER);
        char *builtin_name = parser.current_token->value.string;
        consume_token(LEFT_PAREN);

        // Declare result variable with proper frame
        t_avl_node *var_node = symtable_search_var_scoped(parser.symtable, var_name);
        char result_var[512];

        if (var_node && var_node->ifj_sym_type == SYM_VAR_GLOBAL)
        {
            // Global variable - use GF@ frame
            snprintf(result_var, sizeof(result_var), "GF@%s", var_name);
        }
        else
        {
            // Local variable - use LF@ frame with nesting suffix
            int nesting = (var_node && var_node->ifj_sym_type == SYM_VAR_LOCAL) ? var_node->ifj_data.ifj_var.ifj_nesting_level : 0;
            const char *result_var_name = get_var_name_with_nesting(var_name, nesting);
            snprintf(result_var, sizeof(result_var), "LF@%s", result_var_name);
        }

        // Generate code based on built-in function type
        if (strcmp(builtin_name, "write") == 0)
        {
            // write returns null, but we need to call it and store null in result
            next_token();
            t_ast_node *param = parse_builtin_param();
            generate_builtin_write(param);
            // Store null as the result (write returns null)
            fprintf(stdout, "MOVE %s nil@nil\n", result_var);
            consume_token(RIGHT_PAREN);
        }
        else if (strcmp(builtin_name, "read_str") == 0)
        {
            generate_builtin_read_str(result_var);
            consume_token(RIGHT_PAREN);
        }
        else if (strcmp(builtin_name, "read_num") == 0)
        {
            generate_builtin_read_num(result_var);
            consume_token(RIGHT_PAREN);
        }
        else if (strcmp(builtin_name, "floor") == 0)
        {
            next_token();
            t_ast_node *param = parse_builtin_param();
            generate_builtin_floor(result_var, param);
            consume_token(RIGHT_PAREN);
        }
        else if (strcmp(builtin_name, "str") == 0)
        {
            next_token();
            t_ast_node *param = parse_builtin_param();
            generate_builtin_str(result_var, param);
            consume_token(RIGHT_PAREN);
        }
        else if (strcmp(builtin_name, "length") == 0)
        {
            next_token();
            t_ast_node *param = parse_builtin_param();
            generate_builtin_length(result_var, param);
            consume_token(RIGHT_PAREN);
        }
        else if (strcmp(builtin_name, "chr") == 0)
        {
            next_token();
            t_ast_node *param = parse_builtin_param();
            generate_builtin_chr(result_var, param);
            consume_token(RIGHT_PAREN);
        }
        else if (strcmp(builtin_name, "ord") == 0)
        {
            next_token();
            t_ast_node *param1 = parse_builtin_param();
            consume_token(COMMA);
            next_token();
            t_ast_node *param2 = parse_builtin_param();
            generate_builtin_ord(result_var, param1, param2);
            consume_token(RIGHT_PAREN);
        }
        else if (strcmp(builtin_name, "substring") == 0)
        {
            next_token();
            t_ast_node *param1 = parse_builtin_param();
            consume_token(COMMA);
            next_token();
            t_ast_node *param2 = parse_builtin_param();
            consume_token(COMMA);
            next_token();
            t_ast_node *param3 = parse_builtin_param();
            generate_builtin_substring(result_var, param1, param2, param3);
            consume_token(RIGHT_PAREN);
        }
        else if (strcmp(builtin_name, "strcmp") == 0)
        {
            next_token();
            t_ast_node *param1 = parse_builtin_param();
            consume_token(COMMA);
            next_token();
            t_ast_node *param2 = parse_builtin_param();
            generate_builtin_strcmp(result_var, param1, param2);
            consume_token(RIGHT_PAREN);
        }
        else
        {
            exit_with_error(ERR_SEM_UNDEF,
                            "Semantic error: Unknown built-in function '%s' at line %d",
                            builtin_name, parser.scanner->line);
        }

        consume_token(EOL);
    }
    else if (parser.current_token->type == IDENTIFIER)
    {
        // Môže to byť volanie funkcie alebo výraz - potrebujeme lookahead
        t_token saved_identifier = *parser.current_token;
        next_token();

        if (parser.current_token->type == LEFT_PAREN)
        {
            // Volanie funkcie: identifier(...)
            putback_token(); // Put back '('
            *parser.current_token = saved_identifier;

            char *func_name = parser.current_token->value.string;

            consume_token(LEFT_PAREN);
            int arg_count = arg_list();
            check_token(RIGHT_PAREN);

            // Skontrolujeme či funkcia s touto aritou existuje
            char *mangled_name = mangle_function_name(func_name, arg_count);
            if (mangled_name == NULL)
            {
                exit_with_error(ERR_INTERNAL, "Internal error: Failed to mangle function name");
            }

            t_avl_node *func_node = symtable_search(parser.symtable, mangled_name);
            free(mangled_name);

            if (func_node == NULL)
            {
                exit_with_error(ERR_SEM_UNDEF,
                                "Semantic error: Function '%s' with %d parameters is not defined at line %d",
                                func_name, arg_count, parser.scanner->line);
            }

            // Vygenerujeme volanie funkcie s počtom argumentov
            generate_function_call(func_name, arg_count);

            // Skopírujeme návratovú hodnotu do premennej
            generate_move_retval_to_var(var_name);

            consume_token(EOL);
        }
        else if (parser.current_token->type == EOL)
        {
            // x = y
            // Musíme predať EOL ako druhý token, aby precedenčný parser vedel, že výraz končí
            ast = expression(&saved_identifier, parser.current_token);
            generate_assignment(var_name, ast);
            return;
        }
        else
        {
            // výraz začínajúci identifikátorom x = y + ...
            ast = expression(&saved_identifier, parser.current_token);
            generate_assignment(var_name, ast);
            check_token(EOL);
        }
    }
    else
    {
        // x = 5 + ...
        ast = expression(parser.current_token, NULL);
        generate_assignment(var_name, ast);
        check_token(EOL);
    }
}

void if_statement()
{
    check_token(KEYWORD); // 'if'
    if (parser.current_token->value.keyword != KW_IF)
    {
        exit_with_error(ERR_SYNTAX, "Syntax error: Expected 'if' keyword at line %d", parser.scanner->line);
    }

    // Získame unikátne ID pre labely
    int label_id = get_next_label_id();

    // Ak zbierame premenné v rámci while, zaznamenáme aj tento if label
    if (collecting_loop_vars)
    {
        if (loop_if_labels_count < MAX_LOOP_IFS)
        {
            loop_if_labels[loop_if_labels_count++] = label_id;
        }
    }

    consume_token(LEFT_PAREN); // '('

    // Získame AST podmienky
    t_ast_node *condition_ast = expression(parser.current_token, NULL);

    // Generujeme vyhodnotenie podmienky a skoky
    // Skip DEFVAR if we're collecting loop vars (will be predeclared before loop)
    generate_if_start(condition_ast, label_id, collecting_loop_vars);

    putback_token(); // Putback { after expression

    // Then blok
    generate_if_then(label_id);
    block(); // Parser vygeneruje kód then bloku

    // Prechod na else
    consume_token(KEYWORD); // 'else'
    if (parser.current_token->value.keyword != KW_ELSE)
    {
        exit_with_error(ERR_SYNTAX, "Syntax error: Expected 'else' keyword at line %d", parser.scanner->line);
    }

    generate_if_else_start(label_id);

    // Else blok
    block(); // Parser vygeneruje kód else bloku

    // Koniec if-else
    generate_if_end(label_id);
}

void while_statement()
{
    check_token(KEYWORD); // 'while'
    if (parser.current_token->value.keyword != KW_WHILE)
    {
        exit_with_error(ERR_SYNTAX, "Syntax error: Expected 'while' keyword at line %d", parser.scanner->line);
    }

    // Získame unikátne ID pre labely
    int label_id = get_next_label_id();

    consume_token(LEFT_PAREN); // '('

    // Získame AST podmienky
    t_ast_node *condition_ast = expression(parser.current_token, NULL);

    putback_token(); // Putback { after expression

    // Start collecting variable declarations from loop body
    loop_vars_count = 0;
    loop_if_labels_count = 0; // Reset if labels counter
    collecting_loop_vars = true;

    // Save current stdout to restore later
    FILE *saved_stdout = stdout;
    FILE *temp_buffer = tmpfile();
    if (temp_buffer == NULL)
    {
        exit_with_error(ERR_INTERNAL, "Internal error: Failed to create temporary buffer");
    }
    stdout = temp_buffer;

    // DON'T enter new nesting level yet - we want to collect vars at current nesting
    // Enter new nesting level
    symtable_enter_nesting(parser.symtable);

    // Parse the block (redirected to temp buffer, collecting var names)
    consume_token(LEFT_BRACE);
    consume_token(EOL);
    statement_list();
    check_token(RIGHT_BRACE);

    // Exit nesting level
    symtable_exit_nesting(parser.symtable);

    // Get the generated code from buffer
    fseek(temp_buffer, 0, SEEK_END);
    long buffer_size = ftell(temp_buffer);
    fseek(temp_buffer, 0, SEEK_SET);
    char *loop_body_code = malloc(buffer_size + 1);
    if (loop_body_code == NULL)
    {
        exit_with_error(ERR_INTERNAL, "Internal error: Memory allocation failed");
    }
    fread(loop_body_code, 1, buffer_size, temp_buffer);
    loop_body_code[buffer_size] = '\0';
    fclose(temp_buffer);

    // Restore stdout
    stdout = saved_stdout;
    collecting_loop_vars = false;

    // Preddeklarujeme všetky premenné (condition + collected vars)
    fprintf(stdout, "# While loop %d - predeclaration\n", label_id);
    fprintf(stdout, "DEFVAR LF@%%while_cond_%d\n", label_id);

    // Predeclare all collected loop variables (with full names including nesting)
    for (int i = 0; i < loop_vars_count; i++)
    {
        fprintf(stdout, "DEFVAR LF@%s\n", loop_vars[i]);
    }

    // Predeclare all if condition variables from if statements inside this loop
    for (int i = 0; i < loop_if_labels_count; i++)
    {
        fprintf(stdout, "DEFVAR LF@%%if_cond_%d\n", loop_if_labels[i]);
    }

    // Label pre začiatok cyklu
    generate_while_start(label_id);

    // Generujeme vyhodnotenie podmienky (už bez DEFVAR pre while_cond)
    generate_while_condition(condition_ast, label_id);

    // Output the collected loop body code (without DEFVARs for local vars)
    fprintf(stdout, "%s", loop_body_code);
    free(loop_body_code);

    // Koniec cyklu (skok späť na začiatok)
    generate_while_end(label_id);

    // Clean up collected variable names
    for (int i = 0; i < loop_vars_count; i++)
    {
        free(loop_vars[i]);
    }
    loop_vars_count = 0;
}
void return_statement()
{
    check_token(KEYWORD); // 'return'
    if (parser.current_token->value.keyword != KW_RETURN)
    {
        exit_with_error(ERR_SYNTAX, "Syntax error: Expected 'return' keyword at line %d", parser.scanner->line);
    }
    next_token();
    if (parser.current_token->type == EOL)
    {
        // Prázdny return - vrátime null
        generate_return_value(NULL);
    }
    else
    {
        // Return s výrazom - získame AST a vygenerujeme kód
        t_ast_node *ast = expression(parser.current_token, NULL);
        generate_return_value(ast);
    }
    check_token(EOL);
}

void func_call()
{
    if (parser.current_token->type == KEYWORD && parser.current_token->value.keyword == KW_IFJ)
    {
        consume_token(DOT);
        consume_token(IDENTIFIER);
        char *func_name = parser.current_token->value.string;
        consume_token(LEFT_PAREN);

        // Handle all built-in functions
        if (strcmp(func_name, "write") == 0)
        {
            // Ifj.write(term) - výpis hodnoty
            next_token();
            t_ast_node *param = parse_builtin_param();
            generate_builtin_write(param);
            consume_token(RIGHT_PAREN);
        }
        else if (strcmp(func_name, "read_str") == 0)
        {
            // Ifj.read_str() - načítanie stringu (no parameters, handled in assign)
            consume_token(RIGHT_PAREN);
        }
        else if (strcmp(func_name, "read_num") == 0)
        {
            // Ifj.read_num() - načítanie čísla (no parameters, handled in assign)
            consume_token(RIGHT_PAREN);
        }
        else if (strcmp(func_name, "floor") == 0)
        {
            // Ifj.floor(term) - handled in assign when used with assignment
            next_token();
            consume_token(RIGHT_PAREN);
        }
        else if (strcmp(func_name, "str") == 0)
        {
            // Ifj.str(term) - handled in assign
            next_token();
            consume_token(RIGHT_PAREN);
        }
        else if (strcmp(func_name, "length") == 0)
        {
            // Ifj.length(s) - handled in assign
            next_token();
            consume_token(RIGHT_PAREN);
        }
        else if (strcmp(func_name, "chr") == 0)
        {
            // Ifj.chr(i) - handled in assign
            next_token();
            consume_token(RIGHT_PAREN);
        }
        else if (strcmp(func_name, "ord") == 0)
        {
            // Ifj.ord(s, i) - 2 parameters
            next_token();
            consume_token(COMMA);
            next_token();
            consume_token(RIGHT_PAREN);
        }
        else if (strcmp(func_name, "substring") == 0)
        {
            // Ifj.substring(s, i, j) - 3 parameters
            next_token();
            consume_token(COMMA);
            next_token();
            consume_token(COMMA);
            next_token();
            consume_token(RIGHT_PAREN);
        }
        else if (strcmp(func_name, "strcmp") == 0)
        {
            // Ifj.strcmp(s1, s2) - 2 parameters
            next_token();
            consume_token(COMMA);
            next_token();
            consume_token(RIGHT_PAREN);
        }
        else
        {
            exit_with_error(ERR_SEM_UNDEF,
                            "Semantic error: Unknown built-in function '%s' at line %d",
                            func_name, parser.scanner->line);
        }
    }
    else
    {
        check_token(IDENTIFIER); // function name
        char *func_name = parser.current_token->value.string;

        next_token();
        if (parser.current_token->type == EOL)
        {
            // Static getter - 0 parametrov
            char *mangled_name = mangle_function_name(func_name, 0);
            if (mangled_name == NULL)
            {
                exit_with_error(ERR_INTERNAL, "Internal error: Failed to mangle function name");
            }

            t_avl_node *func_node = symtable_search(parser.symtable, mangled_name);
            free(mangled_name);

            if (func_node == NULL)
            {
                exit_with_error(ERR_SEM_UNDEF,
                                "Semantic error: Function '%s' with 0 parameters is not defined at line %d",
                                func_name, parser.scanner->line);
            }
            return;
        }
        else
        {
            check_token(LEFT_PAREN); // '('
            int arg_count = arg_list();
            check_token(RIGHT_PAREN); // ')'

            // Skontrolujeme či funkcia s touto aritou existuje
            char *mangled_name = mangle_function_name(func_name, arg_count);
            if (mangled_name == NULL)
            {
                exit_with_error(ERR_INTERNAL, "Internal error: Failed to mangle function name");
            }

            t_avl_node *func_node = symtable_search(parser.symtable, mangled_name);
            free(mangled_name);

            if (func_node == NULL)
            {
                exit_with_error(ERR_SEM_UNDEF,
                                "Semantic error: Function '%s' with %d parameters is not defined at line %d",
                                func_name, arg_count, parser.scanner->line);
            }

            // Vygenerujeme volanie funkcie s počtom argumentov
            generate_function_call(func_name, arg_count);
        }
        consume_token(EOL);
    }
}
int arg_list()
{
    next_token();
    if (parser.current_token->type == RIGHT_PAREN)
    {
        // Žiadne argumenty
        return 0;
    }

    // argument - môže byť len literál alebo identifikátor (nie výraz)
    if (parser.current_token->type == STRING_LITERAL)
    {
        generate_push_string_literal(parser.current_token->value.string);
    }
    else if (parser.current_token->type == NUM_INT || parser.current_token->type == NUM_EXP_INT)
    {
        generate_push_int_literal(parser.current_token->value.number_int);
    }
    else if (parser.current_token->type == NUM_FLOAT || parser.current_token->type == NUM_EXP_FLOAT)
    {
        generate_push_float_literal(parser.current_token->value.number_float);
    }
    else if (parser.current_token->type == IDENTIFIER)
    {
        // Skontrolujeme či premenná existuje (hľadáme s najvyšším nesting levelom)
        t_avl_node *var_node = symtable_search_var_scoped(parser.symtable, parser.current_token->value.string);
        if (var_node == NULL)
        {
            exit_with_error(ERR_SEM_UNDEF,
                            "Semantic error: Variable '%s' is not defined at line %d",
                            parser.current_token->value.string, parser.scanner->line);
        }
        generate_push_variable(parser.current_token->value.string);
    }
    else if (parser.current_token->type == KEYWORD &&
             (parser.current_token->value.keyword == KW_NULL_TYPE ||
              parser.current_token->value.keyword == KW_NULL_INST))
    {
        generate_push_null();
    }
    else
    {
        exit_with_error(ERR_SYNTAX,
                        "Syntax error: Function argument must be a literal or identifier at line %d",
                        parser.scanner->line);
    }

    // 1 za tento argument + počet z arg_list_tail
    return 1 + arg_list_tail();
}

int arg_list_tail()
{
    next_token();
    if (parser.current_token->type == COMMA)
    {
        // Ďalší argument
        next_token();

        // Argument - môže byť len literál alebo identifikátor (nie výraz)
        if (parser.current_token->type == STRING_LITERAL)
        {
            generate_push_string_literal(parser.current_token->value.string);
        }
        else if (parser.current_token->type == NUM_INT || parser.current_token->type == NUM_EXP_INT)
        {
            generate_push_int_literal(parser.current_token->value.number_int);
        }
        else if (parser.current_token->type == NUM_FLOAT || parser.current_token->type == NUM_EXP_FLOAT)
        {
            generate_push_float_literal(parser.current_token->value.number_float);
        }
        else if (parser.current_token->type == IDENTIFIER)
        {
            // Skontrolujeme či premenná existuje (hľadáme s najvyšším nesting levelom)
            t_avl_node *var_node = symtable_search_var_scoped(parser.symtable, parser.current_token->value.string);
            if (var_node == NULL)
            {
                exit_with_error(ERR_SEM_UNDEF,
                                "Semantic error: Variable '%s' is not defined at line %d",
                                parser.current_token->value.string, parser.scanner->line);
            }
            generate_push_variable(parser.current_token->value.string);
        }
        else if (parser.current_token->type == KEYWORD &&
                 (parser.current_token->value.keyword == KW_NULL_TYPE ||
                  parser.current_token->value.keyword == KW_NULL_INST))
        {
            generate_push_null();
        }
        else
        {
            exit_with_error(ERR_SYNTAX,
                            "Syntax error: Function argument must be a literal or identifier at line %d",
                            parser.scanner->line);
        }

        // 1 za tento argument + rekurzívne volanie
        return 1 + arg_list_tail();
    }
    else if (parser.current_token->type == RIGHT_PAREN)
    {
        return 0;
    }
    else
    {
        exit_with_error(ERR_SYNTAX, "Syntax error: Expected ',' or ')' at line %d", parser.scanner->line);
        return 0; // unreachable, ale kvôli warning
    }
}

t_ast_node *expression(t_token *token1, t_token *token2)
{
    t_ast_node *ast;
    int err_code;
    err_code = parse_expression(token1, token2, &ast);
    if (err_code)
    {
        exit_with_error(ERR_SEM_OTHER, "Semantic error: Parsing expression %d", parser.scanner->line);
    }
    // For debugging
    // printf("Expression AST:\n");
    // ast_print_tree(ast, 0);
    return ast;
}

void expression_continue()
{
    if (parser.current_token->type == EOL || parser.current_token->type == COMMA)
    {
        return;
    }

    next_token();
    if (is_operator())
    {
        op();
        term();
        expression_continue();
    }
    else
    {
        // epsilon
        return;
    }
}

void term()
{
    if (parser.current_token->type == GLOBAL_VAR)
    {
        // TODO: Handle global variable access
        return;
    }

    if (parser.current_token->type == IDENTIFIER)
    {
        // Uložíme token identifikátora
        t_token saved_identifier = *parser.current_token;

        // Look ahead aby sme zistili, či je to volanie funkcie
        next_token();
        if (parser.current_token->type == LEFT_PAREN)
        {
            // Volanie funkcie: identifier(...)
            putback_token(); // Put back '('
            *parser.current_token = saved_identifier;
            func_call();
        }
        else
        {
            // identifier - put back the lookahead token
            putback_token();
        }
        return;
    }

    if (parser.current_token->type == KEYWORD)
    {
        if (parser.current_token->value.keyword == KW_IFJ)
        {
            // Built-in function call
            func_call();
            return;
        }
        if (parser.current_token->value.keyword == KW_NULL_TYPE ||
            parser.current_token->value.keyword == KW_NULL_INST ||
            parser.current_token->value.keyword == KW_NUM ||
            parser.current_token->value.keyword == KW_STRING)
        {
            return;
        }
    }

    if (parser.current_token->type == NUM_INT ||
        parser.current_token->type == NUM_FLOAT ||
        parser.current_token->type == STRING_LITERAL ||
        parser.current_token->type == NUM_EXP_INT ||
        parser.current_token->type == NUM_EXP_FLOAT)
    {
        return;
    }
    else
    {
        exit_with_error(ERR_SYNTAX, "Syntax error: Invalid term at line %d", parser.scanner->line);
    }
}

void op()
{
    if (!is_operator())
    {
        exit_with_error(ERR_SYNTAX, "Syntax error: Invalid operator at line %d", parser.scanner->line);
    }
    next_token();
}

bool is_operator()
{
    if (parser.current_token->type == OP_ADD ||
        parser.current_token->type == OP_SUB ||
        parser.current_token->type == OP_MUL ||
        parser.current_token->type == OP_DIV ||
        parser.current_token->type == OP_GREATER_THAN ||
        parser.current_token->type == OP_LESS_THAN ||
        parser.current_token->type == OP_GREATER_THAN_EQUAL ||
        parser.current_token->type == OP_LESS_EQUAL ||
        parser.current_token->type == OP_EQUALS ||
        parser.current_token->type == OP_NOT_EQUALS)
    {
        return true;
    }

    if (parser.current_token->type == KEYWORD &&
        parser.current_token->value.keyword == KW_IS)
    {
        return true;
    }

    return false;
}

void eols()
{
    while (check_token_type(EOL))
    {
        next_token();
    }
}

void parse_program()
{
    prolog();
    generate_header();

    program();
    next_token();
    eols();
    consume_token(END_OF_FILE); // Skontrolujeme, či existuje funkcia main
    t_avl_node *main_func = symtable_search(parser.symtable, "main");
    if (main_func == NULL)
    {
        exit_with_error(ERR_SEM_UNDEF,
                        "Semantic error: Function 'main' is not defined");
    }

    // Skontrolujeme, či main je funkcia (nie getter/setter)
    if (main_func->ifj_sym_type != SYM_FUNCTION)
    {
        exit_with_error(ERR_SEM_OTHER,
                        "Semantic error: 'main' must be a function, not a getter or setter");
    }

    // Ak sme generovali main do bufferu, vypíšeme ho teraz na konci
    if (main_output_buffer != NULL)
    {
        // Vrátime sa na začiatok bufferu
        rewind(main_output_buffer);

        // Prečítame a vypíšeme obsah bufferu
        char buffer[4096];
        size_t bytes_read;
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), main_output_buffer)) > 0)
        {
            fwrite(buffer, 1, bytes_read, stdout);
        }

        // Zatvoríme a uvoľníme buffer
        fclose(main_output_buffer);
        main_output_buffer = NULL;
    }
}