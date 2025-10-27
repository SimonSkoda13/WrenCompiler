/*
 * Prekladač jazyka IFJ25
 * VUT FIT
 *
 * Autori:
 *   - Matúš Magyar (xmagyam00)
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <ctype.h>

#include "ast.h"

// Deklarácie funkcií na vytváranie AST uzlov
AstNode *ast_create_program(AstNode *body, int line)
{
    AstNode *node = calloc(1, sizeof(AstNode));
    if (node != NULL)
    {
        node->type = AST_PROGRAM;
        node->line = line;
        node->data.program.body = body;
        node->next = NULL;
        return node;
    }
    return NULL;
};

AstNode *ast_create_function(const char *name, AstNode *params, AstNode *body, int line, FunType fun_type)
{
    AstNode *node = calloc(1, sizeof(AstNode));
    if (node != NULL)
    {
        node->type = AST_FUNCTION;
        node->line = line;
        node->data.function.name = strdup(name);
        node->data.function.params = params;
        node->data.function.body = body;
        node->data.function.fun_type = fun_type;
        node->next = NULL;
        return node;
    }
    return NULL;
}

AstNode *ast_create_var_decl(const char *var_name, AstNode *value, int line)
{
    AstNode *node = calloc(1, sizeof(AstNode));
    if (node != NULL)
    {
        node->type = AST_VAR_DECL;
        node->line = line;
        node->data.var_decl.value = value;
        node->data.var_decl.var_name = strdup(var_name);
        node->next = NULL;
        return node;
    }
    return NULL;
}

AstNode *ast_create_assignment(AstNode *var, AstNode *value, int line)
{
    AstNode *node = calloc(1, sizeof(AstNode));
    if (node != NULL)
    {
        node->type = AST_ASSIGNMENT;
        node->line = line;
        node->data.assignment.var = var;
        node->data.assignment.value = value;
        node->next = NULL;
        return node;
    }
    return NULL;
}

AstNode *ast_create_if_cond(AstNode *condition, AstNode *then_branch, AstNode *else_branch, int line)
{
    AstNode *node = calloc(1, sizeof(AstNode));
    if (node != NULL)
    {
        node->type = AST_IF_COND;
        node->line = line;
        node->data.if_cond.condition = condition;
        node->data.if_cond.then_branch = then_branch;
        node->data.if_cond.else_branch = else_branch;
        node->next = NULL;
        return node;
    }
    return NULL;
}

AstNode *ast_create_while_loop(AstNode *condition, AstNode *body, int line)
{
    AstNode *node = calloc(1, sizeof(AstNode));
    if (node != NULL)
    {
        node->type = AST_WHILE_LOOP;
        node->line = line;
        node->data.while_loop.condition = condition;
        node->data.while_loop.body = body;
        node->next = NULL;
        return node;
    }
    return NULL;
}

AstNode *ast_create_block(AstNode *body, int line)
{
    AstNode *node = calloc(1, sizeof(AstNode));
    if (node != NULL)
    {
        node->type = AST_BLOCK;
        node->line = line;
        node->data.block.body = body;
        node->next = NULL;
        return node;
    }
    return NULL;
}

AstNode *ast_create_return(AstNode *value, int line)
{
    AstNode *node = calloc(1, sizeof(AstNode));
    if (node != NULL)
    {
        node->type = AST_RETURN;
        node->line = line;
        node->next = NULL;
        node->data.return_stmt.value = value;
        return node;
    }
    return NULL;
}

AstNode *ast_create_call(AstNode *object, const char *func_name, AstNode *args, int line)
{
    AstNode *node = calloc(1, sizeof(AstNode));
    if (node != NULL)
    {
        node->type = AST_CALL;
        node->line = line;
        node->data.call.object = object;
        node->data.call.func_name = strdup(func_name);
        node->data.call.args = args;
        node->next = NULL;
        return node;
    }
    return NULL;
}

AstNode *ast_create_binary_op(e_token_type op, AstNode *left, AstNode *right, int line)
{
    AstNode *node = calloc(1, sizeof(AstNode));
    if (node != NULL)
    {
        node->type = AST_BINARY_OP;
        node->line = line;
        node->data.binary.op = op;
        node->data.binary.left = left;
        node->data.binary.right = right;
        node->next = NULL;
        return node;
    }
    return NULL;
}

AstNode *ast_create_unary_op(e_token_type op, AstNode *operand, int line)
{
    AstNode *node = calloc(1, sizeof(AstNode));
    if (node != NULL)
    {
        node->type = AST_UNARY_OP;
        node->line = line;
        node->data.unary.op = op;
        node->data.unary.operand = operand;
        node->next = NULL;
        return node;
    }
    return NULL;
}

AstNode *ast_create_type_check(AstNode *operand, AstNode *type_node, int line)
{
    AstNode *node = calloc(1, sizeof(AstNode));
    if (node != NULL)
    {
        node->type = AST_TYPE_CHECK;
        node->line = line;
        node->data.type_check.type_node = type_node;
        node->data.type_check.operand = operand;
        node->next = NULL;
        return node;
    }
    return NULL;
}

AstNode *ast_create_literal_int(int value, int line)
{
    AstNode *node = calloc(1, sizeof(AstNode));
    if (node != NULL)
    {
        node->type = AST_LITERAL_INT;
        node->line = line;
        node->data.literal.value.int_val = value;
        node->next = NULL;
        return node;
    }
    return NULL;
}

AstNode *ast_create_literal_float(double value, int line)
{
    AstNode *node = calloc(1, sizeof(AstNode));
    if (node != NULL)
    {
        node->type = AST_LITERAL_FLOAT;
        node->line = line;
        node->data.literal.value.float_val = value;
        node->next = NULL;
        return node;
    }
    return NULL;
}
AstNode *ast_create_literal_string(const char *value, int line)
{
    AstNode *node = calloc(1, sizeof(AstNode));
    if (node != NULL)
    {
        node->type = AST_LITERAL_STRING;
        node->line = line;
        node->data.literal.value.str_val = strdup(value);
        node->next = NULL;
        return node;
    }
    return NULL;
}

AstNode *ast_create_literal_null(int line)
{
    AstNode *node = calloc(1, sizeof(AstNode));
    if (node != NULL)
    {
        node->type = AST_LITERAL_NULL;
        node->line = line;
        return node;
    }
    return NULL;
}

AstNode *ast_create_identifier(const char *name, int line)
{
    AstNode *node = calloc(1, sizeof(AstNode));
    if (node != NULL)
    {
        node->type = AST_IDENTIFIER;
        node->line = line;
        node->data.identifier.name = strdup(name);
        node->next = NULL;
        return node;
    }
    return NULL;
}

void ast_free(AstNode *node)
{
    if (node == NULL)
        return;

    switch (node->type)
    {
    case AST_PROGRAM:
        ast_free(node->data.program.body);
        break;
    case AST_FUNCTION:
        free(node->data.function.name);
        ast_free(node->data.function.params);
        ast_free(node->data.function.body);
        break;
    case AST_VAR_DECL:
        free(node->data.var_decl.var_name);
        ast_free(node->data.var_decl.value);
        break;
    case AST_ASSIGNMENT:
        ast_free(node->data.assignment.value);
        ast_free(node->data.assignment.var);
        break;
    case AST_IF_COND:
        ast_free(node->data.if_cond.condition);
        ast_free(node->data.if_cond.else_branch);
        ast_free(node->data.if_cond.then_branch);
        break;
    case AST_WHILE_LOOP:
        ast_free(node->data.while_loop.body);
        ast_free(node->data.while_loop.condition);
        break;
    case AST_BLOCK:
        ast_free(node->data.block.body);
        break;
    case AST_RETURN:
        ast_free(node->data.return_stmt.value);
        break;
    case AST_CALL:
        ast_free(node->data.call.object);
        free(node->data.call.func_name);
        ast_free(node->data.call.args);
        break;
    case AST_BINARY_OP:
        ast_free(node->data.binary.left);
        ast_free(node->data.binary.right);
        break;
    case AST_UNARY_OP:
        ast_free(node->data.unary.operand);
        break;
    case AST_TYPE_CHECK:
        ast_free(node->data.type_check.operand);
        ast_free(node->data.type_check.type_node);
        break;
    case AST_LITERAL_STRING:
        free(node->data.literal.value.str_val);
        break;
    case AST_IDENTIFIER:
        free(node->data.identifier.name);
        break;
    case AST_LITERAL_INT://nic na uvolnenie
    case AST_LITERAL_FLOAT:
    case AST_LITERAL_NULL:
        break;
    }
    ast_free(node->next);

    free(node);
}
