#include <stdio.h>
#include <stdlib.h>
#include "scanner.h"

static const char *keyword_to_string(e_keyword k) {
    switch (k) {
        case KW_CLASS: return "class";
        case KW_IF: return "if";
        case KW_ELSE: return "else";
        case KW_IS: return "is";
        case KW_NULL_INST: return "null";
        case KW_RETURN: return "return";
        case KW_VAR: return "var";
        case KW_WHILE: return "while";
        case KW_IFJ: return "ifj";
        case KW_STATIC: return "static";
        case KW_IMPORT: return "import";
        case KW_FOR: return "for";
        case KW_NUM: return "num";
        case KW_STRING: return "string";
        case KW_NULL_TYPE: return "Null";
        default: return "UNKNOWN_KEYWORD";
    }
}

static void print_token(const t_token *t) {
    switch (t->type) {
    case END_OF_FILE:
        printf("END_OF_FILE\n");
        break;
    case OP_ADD: printf("OP_ADD '+'\n"); break;
    case OP_SUB: printf("OP_SUB '-'\n"); break;
    case OP_MUL: printf("OP_MUL '*'\n"); break;
    case OP_DIV: printf("OP_DIV '/'\n"); break;
    case OP_ASSIGN: printf("OP_ASSIGN '='\n"); break;
    case OP_EQUALS: printf("OP_EQUALS '=='\n"); break;
    case OP_LESS_THAN: printf("OP_LESS_THAN '<'\n"); break;
    case OP_LESS_EQUAL: printf("OP_LESS_EQUAL '<='\n"); break;
    case OP_GREATER_THAN: printf("OP_GREATER_THAN '>'\n"); break;
    case OP_GREATER_THAN_EQUAL: printf("OP_GREATER_THAN_EQUAL '>='\n"); break;
    case OP_NOT_EQUALS: printf("OP_NOT_EQUALS '!='\n"); break;
    case LEFT_PAREN: printf("LEFT_PAREN '('\n"); break;
    case RIGHT_PAREN: printf("RIGHT_PAREN ')'\n"); break;
    case LEFT_BRACE: printf("LEFT_BRACE '{'\n"); break;
    case RIGHT_BRACE: printf("RIGHT_BRACE '}'\n"); break;
    case NUM_INT: printf("NUM_INT %ld\n", t->value.number_int); break;
    case NUM_HEX: printf("NUM_HEX 0x%lx\n", (long)t->value.number_int); break;
    case NUM_FLOAT: printf("NUM_FLOAT %g\n", t->value.number_float); break;
    case NUM_EXP: printf("NUM_EXP %g\n", t->value.number_float); break;
    case STRING_OR_MULTILINE: printf("STRING_OR_MULTILINE\n"); break;
    case STRING_LITERAL:
        printf("STRING_LITERAL %s\n", t->value.string ? t->value.string : "");
        break;
    case IDENTIFIER:
        printf("IDENTIFIER %s\n", t->value.identifier ? t->value.identifier : "");
        break;
    case GLOBAL_VAR:
        printf("GLOBAL_VAR %s\n", t->value.identifier);
        break;
    case EOL:
        printf("EOL\n");
        break;
    case DOT:
        printf("DOT\n");
        break;
    case KEYWORD:
        printf("KEYWORD %s\n", keyword_to_string(t->value.keyword));
        break;
    default:
        printf("UNKNOWN TOKEN (%d)\n", (int)t->type);
        break;
    }
}

int main(int argc, char **argv) {
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
    t_token token;
    /* loop until EOF or lex error */
    while (1) {
        int res = get_next_token(&scanner, &token);
        if (res != 0) {
            fprintf(stderr, "Lexical error at line %d\n", scanner.line);
            return 1;
        }

        print_token(&token);

        if (token.type == END_OF_FILE) break;
    }

    return 0;
}
