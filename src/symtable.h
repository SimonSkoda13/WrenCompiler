/**
 * Symbol table header for IFJ25 compiler
 * AVL tree implementation for storing variables and functions
 *
 * @author Šimon Škoda
 * @date 2025-10-22
 */

#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stdbool.h>

// Types of symbols we can store
typedef enum
{
  SYM_VAR_LOCAL,  // local variable
  SYM_VAR_GLOBAL, // global variable
  SYM_FUNCTION,   // function
  SYM_GETTER,     // getter
  SYM_SETTER      // setter
} e_symbol_type;

// data types for variables
typedef enum
{
  TYPE_UNKNOWN, // not known yet
  TYPE_NUM,     // number
  TYPE_STRING,  // string
  TYPE_NULL     // null value
} e_data_type;

// Variable metadata - stores info about variables
typedef struct
{
  bool ifj_defined;     // is variable defined?
  bool ifj_initialized; // is it initialized?
  int ifj_scope_level;  // scope (0=global, higher=local)
  e_data_type ifj_type; // what type is it
} t_var_metadata;

// Function metadata - stores info about functions
typedef struct
{
  int ifj_param_count;          // how many params
  e_data_type ifj_return_type;  // what does it return
  e_data_type *ifj_param_types; // array of param types
} t_func_metadata;

// AVL tree node - one symbol in the tree
typedef struct avl_node
{
  char *ifj_key;              // name of the symbol
  e_symbol_type ifj_sym_type; // what kind of symbol

  // union saves memory - either var OR func metadata
  union
  {
    t_var_metadata ifj_var;   // if its a variable
    t_func_metadata ifj_func; // if its a function
  } ifj_data;

  int ifj_height;             // height for AVL balancing
  struct avl_node *ifj_left;  // left child
  struct avl_node *ifj_right; // right child
} t_avl_node;

// Main symbol table structure
typedef struct
{
  t_avl_node *ifj_root;  // root of AVL tree
  int ifj_current_scope; // current scope level
} t_symtable;

// Initialize symbol table
void symtable_init(t_symtable *ifj_table);

// Destroy symbol table and free memory
void symtable_destroy(t_symtable *ifj_table);

// Insert variable into table
// Returns true if ok, false if already exists
bool symtable_insert_var(t_symtable *ifj_table, const char *ifj_key,
                         e_symbol_type ifj_type, e_data_type ifj_data_type);

// Insert function into table
// Returns true if success, false otherwise
bool symtable_insert_func(t_symtable *ifj_table, const char *ifj_key,
                          int ifj_param_count, e_data_type *ifj_param_types,
                          e_data_type ifj_return_type, e_symbol_type ifj_sym_type);

// Search for symbol in table
// Returns pointer to node or NULL if not found
t_avl_node *symtable_search(t_symtable *ifj_table, const char *ifj_key);

// Delete symbol from table
// returns true if deleted, false if not found
bool symtable_delete(t_symtable *ifj_table, const char *ifj_key);

// Enter new scope (increase scope level)
void symtable_enter_scope(t_symtable *ifj_table);

// Exit scope and delete all symbols from that scope
void symtable_exit_scope(t_symtable *ifj_table);

// Get current scope level
int symtable_get_scope(t_symtable *ifj_table);

#endif // SYMTABLE_H
