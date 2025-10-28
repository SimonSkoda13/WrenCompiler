/**
 * @file symtable_test.c
 * @brief Test suite for symbol table implementation
 *
 * @author Šimon Škoda
 * @date 2025-10-22
 */

#include "symtable.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

// Test counter
static int ifj_tests_passed = 0;
static int ifj_tests_failed = 0;

// Macro for test assertions
#define TEST_ASSERT(condition, message) \
  do                                    \
  {                                     \
    if (condition)                      \
    {                                   \
      printf("  ✓ %s\n", message);      \
      ifj_tests_passed++;               \
    }                                   \
    else                                \
    {                                   \
      printf("  ✗ %s\n", message);      \
      ifj_tests_failed++;               \
    }                                   \
  } while (0)

/**
 * @brief Test basic initialization and destruction
 */
void test_init_destroy(void)
{
  printf("\n=== Test 1: Init & Destroy ===\n");

  t_symtable ifj_table;
  symtable_init(&ifj_table);

  TEST_ASSERT(ifj_table.ifj_root == NULL, "Table root is NULL after init");
  TEST_ASSERT(ifj_table.ifj_current_scope == 0, "Initial scope is 0");

  symtable_destroy(&ifj_table);
  TEST_ASSERT(ifj_table.ifj_root == NULL, "Table root is NULL after destroy");
}

/**
 * @brief Test basic insert and search operations
 */
void test_insert_search(void)
{
  printf("\n=== Test 2: Insert & Search ===\n");

  t_symtable ifj_table;
  symtable_init(&ifj_table);

  // Insert a local variable
  bool ifj_result = symtable_insert_var(&ifj_table, "ifj_x", SYM_VAR_LOCAL, TYPE_NUM);
  TEST_ASSERT(ifj_result == true, "Insert local variable 'ifj_x'");

  // Search for the variable
  t_avl_node *ifj_node = symtable_search(&ifj_table, "ifj_x");
  TEST_ASSERT(ifj_node != NULL, "Search finds 'ifj_x'");
  TEST_ASSERT(ifj_node != NULL && strcmp(ifj_node->ifj_key, "ifj_x") == 0,
              "Found node has correct key");
  TEST_ASSERT(ifj_node != NULL && ifj_node->ifj_sym_type == SYM_VAR_LOCAL,
              "Found node has correct type");

  // Search for non-existent variable
  t_avl_node *ifj_missing = symtable_search(&ifj_table, "ifj_y");
  TEST_ASSERT(ifj_missing == NULL, "Search for non-existent variable returns NULL");

  // Try to insert duplicate
  bool ifj_duplicate = symtable_insert_var(&ifj_table, "ifj_x", SYM_VAR_LOCAL, TYPE_NUM);
  TEST_ASSERT(ifj_duplicate == false, "Cannot insert duplicate symbol");

  symtable_destroy(&ifj_table);
}

/**
 * @brief Test multiple insertions and AVL balancing
 */
void test_multiple_insert(void)
{
  printf("\n=== Test 3: Multiple Insertions ===\n");

  t_symtable ifj_table;
  symtable_init(&ifj_table);

  // Insert multiple variables to trigger rotations
  const char *ifj_vars[] = {"ifj_a", "ifj_b", "ifj_c", "ifj_d", "ifj_e", "ifj_f"};
  int ifj_count = sizeof(ifj_vars) / sizeof(ifj_vars[0]);

  for (int ifj_i = 0; ifj_i < ifj_count; ifj_i++)
  {
    bool ifj_result = symtable_insert_var(&ifj_table, ifj_vars[ifj_i],
                                          SYM_VAR_LOCAL, TYPE_NUM);
    TEST_ASSERT(ifj_result == true, "Insert variable");
  }

  // Search for all variables
  for (int ifj_i = 0; ifj_i < ifj_count; ifj_i++)
  {
    t_avl_node *ifj_node = symtable_search(&ifj_table, ifj_vars[ifj_i]);
    TEST_ASSERT(ifj_node != NULL, "Search finds inserted variable");
  }

  symtable_destroy(&ifj_table);
}

/**
 * @brief Test scope management
 */
void test_scope_management(void)
{
  printf("\n=== Test 4: Scope Management ===\n");

  t_symtable ifj_table;
  symtable_init(&ifj_table);

  TEST_ASSERT(symtable_get_scope(&ifj_table) == 0, "Initial scope is 0");

  symtable_enter_scope(&ifj_table);
  TEST_ASSERT(symtable_get_scope(&ifj_table) == 1, "Scope is 1 after enter");

  symtable_enter_scope(&ifj_table);
  TEST_ASSERT(symtable_get_scope(&ifj_table) == 2, "Scope is 2 after second enter");

  symtable_exit_scope(&ifj_table);
  TEST_ASSERT(symtable_get_scope(&ifj_table) == 1, "Scope is 1 after exit");

  symtable_exit_scope(&ifj_table);
  TEST_ASSERT(symtable_get_scope(&ifj_table) == 0, "Scope is 0 after second exit");

  symtable_destroy(&ifj_table);
}

/**
 * @brief Test function insertion
 */
void test_function_insert(void)
{
  printf("\n=== Test 5: Function Insertion ===\n");

  t_symtable ifj_table;
  symtable_init(&ifj_table);

  // Create parameter types array
  e_data_type ifj_params[] = {TYPE_NUM, TYPE_STRING};

  // Insert a function
  bool ifj_result = symtable_insert_func(&ifj_table, "ifj_foo", 2, ifj_params, TYPE_NUM);
  TEST_ASSERT(ifj_result == true, "Insert function 'ifj_foo'");

  // Search for the function
  t_avl_node *ifj_node = symtable_search(&ifj_table, "ifj_foo");
  TEST_ASSERT(ifj_node != NULL, "Search finds function 'ifj_foo'");
  TEST_ASSERT(ifj_node != NULL && ifj_node->ifj_sym_type == SYM_FUNCTION,
              "Found node is a function");

  symtable_destroy(&ifj_table);
}

/**
 * @brief Main test runner
 */
int main(void)
{
  printf("╔════════════════════════════════════════╗\n");
  printf("║   Symbol Table Test Suite (IFJ25)     ║\n");
  printf("╚════════════════════════════════════════╝\n");

  test_init_destroy();
  test_insert_search();
  test_multiple_insert();
  test_scope_management();
  test_function_insert();

  printf("\n╔════════════════════════════════════════╗\n");
  printf("║            Test Results                ║\n");
  printf("╠════════════════════════════════════════╣\n");
  printf("║  Passed: %-4d                         ║\n", ifj_tests_passed);
  printf("║  Failed: %-4d                         ║\n", ifj_tests_failed);
  printf("╚════════════════════════════════════════╝\n");

  return ifj_tests_failed > 0 ? 1 : 0;
}
