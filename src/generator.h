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
char* convert_to_escaped_string(const char *input);

/**
 * @brief Generuje hlavičku programu (.IFJcode25)
 */
void generate_header();

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
 */
void generate_function_start(const char *func_name);

/**
 * @brief Generuje kód pre koniec funkcie (POPFRAME, RETURN)
 * @param func_name Názov funkcie (pre budúce rozšírenia)
 */
void generate_function_end(const char *func_name);

/**
 * @brief Generuje kód pre priradenie výrazu do premennej
 * @param var_name Názov premennej do ktorej sa priradí výsledok
 * @param ast AST strom výrazu
 */
void generate_assignment(const char *var_name, t_ast_node *ast);

#endif // WRENCOMPILER_GENERATOR_H