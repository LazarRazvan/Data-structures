/**
 * AVL tree data structure implementation.
 * Copyright (C) 2024 Lazar Razvan.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "queue/queue.h"
#include "tree/avl_tree.h"


/*****************************************************************************/

#define MAX(X, Y) (((X) < (Y)) ? (Y) : (X))


/*****************************************************************************/

static int __avl_tree_node_get_height(avl_tree_node *node)
{
	return node ? node->height : 0;
}

static int __avl_tree_node_get_balance(avl_tree_node *node)
{
	return node ? __avl_tree_node_get_height(node->left) -
								__avl_tree_node_get_height(node->right) : 0;
}

static avl_tree_node* __avl_tree_left_rotate(avl_tree_node *x)
{
	/*
	 *           p                      p
	 *           |                      |
	 *           x                      y
	 *         /   \         =>       /   \
	 *        xL    y                x    yR
	 *            /   \            /  \
	 *           yL   yR          xL  yL
	 */
	avl_tree_node *y, *yL;

	//
	y = x->right;
	yL = y->left;

	// rotate
	y->left = x;
	x->right = yL;

	// update height
	x->height = 1 + MAX(__avl_tree_node_get_height(x->left),
							__avl_tree_node_get_height(x->right));
	y->height = 1 + MAX(__avl_tree_node_get_height(y->left),
							__avl_tree_node_get_height(y->right));

	return y;
}

static avl_tree_node* __avl_tree_right_rotate(avl_tree_node *x)
{
	/*
	 *           p                      p
	 *           |                      |
	 *           x                      y
	 *         /   \         =>       /   \
	 *        y    xR                yL   x
	 *       / \                         / \
	 *      yL yR                       yR  xR
	 */
	avl_tree_node *y, *yR;

	//
	y = x->left;
	yR = y->right;

	// rotate
	y->right = x;
	x->left = yR;

	// update height
	x->height = 1 + MAX(__avl_tree_node_get_height(x->left),
							__avl_tree_node_get_height(x->right));
	y->height = 1 + MAX(__avl_tree_node_get_height(y->left),
							__avl_tree_node_get_height(y->right));

	return y;
}

static avl_tree_node *__avl_tree_node_create(void *data, alloc_fn alloc)
{
	avl_tree_node *node = NULL;

	node = (avl_tree_node *)malloc(sizeof(avl_tree_node));
	if (!node)
		goto error;

	node->data = alloc(data);
	if (!node->data)
		goto node_free;

	node->left = NULL;
	node->right = NULL;
	node->height = 1;

// success
	return node;

node_free:
	free(node);
error:
	return NULL;
}

static avl_tree_node *__avl_tree_insert(avl_tree_node *node, void *data,
										alloc_fn alloc, cmp_fn cmp)
{
	int cmp_id, node_balance;

	if (!node)
		return __avl_tree_node_create(data, alloc);

	// node insert 
	cmp_id = cmp(data, node->data);
	if (cmp_id < 0) {
		node->left = __avl_tree_insert(node->left, data, alloc, cmp);
		if (!node->left)
			return NULL;

	} else if (cmp_id > 0) {
		node->right = __avl_tree_insert(node->right, data, alloc, cmp);
		if (!node->right)
			return NULL;

	} else {
		return node;	// prevent duplicates
	}

	// node height update
	node->height = 1 + MAX(__avl_tree_node_get_height(node->left),
							__avl_tree_node_get_height(node->right));

	// avl tree balance
	node_balance = __avl_tree_node_get_balance(node);

	// left or right-left rotation
	if (node_balance < -1) {
		/**
		 * Insert: 30
		 *
		 * 10 (-2)                      20 (0)
		 *  \           left(10)        /   \
		 *   20 (-1)   ========>    10 (0) 30 (0)
		 *    \
		 *     30 (0)
		 *
		 * 30 > 20 (only left rotate(10))
		 */
		if (cmp(data, node->right->data) > 0)
			return __avl_tree_left_rotate(node);

		/**
		 * Insert: 20
		 * 10 (-2)                 10 (-2)                  20 (0)
		 *  \          right(30)       \     left(10)       /   \
		 *   30 (1)   ========>      20 (-1) =======>    10(0)  30(0)
		 *  /                            \
		 * 20 (0)                      30 (0)
		 *
		 * 20 < 30 (right rotate(30) + left rotate(10))
		 */
		if (cmp(data, node->right->data) < 0) {
			node->right = __avl_tree_right_rotate(node->right);
			return __avl_tree_left_rotate(node);
		}
	}

	// right or left-right rotation
	if (node_balance > 1) {
		/**
		 * Insert: 10
		 *
		 *    30 (2)                 20 (0)
		 *      /     right(30)      /   \
		 *   20 (1)   ========>  10 (0) 30 (0)
		 *    /
		 *  10 (0)
		 *
		 * 10 < 20 (only right rotate(30))
		 */
		if (cmp(data, node->left->data) < 0)
			return __avl_tree_right_rotate(node);

		/**
		 * Insert: 25
		 *
		 *    30 (2)                 30 (2)                   25(0)
		 *      /     left(20)      /        right(30)        /  \
		 *   20 (-1) ========>  25 (1)      ===========>  20 (0)  30 (0)
		 *     \                  /
		 *    25 (0)            20 (0)
		 *
		 * 25 > 20 (left rotate(20) + right rotate(30))
		 */
		if (cmp(data, node->left->data) > 0) {
			node->left = __avl_tree_left_rotate(node->left);
			return __avl_tree_right_rotate(node);
		}
	}

	return node;
}

static void __avl_tree_destroy(avl_tree_node *node, free_fn free)
{
	if (!node)
		return;

	__avl_tree_destroy(node->left, free);
	__avl_tree_destroy(node->right, free);

	free(node->data);
	free(node);
}

static void __avl_tree_in_order_print(avl_tree_node *node, print_fn print)
{
	if (!node)
		return;

	__avl_tree_in_order_print(node->left, print);
	print(node->data);
	__avl_tree_in_order_print(node->right, print);
}

static void __avl_tree_pre_order_print(avl_tree_node *node, print_fn print)
{
	if (!node)
		return;

	print(node->data);
	__avl_tree_in_order_print(node->left, print);
	__avl_tree_in_order_print(node->right, print);
}

static void __avl_tree_post_order_print(avl_tree_node *node, print_fn print)
{
	if (!node)
		return;

	__avl_tree_in_order_print(node->left, print);
	__avl_tree_in_order_print(node->right, print);
	print(node->data);
}

static void __avl_tree_level_order_print(avl_tree_node *node, print_fn print)
{
	avl_tree_node *crt;
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
		crt = (avl_tree_node *)queue_dequeue(queue);
		assert(crt);

		// print element in queue
		print(crt->data);

		// add element leaves in queue
		if (crt->left)
			assert(!queue_enqueue(queue, crt->left));

		if (crt->right)
			assert(!queue_enqueue(queue, crt->right));
	}

	// destroy queue
	queue_destroy(queue);
}


/*****************************************************************************/

/**
 * Create an AVL tree entry.
 *
 * @alloc	: Function to create node inforation.
 * @free	: Function to free node inforation.
 * @cmp		: Function to compare to node information.
 * @print	: Function to print node information.
 *
 * Return new allocated entry on success and false otherwise.
 */
avl_tree_entry *avl_tree_create(alloc_fn alloc, free_fn free, cmp_fn cmp,
								print_fn print)
{
	avl_tree_entry *entry = NULL;

	// create entry
	entry = (avl_tree_entry *)malloc(sizeof(avl_tree_entry));
	if (!entry)
		return NULL;

	// initialize entry
	entry->root		= NULL;
	entry->alloc	= alloc;
	entry->free		= free;
	entry->cmp		= cmp;
	entry->print	= print;

	return entry;
}

/**
 * Destroy (free memory) for an AVL tree.
 */
void avl_tree_destroy(avl_tree_entry *entry)
{
	if (!entry || !entry->free)
		return;

	__avl_tree_destroy(entry->root, entry->free);
	free(entry);
}


/*****************************************************************************/

/**
 * Insert an entry to AVL tree. (O(log(n)))
 */
int avl_tree_insert(avl_tree_entry *entry, void *data)
{
	avl_tree_node *node;

	if (!entry || !data || !entry->alloc || !entry->cmp)
		goto error;

	//
	node = __avl_tree_insert(entry->root, data, entry->alloc, entry->cmp);
	if (!node)
		goto error;	// do not alter the root

// success
	entry->root = node;
	return 0;

error:
	return -1;
}


/*****************************************************************************/

/**
 * Print an AVL tree in pre-order. (left + root + right).
 */
void avl_tree_in_order_print(avl_tree_entry *entry)
{
	if (!entry || !entry->print)
		return;

	if (!entry->root) {
		printf("AVL Tree is empty!\n");
		return;
	}

	__avl_tree_in_order_print(entry->root, entry->print);
}

/**
 * Print an AVL tree in pre-order. (root + left + right).
 */
void avl_tree_pre_order_print(avl_tree_entry *entry)
{
	if (!entry || !entry->print)
		return;

	if (!entry->root) {
		printf("AVL Tree is empty!\n");
		return;
	}

	__avl_tree_pre_order_print(entry->root, entry->print);
}

/**
 * Print an AVL tree in post-order. (left + right + root).
 */
void avl_tree_post_order_print(avl_tree_entry *entry)
{
	if (!entry || !entry->print)
		return;

	if (!entry->root) {
		printf("AVL Tree is empty!\n");
		return;
	}

	__avl_tree_post_order_print(entry->root, entry->print);
}

void avl_tree_level_order_print(avl_tree_entry *entry)
{
	if (!entry || !entry->print)
		return;

	if (!entry->root) {
		printf("AVL Tree is empty!\n");
		return;
	}

	__avl_tree_level_order_print(entry->root, entry->print);
}
