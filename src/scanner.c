/*
 * Prekladač jazyka IFJ25
 * VUT FIT
 *
 * Autori:
 *   - Martin Michálik (xmicham00)
 */
#include "scanner.h"
#include "errors.h"

#define LEX_OK 0
#define LEX_ERROR 1

e_error_code err_code;

/**
 * Funkcia, ktorá načíta ďalší token zo vstupného súboru.
 * Alebo vráti lexikálnu chybu.
 * @param scanner Ukazovateľ na scanner.
 * @param token štruktúra tokenu do ktorej bude prečitaný token predaný.
 * @return 0 ak sa podarilo načítať token, 1 ak nastala lexikálna chyba.
 */
int get_next_token(t_scanner *scanner, t_token *token) {
    err_code = NO_ERROR; // Reset erroru pri každom volaní
    int c;
    c = skip(scanner); //skip whitespace
    
    switch (c) {
        case EOF:
            token->type = END_OF_FILE;
            return LEX_OK;
        case ',':
            token->type = COMMA;
            return LEX_OK;
        case '.':
            token->type = DOT;
            return LEX_OK;
        case '\n':
            token->type = EOL;
            return LEX_OK;
        case '+':
            token->type = OP_ADD;
            return LEX_OK;
        case '-':
            token->type = OP_SUB;
            return LEX_OK;
        case '*':
            token->type = OP_MUL;
            return LEX_OK;
        case '=':
            token->type = OP_ASSIGN;
            c = next_char(scanner);
            if (c == '=') {
                token->type = OP_EQUALS;
            } else {
                putback(c, scanner);
            }
            return LEX_OK;
        case '<':
            token->type = OP_LESS_THAN;
            c = next_char(scanner);
            if (c == '=') {
                token->type = OP_LESS_EQUAL;
            } else {
                putback(c, scanner);
            }
            return LEX_OK;
        case '>':
            token->type = OP_GREATER_THAN;
            c = next_char(scanner);
            if (c == '=') {
                token->type = OP_GREATER_THAN_EQUAL;
            } else {
                putback(c, scanner);
            }
            return LEX_OK;
        case '!':
            c = next_char(scanner);
            if (c == '=') {
                token->type = OP_NOT_EQUALS;
                return LEX_OK;
            } 
            // Return lex error code
            err_code = ERR_LEXICAL;
            return LEX_ERROR;
        case '(':
            token->type = LEFT_PAREN;
            return LEX_OK;
        case ')':
            token->type = RIGHT_PAREN;
            return LEX_OK;
        case '{':
            token->type = LEFT_BRACE;
            return LEX_OK;
        case '}':
            token->type = RIGHT_BRACE;
            return LEX_OK;
        case '/':
            c = next_char(scanner);
            if (c == '/') {
                // Single-line comment
                while (c != '\n') {
                    c = next_char(scanner);
                }
                //Komentáre ignorujeme ale EOL musíme zachovať
                token->type = EOL;
                return LEX_OK;
            } else if (c == '*') {
                // Multi-line comment
                int multiline = ignore_multiline_comment(scanner);
                if (multiline == 1) {
                    err_code = ERR_LEXICAL;
                    return LEX_ERROR;
                }
                token->type = EOL;
                return LEX_OK;
            } else {
                putback(c, scanner);
                token->type = OP_DIV;
                return LEX_OK;
            }
        case '"': {
            int cap = 256; //počiatočná kapacita bufferu pre string (256 lebo prečo nie)
            int len = 0; //aktuálna dĺžka stringu

            char *buf = malloc(cap);
            if (buf == NULL) return 1;

            c = next_char(scanner);
            while (c != '"' && c != EOF && c != '\n') {
                // Handle escape sequences
                if (c == '\\') {
                    int esc = next_char(scanner);
                    if (esc == EOF) { 
                        free(buf); 
                        err_code = ERR_LEXICAL;
                        return LEX_ERROR; 
                    }
                    char out;
                    switch (esc) {
                        case 'n': 
                            out = '\n'; 
                            break;
                        case 't': 
                            out = '\t'; 
                            break;
                        case 'r':   
                            out = '\r';   
                            break;
                        case '\\': 
                            out = '\\';   
                            break;
                        case '"': 
                            out = '"'; 
                            break;
                        case '0': 
                            out = '\0'; 
                            break;
                        default: 
                            out = (char)esc; 
                            break;
                    }
                    //Treba kontrolovať či neprekračujeme kapacitu bufferu
                    if (len + 1 >= cap) {
                        cap *= 2;
                        char *tmp = realloc(buf, cap);
                        if (tmp == NULL) { 
                            free(buf); 
                            err_code = ERR_INTERNAL;
                            return LEX_ERROR; 
                        }
                        buf = tmp;
                    }
                    buf[len++] = out;
                    c = next_char(scanner);
                    continue; //escape handled, continue
                }
                //Podobne ako vyšsie, kontrola kapacity bufferu
                if (len + 1 >= cap) {
                    cap *= 2;
                    char *tmp = realloc(buf, cap);
                    if (!tmp) { 
                      free(buf); 
                      err_code = ERR_INTERNAL;
                      return LEX_ERROR;
                    }
                    buf = tmp;
                }
                //vložíme znak do bufferu a pokračujem v čitaní stringu
                buf[len++] = (char)c;
                c = next_char(scanner);
            }
            //unterminated string     
            if (c != '"') { 
              free(buf); 
              err_code = ERR_LEXICAL;
              return LEX_ERROR; 
            } 
            c = next_char(scanner);
            //multi-line string handling
            if (c == '"') {
                while (true) {
                  c = next_char(scanner);
                  if (c == EOF) { 
                    free(buf); 
                    err_code = ERR_LEXICAL;
                    return LEX_ERROR;
                  }
                  //Ak narazíme na """, tak končíme multi-line string
                  if (c == '"') {
                    char c1 = next_char(scanner);
                    char c2 = next_char(scanner);
                    if (c1 == '"' && c2 == '"') {
                      // End of multi-line string
                      break;
                    } else {
                        if (len + 1 >= cap) {
                            cap *= 2;
                            char *tmp = realloc(buf, cap);
                            if (!tmp) { 
                                free(buf); 
                                err_code = ERR_INTERNAL;
                                return LEX_ERROR; 
                            }
                            buf = tmp;
                        }
                        buf[len++] = (char)c;
                        buf[len++] = (char)c1;
                        buf[len++] = (char)c2;
                    }
                  } else {
                      //Zase len kontrola kapacity bufferu
                      if (len + 1 >= cap) {
                          cap *= 2;
                          char *tmp = realloc(buf, cap);
                          if (!tmp) { 
                            free(buf); 
                            err_code = ERR_INTERNAL;
                            return LEX_ERROR; 
                          }
                          buf = tmp;
                      }
                      buf[len++] = (char)c; //Pridame znak do bufferu
                  }
                }
            } else {
                //Putback lebo sme prečitali znak ktorý nebol "
                putback(c, scanner);
            }
            //Ukončíme string a vytvoríme token
            buf[len] = '\0';
            token->type = STRING_LITERAL;
            token->value.string = buf;
            return LEX_OK;
        }
        case '0':
            c = next_char(scanner);
            if (c == 'x' || c == 'X') {
                //hexadecimal number
                char buf[64];
                int i = 0;
                int d = next_char(scanner);

                while (d != EOF && isxdigit(d)) {
                    if (i < (int)sizeof(buf) - 1) buf[i++] = (char)d;
                    d = next_char(scanner);
                }

                if (i == 0) {
                    putback(d, scanner);
                    err_code = ERR_LEXICAL;
                    return LEX_ERROR;
                }

                buf[i] = '\0';
                putback(d, scanner);

                errno = 0;
                long val = strtoull(buf, NULL, 16);
                if (errno == ERANGE) {
                    err_code = ERR_INTERNAL;
                    return LEX_ERROR;
                }

                token->type = NUM_HEX;
                token->value.number_int = val;
                return LEX_OK;
            }
            char int_buf[64];
            int int_part_len = 0;
            while (isdigit(c)) {
                int_buf[int_part_len++] = (char)c;
                c = next_char(scanner);
            }
            int_buf[int_part_len] = '\0';
            
            if (c == '.') {
                c = next_char(scanner);
                if (!isdigit(c)) {
                    putback(c, scanner);
                    err_code = ERR_LEXICAL;
                    return LEX_ERROR; //lex error
                }
                char float_buf[64];   
                int float_len = 0;
                while (isdigit(c)) {
                    float_buf[float_len++] = (char)c;
                    c = next_char(scanner);
                }
                float_buf[float_len] = '\0';
                int_buf[int_part_len] = '\0';
                // "<int>.<frac>[e[+-]<digits>]"
                char num_buf[256];
                int nwritten = snprintf(num_buf, sizeof num_buf, "%s.%s", int_buf, float_buf);
                if (nwritten < 0 || nwritten >= (int)sizeof num_buf) {
                    err_code = ERR_INTERNAL; //overflow
                    return LEX_ERROR; 
                }
                
                if (c == 'e' || c == 'E') {
                    c = next_char(scanner);
                    int sign = 1;
                    if (c == '-' || c == '+') {
                        if (c == '-') sign = -1;
                        c = next_char(scanner);
                    }
                    if (!isdigit(c)) {
                        err_code = ERR_LEXICAL;
                        return LEX_ERROR; //lex error
                    }
                    char exp_buf[16];
                    int exp_len = 0;
                    while (isdigit(c)) {
                        exp_buf[exp_len++] = (char)c;
                        c = next_char(scanner);
                    }
                    exp_buf[exp_len] = '\0';
                    float float_part = strtof(num_buf, NULL);
                    int exp = strtol(exp_buf, NULL, 10);
                    float val = float_part * powf(10, sign * exp);
                    putback(c, scanner);
                    token->type = NUM_EXP_FLOAT;
                    token->value.number_float = val;
                    return LEX_OK;
                }
                putback(c, scanner);
                float val = strtof(num_buf, NULL);
                token->type = NUM_FLOAT;
                token->value.number_float = val;
                return LEX_OK;
            }
            if (c == 'e' || c == 'E') {
                c = next_char(scanner);
                int sign = 1;
                if (c == '-' || c == '+') {
                    if (c == '-') sign = -1;
                    c = next_char(scanner);
                }
                if (!isdigit(c)) {
                    err_code = ERR_LEXICAL;
                    return LEX_ERROR; //lex error
                }
                char exp_buf[16];
                int i = 0;
                while (isdigit(c)) {
                    exp_buf[i++] = (char)c;
                    c = next_char(scanner);
                }
                exp_buf[i] = '\0';
                int_buf[int_part_len] = '\0';

                long val_int = strtol(int_buf, NULL, 10);
                int exp = strtol(exp_buf, NULL, 10);
                double val = val_int * pow(10, sign * exp); 
                putback(c, scanner);
                if (sign == 1) {
                    token->type = NUM_EXP_INT;
                    token->value.number_int = (long)val;
                    return LEX_OK;
                } else {
                    token->type = NUM_EXP_FLOAT;
                    token->value.number_float = val;
                    return LEX_OK;
                }
            }
            
            putback(c, scanner);
            int val = strtol(int_buf, NULL, 10);
            token->type = NUM_INT;
            token->value.number_int = val;
            return LEX_OK;
        case '_':
            c = next_char(scanner);
            if (c != '_') {
                return 1;
            } else {
                c = next_char(scanner);
                char buf[256];
                int i = 0;
                while (isalnum(c) || c == '_') {
                    buf[i++] = c;
                    c = next_char(scanner);
                }
                buf[i] = '\0';
                putback(c, scanner);

                token->type = GLOBAL_VAR;
                int id_len = strlen(buf) + 1;
                char *id = malloc(sizeof(char) * id_len);
                if (id == NULL) {
                    err_code = ERR_INTERNAL;
                    return LEX_ERROR;
                }
                memcpy(id, buf, id_len);
                token->value.identifier = id;
                return LEX_OK;
            }
            
        default: {
            //Handle identifiers and keywords. Musí sa to riešiť v default lebo nemôžeme urobiť
            //case isalpha(c): plus asi bude treba riešiť aj iné veci
            if (isalpha(c)) {
                //Ak sa jedná o identifikátor alebo kľúčové slovo
                char buf[256]; //Zatiaľ fixná veľkosť bufferu pre identifikátor
                int i = 0;

                buf[i++] = c;
                c = next_char(scanner);
                while (isalnum(c) || c == '_') {
                    if (i < (int)sizeof(buf) - 1) buf[i++] = (char)c;
                    c = next_char(scanner);
                }
                buf[i] = '\0';
                putback(c, scanner);
                check_keyword(buf, token);
                if (token->type == IDENTIFIER) {
                    int id_len = strlen(buf) + 1;
                    char *id = malloc(sizeof(char) * id_len);
                    if (id == NULL) {
                        err_code = ERR_INTERNAL;
                        return LEX_ERROR;
                    }
                    memcpy(id, buf, id_len);
                    token->value.identifier = id;
                }
                return LEX_OK;
            }

            if (isdigit(c)) {
                char int_buf[64];
                int int_part_len = 0;
                while (isdigit(c)) {
                    int_buf[int_part_len++] = c;
                    c = next_char(scanner);
                }
                if (c == '.') {
                    c = next_char(scanner);
                    if (!isdigit(c)) {
                        putback(c, scanner);
                        err_code = ERR_LEXICAL;
                        return LEX_ERROR; //lex error
                    }
                    char float_buf[64];   
                    int dec_part_len = 0;
                    while (isdigit(c)) {
                        float_buf[dec_part_len++] = (char)c;
                        c = next_char(scanner);
                    }
                    float_buf[dec_part_len] = '\0';
                    int_buf[int_part_len] = '\0';
                    // "<int>.<frac>[e[+-]<digits>]"
                    char num_buf[256];
                    int nwritten = snprintf(num_buf, sizeof num_buf, "%s.%s", int_buf, float_buf);
                    if (nwritten < 0 || nwritten >= (int)sizeof num_buf) {
                        err_code = ERR_INTERNAL; //overflow
                        return LEX_ERROR;
                    }
                
                    if (c == 'e' || c == 'E') {
                        c = next_char(scanner);
                        int sign = 1;
                        if (c == '-' || c == '+') {
                            if (c == '-') sign = -1;
                            c = next_char(scanner);
                        }
                        if (!isdigit(c)) {
                            err_code = ERR_LEXICAL;
                            return LEX_ERROR; //lex error
                        }
                        char exp_buf[16];
                        int i = 0;
                        while (isdigit(c)) {
                            exp_buf[i++] = (char)c;
                            c = next_char(scanner);
                        }
                        exp_buf[i] = '\0';
                        float float_part = strtof(num_buf, NULL);
                        int exp = strtol(exp_buf, NULL, 10);
                        float val = float_part * powf(10, sign * exp);
                        putback(c, scanner);
                        token->type = NUM_EXP_FLOAT;
                        token->value.number_float = val;
                        return LEX_OK;
                    }
                    putback(c, scanner);
                    float val = strtof(num_buf, NULL);
                    token->type = NUM_FLOAT;
                    token->value.number_float = val;
                    return LEX_OK;
                }
                if (c == 'e' || c == 'E') {
                    c = next_char(scanner);
                    int sign = 1;
                    if (c == '-' || c == '+') {
                        if (c == '-') sign = -1;
                        c = next_char(scanner);
                    }
                    if (!isdigit(c)) {
                        err_code = ERR_LEXICAL;
                        return LEX_ERROR; //lex error
                    }
                    char exp_buf[16];
                    int i = 0;
                    while (isdigit(c)) {
                        exp_buf[i++] = (char)c;
                        c = next_char(scanner);
                    }
                    exp_buf[i] = '\0';
                    int_buf[int_part_len] = '\0';

                    long val_int = strtol(int_buf, NULL, 10);
                    int exp = strtol(exp_buf, NULL, 10);
                    double val = val_int * pow(10, sign * exp); 
                    putback(c, scanner);
                    if (sign == 1) {
                        token->type = NUM_EXP_INT;
                        token->value.number_int = (long)val;
                        return LEX_OK;
                    } else {
                        token->type = NUM_EXP_FLOAT;
                        token->value.number_float = val;
                        return LEX_OK;
                    }
                }
                
                putback(c, scanner);
                int_buf[int_part_len] = '\0';
                int val = strtol(int_buf, NULL, 10);
                token->type = NUM_INT;
                token->value.number_int = val;
                return LEX_OK;
            }
        }
    }
    err_code = ERR_LEXICAL;
    return LEX_ERROR; //lex error lebo už by teoreticky nemalo byť čo čitať z inputu
}

/**
 * Rekurzívna funkcia, ktorá zaistí že sa správne budú ignorovať aj vnorené multiline komentáre
 * lebo wren to dovoľuje
 * @param scanner Ukazovateľ na scanner
 */
int ignore_multiline_comment(t_scanner *scanner) {
    while (true) {
        char c = next_char(scanner);
        if (c == '*' && next_char(scanner) == '/') {
            return 0;
        }
        if (c == '/' && next_char(scanner) == '*') {
            if (ignore_multiline_comment(scanner) == 1) {
                return 1;
            }
        }
        if (c == EOF) {
            return 1;
        }
    }
}

/**
 * Načíta ďalší znak zo vstupného súboru.
 * Alebo použije znak ktorý bol vrátený späť.
 * @param scanner Ukazovateľ na scanner.
 * @return Načítaný znak alebo EOF.
 */
char next_char(t_scanner *scanner) {
    char c;

    if (scanner->putback) 
    {
      c = scanner->putback;
      scanner->putback = 0;
      return c;
    }

    c = fgetc(scanner->stream);
    if ('\n' == c)
    {
      scanner->line++;
    }
    return c;
}

/**
 * Vráti znak späť do skenera.
 * Fajn trik na to aby som nemusel používať ungetc.
 * @param c Znak ktorý sa má vrátiť späť.
 * @param scanner Ukazovateľ na scanner.
 */
void putback(int c, t_scanner *scanner)
{
    scanner->putback = c;
}

/**
 * Preskočí biele znaky (whitespace) okrem znakov nových riadkov <EOL>.
 * @param scanner Ukazovateľ na scanner.
 * @return Prvý ne-biely znak alebo EOF.
 */
int skip(t_scanner *scanner)
{
    int c;

    c = next_char(scanner);
    while (' ' == c || '\t' == c || '\r' == c || '\f' == c) {
        c = next_char(scanner);
    }
    return c;
}

void check_keyword(char *possible, t_token *token) {
    if (strcmp(possible, "class") == 0) {
        token->value.keyword = KW_CLASS;
        token->type = KEYWORD;
    } else if (strcmp(possible, "if") == 0) {
        token->value.keyword = KW_IF;
        token->type = KEYWORD;
    } else if (strcmp(possible, "else") == 0) {
        token->value.keyword = KW_ELSE;
        token->type = KEYWORD;
    } else if (strcmp(possible, "is") == 0) {
        token->value.keyword = KW_IS;
        token->type = KEYWORD;
    } else if (strcmp(possible, "null") == 0) {
        token->value.keyword = KW_NULL_INST;
        token->type = KEYWORD;
    } else if (strcmp(possible, "return") == 0) {
        token->value.keyword = KW_RETURN;
        token->type = KEYWORD;
    } else if (strcmp(possible, "var") == 0) {
        token->value.keyword = KW_VAR;
        token->type = KEYWORD;
    } else if (strcmp(possible, "while") == 0) {
        token->value.keyword = KW_WHILE;
        token->type = KEYWORD;
    } else if (strcmp(possible, "Ifj") == 0) {
        token->value.keyword = KW_IFJ;
        token->type = KEYWORD;
    } else if (strcmp(possible, "static") == 0) {
        token->value.keyword = KW_STATIC;
        token->type = KEYWORD;
    } else if(strcmp(possible, "import") == 0) {
        token->value.keyword = KW_IMPORT;
        token->type = KEYWORD;
    } else if(strcmp(possible, "for") == 0) {
        token->value.keyword = KW_FOR;
        token->type = KEYWORD;
    } else if(strcmp(possible, "Num") == 0) {
        token->value.keyword = KW_NUM;
        token->type = KEYWORD;
    } else if(strcmp(possible, "String") == 0) {
        token->value.keyword = KW_STRING;
        token->type = KEYWORD;
    } else if(strcmp(possible, "Null") == 0){
        token->value.keyword = KW_NULL_TYPE;
        token->type = KEYWORD;
    } else{
        token->type = IDENTIFIER;      
    }   
}