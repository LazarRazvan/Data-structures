/**
 * Binary tree data structure test.
 * Copyright (C) 2024 Lazar Razvan.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "binary_tree/binary_tree.h"

/*****************************************************************************/
//
// Basic test binary tree creation, and traversal using integers.
//

static void *basic_test_alloc_cb(void *data)
{
	void *p;

	p = malloc(sizeof(int));
	if (!p)
		return NULL;

	memcpy(p, data, sizeof(int));

	return p;
}

static void basic_test_free_cb(void *data)
{
	free(data);
}

static void basic_test_print_cb(void *data)
{
	printf("%d ", *((int *)data));
}

static void basic_test(void)
{
	int rv, val;
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
	val = 1;
	root = binary_tree_node_create(&val, basic_test_alloc_cb);
	if (!root) {
		rv = -1; goto error;
	}

	//
	val = 2;
	root->left = binary_tree_node_create(&val, basic_test_alloc_cb);
	if (!root->left) {
		rv = -2; goto error;
	}

	//
	val = 3;
	root->right = binary_tree_node_create(&val, basic_test_alloc_cb);
	if (!root->right) {
		rv = -3; goto error;
	}

	//
	val = 4;
	root->left->left = binary_tree_node_create(&val, basic_test_alloc_cb);
	if (!root->left->left) {
		rv = -4; goto error;
	}

	//
	val = 5;
	root->left->right = binary_tree_node_create(&val, basic_test_alloc_cb);
	if (!root->left->right) {
		rv = -5; goto error;
	}

	//
	val = 6;
	root->right->left = binary_tree_node_create(&val, basic_test_alloc_cb);
	if (!root->right->left) {
		rv = -6; goto error;
	}

	//
	val = 7;
	root->right->right = binary_tree_node_create(&val, basic_test_alloc_cb);
	if (!root->right->right) {
		rv = -7; goto error;
	}
	
error:
	binary_tree_node_destroy(root, basic_test_free_cb);
	printf("[%s] Error: %d\n", __func__, rv);
}


/*****************************************************************************/

int main()
{
	basic_test();

	return 0;
}
