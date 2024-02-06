/**
 * Binary tree data structure problems .
 * Copyright (C) 2024 Lazar Razvan.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tree/binary_tree_problems.h"

/*****************************************************************************/

#define ARRAY_SIZE(array) 		(sizeof(array) / sizeof(*array))


/*****************************************************************************/

static void binary_tree_by_travelsals_test(void)
{
	int rv = 0;
	binary_tree_node *root = NULL;

	//
	printf("\n\nRunning %s ...\n\n", __func__);

	/****************************************************************
	 * Create a binary tree using in order traversal and level
	 * traversal
	 ***************************************************************/
	int in[] = {4, 8, 10, 12, 14, 20, 22};
	int lvl[] = {20, 8, 22, 4, 12, 10, 14};

	root = binary_tree_by_traversals(in, ARRAY_SIZE(in), lvl, ARRAY_SIZE(lvl));
	if (!root) {
		rv = -1; goto tree_destroy;
	}

	/****************************************************************
	 * Levelorder print.
	 ***************************************************************/
	printf("Levelorder:\n");
	binary_tree_level_order_print(root);
	printf("\n");

tree_destroy:
	binary_tree_node_destroy(root);

	if (rv)
		printf("[%s] Error: %d\n", __func__, rv);
}


/*****************************************************************************/

static void binary_tree_is_sum_tree_test(void)
{
	int rv = 0;
	binary_tree_node *node = NULL;
	binary_tree_node *root = NULL;

	//
	printf("\n\nRunning %s ...\n\n", __func__);

	/****************************************************************
	 * Create a binary tree
	 *      26
	 *    /   \
	 *   10     3
	 *  / \    / \
	 * 4   6  1   2
	 ***************************************************************/
	root = binary_tree_insert(NULL, 26);
	if (!root) {
		rv = -1; goto tree_destroy;
	}

	int keys[] = {10, 3, 4, 6, 1, 2};
	for (int i = 0; i < ARRAY_SIZE(keys); i++) {
		node = binary_tree_insert(root, keys[i]);
		if (!node) {
			rv = -2; goto tree_destroy;
		}
	}

	/****************************************************************
	 * Levelorder print.
	 ***************************************************************/
	printf("Levelorder:\n");
	binary_tree_level_order_print(root);
	printf("\n");

	/****************************************************************
	 * Check if sum tree
	 ***************************************************************/
	printf("Sumtree: %s\n", binary_tree_is_sum_tree(root) ? "true" : "false");

	/****************************************************************
	 * Remove root element
	 ***************************************************************/
	printf("Remove root...\n");
	root = binary_tree_delete(root, 26);

	/****************************************************************
	 * Levelorder print.
	 ***************************************************************/
	printf("Levelorder:\n");
	binary_tree_level_order_print(root);
	printf("\n");

	/****************************************************************
	 * Check if sum tree
	 ***************************************************************/
	printf("Sumtree: %s\n", binary_tree_is_sum_tree(root) ? "true" : "false");

tree_destroy:
	binary_tree_node_destroy(root);

	if (rv)
		printf("[%s] Error: %d\n", __func__, rv);
}


/*****************************************************************************/

static void binary_tree_cousins_test(void)
{
	int rv = 0;
	binary_tree_node *root = NULL;
	binary_tree_node *node = NULL;

	//
	printf("\n\nRunning %s ...\n\n", __func__);

	/****************************************************************
	 * Create a binary tree
	 *      26
	 *    /   \
	 *   10     3
	 *  / \    / \
	 * 4   6  1   2
	 ***************************************************************/
	root = binary_tree_insert(NULL, 26);
	if (!root) {
		rv = -1; goto tree_destroy;
	}

	int keys[] = {10, 3, 4, 6, 1, 2};
	for (int i = 0; i < ARRAY_SIZE(keys); i++) {
		node = binary_tree_insert(root, keys[i]);
		if (!node) {
			rv = -2; goto tree_destroy;
		}
	}

	/****************************************************************
	 * Levelorder print.
	 ***************************************************************/
	printf("Levelorder:\n");
	binary_tree_level_order_print(root);
	printf("\n");

	/****************************************************************
	 * Check cousins.
	 ***************************************************************/
	printf("Cousins(%d; %d): %s\n", 26, 10,
			binary_tree_cousins(root, 26, 10) ? "yes" : "no");
	//
	printf("Cousins(%d; %d): %s\n", 10, 2,
			binary_tree_cousins(root, 10, 2) ? "yes" : "no");
	//
	printf("Cousins(%d; %d): %s\n", 10, 6,
			binary_tree_cousins(root, 10, 6) ? "yes" : "no");
	//
	printf("Cousins(%d; %d): %s\n", 4, 6,
			binary_tree_cousins(root, 4, 6) ? "yes" : "no");
	//
	printf("Cousins(%d; %d): %s\n", 4, 1,
			binary_tree_cousins(root, 4, 1) ? "yes" : "no");
	//
	printf("Cousins(%d; %d): %s\n", 4, 2,
			binary_tree_cousins(root, 4, 2) ? "yes" : "no");
	//
	printf("Cousins(%d; %d): %s\n", 6, 1,
			binary_tree_cousins(root, 6, 1) ? "yes" : "no");
	//
	printf("Cousins(%d; %d): %s\n", 6, 2,
			binary_tree_cousins(root, 6, 2) ? "yes" : "no");
	//
	printf("Cousins(%d; %d): %s\n", 1, 2,
			binary_tree_cousins(root, 1, 2) ? "yes" : "no");

tree_destroy:
	binary_tree_node_destroy(root);

	if (rv)
		printf("[%s] Error: %d\n", __func__, rv);
}


/*****************************************************************************/

static void binary_tree_perfect_test(void)
{
	int rv = 0;
	binary_tree_node *root = NULL;
	binary_tree_node *node = NULL;

	//
	printf("\n\nRunning %s ...\n\n", __func__);

	/****************************************************************
	 * Create a binary tree
	 *      26
	 *    /   \
	 *   10     3
	 *  / \    /
	 * 4   6  1
	 ***************************************************************/
	root = binary_tree_insert(NULL, 26);
	if (!root) {
		rv = -1; goto tree_destroy;
	}

	int keys[] = {10, 3, 4, 6, 1};
	for (int i = 0; i < ARRAY_SIZE(keys); i++) {
		node = binary_tree_insert(root, keys[i]);
		if (!node) {
			rv = -2; goto tree_destroy;
		}
	}

	/****************************************************************
	 * Levelorder print.
	 ***************************************************************/
	printf("Levelorder:\n");
	binary_tree_level_order_print(root);
	printf("\n");

	/****************************************************************
	 * Perfect tree.
	 ***************************************************************/
	printf("Perfect tree: %s\n", binary_tree_is_perfect(root) ? "yes" : "no");

	/****************************************************************
	 * Updatebinary tree
	 *      26
	 *    /   \
	 *   10     3
	 *  / \    / \
	 * 4   6  1   2
	 ***************************************************************/
	node = binary_tree_insert(root, 2);
	if (!node) {
		rv = -3; goto tree_destroy;
	}

	/****************************************************************
	 * Levelorder print.
	 ***************************************************************/
	printf("Levelorder:\n");
	binary_tree_level_order_print(root);
	printf("\n");

	/****************************************************************
	 * Perfect tree.
	 ***************************************************************/
	printf("Perfect tree: %s\n", binary_tree_is_perfect(root) ? "yes" : "no");

tree_destroy:
	binary_tree_node_destroy(root);

	if (rv)
		printf("[%s] Error: %d\n", __func__, rv);
}


/*****************************************************************************/


int main()
{
	//
	binary_tree_by_travelsals_test();
	//
	binary_tree_is_sum_tree_test();
	//
	binary_tree_cousins_test();
	//
	binary_tree_perfect_test();

	return 0;
}
