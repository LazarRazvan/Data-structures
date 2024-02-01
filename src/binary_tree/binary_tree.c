/**
 * Binary tree data structure implementation.
 * Copyright (C) 2024 Lazar Razvan.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "queue/queue.h"
#include "binary_tree/binary_tree.h"


/*****************************************************************************/

/**
 * Create a node in a binary tree.
 *
 * @data: Data to be stored by current node.
 * @cb	: Callback to allocate and set memory for node data.
 *
 * Return new allocated node on success and false otherwise.
 */
binary_tree_node *binary_tree_node_create(void *data, alloc_cb cb)
{
	binary_tree_node *node = NULL;

	// validate data
	if (!data || !cb)
		return NULL;

	// create node
	node = (binary_tree_node *)malloc(sizeof(binary_tree_node_create));
	if (!node)
		return NULL;

	// create node data
	node->data = cb(data);
	if (!node->data)
		goto free_node;

	// reset leaves
	node->left = node->right = NULL;

	return node;

free_node:
	free(node);

	return NULL;
}


/**
 * Destroy a node in a binary tree. Note that function destroy all the
 * childrend (if any) for a given node.
 *
 * @node: Node to be destroyed.
 * @cb	: Callback to free memory for node data.
 */
void binary_tree_node_destroy(binary_tree_node *node, free_cb cb)
{
	if (!node)
		return;

	//
	binary_tree_node_destroy(node->left, cb);
	binary_tree_node_destroy(node->right, cb);

	// destroy node
	cb(node);
	free(node);
}


/*****************************************************************************/

/**
 * Print a binary tree in order. (left + root + right).
 *
 * @node: Binary tree node.
 * @cb	: Callback to print node data.
 */
void binary_tree_in_order_print(binary_tree_node *node, print_cb cb)
{
	if (!node)
		return;

	binary_tree_in_order_print(node->left, cb);
	cb(node->data);
	binary_tree_in_order_print(node->right, cb);
}


/**
 * Print a binary tree in pre-order. (root + left + right).
 *
 * @node: Binary tree node.
 * @cb	: Callback to print node data.
 */
void binary_tree_pre_order_print(binary_tree_node *node, print_cb cb)
{
	if (!node)
		return;

	cb(node->data);
	binary_tree_in_order_print(node->left, cb);
	binary_tree_in_order_print(node->right, cb);
}


/**
 * Print a binary tree in post-order. (left + right + root).
 *
 * @node: Binary tree node.
 * @cb	: Callback to print node data.
 */
void binary_tree_post_order_print(binary_tree_node *node, print_cb cb)
{
	if (!node)
		return;

	binary_tree_in_order_print(node->left, cb);
	binary_tree_in_order_print(node->right, cb);
	cb(node->data);
}


/**
 * Print a binary tree in level-order.
 *
 * @node: Binary tree node.
 * @cb	: Callback to print node data.
 */
void binary_tree_level_order_print(binary_tree_node *node, print_cb cb)
{
	binary_tree_node *crt;
	queue_t *queue = NULL;

	//
	if (!node)
		return;

	// create queue for printing
	queue = queue_create(128);
	if (!queue)
		return;

	//
	assert(queue_enqueue(queue, node));

	while (!queue_is_empty(queue)) {
		// get first in queue
		crt = (binary_tree_node *)queue_dequeue(queue);
		assert(crt);

		// print element in queue
		cb(crt);

		// add element leaves in queue
		if (crt->left)
			assert(queue_enqueue(queue, crt->left));

		if (crt->right)
			assert(queue_enqueue(queue, crt->right));
	}

	// destroy queue
	queue_destroy(queue);
}
