/*
 * Prekladač jazyka IFJ25
 * VUT FIT
 *
 * Autori:
 *   - Martin Michálik (xmicham00)
 */

#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>


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
  NUM_EXP,
  END_OF_FILE,
  STRING_OR_MULTILINE,
  STRING_LITERAL,
  IDENTIFIER_OR_KEYWORD,
  GLOBAL_VAR
} e_token_type;

/**
 * Union pre hodnotu tokenu.
 * Môže byť identifikátor, string, float alebo int.
 * Ak je typ tokenu IDENTIFIER_OR_KEYWORD identifier drží názov identifikátora alebo kľúčového slova.
 * Ak je typ tokenu STRING_LITERAL string drží hodnotu literálu.
 * Ak je typ tokenu NUM_FLOAT, NUM_EXP number_float drží hodnotu float čísla.
 * Ak je typ tokenu NUM_INT, NUM_HEX alebo number_int drží
 */
typedef union {
	char *identifier;
	char *string;
	double number_float;
	long number_int;
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
int next_char(t_scanner *scanner);
void putback(int c, t_scanner *scanner);
int skip(t_scanner *scanner);
