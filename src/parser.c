/*
 * Prekladač jazyka IFJ25
 * VUT FIT
 *
 * Autori:
 *   - Martin Michálik (xmicham00)
 */

 #include "parser.h"

// DEBUG
void ast_print_tree(t_ast_node *node, int depth) {
    if (node == NULL) {
        return;
    }
    // Print indentation
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
    
    // Print node information
    switch (node->token->type) {
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
                node->token->value.keyword == KW_NULL_INST) {
                printf("KEYWORD (%s)\n", "null");
            } else if (node->token->value.keyword == KW_NUM) {
                printf("KEYWORD (%s)\n", "Num");
            } else if (node->token->value.keyword == KW_STRING) {
                printf("KEYWORD (%s)\n", "String");
            } else
            printf("KEYWORD (%s)\n", "unknown");
            break;
        default:
            printf("UNKNOWN_TYPE (%d)\n", node->token->type);
            break;
    }
    
    // Recursively print left and right children
    if (node->left != NULL) {
        ast_print_tree(node->left, depth + 1);
    }
    if (node->right != NULL) {
        ast_print_tree(node->right, depth + 1);
    }
}


 void next_token() {
    // Check if we have a putback token first
    if (parser.has_putback) {
        *parser.current_token = *parser.putback_token;
        parser.has_putback = false;
        return;
    }
    
    int lex_result = get_next_token(parser.scanner, parser.current_token);
    if (lex_result == 1) {
        exit_with_error(ERR_LEXICAL, 
          "Lexical error encountered while getting next token at line %d", 
          parser.scanner->line);
    }
}

void putback_token() {
    if (parser.has_putback) {
        exit_with_error(ERR_INTERNAL, 
          "Internal error: Cannot putback token twice at line %d", 
          parser.scanner->line);
    }
    
    // Copy current token to putback storage
    *parser.putback_token = *parser.current_token;
    parser.has_putback = true;
}

void check_token(e_token_type expected_type) {
    if (parser.current_token->type != expected_type) {
      exit_with_error(ERR_SYNTAX, 
        "Syntax error: Expected token type %d but got %d at line %d",
        expected_type, 
        parser.current_token->type,
        parser.scanner->line);
    }
}

void consume_token(e_token_type expected_type) {
    next_token();
    check_token(expected_type);
}

bool check_token_type(e_token_type expected_type) {
    return parser.current_token->type == expected_type;
}

void prolog() {
    // Consume any leading whitespace or EOLs
    next_token();
    if (parser.current_token->type == EOL) {
        eols();
    }
    check_token(KEYWORD); // 'import'
    if (parser.current_token->value.keyword != KW_IMPORT) {
        exit_with_error(ERR_SYNTAX, "Syntax error: Expected 'import' keyword at line %d", parser.scanner->line);
    }
    consume_token(STRING_LITERAL); // "ifj25"
    if (parser.current_token->value.string == NULL || strcmp(parser.current_token->value.string, "ifj25") != 0) {
        exit_with_error(ERR_SYNTAX, "Syntax error: Expected 'ifj25' string literal at line %d", parser.scanner->line);
    }
    consume_token(KEYWORD); // 'for'
    if (parser.current_token->value.keyword != KW_FOR) {
        exit_with_error(ERR_SYNTAX, "Syntax error: Expected 'for' keyword at line %d", parser.scanner->line);
    }
    consume_token(KEYWORD); // Ifj
    if (parser.current_token->value.keyword != KW_IFJ) {
        exit_with_error(ERR_SYNTAX, "Syntax error: Expected 'Ifj' keyword at line %d", parser.scanner->line);
    }
    next_token();
    eols();
}

void program() {
    check_token(KEYWORD); // 'class'
    if (parser.current_token->value.keyword != KW_CLASS) {
        exit_with_error(ERR_SYNTAX, "Syntax error: Expected 'class' keyword at line %d", parser.scanner->line);
    }
    consume_token(IDENTIFIER); // Program
    if (parser.current_token->value.string == NULL || strcmp(parser.current_token->value.string, "Program") != 0) {
        exit_with_error(ERR_SYNTAX, "Syntax error: Expected 'Program' identifier at line %d", parser.scanner->line);
    }
    consume_token(LEFT_BRACE); // '{'
    consume_token(EOL); 
    func_list();
    check_token(RIGHT_BRACE); // '}'
}

void func_list() {
    next_token();
    if (parser.current_token->type == EOL) {
      eols(); // consume any EOLs between functions
    }

    // Base case: end of class
    if (parser.current_token->type == RIGHT_BRACE) {
        return;  // ✓ Stop recursion
    }
    
    if (parser.current_token->type != KEYWORD) {
        exit_with_error(ERR_SYNTAX, "Syntax error: Expected function declaration %d", parser.scanner->line);
    } else if (parser.current_token->type == KEYWORD && parser.current_token->value.keyword == KW_STATIC) {
       func();
    } else {
      exit_with_error(ERR_SYNTAX, "Unknown Syntax error at line %d", parser.scanner->line);
    }
    func_list();
}

void func() {
    check_token(KEYWORD); // 'static'
    if (parser.current_token->value.keyword != KW_STATIC) {
        exit_with_error(ERR_SYNTAX, "Syntax error: Expected 'static' keyword at line %d", parser.scanner->line);
    }
    consume_token(IDENTIFIER); // function IDENTIFIER
    next_token();
    // static getter
    if (parser.current_token->type == LEFT_BRACE) {
        putback_token(); 
        block();
        return;
    } else if (parser.current_token->type == OP_ASSIGN) { // static setter
        consume_token(LEFT_PAREN);
        consume_token(IDENTIFIER);
        consume_token(RIGHT_PAREN);
        block();
        return;
    }
    
    //TODO: Handle function 
    check_token(LEFT_PAREN); // '('
    param_list();
    check_token(RIGHT_PAREN); // ')'
    block(); // function body
}

void param_list() {
    next_token();
    if (parser.current_token->type == RIGHT_PAREN) {
        return;
    }
    check_token(IDENTIFIER);
    param_list_tail();
}

void param_list_tail() {
    next_token();
    if (parser.current_token->type == COMMA) {
        consume_token(IDENTIFIER);
        param_list_tail();
    } else if (parser.current_token->type == RIGHT_PAREN) {
        return;
    } else {
        exit_with_error(ERR_SYNTAX, "Syntax error: Expected ',' or ')' at line %d", parser.scanner->line);
    }
}

void block() {
    consume_token(LEFT_BRACE); // '{'
    consume_token(EOL); // consume EOL after '{'
    statement_list();
    check_token(RIGHT_BRACE); // '}'
}
void statement_list() {
    next_token();
    if (parser.current_token->type == RIGHT_BRACE) {
        return;
    }
    statement();
    statement_list();
}
void statement() {
    if (parser.current_token->type == KEYWORD) {
        if (parser.current_token->value.keyword == KW_VAR) {
            var_decl();
        } else if (parser.current_token->value.keyword == KW_IF) {
            if_statement();
        } else if (parser.current_token->value.keyword == KW_WHILE) {
            while_statement();
        } else if (parser.current_token->value.keyword == KW_RETURN) {
            return_statement();
        } else if (parser.current_token->value.keyword == KW_IFJ) {
            func_call();
        } else {
            exit_with_error(ERR_SYNTAX, "Syntax error: Unexpected keyword in statement at line %d", parser.scanner->line);
        }
    } else if (parser.current_token->type == IDENTIFIER) {
        assign();
    } else if (parser.current_token->type == GLOBAL_VAR) {
        assign();
    } else if (parser.current_token->type == LEFT_BRACE) {
        putback_token();
        block();
    }
}

void var_decl() {
    check_token(KEYWORD); // 'var'
    consume_token(IDENTIFIER); // variable name
    consume_token(EOL);
}

void assign() {
    consume_token(OP_ASSIGN); // '='
    next_token();
    
    // Check if it's a function call or expression
    if (parser.current_token->type == KEYWORD && 
        parser.current_token->value.keyword == KW_IFJ) {
        // Built-in function call: Ifj.functionName(...)
        func_call();
        consume_token(EOL);
    } else if (parser.current_token->type == IDENTIFIER) {
        // Could be func_call or expression - need lookahead
        t_token saved_identifier = *parser.current_token;
        next_token();
        
        if (parser.current_token->type == LEFT_PAREN) {
            // It's a function call
            putback_token(); // Put back '('
            *parser.current_token = saved_identifier; // Restore identifier
            func_call();
            check_token(EOL);
        } else if (parser.current_token->type == EOL) {
            // Simple identifier assignment: x = y
            return;
        } else {
            // It's an expression starting with identifier
            expression(&saved_identifier, parser.current_token);
            check_token(EOL);
        }
    } else {
        // It's an expression (literal, parenthesized expr, etc.)
        expression(parser.current_token, NULL);
        check_token(EOL);
    }
}

void if_statement() {
    check_token(KEYWORD); // 'if'
    if (parser.current_token->value.keyword != KW_IF) {
        exit_with_error(ERR_SYNTAX, "Syntax error: Expected 'if' keyword at line %d", parser.scanner->line);
    }
    
    consume_token(LEFT_PAREN); // '('
    expression(parser.current_token, NULL);
    putback_token(); // Putback { after expression
    block();
    consume_token(KEYWORD); // 'else'
    if (parser.current_token->value.keyword != KW_ELSE) {
        exit_with_error(ERR_SYNTAX, "Syntax error: Expected 'else' keyword at line %d", parser.scanner->line);
    }
    block();    
}

void while_statement() {
    check_token(KEYWORD); // 'while'
    if (parser.current_token->value.keyword != KW_WHILE) {
        fprintf(stderr, "Syntax error: Expected 'while' keyword\n");
        // Error handling
    }
    
    consume_token(LEFT_PAREN); // '('
    expression(parser.current_token, NULL);
    putback_token(); // Putback { after expression
    block();
}

void return_statement() {
    check_token(KEYWORD); // 'return'
    if (parser.current_token->value.keyword != KW_RETURN) {
        exit_with_error(ERR_SYNTAX, "Syntax error: Expected 'return' keyword at line %d", parser.scanner->line);
    }
    next_token();
    if (parser.current_token->type == EOL) {
        //TODO: Handle empty return
    } else {
        expression(parser.current_token, NULL);
    }
    check_token(EOL);
}

void func_call() {
    if (parser.current_token->type == KEYWORD && parser.current_token->value.keyword == KW_IFJ) {
        //TODO: Handle built-in function calls
        consume_token(DOT);
        consume_token(IDENTIFIER);
        consume_token(LEFT_PAREN); // '('
        arg_list();
        check_token(RIGHT_PAREN); // ')'
    } else {
        check_token(IDENTIFIER); // function name
        next_token();
        if (parser.current_token->type == EOL) {
            // Static getter
            return;
        } else {
            check_token(LEFT_PAREN); // '('
            arg_list();
            check_token(RIGHT_PAREN); // ')'
        }
        consume_token(EOL);
    }
}
void arg_list() {
    next_token();
    if (parser.current_token->type == RIGHT_PAREN) {
        return;
    }
    //TODO: handle function argument
    next_token();
    arg_list_tail();
}

void arg_list_tail() {
    if (parser.current_token->type == COMMA) {
        next_token();
        arg_list_tail();
    } else if (parser.current_token->type == RIGHT_PAREN) {
        return;
    } else {
        exit_with_error(ERR_SYNTAX, "Syntax error: Expected ',' or ')' at line %d", parser.scanner->line);
    }
}

void expression(t_token *token1, t_token *token2) {
    t_ast_node *ast;
    int err_code;
    err_code = parse_expression(token1, token2, &ast);
    if (err_code) {
        exit_with_error(ERR_SEM_OTHER, "Semantic error: Parsing expression %d", parser.scanner->line);
    }
    // For debugging
    // printf("Expression AST:\n");
    // ast_print_tree(ast, 0);
}

void expression_continue() {
    if (parser.current_token->type == EOL || parser.current_token->type == COMMA) {
        return; //If term was function call ending with EOL
    }
    
    next_token();
    if (is_operator()) {
        op();
        term();
        expression_continue();
    } else {
        // Epsilon production, do nothing
        return;
    }
}

void term() {
    if (parser.current_token->type == GLOBAL_VAR){
        //TODO: Handle global variable access
        return;
    }
    
    if (parser.current_token->type == IDENTIFIER) {
        // Save the identifier token
        t_token saved_identifier = *parser.current_token;
        
        // Look ahead to check if it's a function call
        next_token();
        if (parser.current_token->type == LEFT_PAREN) {
            // It's a function call - restore identifier and parse as func_call
            putback_token(); // Put back the '('
            *parser.current_token = saved_identifier; // Restore identifier
            func_call();
        } else {
            // Just an identifier - put back the lookahead token
            putback_token();
            // The identifier is already in current_token (saved_identifier)
            // Just treat it as a term (variable reference)
        }
        return;
    }
    
    if (parser.current_token->type == KEYWORD) {
        if (parser.current_token->value.keyword == KW_IFJ) {
            // Built-in function call
            func_call();
            return;
        }
        if (parser.current_token->value.keyword == KW_NULL_TYPE ||
            parser.current_token->value.keyword == KW_NULL_INST ||
            parser.current_token->value.keyword == KW_NUM ||
            parser.current_token->value.keyword == KW_STRING) {
            return;
        }
    }
    
    if (parser.current_token->type == NUM_INT ||
        parser.current_token->type == NUM_FLOAT ||
        parser.current_token->type == STRING_LITERAL ||
        parser.current_token->type == NUM_EXP_INT ||
        parser.current_token->type == NUM_EXP_FLOAT) {
        return;
    } else {
        exit_with_error(ERR_SYNTAX, "Syntax error: Invalid term at line %d", parser.scanner->line);
    }
}

void op() {
    if (!is_operator()) {
        exit_with_error(ERR_SYNTAX, "Syntax error: Invalid operator at line %d", parser.scanner->line);
    }
    next_token();
}

bool is_operator() {
    if (parser.current_token->type == OP_ADD ||
        parser.current_token->type == OP_SUB ||
        parser.current_token->type == OP_MUL ||
        parser.current_token->type == OP_DIV ||
        parser.current_token->type == OP_GREATER_THAN ||
        parser.current_token->type == OP_LESS_THAN ||
        parser.current_token->type == OP_GREATER_THAN_EQUAL ||
        parser.current_token->type == OP_LESS_EQUAL ||
        parser.current_token->type == OP_EQUALS ||
        parser.current_token->type == OP_NOT_EQUALS) {
        return true;
    }
    
    if (parser.current_token->type == KEYWORD && 
        parser.current_token->value.keyword == KW_IS) {
        return true;
    }
    
    return false;
}

void eols() {
    while (check_token_type(EOL)) {
        next_token();
    }
}

void parse_program() {
    prolog();
    program();
    next_token();
    eols();
    consume_token(END_OF_FILE);
}