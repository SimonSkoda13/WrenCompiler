/*
 * Prekladač jazyka IFJ25
 * VUT FIT
 *
 * Autori:
 *   - Matúš Magyar (xmagyam00)
 */

 /*Poznamka - mozeme pridat aj unarne a logicke operatory*/


#ifndef WRENCOMPILER_AST_H
#define WRENCOMPILER_AST_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <scanner.h>//pouzitie enum e_token_type


typedef enum {//Typy uzlov
    AST_PROGRAM,
    AST_FUNCTION,
    AST_VAR_DECL,
    AST_ASSIGNMENT,
    AST_IF_COND,
    AST_WHILE_LOOP,
    AST_BLOCK,
    AST_RETURN,
    AST_CALL,
    AST_BINARY_OP,
    AST_UNARY_OP,     
    AST_TYPE_CHECK,   
    AST_LITERAL_INT,
    AST_LITERAL_FLOAT,
    AST_LITERAL_STRING,
    AST_LITERAL_NULL,
    AST_IDENTIFIER
} AstNodeType;


typedef enum {//typy funkcií pre lepsie rozlisovanie
    FUN_REGULAR,
    FUN_GETTER,
    FUN_SETTER
} FunType;


//Štruktúra uzla AST
typedef struct AstNode {
    AstNodeType type;//typ uzla
    int line; //informacia o riadku pre chybové hlášky
    union {
        struct {    
            struct AstNode *body; //telo programu
        } program;
        struct {
            char *name;
            struct AstNode *params; //zoznam parametrov
            struct AstNode *body; //telo funkcie
            FunType fun_type; //typ funkcie podla enum FunType
        } function;
        struct {
            char *var_name;
            struct AstNode *value; //inicializačná hodnota
        } var_decl;
        struct { // pre priradenia
            struct AstNode *var;
            struct AstNode *value;
        } assignment;
        struct {
            struct AstNode *condition;
            struct AstNode *then_branch;
            struct AstNode *else_branch;
        } if_cond;
        struct {
            struct AstNode *condition;
            struct AstNode *body;
        } while_loop;
        struct {
            struct AstNode *body;
        } block;
        struct {
            struct AstNode *value;
        } return_stmt;
        struct {
            struct AstNode *object;
            char *func_name;
            struct AstNode *args; //zoznam argumentov
        } call;
        struct { // pre binarne op.
            struct AstNode *left;
            struct AstNode *right;
            e_token_type op; // typ operácie
        } binary;
        struct {
            struct AstNode *operand;
            e_token_type op; // typ operácie
        } unary;
        struct {
            struct AstNode *operand;
            struct AstNode *type_node; // uzol reprezentujúci typ (LiteralType)
        } type_check;
        struct { // pre literály
            union {
                int int_val;
                double float_val;
                char *str_val;
            } value;
        } literal;
        struct { // pre identifikátory
            char *name;
        } identifier;
    } data;
    struct AstNode *next; // pre zoznamy (napr. zoznam príkazov v bloku)
} AstNode;

//Funckcie na vytváranie uzlov AST
AstNode *ast_create_program(AstNode *body, int line);//program
AstNode *ast_create_function(const char *name, AstNode *params, AstNode *body, int line, FunType fun_type);//function
AstNode *ast_create_var_decl(const char *var_name, AstNode *value, int line);//var declaration
AstNode *ast_create_assignment(AstNode *var, AstNode *value, int line);//assignment
AstNode *ast_create_if_cond(AstNode *condition, AstNode *then_branch,AstNode *else_branch, int line);//if
AstNode *ast_create_while_loop(AstNode *condition, AstNode *body, int line);//while
AstNode *ast_create_block(AstNode *body, int line);//block
AstNode *ast_create_return(AstNode *value, int line);//return statement
AstNode *ast_create_call(AstNode *object, const char *func_name, AstNode *args, int line);//fun call
AstNode *ast_create_binary_op(e_token_type op, AstNode *left, AstNode *right, int line);//binary op
AstNode *ast_create_literal_int(int value, int line);//lit int
AstNode *ast_create_literal_float(double value, int line);//lit float
AstNode *ast_create_literal_string(const char *value, int line);//lit string
AstNode *ast_create_literal_null(int line);//lit null
AstNode *ast_create_identifier(const char *name, int line);//identifier

void ast_free(AstNode *node);



#endif // WRENCOMPILER_AST_H