/**
 * Binary tree data structure test.
 * Copyright (C) 2024 Lazar Razvan.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "binary_tree/binary_tree.h"

/*****************************************************************************/

static void basic_test(void)
{
	int rv = 0;
	binary_tree_node *root = NULL;

	/*
            1
        /      \
       2        3
      /  \     / \
     4   5    6  7
	*/

	/****************************************************************
	 * Create binary tree.
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
	
tree_destroy:
	binary_tree_node_destroy(root);

	if (rv)
		printf("[%s] Error: %d\n", __func__, rv);
}


/*****************************************************************************/

int main()
{
	basic_test();

	return 0;
}
