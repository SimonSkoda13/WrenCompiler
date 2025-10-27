#include "ast.h"
#include <stdio.h>

int main() {
    // Vytvorenie literálu a identifikátora
    AstNode *lit = ast_create_literal_int(42, 1);
    AstNode *id = ast_create_identifier("x", 1);

    // Priradenie: x = 42
    AstNode *assign = ast_create_assignment(id, lit, 1);

    // Blok s jedným príkazom
    AstNode *block = ast_create_block(assign, 1);

    // Funkcia s blokom ako telom
    AstNode *func = ast_create_function("main", NULL, block, 1, FUN_REGULAR);

    // Program s jednou funkciou
    AstNode *prog = ast_create_program(func, 1);

    // Výpis typov uzlov
    printf("Program node type: %d\n", prog->type);
    printf("Function node type: %d\n", func->type);
    printf("Block node type: %d\n", block->type);
    printf("Assignment node type: %d\n", assign->type);
    printf("Identifier node type: %d\n", id->type);
    printf("Literal node type: %d\n", lit->type);

    // Uvoľnenie celej štruktúry
    ast_free(prog);

    return 0;
}