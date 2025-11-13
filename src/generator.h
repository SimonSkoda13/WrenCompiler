/*
 * Prekladač jazyka IFJ25
 * VUT FIT
 *
 * Autori:
 *   - Martin Michálik (xmicham00)
 */

#ifndef WRENCOMPILER_GENERATOR_H
#define WRENCOMPILER_GENERATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "errors.h"
#include "ast.h"
#include "symtable.h"

// Forward deklarácia pre t_param_list
#define MAX_PARAMS 32
typedef struct
{
    char *names[MAX_PARAMS];
    int count;
} t_param_list;

/**
 * @brief Kontroluje, či znak musí byť escapovaný
 * @param c Znak na kontrolu
 * @return 1 ak znak musí byť escapovaný, 0 inak
 */
int must_escape(unsigned char c);

/**
 * @brief Počíta dĺžku výsledného stringu s escape sekvenciami
 * @param input Vstupný string
 * @return Dĺžka escapovaného stringu
 */
size_t calculate_escaped_length(const char *input);

/**
 * @brief Konvertuje klasický C string na formát s escape sekvenciami
 * @param input Vstupný string (char*)
 * @return Nově alokovaný string s escape sekvenciami, alebo NULL pri chybe
 *         Volajúci musí uvoľniť pamäť pomocou free()
 */
char *convert_to_escaped_string(const char *input);

/**
 * @brief Vytvára manglované meno funkcie pre podporu preťažovania
 * @param func_name Pôvodný názov funkcie
 * @param param_count Počet parametrov funkcie
 * @return Nově alokovaný string s manglovaným menom (napr. "foo$arity2")
 *         Volajúci musí uvoľniť pamäť pomocou free()
 */
char *mangle_function_name(const char *func_name, int param_count);

/**
 * @brief Vytvára manglované meno pre statický getter
 * @param getter_name Pôvodný názov getteru
 * @return Nově alokovaný string s manglovaným menom (napr. "getValue$get")
 *         Volajúci musí uvoľniť pamäť pomocou free()
 */
char *mangle_getter_name(const char *getter_name);

/**
 * @brief Vytvára manglované meno pre statický setter
 * @param setter_name Pôvodný názov setteru
 * @return Nově alokovaný string s manglovaným menom (napr. "setValue$set")
 *         Volajúci musí uvoľniť pamäť pomocou free()
 */
char *mangle_setter_name(const char *setter_name);

/**
 * @brief Generuje unikátny názov premennej s nesting levelom
 * @param var_name Základný názov premennej
 * @param nesting_level Úroveň vnorenia (0 = parameter, 1+ = vnorené bloky)
 * @return Ukazovateľ na statický buffer s unikátnym názvom
 */
const char *get_var_name_with_nesting(const char *var_name, int nesting_level);

/**
 * @brief Nastaví globálnu tabuľku symbolov pre generátor
 * @param ifj_symtable Ukazovateľ na tabuľku symbolov
 */
void generator_set_symtable(t_symtable *ifj_symtable);

/**
 * @brief Získa globálnu tabuľku symbolov
 * @return Ukazovateľ na tabuľku symbolov
 */
t_symtable *get_global_symtable();

/**
 * @brief Generuje hlavičku programu (.IFJcode25)
 */
void generate_header();

/**
 * @brief Generuje definície všetkých builtin funkcií ako callable subroutines
 */
void generate_builtin_function_definitions();

/**
 * @brief Generuje kód pre výpis string literálu (WRITE string@...)
 * @param string String literál na výpis
 */
void builtin_write_string_literal(char *string);
void builtin_write_float_literal(double number);
void builtin_write_integer_literal(long long number);
void builtin_write_var(char *var_id);

/**
 * @brief Generuje kód pre deklaráciu premennej (DEFVAR LF@var_name)
 * @param var_name Názov premennej
 */
void generate_var_declaration(const char *var_name);

/**
 * @brief Generuje kód pre začiatok funkcie (LABEL, CREATEFRAME, PUSHFRAME)
 * @param func_name Názov funkcie
 * @param mangled_name Manglované meno funkcie
 * @param params Zoznam parametrov funkcie
 */
void generate_function_start(const char *func_name, const char *mangled_name, t_param_list *params);

/**
 * @brief Generuje všetky DEFVAR pre premenné funkcie
 */
void generate_all_function_defvars();

/**
 * @brief Začne bufferovanie kódu tela funkcie
 */
void start_function_body_buffering();

/**
 * @brief Ukončí bufferovanie a vypíše DEFVAR + telo funkcie
 */
void end_function_body_buffering();

/**
 * @brief Generuje kód pre koniec funkcie (POPFRAME, RETURN)
 * @param func_name Názov funkcie (pre budúce rozšírenia)
 */
void generate_function_end(const char *func_name);

/**
 * @brief Generuje kód pre návratovú hodnotu funkcie (pushnutie na zásobník)
 * @param ast AST strom výrazu ktorý sa má vrátiť (NULL pre prázdny return)
 */
void generate_return_value(t_ast_node *ast);

/**
 * @brief Generuje kód pre pushnutie argumentu na zásobník
 * @param ast AST strom výrazu pre argument
 */
void generate_push_argument(t_ast_node *ast);

/**
 * @brief Generuje kód pre pushnutie string literálu na zásobník
 * @param str String literál
 */
void generate_push_string_literal(const char *str);

/**
 * @brief Generuje kód pre pushnutie integer literálu na zásobník
 * @param value Integer hodnota
 */
void generate_push_int_literal(long long value);

/**
 * @brief Generuje kód pre pushnutie float literálu na zásobník
 * @param value Float hodnota
 */
void generate_push_float_literal(double value);

/**
 * @brief Generuje kód pre pushnutie premennej na zásobník
 * @param var_name Názov premennej
 */
void generate_push_variable(const char *var_name);

/**
 * @brief Generuje kód pre pushnutie null hodnoty na zásobník
 */
void generate_push_null();

/**
 * @brief Generuje kód pre volanie užívateľskej funkcie
 * @param func_name Názov funkcie ktorú voláme
 * @param arg_count Počet argumentov funkcie (pre name mangling)
 */
void generate_function_call(const char *func_name, int arg_count);

/**
 * @brief Generuje kód pre volanie statického settera
 * @param setter_name Názov settera ktorý voláme
 */
void generate_setter_call(const char *setter_name);

/**
 * @brief Generuje kód pre presun návratovej hodnoty funkcie do premennej
 * @param var_name Názov premennej do ktorej sa priradí návratová hodnota
 */
void generate_move_retval_to_var(const char *var_name);

/**
 * @brief Generuje kód pre priradenie výrazu do premennej
 * @param var_name Názov premennej do ktorej sa priradí výsledok
 * @param ast AST strom výrazu
 */
void generate_assignment(const char *var_name, t_ast_node *ast);

/**
 * @brief Pomocná funkcia pre vytvorenie hodnoty (literálu alebo premennej) pre inštrukcie
 * @param node AST uzol
 * @param result Buffer pre výsledok (formát "int@123", "string@...", "LF@var")
 * @param result_size Veľkosť bufferu
 */
void get_value_string(t_ast_node *node, char *result, size_t result_size);

/**
 * @brief Rekurzívna funkcia pre generovanie kódu z AST
 * @param node Aktuálny AST uzol
 * @param result_var Názov premennej kde sa uloží výsledok (formát: "LF@__tmp0")
 * @return 0 pri úspechu, inak error kód
 */
int generate_expression_code(t_ast_node *node, char *result_var, size_t result_var_size);

/**
 * @brief Získa ďalšie unikátne ID pre label
 * @return Unikátne číslo pre label
 */
int get_next_label_id(void);

/**
 * @brief Generuje začiatok if-else konštrukcie (vyhodnotenie podmienky)
 * @param condition_ast AST uzol s podmienkou
 * @param label_id Unikátne ID pre labely
 */
void generate_if_start(t_ast_node *condition_ast, int label_id);

/**
 * @brief Generuje prechod medzi then a else vetvou
 * @param label_id Unikátne ID pre labely
 */
void generate_if_else_start(int label_id);

/**
 * @brief Generuje koniec if-else konštrukcie
 * @param label_id Unikátne ID pre labely
 */
void generate_if_end(int label_id);

/**
 * @brief Generuje začiatok while cyklu
 * @param label_id Unikátne ID pre labely
 */
void generate_while_start(int label_id);

/**
 * @brief Generuje vyhodnotenie podmienky while cyklu
 * @param condition_ast AST uzol s podmienkou
 * @param label_id Unikátne ID pre labely
 */
void generate_while_condition(t_ast_node *condition_ast, int label_id);

/**
 * @brief Generuje koniec while cyklu
 * @param label_id Unikátne ID pre labely
 */
void generate_while_end(int label_id);

#endif // WRENCOMPILER_GENERATOR_H