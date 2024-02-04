/**
 * Binary tree data structure test.
 * Copyright (C) 2024 Lazar Razvan.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tree/binary_tree.h"

/*****************************************************************************/

static void basic_test(void)
{
	int rv = 0;
	binary_tree_node *root = NULL;

	//
	printf("\n\nRunning %s ...\n\n", __func__);

	/****************************************************************
	 * Recursive depth.
	 ***************************************************************/
	printf("Recursive depth empty: %d\n", binary_tree_depth_recursive(root));

	/****************************************************************
	 * Iterative depth.
	 ***************************************************************/
	printf("Iterative depth empty: %d\n", binary_tree_depth_iterative(root));


	/****************************************************************
	 * Create binary tree.
            1
        /      \
       2        3
      /  \     / \
     4   5    6  7
	 			 \
				  8
				  \
				   9
	 ***************************************************************/
	//
	root = binary_tree_node_create(1);
	if (!root) {
		rv = -1; goto tree_destroy;
	}

	//
	root->left = binary_tree_node_create(2);
	if (!root->left) {
		rv = -2; goto tree_destroy;
	}

	//
	root->right = binary_tree_node_create(3);
	if (!root->right) {
		rv = -3; goto tree_destroy;
	}

	//
	root->left->left = binary_tree_node_create(4);
	if (!root->left->left) {
		rv = -4; goto tree_destroy;
	}

	//
	root->left->right = binary_tree_node_create(5);
	if (!root->left->right) {
		rv = -5; goto tree_destroy;
	}

	//
	root->right->left = binary_tree_node_create(6);
	if (!root->right->left) {
		rv = -6; goto tree_destroy;
	}

	//
	root->right->right = binary_tree_node_create(7);
	if (!root->right->right) {
		rv = -7; goto tree_destroy;
	}

	//
	root->right->right->right = binary_tree_node_create(8);
	if (!root->right->right->right) {
		rv = -8; goto tree_destroy;
	}

	//
	root->right->right->right->right = binary_tree_node_create(9);
	if (!root->right->right->right->right) {
		rv = -9; goto tree_destroy;
	}

	/****************************************************************
	 * Preorder print.
	 ***************************************************************/
	printf("Preorder:\n");
	binary_tree_pre_order_print(root);
	printf("\n");

	/****************************************************************
	 * Inorder print.
	 ***************************************************************/
	printf("Inorder:\n");
	binary_tree_in_order_print(root);
	printf("\n");

	/****************************************************************
	 * Postorder print.
	 ***************************************************************/
	printf("Postorder:\n");
	binary_tree_post_order_print(root);
	printf("\n");

	/****************************************************************
	 * Levelorder print.
	 ***************************************************************/
	printf("Levelorder:\n");
	binary_tree_level_order_print(root);
	printf("\n");

	/****************************************************************
	 * Recursive depth.
	 ***************************************************************/
	printf("Recursive depth: %d\n", binary_tree_depth_recursive(root));

	/****************************************************************
	 * Iterative depth.
	 ***************************************************************/
	printf("Iterative depth: %d\n", binary_tree_depth_iterative(root));


tree_destroy:
	binary_tree_node_destroy(root);

	if (rv)
		printf("[%s] Error: %d\n", __func__, rv);
}


/*****************************************************************************/

static void insert_test(void)
{
	int rv = 0;
	binary_tree_node *root = NULL;
	binary_tree_node *node = NULL;

	//
	printf("\n\nRunning %s ...\n\n", __func__);

	/****************************************************************
	 * Recursive depth.
	 ***************************************************************/
	printf("Recursive depth empty: %d\n", binary_tree_depth_recursive(root));

	/****************************************************************
	 * Iterative depth.
	 ***************************************************************/
	printf("Iterative depth empty: %d\n", binary_tree_depth_iterative(root));


	/****************************************************************
	 * Create binary tree using insert method.
            1
        /      \
       2        3
      /  \     / \
     4   5    6  7
	 /\
	 8 9
	 ***************************************************************/
	//
	root = binary_tree_insert(root, 1);
	if (!root) {
		rv = -1; goto tree_destroy;
	}

	for (int i = 2; i <= 9; i++) {
		node = binary_tree_insert(root, i);
		if (!node) {
			rv = -2; goto tree_destroy;
		}
	}

	/****************************************************************
	 * Preorder print.
	 ***************************************************************/
	printf("Preorder:\n");
	binary_tree_pre_order_print(root);
	printf("\n");

	/****************************************************************
	 * Inorder print.
	 ***************************************************************/
	printf("Inorder:\n");
	binary_tree_in_order_print(root);
	printf("\n");

	/****************************************************************
	 * Postorder print.
	 ***************************************************************/
	printf("Postorder:\n");
	binary_tree_post_order_print(root);
	printf("\n");

	/****************************************************************
	 * Levelorder print.
	 ***************************************************************/
	printf("Levelorder:\n");
	binary_tree_level_order_print(root);
	printf("\n");

	/****************************************************************
	 * Levelorder spiral print.
	 ***************************************************************/
	printf("Levelorder spiral:\n");
	binary_tree_level_order_spiral_print(root);
	printf("\n");

	/****************************************************************
	 * Recursive depth.
	 ***************************************************************/
	printf("Recursive depth: %d\n", binary_tree_depth_recursive(root));

	/****************************************************************
	 * Iterative depth.
	 ***************************************************************/
	printf("Iterative depth: %d\n", binary_tree_depth_iterative(root));


tree_destroy:
	binary_tree_node_destroy(root);

	if (rv)
		printf("[%s] Error: %d\n", __func__, rv);
}


/*****************************************************************************/

static void insert_delete_test(void)
{
	int rv = 0;
	binary_tree_node *root = NULL;
	binary_tree_node *node = NULL;

	//
	printf("\n\nRunning %s ...\n\n", __func__);

	/****************************************************************
	 * Create binary tree using insert method.
            1
	 ***************************************************************/
	root = binary_tree_insert(root, 1);
	if (!root) {
		rv = -1; goto tree_destroy;
	}

	// Print level order
	printf("Levelorder:\n");
	binary_tree_level_order_print(root);
	printf("\n");

	// Remove element not in tree
	printf("Trying to remove node 5:\n");
	root = binary_tree_delete(root, 5);

	// Print level order
	printf("Levelorder:\n");
	binary_tree_level_order_print(root);
	printf("\n");

	// Remove element in tree (root)
	printf("Trying to remove node 1:\n");
	root = binary_tree_delete(root, 1);

	// Print level order
	printf("Levelorder:\n");
	binary_tree_level_order_print(root);
	printf("\n");

	/****************************************************************
	 * Create binary tree using insert method.
            1
        /      \
       2        3
      /  \     / \
     4   5    6  7
	 /\
	 8 9
	 ***************************************************************/
	//

	root = binary_tree_insert(root, 1);
	if (!root) {
		rv = -1; goto tree_destroy;
	}

	for (int i = 2; i <= 9; i++) {
		node = binary_tree_insert(root, i);
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
	 * Delete node 2.
            1
        /      \
       9        3
      /  \     / \
     4   5    6  7
	 /
	 8
	 ***************************************************************/
	printf("Trying to remove node 2:\n");
	root = binary_tree_delete(root, 2);

	/****************************************************************
	 * Levelorder print.
	 ***************************************************************/
	printf("Levelorder:\n");
	binary_tree_level_order_print(root);
	printf("\n");

	/****************************************************************
	 * Delete node 1.
            8
        /      \
       9        3
      /  \     / \
     4   5    6  7
	 ***************************************************************/
	printf("Trying to remove node 1:\n");
	root = binary_tree_delete(root, 1);

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

int main()
{
	//
	basic_test();

	//
	insert_test();

	//
	insert_delete_test();

	return 0;
}
