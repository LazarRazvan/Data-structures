/**
 * Binary tree data structure implementation.
 * Copyright (C) 2024 Lazar Razvan.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "queue/queue.h"
#include "stack/stack.h"
#include "tree/binary_tree.h"

/*****************************************************************************/


#define MAX(X, Y) (((X) < (Y)) ? (Y) : (X))

/*****************************************************************************/

/**
 * Create a node in a binary tree.
 *
 * @data: Data to be stored by current node.
 *
 * Return new allocated node on success and false otherwise.
 */
binary_tree_node *binary_tree_node_create(int data)
{
	binary_tree_node *node = NULL;

	// create node
	node = (binary_tree_node *)malloc(sizeof(binary_tree_node));
	if (!node)
		return NULL;

	// initialize node
	node->data = data;
	node->left = node->right = NULL;

	return node;
}


/**
 * Destroy a node in a binary tree. Note that function destroy all the
 * childrend (if any) for a given node.
 *
 * @node: Node to be destroyed.
 */
void binary_tree_node_destroy(binary_tree_node *node)
{
	if (!node)
		return;

	//
	binary_tree_node_destroy(node->left);
	binary_tree_node_destroy(node->right);

	// destroy node
	free(node);
}


/**
 * Check if a binary tree node is leaf or not.
 *
 * @node: Node to be checked.
 */
bool binary_tree_node_is_leaf(binary_tree_node *node)
{
	if (!node)
		return false;

	return (!node->left && !node->right);
}


/*****************************************************************************/

/**
 * Insert a node in a binary tree using level order. Iterate tree nodes using
 * level order traversal. When a node left child is empty, insert the new node.
 * Otherwise, when a node right child is empty, insert the new node.
 *
 * @root: Binary tree root.
 * @data: Data to be stored by new node.
 *
 * Return new created node on success and NULL otherwise.
 */
binary_tree_node *binary_tree_insert(binary_tree_node *root, int data)
{
	queue_t *queue = NULL;
	binary_tree_node *crt = NULL;
	binary_tree_node *node = NULL;

	// if empty, create tree root
	if (!root) {
		node = binary_tree_node_create(data);
		goto finish;
	}

	// level order traversal to find the slot for the new node
	queue = queue_create(128);
	assert(queue);

	//
	assert(!queue_enqueue(queue, root));

	while (!queue_is_empty(queue)) {
		// get first in queue
		crt = (binary_tree_node *)queue_dequeue(queue);
		assert(crt);

		// left child
		if (!crt->left) {
			node = binary_tree_node_create(data);
			if (!node)
				goto free_queue;

			//
			crt->left = node;
			break;
		} else {
			assert(!queue_enqueue(queue, crt->left));
		}

		// right child
		if (!crt->right) {
			node = binary_tree_node_create(data);
			if (!node)
				goto free_queue;

			//
			crt->right = node;
			break;
		} else {
			assert(!queue_enqueue(queue, crt->right));
		}
	}

free_queue:
	queue_destroy(queue);

finish:
	return node;
}


/**
 * Delete a node from a binary tree. The node deletion in a binary tree is
 * performed by replacing the node to be deleted with the bottom most and
 * right most node to make sure the tree is kept balanced.
 *
 * @root: Binary tree root.
 * @data: Node with data to be deleted.
 *
 * Return binary tree root on success and NULL otherwise.
 *
 * Note: If multiple nodes with same data reside in the tree, the last node
 * in level order traversal will be removed.
 */
binary_tree_node *binary_tree_delete(binary_tree_node *root, int data)
{
	queue_t *queue = NULL;
	binary_tree_node *to_delete = NULL;		// node to be deleted
	binary_tree_node *to_replace = NULL;	// bottom and right most node
	binary_tree_node *parent_to_replace = NULL;	// parent of node to replace

	// error for empty root
	if (!root)
		return NULL;

	//
	queue = queue_create(128);
	assert(queue);

	//
	assert(!queue_enqueue(queue, root));

	// use "to_replace" because at the end of the loop will store the
	// bottom most and right most node (to repalce the deleted node)
	while (!queue_is_empty(queue)) {

		// get first in queue
		to_replace = (binary_tree_node *)queue_dequeue(queue);
		assert(to_replace);

		// lookup for node to be deleted
		if (to_replace->data == data)
			to_delete = to_replace;

		// store last parent to have children
		if (to_replace->left) {
			parent_to_replace = to_replace;
			assert(!queue_enqueue(queue, to_replace->left));
		}

		if (to_replace->right) {
			parent_to_replace = to_replace;
			assert(!queue_enqueue(queue, to_replace->right));
		}
	}

	// error if node to be deleted was not found
	if (!to_delete)
		goto free_queue;

	// node to be deleted found, critical error if replace node is NULL
	assert(to_replace);

	// node to replace has no parent, tree only has root node and try to remove
	if (!parent_to_replace) {
		assert(to_delete == root);
		assert(to_replace == root);

		//
		binary_tree_node_destroy(root);
		root = NULL;
		goto free_queue;
	}

	// remove node (copy data and remove 'to_replace' node)
	to_delete->data = to_replace->data;

	//
	if (parent_to_replace->left == to_replace)
		parent_to_replace->left = NULL;
	else
		parent_to_replace->right = NULL;

	//
	binary_tree_node_destroy(to_replace);


free_queue:
	queue_destroy(queue);

	return root;
}


/*****************************************************************************/

/**
 * Print a binary tree in order. (left + root + right).
 *
 * @node: Binary tree node.
 */
void binary_tree_in_order_print(binary_tree_node *node)
{
	if (!node)
		return;

	binary_tree_in_order_print(node->left);
	printf("%d ", node->data);
	binary_tree_in_order_print(node->right);
}


/**
 * Print a binary tree in pre-order. (root + left + right).
 *
 * @node: Binary tree node.
 */
void binary_tree_pre_order_print(binary_tree_node *node)
{
	if (!node)
		return;

	printf("%d ", node->data);
	binary_tree_pre_order_print(node->left);
	binary_tree_pre_order_print(node->right);
}


/**
 * Print a binary tree in post-order. (left + right + root).
 *
 * @node: Binary tree node.
 */
void binary_tree_post_order_print(binary_tree_node *node)
{
	if (!node)
		return;

	binary_tree_post_order_print(node->left);
	binary_tree_post_order_print(node->right);
	printf("%d ", node->data);
}


/**
 * Print a binary tree in level-order.
 *
 * @node: Binary tree node.
 */
void binary_tree_level_order_print(binary_tree_node *node)
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
	assert(!queue_enqueue(queue, node));

	while (!queue_is_empty(queue)) {
		// get first in queue
		crt = (binary_tree_node *)queue_dequeue(queue);
		assert(crt);

		// print element in queue
		printf("%d ", crt->data);

		// add element leaves in queue
		if (crt->left)
			assert(!queue_enqueue(queue, crt->left));

		if (crt->right)
			assert(!queue_enqueue(queue, crt->right));
	}

	// destroy queue
	queue_destroy(queue);
}


/**
 * Print a binary tree in level-order spiral.
 *
 * @node: Binary tree node.
 */
void binary_tree_level_order_spiral_print(binary_tree_node *node)
{
	binary_tree_node *crt;
	stack_t *s_ltr = NULL;	// left-to-right stack
	stack_t *s_rtl = NULL;	// right-to-left stack
	//
	if (!node)
		return;

	// create queue for printing
	s_ltr = stack_create();
	s_rtl = stack_create();
	assert(s_ltr || s_rtl);

	// insert stack into right-to-left stack
	assert(!stack_push(s_rtl, node));

	//
	while (!stack_is_empty(s_ltr) || !stack_is_empty(s_rtl)) {

		// level print from right-to-left; while printing all nodes, insert
		// children into left-to-right stack (since it is a stack, insert
		// first the right children and than the left one; left most node will
		// be the head of the stack)
		while (!stack_is_empty(s_rtl)) {
			crt = stack_pop(s_rtl);
			assert(crt);

			//
			printf("%d ", crt->data);

			//
			if (crt->right)
				assert(!stack_push(s_ltr, crt->right));

			if (crt->left)
				assert(!stack_push(s_ltr, crt->left));
		}

		// level print from left-to-right; while printing all nodes, insert
		// children into right-to-left stack (since it is a stack, insert
		// first the left children and than the right one; right most node will
		// be the head of the stack)
		while (!stack_is_empty(s_ltr)) {
			crt = stack_pop(s_ltr);
			assert(crt);

			//
			printf("%d ", crt->data);

			//
			if (crt->left)
				assert(!stack_push(s_rtl, crt->left));

			if (crt->right)
				assert(!stack_push(s_rtl, crt->right));
		}

	}

	// destroy stacks
	stack_destroy(s_ltr);
	stack_destroy(s_rtl);
}


/**
 * Print a binary tree in level-order reverse.
 *
 * @node: Binary tree node.
 */
void binary_tree_level_order_reverse_print(binary_tree_node *node)
{
	binary_tree_node *crt;
	stack_t *stack = NULL;
	queue_t *queue = NULL;

	//
	if (!node)
		return;

	//
	stack = stack_create();
	queue = queue_create(128);
	assert(stack || queue);

	//
	assert(!queue_enqueue(queue, node));

	// level order traversal and push each node to stack
	while (!queue_is_empty(queue)) {
		crt = (binary_tree_node *)queue_dequeue(queue);
		assert(crt);

		//
		assert(!stack_push(stack, crt));

		//
		if (crt->left)
			assert(!queue_enqueue(queue, crt->left));

		if (crt->right)
			assert(!queue_enqueue(queue, crt->right));
	}

	// reverse print using stack
	while (!stack_is_empty(stack)) {
		crt = stack_pop(stack);
		assert(crt);

		//
		printf("%d ", crt->data);
	}

	// destroy data structures
	stack_destroy(stack);
	queue_destroy(queue);
}


/**
 * Print a binary tree in pre-order (root + left + right) without usign
 * recursion and using an explicit stack.
 *
 * @node: Binary tree node.
 */
void binary_tree_pre_order_iterative_print(binary_tree_node *node)
{
	binary_tree_node *crt;
	stack_t *stack = NULL;

	//
	if (!node)
		return;

	//
	stack = stack_create();
	assert(stack);

	// add root to stack
	assert(!stack_push(stack, node));

	//
	while (!stack_is_empty(stack)) {
		crt = stack_pop(stack);
		assert(crt);

		//
		printf("%d ", crt->data);

		// push children to stack (first right than left)
		if (crt->right)
			assert(!stack_push(stack, crt->right));

		if (crt->left)
			assert(!stack_push(stack, crt->left));
	}

	// destroy data structures
	stack_destroy(stack);
}


/**
 * Print a binary tree in post-order (left + right + root) without usign
 * recursion by using two stack data structures.
 *
 * The base idea is that the post-order can be seen as a reverse list for a
 * custom pre-order traversal (root + right + left)
 *
 * @node: Binary tree node.
 */
void binary_tree_post_order_iterative_print(binary_tree_node *node)
{
	binary_tree_node *crt;
	stack_t *stack1 = NULL;
	stack_t *stack2 = NULL;

	//
	if (!node)
		return;

	//
	stack1 = stack_create();
	stack2 = stack_create();
	assert(stack1 || stack2);

	// add root to stack
	assert(!stack_push(stack1, node));

	// custom pre-order (root + right + left)
	while (!stack_is_empty(stack1)) {
		crt = stack_pop(stack1);
		assert(crt);

		//
		assert(!stack_push(stack2, crt));

		// push children to stack (first left than right)
		if (crt->left)
			assert(!stack_push(stack1, crt->left));

		if (crt->right)
			assert(!stack_push(stack1, crt->right));
	}

	// print post-order traversal (left + right + root)
	while (!stack_is_empty(stack2)) {
		crt = stack_pop(stack2);
		assert(crt);

		//
		printf("%d ", crt->data);
	}

	// destroy data structures
	stack_destroy(stack1);
	stack_destroy(stack2);
}


/**
 * Print a binary tree in diagonal (root and right children are always first
 * than left children).
 *
 * @node: Binary tree node.
 */
void binary_tree_diagonal_print(binary_tree_node *node)
{
	queue_t *queue = NULL;

	//
	if (!node)
		return;

	//
	queue = queue_create(128);
	assert(queue);

	// for a node print all right children path and store left children to
	// queue
	while (node) {
		printf("%d ", node->data);

		// if left children, store to queue
		if (node->left)
			assert(!queue_enqueue(queue, node->left));

		// if right children, continue iteration and print
		if (node->right) {
			node = node->right;
			continue;
		}

		// no right children, check nodes in queue
		node = queue_dequeue(queue);
		if (!node)
			break;	// no more left children
	}

	// destroy data structures
	queue_destroy(queue);
}


/*****************************************************************************/

/**
 * Get depth of a binary tree using recursive method.
 *
 * @node: Binary tree root.
 *
 * Depth represent the number of edges from root to the deepest leaf node.
 * The depth of an empty tree is 0 and the depth of a tree with a single node
 * is 1.
 */
int binary_tree_depth_recursive(binary_tree_node *node)
{
	if (!node)
		return 0;

	return MAX(binary_tree_depth_recursive(node->left),
				binary_tree_depth_recursive(node->right)) + 1;
}


/**
 * Get depth of a binary tree using iterative method (level order)
 *
 * @node: Binary tree root.
 *
 * Depth represent the number of edges from root to the deepest leaf node.
 * The depth of an empty tree is 0 and the depth of a tree with a single node
 * is 1.
 */
int binary_tree_depth_iterative(binary_tree_node *node)
{
	int depth = 0;
	unsigned int size;
	binary_tree_node *crt;
	queue_t *queue = NULL;

	//
	if (!node)
		goto success;

	// create queue
	queue = queue_create(128);
	assert(queue);

	// add root to queue
	assert(!queue_enqueue(queue, node));

	while (!queue_is_empty(queue)) {
		// increase depth
		depth++;

		// loop to dequeue all nodes on the same level
		size = queue_size(queue);
		for (int i = 0; i < size; i++) {
			crt = (binary_tree_node *)queue_dequeue(queue);
			assert(crt);

			// add children in queue
			if (crt->left)
				assert(!queue_enqueue(queue, crt->left));

			if (crt->right)
				assert(!queue_enqueue(queue, crt->right));
		}
	}


	// destroy queue
	queue_destroy(queue);

success:

	return depth;
}


/*****************************************************************************/

/**
 * Get size(number of nodes) of a binary tree.
 *
 * @node: Binary tree root.
 */
int binary_tree_size(binary_tree_node *node)
{
	if (!node)
		return 0;

	return (binary_tree_size(node->left) + binary_tree_size(node->right) + + 1);
}
