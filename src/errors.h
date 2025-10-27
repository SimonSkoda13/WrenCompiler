/*
 * Prekladač jazyka IFJ25
 * VUT FIT
 *
 * Autori:
 *   - Martin Michálik (xmicham00)
 */

 #ifndef WRENCOMPILER_ERRORS_H
 #define WRENCOMPILER_ERRORS_H

 #include <stdio.h>
 #include <stdlib.h>
 #include <stdarg.h>

/**
 * @brief Návratové chybové kódy prekladača IFJ25
 */
typedef enum {
    NO_ERROR = 0,           // Úspech - preklad prebehol bez chýb
    
    // Chyby prekladača (statické)
    ERR_LEXICAL = 1,            // Lexikálna chyba (chybná štruktúra lexému)
    ERR_SYNTAX = 2,             // Syntaktická chyba (chybná syntax, chýbajúca kostra)
    ERR_SEM_UNDEF = 3,          // Nedefinovaná funkcia alebo premenná
    ERR_SEM_REDEF = 4,          // Redefinícia funkcie alebo premennej
    ERR_SEM_PARAMS = 5,         // Nesprávny počet argumentov/typ parametru vstavanej funkcie
    ERR_SEM_TYPE_COMPAT = 6,    // Typová kompatibilita vo výrazoch
    ERR_SEM_OTHER = 10,         // Ostatné sémantické chyby
    
    // Behové chyby (dynamické - za behu)
    ERR_RUNTIME_PARAM_TYPE = 25,    // Behová chyba - zlý typ parametru vstavanej funkcie
    ERR_RUNTIME_TYPE_COMPAT = 26,   // Behová chyba - typová kompatibilita vo výrazoch
    
    // Interná chyba
    ERR_INTERNAL = 99           // Interná chyba prekladača (alokácia pamäte atď.)
} e_error_code;

extern e_error_code err_code;

void exit_with_error(e_error_code code, const char *format, ...);

#endif /* WRENCOMPILER_ERRORS_H */