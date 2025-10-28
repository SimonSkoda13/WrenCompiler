/*
 * Prekladač jazyka IFJ25
 * VUT FIT
 *
 * Autori:
 *   - Martin Michálik (xmicham00)
 */
#ifndef WRENCOMPILER_SCANNER_H
#define WRENCOMPILER_SCANNER_H

#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>




/**
 * Enum pre typ tokenu.
 * Dúfam že toto je dost self-explanatory.
 */
typedef enum {
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_ASSIGN,
  OP_EQUALS,
  OP_LESS_THAN,
  OP_LESS_EQUAL,
  OP_GREATER_THAN,
  OP_GREATER_THAN_EQUAL,
  OP_NOT_EQUALS,
  LEFT_PAREN,
  RIGHT_PAREN,
  LEFT_BRACE,
  RIGHT_BRACE,
  NUM_INT,
  NUM_FLOAT,
  NUM_HEX,
  NUM_EXP_FLOAT,
  NUM_EXP_INT,
  END_OF_FILE,
  STRING_OR_MULTILINE,
  STRING_LITERAL,
  IDENTIFIER,
  GLOBAL_VAR,
  EOL,
  DOT,
  KEYWORD,
  COMMA
} e_token_type;

/**
 * Enumy pre kľúčové slová. Treba si uvedomiť že KW_NULL_INST je pre kľúčové slovo 'null' čo je vlastne
 * literal pre Null a kľúčové slovo 'Null' reprezentuje dátovy typ preto KW_NULL_TYPE. 
 * Je to niečo podobne ako je rozdiel medzi classou a jej inštanciou
 */
typedef enum {
  KW_CLASS,
  KW_IF,
  KW_ELSE,
  KW_IS,
  KW_NULL_INST,
  KW_RETURN,
  KW_VAR,
  KW_WHILE,
  KW_IFJ,
  KW_STATIC, 
  KW_IMPORT,
  KW_FOR,
  KW_NUM,
  KW_STRING,
  KW_NULL_TYPE
} e_keyword;

/**
 * Union pre hodnotu tokenu.
 * Môže byť identifikátor, string, float alebo int.
 * Ak je typ tokenu IDENTIFIER_OR_KEYWORD identifier drží názov identifikátora alebo kľúčového slova.
 * Ak je typ tokenu STRING_LITERAL string drží hodnotu literálu.
 * Ak je typ tokenu NUM_FLOAT, NUM_EXP number_float drží hodnotu float čísla.
 * Ak je typ tokenu NUM_INT, NUM_HEX alebo number_int drží
 * Ak je typ tokenu KEYWORD keyword reprezentuje enum daného kľúčového slova
 */
typedef union {
	char *identifier;
	char *string;
	double number_float;
	long number_int;
  e_keyword keyword;
} u_token_value;


/**
 * Štruktúra pre token.
 * Zatiaľ len typ a hodnota. Ak bude potrebné rozšíri sa o ďalšie atribúty.
 */
typedef struct {
  e_token_type type;
  u_token_value value;
} t_token;

/**
 * Štruktúra pre scanner.
 * Obsahuje informácie o stave skenera.
 * Line je aktuálny riadok v súbore (pre chybové hlášky).
 * Putback je znak ktorý bol vrátený späť do skenera.
 * Stream je vstupný súbor.
 */
typedef struct {
	int line;
	int putback;
  FILE *stream;
} t_scanner;

/**
 * Deklarácie funkcií skenera.
 */
int get_next_token(t_scanner *scanner, t_token *token);
char next_char(t_scanner *scanner);
void putback(int c, t_scanner *scanner);
int skip(t_scanner *scanner);
int ignore_multiline_comment(t_scanner *scanner);
void check_keyword(char *possible, t_token *token);

#endif /* WRENCOMPILER_SCANNER_H */