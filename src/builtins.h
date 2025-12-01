/*
 * Prekladač jazyka IFJ25
 * VUT FIT
 *
 * Autori:
 *   - Matúš Magyar (xmagyam00)
 *   - Martin Michálik (xmicham00)
 */

#ifndef WRENCOMPILER_BUILTINS_H
#define WRENCOMPILER_BUILTINS_H

#include "ast.h"

/**
 * @brief Generuje volanie Ifj.write(term)
 * Vypíše WRITE inštrukciu s parametrom (literál alebo premenná)
 * @param param_node AST uzol parametra
 */
void generate_builtin_write(t_ast_node *param_node);

/**
 * @brief Generuje volanie Ifj.read_str()
 * Vypíše READ inštrukciu pre načítanie stringu
 * @param result_var Názov premennej pre výsledok (formát: "LF@var")
 */
void generate_builtin_read_str(const char *result_var);

/**
 * @brief Generuje volanie Ifj.read_num()
 * Vypíše READ inštrukciu pre načítanie čísla
 * @param result_var Názov premennej pre výsledok (formát: "LF@var")
 */
void generate_builtin_read_num(const char *result_var);

/**
 * @brief Generuje volanie Ifj.floor(term)
 * Vypíše FLOAT2INT alebo MOVE inštrukciu
 * @param result_var Názov premennej pre výsledok
 * @param param_node AST uzol parametra (číslo)
 */
void generate_builtin_floor(const char *result_var, t_ast_node *param_node);

/**
 * @brief Generuje volanie Ifj.str(term)
 * Konvertuje term na string reprezentáciu
 * @param result_var Názov premennej pre výsledok
 * @param param_node AST uzol parametra
 */
void generate_builtin_str(const char *result_var, t_ast_node *param_node);

/**
 * @brief Generuje volanie Ifj.length(s)
 * Vypíše STRLEN inštrukciu s type checkingom
 * @param result_var Názov premennej pre výsledok
 * @param param_node AST uzol parametra (string)
 */
void generate_builtin_length(const char *result_var, t_ast_node *param_node);

/**
 * @brief Generuje volanie Ifj.substring(s, i, j)
 * Extrahuje podreťazec z reťazca s
 * @param result_var Názov premennej pre výsledok
 * @param param_node AST uzol pre string
 * @param i_node AST uzol pre začiatočný index
 * @param j_node AST uzol pre konečný index
 */
void generate_builtin_substring(const char *result_var, t_ast_node *param_node, t_ast_node *i_node, t_ast_node *j_node);

/**
 * @brief Generuje volanie Ifj.strcmp(s1, s2)
 * Lexikograficky porovná dva stringy, vracia -1/0/1
 * @param result_var Názov premennej pre výsledok
 * @param str1_node AST uzol pre prvý string
 * @param str2_node AST uzol pre druhý string
 */
void generate_builtin_strcmp(const char *result_var, t_ast_node *str1_node, t_ast_node *str2_node);

/**
 * @brief Generuje volanie Ifj.ord(s, i)
 * Vráti ASCII hodnotu znaku na pozícii i v reťazci s
 * @param result_var Názov premennej pre výsledok
 * @param param_node AST uzol pre string
 * @param i_node AST uzol pre index
 */
void generate_builtin_ord(const char *result_var, t_ast_node *param_node, t_ast_node *i_node);
/**
 * @brief Generuje volanie Ifj.chr(i)
 * Vypíše INT2CHAR inštrukciu s type checkingom
 * @param result_var Názov premennej pre výsledok
 * @param i_node AST uzol parametra (ASCII kód)
 */
void generate_builtin_chr(const char *result_var, t_ast_node *i_node);

#endif // WRENCOMPILER_BUILTINS_H