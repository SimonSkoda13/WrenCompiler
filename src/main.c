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

int main(int argc, char *argv[])
{
  printf("IFJ25 Compiler - Development Version\n");
  printf("=====================================\n\n");

  if (argc < 2)
  {
    fprintf(stderr, "Usage: %s <source_file.ifj>\n", argv[0]);
    return 1;
  }

  printf("Source file: %s\n", argv[1]);
  printf("\nCompiler initialization successful!\n");
  printf("TODO: Implement lexer, parser, semantic analysis, and code generation.\n");

  return 0;
}
