/*
 * Prekladač jazyka IFJ25
 * VUT FIT
 *
 * Autori:
 *   - Martin Michálik (xmicham00)
 */
#include "scanner.h";

/**
 * Funkcia, ktorá načíta ďalší token zo vstupného súboru.
 * Alebo vráti lexikálnu chybu.
 * @param scanner Ukazovateľ na scanner.
 * @param token štruktúra tokenu do ktorej bude prečitaný token predaný.
 * @return 0 ak sa podarilo načítať token, 1 ak nastala lexikálna chyba.
 */
int get_next_token(t_scanner *scanner, t_token *token) {
    int c;
    c = skip(scanner); //skip whitespace

    //Takto sa robí FSM v C-éčku
    switch (c) {
        case EOF:
            token->type = END_OF_FILE;
            return 0;
        case '+':
            token->type = OP_ADD;
            return 0;
        case '-':
            token->type = OP_SUB;
            return 0;
        case '*':
            token->type = OP_MUL;
            return 0;
        case '=':
            token->type = OP_ASSIGN;
            c = next_char(scanner);
            if (c == '=') {
                token->type = OP_EQUALS;
            } else {
                putback(c, scanner);
            }
            return 0;
        case '<':
            token->type = OP_LESS_THAN;
            c = next_char(scanner);
            if (c == '=') {
                token->type = OP_LESS_EQUAL;
            } else {
                putback(c, scanner);
            }
            return 0;
        case '>':
            token->type = OP_GREATER_THAN;
            c = next_char(scanner);
            if (c == '=') {
                token->type = OP_GREATER_THAN_EQUAL;
            } else {
                putback(c, scanner);
            }
            return 0;
        case '!':
            c = next_char(scanner);
            if (c == '=') {
                token->type = OP_NOT_EQUALS;
                return 0;
            } 
            // Return lex error code
            return 1;
        case '(':
            token->type = LEFT_PAREN;
            return 0;
        case ')':
            token->type = RIGHT_PAREN;
            return 0;
        case '{':
            token->type = LEFT_BRACE;
            return 0;
        case '}':
            token->type = RIGHT_BRACE;
            return 0;
        case '/':
            c = next_char(scanner);
            if (c == '/') {
                // Single-line comment
                while (c != '\n') {
                    c = next_char(scanner);
                }
                //Komentáre ignorujeme takže nevytvoríme token ale načítame ďalší
                return get_next_token(scanner, token);
            } else if (c == '*') {
                // Multi-line comment
                int prev = 0;
                while (true) {
                    c = next_char(scanner);
                    if (c == EOF) {
                        // Return lex error code for unclosed comment
                        return 1;
                    }
                    if (prev == '*' && c == '/') {
                        break; //Koniec multi-line komentára
                    }
                    prev = c;
                }
                return get_next_token(scanner, token);
            } else {
                putback(c, scanner);
                token->type = OP_DIV;
                return 0;
            }
        case '"': {
            //Toto je diablovo dielo lebo musíme podporovať multiline stringy cez """
            //a zároveň escapovanie znakov
            int cap = 256; //počiatočná kapacita bufferu pre string (256 lebo prečo nie)
            int len = 0; //aktuálna dĺžka stringu

            char *buf = malloc(cap);
            if (buf == NULL) return 1;

            c = next_char(scanner); //načítame ďalší znak a môže sa začať peklo 
            while (c != '"' && c != EOF && c != '\n') {
                // Handle escape sequences
                if (c == '\\') {
                    int esc = next_char(scanner);
                    if (esc == EOF) { 
                        free(buf); 
                        return 1; 
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
                            return 1; 
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
                      return 1; 
                    }
                    buf = tmp;
                }
                //vložíme znak do bufferu a pokračujem v čitaní stringu
                buf[len++] = (char)c;
                c = next_char(scanner);
            }

            if (c != '"') // unterminated string
            { 
              free(buf); 
              return 1; 
            } 
            c = next_char(scanner);
            //multi-line string handling
            if (c == '"') {
                while (true) {
                  c = next_char(scanner);
                  if (c == EOF) { 
                    free(buf); 
                    return 1;
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
                            char *n = realloc(buf, cap);
                            if (!n) { free(buf); return 1; }
                            buf = n;
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
                            return 1; 
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
            return 0;
        }
        //TODO: čísla
        case '0':
            //Toto je tiež sranda lebo čísla môžu byť hexadecimálne a exponenciálne
            c = next_char(scanner);
            if (c == 'x' || c == 'X') {
                //hexadecimal number
                char buf[64];
                int i = 0;
                int d = next_char(scanner);

                /* collect hex digits */
                while (d != EOF && isxdigit(d)) {
                    if (i < (int)sizeof(buf) - 1) buf[i++] = (char)d;
                    d = next_char(scanner);
                }

                if (i == 0) { /* no hex digits -> lex error */
                    putback(d, scanner);
                    return 1;
                }

                buf[i] = '\0';
                putback(d, scanner);

                errno = 0;
                long val = strtoull(buf, NULL, 16);
                if (errno == ERANGE) return 1; //overflow

                token->type = NUM_HEX;
                token->value.number_int = val;
                return 0;
            } else if (c == '.') {
              
            }
            
            
        default: {
            //Handle identifiers and keywords. Musí sa to riešiť v default lebo nemôžeme urobiť
            //case isalpha(c): plus asi bude treba riešiť aj iné veci
            if (isalpha(c)) {
                //Ak sa jedná o identifikátor alebo kľúčové slovo
                char buf[256]; //Zatiaľ fixná veľkosť bufferu pre identifikátor
                int i = 0;

                buf[i++] = (char)c;
                c = next_char(scanner);
                while (isalnum(c) || c == '_') {
                    if (i < (int)sizeof(buf) - 1) buf[i++] = (char)c;
                    c = next_char(scanner);
                }
                buf[i] = '\0';
                putback(c, scanner);

                token->type = IDENTIFIER_OR_KEYWORD;
                token->value.identifier = strdup(buf);
                return 0;
            }
        }
    }
    return 1; //lex error lebo už by teoreticky nemalo byť čo čitať z inputu
}

/**
 * Načíta ďalší znak zo vstupného súboru.
 * Alebo použije znak ktorý bol vrátený späť.
 * @param scanner Ukazovateľ na scanner.
 * @return Načítaný znak alebo EOF.
 */
int next_char(t_scanner *scanner)
{
    int c;

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
 * Preskočí biele znaky (whitespace).
 * @param scanner Ukazovateľ na scanner.
 * @return Prvý ne-biely znak alebo EOF.
 */
int skip(t_scanner *scanner)
{
    int c;

    c = next_char(scanner);
    while (' ' == c || '\t' == c || '\n' == c || '\r' == c || '\f' == c) {
        c = next_char(scanner);
    }
    return c;
}