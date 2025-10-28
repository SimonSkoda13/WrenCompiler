/*
 * Prekladač jazyka IFJ25
 * VUT FIT
 *
 * Autori:
 *   - Martin Michálik (xmicham00)
 */

#include "errors.h"
#include <stdarg.h>

void exit_with_error(e_error_code code, const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "Error %d: ", code);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(code);
}
