/**
 * @file main.c
 * @brief Main entry point for IFJ25 compiler
 *
 * Project: IFJ Compiler
 * Team: [Your Team Name]
 * Members: [Your Names]
 */

#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

int main(int argc, char *argv[])
{
    t_scanner scanner;
    FILE *f = NULL;

    if (argc >= 2) {
        f = fopen(argv[1], "r");
        if (!f) {
            fprintf(stderr, "Failed to open input file: %s\n", argv[1]);
            return 1;
        }
        scanner.stream = f;
    } else {
        scanner.stream = stdin;
    }

    /* initialize scanner state members used by scanner.c */
    /* these fields are present in your scanner implementation */
    scanner.line = 1;
    scanner.putback = 0;
    t_token current_token, putback_token;
    
    /* initialize symbol tables */
    t_symtable symtable;
    t_symtable global_symtable;
    symtable_init(&symtable);
    symtable_init(&global_symtable);
    
    parser.scanner = &scanner;
    parser.current_token = &current_token;
    parser.putback_token = &putback_token;  
    parser.has_putback = false;
    parser.symtable = &symtable;
    parser.global_symtable = &global_symtable;
    
    // Nastavíme globálnu symtable pre generátor (pre gettery vo výrazoch)
    generator_set_symtable(&symtable);
    // Nastavíme tabuľku pre globálne premenné (pre správne generovanie DEFVAR)
    generator_set_global_symtable(&global_symtable);
    
    parse_program();
    
    /* cleanup symbol tables */
    symtable_destroy(&symtable);
    symtable_destroy(&global_symtable);
    
    return 0;
}
