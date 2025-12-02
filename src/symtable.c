/*
 * Prekladač jazyka IFJ25
 * VUT FIT
 *
 * Autori:
 *   - Martin Michálik (xmicham00)
 *   - Šimon Škoda (xskodas00)
 *   - Jaroslav Vrbiniak (xvrbinj00)
 */

#define _POSIX_C_SOURCE 200809L
#include "symtable.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// Helper functions for AVL tree operations
// ============================================================================

/**
 * @brief Get height of a node
 * @param ifj_node Node to get height from
 * @return Height of node, or 0 if NULL
 */
static int get_height(t_avl_node *ifj_node)
{
  if (ifj_node == NULL)
  {
    return 0;
  }
  return ifj_node->ifj_height;
}

/**
 * @brief Get balance factor of a node
 * @param ifj_node Node to get balance factor from
 * @return Balance factor (left_height - right_height)
 */
static int get_balance(t_avl_node *ifj_node)
{
  if (ifj_node == NULL)
  {
    return 0;
  }
  return get_height(ifj_node->ifj_left) - get_height(ifj_node->ifj_right);
}

/**
 * @brief Update height of a node based on children heights
 * @param ifj_node Node to update height for
 */
static void update_height(t_avl_node *ifj_node)
{
  if (ifj_node == NULL)
  {
    return;
  }

  int ifj_left_height = get_height(ifj_node->ifj_left);
  int ifj_right_height = get_height(ifj_node->ifj_right);
  ifj_node->ifj_height = 1 + (ifj_left_height > ifj_right_height ? ifj_left_height : ifj_right_height);
}

// ============================================================================
// AVL rotation functions (required by IAL assignment)
// ============================================================================

/**
 * @brief Perform RR (right-right) rotation
 * @param ifj_node Root of subtree to rotate
 * @return New root after rotation
 */
static t_avl_node *far_right(t_avl_node *ifj_node)
{
  // save right child - it will be new root
  t_avl_node *ifj_y = ifj_node->ifj_right;
  t_avl_node *ifj_T2 = ifj_y->ifj_left;

  // do the rotation
  ifj_y->ifj_left = ifj_node;
  ifj_node->ifj_right = ifj_T2;

  // update heights (important to do node first!)
  update_height(ifj_node);
  update_height(ifj_y);

  return ifj_y;
}

/**
 * @brief Perform LL (left-left) rotation
 * @param ifj_node Root of subtree to rotate
 * @return New root after rotation
 */
static t_avl_node *far_left(t_avl_node *ifj_node)
{
  // mirror of far_right
  t_avl_node *ifj_y = ifj_node->ifj_left;
  t_avl_node *ifj_T3 = ifj_y->ifj_right;

  // rotate
  ifj_y->ifj_right = ifj_node;
  ifj_node->ifj_left = ifj_T3;

  // update heights
  update_height(ifj_node);
  update_height(ifj_y);

  return ifj_y;
}

/**
 * @brief Perform RL (right-left) rotation
 * @param ifj_node Root of subtree to rotate
 * @return New root after rotation
 */
static t_avl_node *right_left(t_avl_node *ifj_node)
{
  // double rotation - first left then right
  ifj_node->ifj_right = far_left(ifj_node->ifj_right);
  return far_right(ifj_node);
}

/**
 * @brief Perform LR (left-right) rotation
 * @param ifj_node Root of subtree to rotate
 * @return New root after rotation
 */
static t_avl_node *left_right(t_avl_node *ifj_node)
{
  // double rotation - first right then left
  ifj_node->ifj_left = far_right(ifj_node->ifj_left);
  return far_left(ifj_node);
}

/**
 * @brief Balance an AVL node after insertion or deletion
 * @param ifj_node Node to balance
 * @return Balanced node (may be different from input)
 */
static t_avl_node *balance_node(t_avl_node *ifj_node)
{
  if (ifj_node == NULL)
  {
    return NULL;
  }

  int ifj_balance = get_balance(ifj_node);

  // Left heavy case
  if (ifj_balance > 1)
  {
    // check if LL or LR
    if (get_balance(ifj_node->ifj_left) >= 0)
    {
      // LL case
      return far_left(ifj_node);
    }
    else
    {
      // LR case
      return left_right(ifj_node);
    }
  }

  // Right heavy case
  if (ifj_balance < -1)
  {
    // check if RR or RL
    if (get_balance(ifj_node->ifj_right) <= 0)
    {
      // RR case
      return far_right(ifj_node);
    }
    else
    {
      // RL case
      return right_left(ifj_node);
    }
  }

  // already balanced
  return ifj_node;
}

// ============================================================================
// Node creation and destruction
// ============================================================================

/**
 * @brief Create a new AVL node
 * @param ifj_key Symbol identifier
 * @param ifj_type Symbol type
 * @return Pointer to newly created node, or NULL on failure
 */
static t_avl_node *create_node(const char *ifj_key, e_symbol_type ifj_type)
{
  // allocate node
  t_avl_node *ifj_new = (t_avl_node *)malloc(sizeof(t_avl_node));
  if (ifj_new == NULL)
  {
    return NULL;
  }

  // allocate and copy key string
  ifj_new->ifj_key = (char *)malloc(strlen(ifj_key) + 1);
  if (ifj_new->ifj_key == NULL)
  {
    free(ifj_new);
    return NULL;
  }
  strcpy(ifj_new->ifj_key, ifj_key);

  // set basic fields
  ifj_new->ifj_sym_type = ifj_type;
  ifj_new->ifj_height = 1; // new nodes are leafs
  ifj_new->ifj_left = NULL;
  ifj_new->ifj_right = NULL;

  // initialize metadata based on symbol type
  if (ifj_type == SYM_VAR_LOCAL || ifj_type == SYM_VAR_GLOBAL)
  {
    ifj_new->ifj_data.ifj_var.ifj_defined = true;
    ifj_new->ifj_data.ifj_var.ifj_initialized = false;
    ifj_new->ifj_data.ifj_var.ifj_scope_level = 0; // will be set later
    ifj_new->ifj_data.ifj_var.ifj_type = TYPE_UNKNOWN;
  }
  else if (ifj_type == SYM_FUNCTION || ifj_type == SYM_GETTER || ifj_type == SYM_SETTER)
  {
    ifj_new->ifj_data.ifj_func.ifj_param_count = 0;
    ifj_new->ifj_data.ifj_func.ifj_return_type = TYPE_UNKNOWN;
    ifj_new->ifj_data.ifj_func.ifj_param_types = NULL;
  }

  return ifj_new;
}

/**
 * @brief Recursively destroy all nodes in a subtree
 * @param ifj_node Root of subtree to destroy
 */
static void destroy_node(t_avl_node *ifj_node)
{
  // Post-order traversal: left, right, then current
  if (ifj_node == NULL)
  {
    return;
  }

  destroy_node(ifj_node->ifj_left);
  destroy_node(ifj_node->ifj_right);

  // Free key string
  free(ifj_node->ifj_key);

  // Free parameter types array if function
  if (ifj_node->ifj_sym_type == SYM_FUNCTION &&
      ifj_node->ifj_data.ifj_func.ifj_param_types != NULL)
  {
    free(ifj_node->ifj_data.ifj_func.ifj_param_types);
  }

  free(ifj_node);
}

// ============================================================================
// Public API implementation
// ============================================================================

void symtable_init(t_symtable *ifj_table)
{
  if (ifj_table == NULL)
  {
    return;
  }

  ifj_table->ifj_root = NULL;
  ifj_table->ifj_current_scope = 0;
  ifj_table->ifj_current_nesting = 0;
  ifj_table->ifj_block_stack_top = 0;
  ifj_table->ifj_next_block_id = 0;
  ifj_table->ifj_function_vars_count = 0;
  for (int i = 0; i < MAX_FUNCTION_VARS; i++)
  {
    ifj_table->ifj_function_vars[i] = NULL;
  }
}

void symtable_destroy(t_symtable *ifj_table)
{
  if (ifj_table == NULL)
  {
    return;
  }

  // Clear function variables
  symtable_clear_function_vars(ifj_table);

  destroy_node(ifj_table->ifj_root);
  ifj_table->ifj_root = NULL;
  ifj_table->ifj_current_scope = 0;
}

/**
 * @brief Recursive insert helper
 * @param ifj_node Current node
 * @param ifj_key Key to insert
 * @param ifj_type Symbol type
 * @return Root of modified subtree
 */
static t_avl_node *insert_recursive(t_avl_node *ifj_node, const char *ifj_key,
                                    e_symbol_type ifj_type)
{
  // base case - create new node
  if (ifj_node == NULL)
  {
    return create_node(ifj_key, ifj_type);
  }

  // standard BST insert
  int ifj_cmp = strcmp(ifj_key, ifj_node->ifj_key);

  if (ifj_cmp < 0)
  {
    // go left
    ifj_node->ifj_left = insert_recursive(ifj_node->ifj_left, ifj_key, ifj_type);
  }
  else if (ifj_cmp > 0)
  {
    // go right
    ifj_node->ifj_right = insert_recursive(ifj_node->ifj_right, ifj_key, ifj_type);
  }
  else
  {
    // duplicate key - shouldn't happen
    return ifj_node;
  }

  // update height after insertion
  update_height(ifj_node);

  // balance the tree
  return balance_node(ifj_node);
}

bool symtable_insert_var(t_symtable *ifj_table, const char *ifj_key,
                         e_symbol_type ifj_type, e_data_type ifj_data_type)
{
  if (ifj_table == NULL || ifj_key == NULL)
  {
    return false;
  }

  // For local variables, create unique key with block ID suffix
  // This allows same variable name in different blocks (even at same nesting level)
  char unique_key[256];
  int current_block_id = 0;
  
  if (ifj_type == SYM_VAR_LOCAL && ifj_table->ifj_block_stack_top > 0)
  {
    // Get current block ID from top of stack
    current_block_id = ifj_table->ifj_block_stack[ifj_table->ifj_block_stack_top - 1];
    snprintf(unique_key, sizeof(unique_key), "%s$b%d", ifj_key, current_block_id);
  }
  else
  {
    // Parameters (no block yet) and global variables use name as-is
    snprintf(unique_key, sizeof(unique_key), "%s", ifj_key);
  }

  // Check if this specific key already exists
  if (symtable_search(ifj_table, unique_key) != NULL)
  {
    return false; // Symbol with this exact key already exists
  }

  // Insert using recursive helper with unique key
  t_avl_node *ifj_new_root = insert_recursive(ifj_table->ifj_root, unique_key, ifj_type);

  if (ifj_new_root == NULL)
  {
    return false; // insertion failed
  }

  ifj_table->ifj_root = ifj_new_root;

  // find the node we just inserted and set metadata
  t_avl_node *ifj_inserted = symtable_search(ifj_table, unique_key);
  if (ifj_inserted != NULL)
  {
    ifj_inserted->ifj_data.ifj_var.ifj_type = ifj_data_type;
    ifj_inserted->ifj_data.ifj_var.ifj_scope_level = ifj_table->ifj_current_scope;
    ifj_inserted->ifj_data.ifj_var.ifj_nesting_level = ifj_table->ifj_current_nesting;
    ifj_inserted->ifj_data.ifj_var.ifj_block_id = current_block_id;
  }

  return true;
}

bool symtable_insert_func(t_symtable *ifj_table, const char *ifj_key,
                          int ifj_param_count, e_data_type *ifj_param_types,
                          e_data_type ifj_return_type, e_symbol_type ifj_sym_type)
{
  if (ifj_table == NULL || ifj_key == NULL)
  {
    return false;
  }

  // check if already exists
  if (symtable_search(ifj_table, ifj_key) != NULL)
  {
    return false;
  }

  // insert function node (SYM_FUNCTION, SYM_GETTER, or SYM_SETTER)
  t_avl_node *ifj_new_root = insert_recursive(ifj_table->ifj_root, ifj_key, ifj_sym_type);

  if (ifj_new_root == NULL)
  {
    return false;
  }

  ifj_table->ifj_root = ifj_new_root;

  // find inserted node and set function metadata
  t_avl_node *ifj_func = symtable_search(ifj_table, ifj_key);
  if (ifj_func != NULL)
  {
    ifj_func->ifj_data.ifj_func.ifj_param_count = ifj_param_count;
    ifj_func->ifj_data.ifj_func.ifj_return_type = ifj_return_type;

    // allocate param types array if needed
    if (ifj_param_count > 0 && ifj_param_types != NULL)
    {
      ifj_func->ifj_data.ifj_func.ifj_param_types =
          (e_data_type *)malloc(sizeof(e_data_type) * ifj_param_count);

      if (ifj_func->ifj_data.ifj_func.ifj_param_types == NULL)
      {
        // memory allocation failed - cleanup
        symtable_delete(ifj_table, ifj_key);
        return false;
      }

      // copy parameter types
      for (int ifj_i = 0; ifj_i < ifj_param_count; ifj_i++)
      {
        ifj_func->ifj_data.ifj_func.ifj_param_types[ifj_i] = ifj_param_types[ifj_i];
      }
    }
  }

  return true;
}

/**
 * @brief Recursive search helper
 * @param ifj_node Current node
 * @param ifj_key Key to search for
 * @return Node if found, NULL otherwise
 */
static t_avl_node *search_recursive(t_avl_node *ifj_node, const char *ifj_key)
{
  if (ifj_node == NULL)
  {
    return NULL;
  }

  int ifj_cmp = strcmp(ifj_key, ifj_node->ifj_key);

  if (ifj_cmp == 0)
  {
    return ifj_node;
  }
  else if (ifj_cmp < 0)
  {
    return search_recursive(ifj_node->ifj_left, ifj_key);
  }
  else
  {
    return search_recursive(ifj_node->ifj_right, ifj_key);
  }
}

t_avl_node *symtable_search(t_symtable *ifj_table, const char *ifj_key)
{
  if (ifj_table == NULL || ifj_key == NULL)
  {
    return NULL;
  }

  return search_recursive(ifj_table->ifj_root, ifj_key);
}

/**
 * @brief Recursive delete helper
 * @param ifj_node Current node
 * @param ifj_key Key to delete
 * @param ifj_deleted Output parameter indicating if deletion occurred
 * @return Root of modified subtree
 */
static t_avl_node *delete_recursive(t_avl_node *ifj_node, const char *ifj_key,
                                    bool *ifj_deleted)
{
  if (ifj_node == NULL)
  {
    *ifj_deleted = false;
    return NULL;
  }

  int ifj_cmp = strcmp(ifj_key, ifj_node->ifj_key);

  if (ifj_cmp < 0)
  {
    // search left
    ifj_node->ifj_left = delete_recursive(ifj_node->ifj_left, ifj_key, ifj_deleted);
  }
  else if (ifj_cmp > 0)
  {
    // search right
    ifj_node->ifj_right = delete_recursive(ifj_node->ifj_right, ifj_key, ifj_deleted);
  }
  else
  {
    // found the node to delete
    *ifj_deleted = true;

    // case 1: no children or one child
    if (ifj_node->ifj_left == NULL || ifj_node->ifj_right == NULL)
    {
      t_avl_node *ifj_temp = ifj_node->ifj_left ? ifj_node->ifj_left : ifj_node->ifj_right;

      // no children
      if (ifj_temp == NULL)
      {
        ifj_temp = ifj_node;
        ifj_node = NULL;
        
        // free deleted node
        free(ifj_temp->ifj_key);
        if (ifj_temp->ifj_sym_type == SYM_FUNCTION &&
            ifj_temp->ifj_data.ifj_func.ifj_param_types != NULL)
        {
          free(ifj_temp->ifj_data.ifj_func.ifj_param_types);
        }
        free(ifj_temp);
      }
      else // one child - musíme použiť pointer swap namiesto shallow copy
      {
        // Uložíme si pointery ktoré musíme uvoľniť
        char *old_key = ifj_node->ifj_key;
        e_data_type *old_param_types = NULL;
        if (ifj_node->ifj_sym_type == SYM_FUNCTION &&
            ifj_node->ifj_data.ifj_func.ifj_param_types != NULL)
        {
          old_param_types = ifj_node->ifj_data.ifj_func.ifj_param_types;
        }
        
        // Skopírujeme všetky skalárne hodnoty
        ifj_node->ifj_key = ifj_temp->ifj_key;
        ifj_node->ifj_sym_type = ifj_temp->ifj_sym_type;
        ifj_node->ifj_data = ifj_temp->ifj_data;
        ifj_node->ifj_height = ifj_temp->ifj_height;
        ifj_node->ifj_left = ifj_temp->ifj_left;
        ifj_node->ifj_right = ifj_temp->ifj_right;
        
        // Uvoľníme staré dáta z ifj_node
        free(old_key);
        if (old_param_types != NULL)
        {
          free(old_param_types);
        }
        
        // Uvoľníme len ifj_temp node (nie jeho dáta, lebo ich teraz vlastní ifj_node)
        free(ifj_temp);
      }
    }
    else
    {
      // case 2: two children - find inorder succesor (min in right subtree)
      t_avl_node *ifj_temp = ifj_node->ifj_right;
      while (ifj_temp->ifj_left != NULL)
      {
        ifj_temp = ifj_temp->ifj_left;
      }

      // copy succesor data to current node
      // Uložíme si starý typ PRED zmenou
      e_symbol_type old_sym_type = ifj_node->ifj_sym_type;
      e_data_type *old_param_types = NULL;
      if (old_sym_type == SYM_FUNCTION &&
          ifj_node->ifj_data.ifj_func.ifj_param_types != NULL) {
        old_param_types = ifj_node->ifj_data.ifj_func.ifj_param_types;
      }
      
      free(ifj_node->ifj_key);
      ifj_node->ifj_key = (char *)malloc(strlen(ifj_temp->ifj_key) + 1);
      strcpy(ifj_node->ifj_key, ifj_temp->ifj_key);
      ifj_node->ifj_sym_type = ifj_temp->ifj_sym_type;
      
      // Deep copy of data - musíme skopírovať aj alokované polia
      if (ifj_temp->ifj_sym_type == SYM_FUNCTION || 
          ifj_temp->ifj_sym_type == SYM_GETTER || 
          ifj_temp->ifj_sym_type == SYM_SETTER) {
        // Uvoľníme staré param_types ak existovali
        if (old_param_types != NULL) {
          free(old_param_types);
        }
        
        ifj_node->ifj_data.ifj_func.ifj_param_count = ifj_temp->ifj_data.ifj_func.ifj_param_count;
        ifj_node->ifj_data.ifj_func.ifj_return_type = ifj_temp->ifj_data.ifj_func.ifj_return_type;
        
        // Deep copy param_types array
        if (ifj_temp->ifj_data.ifj_func.ifj_param_types != NULL) {
          int param_count = ifj_temp->ifj_data.ifj_func.ifj_param_count;
          ifj_node->ifj_data.ifj_func.ifj_param_types = malloc(param_count * sizeof(e_data_type));
          for (int i = 0; i < param_count; i++) {
            ifj_node->ifj_data.ifj_func.ifj_param_types[i] = ifj_temp->ifj_data.ifj_func.ifj_param_types[i];
          }
        } else {
          ifj_node->ifj_data.ifj_func.ifj_param_types = NULL;
        }
      } else {
        // Pre premenné len skopírujeme union (shallow copy je OK)
        ifj_node->ifj_data = ifj_temp->ifj_data;
      }

      // delete the succesor
      bool ifj_temp_deleted;
      ifj_node->ifj_right = delete_recursive(ifj_node->ifj_right, ifj_temp->ifj_key, &ifj_temp_deleted);
    }
  }

  // if tree had only one node
  if (ifj_node == NULL)
  {
    return NULL;
  }

  // update height
  update_height(ifj_node);

  // balance tree
  return balance_node(ifj_node);
}

bool symtable_delete(t_symtable *ifj_table, const char *ifj_key)
{
  if (ifj_table == NULL || ifj_key == NULL)
  {
    return false;
  }

  bool ifj_deleted = false;
  ifj_table->ifj_root = delete_recursive(ifj_table->ifj_root, ifj_key, &ifj_deleted);
  return ifj_deleted;
}

void symtable_enter_scope(t_symtable *ifj_table)
{
  if (ifj_table == NULL)
  {
    return;
  }

  ifj_table->ifj_current_scope++;
}

// helper function to collect keys from specific scope
static void collect_scope_keys(t_avl_node *ifj_node, int ifj_scope,
                               char **ifj_keys, int *ifj_count, int ifj_max)
{
  if (ifj_node == NULL || *ifj_count >= ifj_max)
  {
    return;
  }

  // traverse left
  collect_scope_keys(ifj_node->ifj_left, ifj_scope, ifj_keys, ifj_count, ifj_max);

  // check if current node is from this scope
  if ((ifj_node->ifj_sym_type == SYM_VAR_LOCAL ||
       ifj_node->ifj_sym_type == SYM_VAR_GLOBAL) &&
      ifj_node->ifj_data.ifj_var.ifj_scope_level == ifj_scope)
  {
    // IMPORTANT: Copy the key string, don't just store pointer!
    // The tree will be modified during deletion, which may change key pointers
    ifj_keys[*ifj_count] = malloc(strlen(ifj_node->ifj_key) + 1);
    if (ifj_keys[*ifj_count] != NULL) {
      strcpy(ifj_keys[*ifj_count], ifj_node->ifj_key);
      (*ifj_count)++;
    }
  }

  // traverse right
  collect_scope_keys(ifj_node->ifj_right, ifj_scope, ifj_keys, ifj_count, ifj_max);
}

void symtable_exit_scope(t_symtable *ifj_table)
{
  if (ifj_table == NULL || ifj_table->ifj_current_scope == 0)
  {
    return;
  }

  int ifj_old_scope = ifj_table->ifj_current_scope;
  ifj_table->ifj_current_scope--;

  // collect all keys from exiting scope
  char *ifj_to_delete[1000]; // should be enough for most cases
  int ifj_delete_count = 0;

  collect_scope_keys(ifj_table->ifj_root, ifj_old_scope,
                     ifj_to_delete, &ifj_delete_count, 1000);

  // delete collected keys
  for (int ifj_i = 0; ifj_i < ifj_delete_count; ifj_i++)
  {
    symtable_delete(ifj_table, ifj_to_delete[ifj_i]);
    
    // Free the copied key string
    free(ifj_to_delete[ifj_i]);
  }
}

int symtable_get_scope(t_symtable *ifj_table)
{
  if (ifj_table == NULL)
  {
    return -1;
  }

  return ifj_table->ifj_current_scope;
}

void symtable_enter_nesting(t_symtable *ifj_table)
{
  if (ifj_table == NULL)
  {
    return;
  }

  ifj_table->ifj_current_nesting++;
  
  // Push new block ID onto stack
  if (ifj_table->ifj_block_stack_top < MAX_NESTING_DEPTH)
  {
    int new_block_id = ifj_table->ifj_next_block_id++;
    ifj_table->ifj_block_stack[ifj_table->ifj_block_stack_top++] = new_block_id;
  }
}

// Helper to collect keys from specific block
static void collect_nesting_keys(t_avl_node *ifj_node, int ifj_scope, int ifj_block_id,
                                  char **ifj_keys, int *ifj_count, int ifj_max)
{
  if (ifj_node == NULL || *ifj_count >= ifj_max)
  {
    return;
  }

  // traverse left
  collect_nesting_keys(ifj_node->ifj_left, ifj_scope, ifj_block_id, ifj_keys, ifj_count, ifj_max);

  // check if current node is from this scope and block
  if ((ifj_node->ifj_sym_type == SYM_VAR_LOCAL) &&
      ifj_node->ifj_data.ifj_var.ifj_scope_level == ifj_scope &&
      ifj_node->ifj_data.ifj_var.ifj_block_id == ifj_block_id)
  {
    // Copy the key string
    ifj_keys[*ifj_count] = malloc(strlen(ifj_node->ifj_key) + 1);
    if (ifj_keys[*ifj_count] != NULL) {
      strcpy(ifj_keys[*ifj_count], ifj_node->ifj_key);
      (*ifj_count)++;
    }
  }

  // traverse right
  collect_nesting_keys(ifj_node->ifj_right, ifj_scope, ifj_block_id, ifj_keys, ifj_count, ifj_max);
}

void symtable_exit_nesting(t_symtable *ifj_table)
{
  if (ifj_table == NULL || ifj_table->ifj_current_nesting == 0)
  {
    return;
  }

  ifj_table->ifj_current_nesting--;
  
  // Pop block ID from stack
  if (ifj_table->ifj_block_stack_top > 0)
  {
    int exiting_block_id = ifj_table->ifj_block_stack[--ifj_table->ifj_block_stack_top];
    int ifj_current_scope = ifj_table->ifj_current_scope;

    // collect all keys from exiting block
    char *ifj_to_delete[1000];
    int ifj_delete_count = 0;

    collect_nesting_keys(ifj_table->ifj_root, ifj_current_scope, exiting_block_id,
                         ifj_to_delete, &ifj_delete_count, 1000);

    // delete collected keys
    for (int ifj_i = 0; ifj_i < ifj_delete_count; ifj_i++)
    {
      symtable_delete(ifj_table, ifj_to_delete[ifj_i]);
      free(ifj_to_delete[ifj_i]);
    }
  }
}

// Helper to check if key matches base name (with or without $bX suffix)
static bool key_matches_base(const char *ifj_key, const char *ifj_base_key)
{
  size_t base_len = strlen(ifj_base_key);
  
  // Check if ifj_key starts with ifj_base_key
  if (strncmp(ifj_key, ifj_base_key, base_len) != 0)
  {
    return false;
  }
  
  // Check if after base_key there's either end of string or $bX suffix
  if (ifj_key[base_len] == '\0' || strncmp(&ifj_key[base_len], "$b", 2) == 0)
  {
    return true;
  }
  
  return false;
}

// Helper to find variable visible from current block stack
// Searches for variable with matching base name in current block or any parent block
static void find_visible_var(t_avl_node *ifj_node, const char *ifj_base_key,
                             int *ifj_block_stack, int ifj_stack_size,
                             t_avl_node **ifj_best, int *ifj_best_stack_pos)
{
  if (ifj_node == NULL)
  {
    return;
  }

  // Search entire tree
  find_visible_var(ifj_node->ifj_left, ifj_base_key, ifj_block_stack, ifj_stack_size, ifj_best, ifj_best_stack_pos);
  
  // Check if current node matches base key
  if (ifj_node->ifj_sym_type == SYM_VAR_LOCAL && key_matches_base(ifj_node->ifj_key, ifj_base_key))
  {
    // Parameters (block_id=0) are always visible
    if (ifj_node->ifj_data.ifj_var.ifj_block_id == 0)
    {
      // Parameter - always visible, but prefer variables from nested blocks
      if (*ifj_best_stack_pos < 0)
      {
        *ifj_best = ifj_node;
        *ifj_best_stack_pos = -1; // Mark as parameter (lower priority than any block)
      }
    }
    else
    {
      // Check if this variable's block is in our stack (i.e., it's visible)
      // Iterate from newest (top) to oldest (bottom)
      for (int i = ifj_stack_size - 1; i >= 0; i--)
      {
        if (ifj_node->ifj_data.ifj_var.ifj_block_id == ifj_block_stack[i])
        {
          // Found a variable in a visible block
          // Take first match (newest/most nested block wins)
          if (i >= *ifj_best_stack_pos)
          {
            *ifj_best = ifj_node;
            *ifj_best_stack_pos = i;
          }
          break;
        }
      }
    }
  }
  
  find_visible_var(ifj_node->ifj_right, ifj_base_key, ifj_block_stack, ifj_stack_size, ifj_best, ifj_best_stack_pos);
}

t_avl_node *symtable_search_var_scoped(t_symtable *ifj_table, const char *ifj_key)
{
  if (ifj_table == NULL || ifj_key == NULL)
  {
    return NULL;
  }

  t_avl_node *ifj_best = NULL;
  int ifj_best_stack_pos = -1;

  // Search for variable visible from current block stack
  find_visible_var(ifj_table->ifj_root, ifj_key, 
                   ifj_table->ifj_block_stack, ifj_table->ifj_block_stack_top,
                   &ifj_best, &ifj_best_stack_pos);

  return ifj_best;
}

// Helper to find variable at specific scope and block
static void find_var_at_block(t_avl_node *ifj_node, const char *ifj_base_key,
                              int ifj_scope, int ifj_block_id, t_avl_node **ifj_result)
{
  if (ifj_node == NULL || *ifj_result != NULL)
  {
    return;
  }

  // Search entire tree since keys now have $bX suffixes
  find_var_at_block(ifj_node->ifj_left, ifj_base_key, ifj_scope, ifj_block_id, ifj_result);

  // Check if this node's key matches the base name pattern
  if (key_matches_base(ifj_node->ifj_key, ifj_base_key) &&
      ifj_node->ifj_sym_type == SYM_VAR_LOCAL &&
      ifj_node->ifj_data.ifj_var.ifj_scope_level == ifj_scope &&
      ifj_node->ifj_data.ifj_var.ifj_block_id == ifj_block_id)
  {
    *ifj_result = ifj_node;
    return;
  }

  find_var_at_block(ifj_node->ifj_right, ifj_base_key, ifj_scope, ifj_block_id, ifj_result);
}

t_avl_node *symtable_search_var_current_nesting(t_symtable *ifj_table, const char *ifj_key)
{
  if (ifj_table == NULL || ifj_key == NULL)
  {
    return NULL;
  }

  t_avl_node *ifj_result = NULL;
  int ifj_scope = ifj_table->ifj_current_scope;
  
  // Get current block ID from top of stack (or 0 if no block)
  int ifj_block_id = 0;
  if (ifj_table->ifj_block_stack_top > 0)
  {
    ifj_block_id = ifj_table->ifj_block_stack[ifj_table->ifj_block_stack_top - 1];
  }

  find_var_at_block(ifj_table->ifj_root, ifj_key, ifj_scope, ifj_block_id, &ifj_result);

  return ifj_result;
}

void symtable_start_function(t_symtable *ifj_table)
{
  if (ifj_table == NULL)
  {
    return;
  }
  
  // Clear any previous function variables
  symtable_clear_function_vars(ifj_table);
}

void symtable_add_function_var(t_symtable *ifj_table, const char *var_name_with_suffix)
{
  if (ifj_table == NULL || var_name_with_suffix == NULL)
  {
    return;
  }
  
  if (ifj_table->ifj_function_vars_count >= MAX_FUNCTION_VARS)
  {
    // Error: too many variables
    return;
  }
  
  // Check if variable is already in the list (avoid duplicates)
  for (int i = 0; i < ifj_table->ifj_function_vars_count; i++)
  {
    if (ifj_table->ifj_function_vars[i] != NULL &&
        strcmp(ifj_table->ifj_function_vars[i], var_name_with_suffix) == 0)
    {
      // Already added
      return;
    }
  }
  
  // Add new variable
  ifj_table->ifj_function_vars[ifj_table->ifj_function_vars_count] = strdup(var_name_with_suffix);
  ifj_table->ifj_function_vars_count++;
}

void symtable_clear_function_vars(t_symtable *ifj_table)
{
  if (ifj_table == NULL)
  {
    return;
  }
  
  // Free all stored variable names
  for (int i = 0; i < ifj_table->ifj_function_vars_count; i++)
  {
    if (ifj_table->ifj_function_vars[i] != NULL)
    {
      free(ifj_table->ifj_function_vars[i]);
      ifj_table->ifj_function_vars[i] = NULL;
    }
  }
  
  ifj_table->ifj_function_vars_count = 0;
}

bool symtable_is_global_var(const char *ifj_key)
{
  if (ifj_key == NULL || strlen(ifj_key) < 2)
  {
    return false;
  }
  
  return (ifj_key[0] == '_' && ifj_key[1] == '_');
}

bool symtable_insert_global_var(t_symtable *ifj_table, const char *ifj_key, e_data_type ifj_data_type)
{
  if (ifj_table == NULL || ifj_key == NULL)
  {
    return false;
  }
  
  // Check if already exists
  t_avl_node *existing = symtable_search(ifj_table, ifj_key);
  if (existing != NULL)
  {
    // Already exists - update type if needed
    existing->ifj_data.ifj_var.ifj_type = ifj_data_type;
    return true;
  }
  
  // Insert new global variable
  t_avl_node *ifj_new_root = insert_recursive(ifj_table->ifj_root, ifj_key, SYM_VAR_GLOBAL);
  
  if (ifj_new_root == NULL)
  {
    return false;
  }
  
  ifj_table->ifj_root = ifj_new_root;
  
  // Set metadata for global variable
  t_avl_node *ifj_inserted = symtable_search(ifj_table, ifj_key);
  if (ifj_inserted != NULL)
  {
    ifj_inserted->ifj_data.ifj_var.ifj_type = ifj_data_type;
    ifj_inserted->ifj_data.ifj_var.ifj_defined = true;
    ifj_inserted->ifj_data.ifj_var.ifj_initialized = false;
    ifj_inserted->ifj_data.ifj_var.ifj_defvar_generated = false;
    ifj_inserted->ifj_data.ifj_var.ifj_scope_level = 0; // global scope
    ifj_inserted->ifj_data.ifj_var.ifj_nesting_level = 0;
    ifj_inserted->ifj_data.ifj_var.ifj_block_id = 0;
  }
  
  return true;
}

void symtable_mark_global_initialized(t_symtable *ifj_table, const char *ifj_key)
{
  if (ifj_table == NULL || ifj_key == NULL)
  {
    return;
  }
  
  t_avl_node *node = symtable_search(ifj_table, ifj_key);
  if (node != NULL && node->ifj_sym_type == SYM_VAR_GLOBAL)
  {
    node->ifj_data.ifj_var.ifj_initialized = true;
  }
}
