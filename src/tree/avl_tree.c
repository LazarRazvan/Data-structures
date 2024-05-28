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

//
// NODES
//
static int __avl_node_get_height(avl_tree_node *node)
{
	return node ? node->height : 0;
}

static int __avl_node_get_balance(avl_tree_node *node)
{
	return node ? __avl_node_get_height(node->left) -
								__avl_node_get_height(node->right) : 0;
}

static void __avl_node_update_height(avl_tree_node *node)
{
	node->height = 1 + MAX(__avl_node_get_height(node->left),
							__avl_node_get_height(node->right));
}

static int __avl_node_is_leaf(avl_tree_node *node)
{
	return !(node->left || node->right);
}

static int __avl_node_has_one_child(avl_tree_node *node)
{
	return ((node->left && !node->right) || (!node->left && node->right));
}

avl_tree_node * __avl_node_get_min_node(avl_tree_node *node)
{
	while (node->left)
		node = node->left;

	return node;
}

static avl_tree_node *__avl_node_create(void *data, alloc_fn _alloc)
{
	avl_tree_node *node = NULL;

	node = (avl_tree_node *)malloc(sizeof(avl_tree_node));
	if (!node)
		goto error;

	node->data = _alloc(data);
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

static void __avl_node_destroy(avl_tree_node *node, free_fn _free)
{
	_free(node->data);
	free(node);
}

static int __avl_node_copy_content(avl_tree_node *dst, avl_tree_node *src,
									alloc_fn _alloc, free_fn _free)
{
	//
	_free(dst->data);

	//
	*dst = *src;

	//
	dst->data = _alloc(src->data);
	if (!dst->data)
		return -1;

	return 0;
}

static int __avl_node_copy_data(avl_tree_node *dst, avl_tree_node *src,
								alloc_fn _alloc, free_fn _free)
{
	//
	_free(dst->data);
	dst->data = _alloc(src->data);
	if (!dst->data)
		return -1;

	return 0;
}

//
// TREE
//
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
	__avl_node_update_height(x);
	__avl_node_update_height(y);

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
	__avl_node_update_height(x);
	__avl_node_update_height(y);

	return y;
}

static avl_tree_node *__avl_tree_balance(avl_tree_node *node)
{
	int balance;

	// update node height
	__avl_node_update_height(node);

	// get balance factor
	balance = __avl_node_get_balance(node);

	// right height is heigher than left height
	if (balance < -1) {
		/**
		 * Check if right rotation is required first.
		 *
		 * 10 (-2)                 10 (-2)
		 *  \          right(30)       \
		 *   30 (1)   ========>      20 (-1)
		 *  /                            \
		 * 20 (0)                      30 (0)
		 */
		if (__avl_node_get_balance(node->right) > 0)
			node->right = __avl_tree_right_rotate(node->right);

		/**
		 * 10 (-2)                      20 (0)
		 *  \           left(10)        /   \
		 *   20 (-1)   ========>    10 (0) 30 (0)
		 *    \
		 *     30 (0)
		 */
		return __avl_tree_left_rotate(node);
	}

	// left height is heigher than right height
	if (balance > 1) {
		/**
		 *    30 (2)                 30 (2)
		 *      /     left(20)      /
		 *   20 (-1) ========>  25 (1)
		 *     \                  /
		 *    25 (0)            20 (0)
		 */
		if (__avl_node_get_balance(node->left) < 0)
			node->left = __avl_tree_left_rotate(node->left);

		/**
		 *    30 (2)                 20 (0)
		 *      /     right(30)      /   \
		 *   20 (1)   ========>  10 (0) 30 (0)
		 *    /
		 *  10 (0)
		 */
		return __avl_tree_right_rotate(node);
	}

	return node;
}

static avl_tree_node *__avl_tree_insert(avl_tree_node *node, void *data,
										alloc_fn _alloc, cmp_fn _cmp)
{
	int cmp_id;

	if (!node)
		return __avl_node_create(data, _alloc);

	// node insert 
	cmp_id = _cmp(data, node->data);
	if (cmp_id < 0) {
		node->left = __avl_tree_insert(node->left, data, _alloc, _cmp);
		if (!node->left)
			return NULL;

	} else if (cmp_id > 0) {
		node->right = __avl_tree_insert(node->right, data, _alloc, _cmp);
		if (!node->right)
			return NULL;

	} else {
		return node;	// prevent duplicates
	}

	return __avl_tree_balance(node);
}

static avl_tree_node *__avl_tree_delete(avl_tree_node *node, void *data,
										alloc_fn _alloc, free_fn _free,
										cmp_fn _cmp)
{
	int cmp_id;
	avl_tree_node *tmp;

	if (!node)
		return NULL;

	// node delete
	cmp_id = _cmp(data, node->data);

	// left lookup
	if (cmp_id < 0)
		node->left = __avl_tree_delete(node->left, data, _alloc, _free, _cmp);

	// right lookup
	if (cmp_id > 0)
		node->right = __avl_tree_delete(node->right, data, _alloc, _free, _cmp);

	// node deletion
	if (cmp_id == 0) {
		// leaf node
		if (__avl_node_is_leaf(node)) {
			__avl_node_destroy(node, _free);
			return NULL;
		}

		if (__avl_node_has_one_child(node)) {
			// one child node (replace content of node and child)
			tmp = node->left ? node->left : node->right;

			//
			if (__avl_node_copy_content(node, tmp, _alloc, _free))
				return NULL;

			//
			__avl_node_destroy(tmp, _free);
		} else {
			// node with two children (replace data with right subtree min)
			tmp = __avl_node_get_min_node(node->right);

			//
			if (__avl_node_copy_data(node, tmp, _alloc, _free))
				return NULL;

			//
			node->right = __avl_tree_delete(node->right, tmp->data, _alloc,
											_free, _cmp);
		}
	}

	return __avl_tree_balance(node);
}

static int __avl_tree_lookup(avl_tree_node *node, void *data, cmp_fn _cmp)
{
	int cmp_id;

	if (!node)
		return 0;

	// node compare
	cmp_id = _cmp(data, node->data);

	// left lookup
	if (cmp_id < 0)
		return __avl_tree_lookup(node->left, data, _cmp);

	// right lookup
	if (cmp_id > 0)
		return __avl_tree_lookup(node->right, data, _cmp);

	// node found
	return 1;
}

static void __avl_tree_destroy(avl_tree_node *node, free_fn _free)
{
	if (!node)
		return;

	__avl_tree_destroy(node->left, _free);
	__avl_tree_destroy(node->right, _free);

	//
	__avl_node_destroy(node, _free);
}

static void __avl_tree_in_order_print(avl_tree_node *node, print_fn _print)
{
	if (!node)
		return;

	__avl_tree_in_order_print(node->left, _print);
	_print(node->data);
	__avl_tree_in_order_print(node->right, _print);
}

static void __avl_tree_pre_order_print(avl_tree_node *node, print_fn _print)
{
	if (!node)
		return;

	_print(node->data);
	__avl_tree_in_order_print(node->left, _print);
	__avl_tree_in_order_print(node->right, _print);
}

static void __avl_tree_post_order_print(avl_tree_node *node, print_fn _print)
{
	if (!node)
		return;

	__avl_tree_in_order_print(node->left, _print);
	__avl_tree_in_order_print(node->right, _print);
	_print(node->data);
}

static void __avl_tree_level_order_print(avl_tree_node *node, print_fn _print)
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
		_print(crt->data);

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
 * @_alloc	: Function to create node inforation.
 * @_free	: Function to free node inforation.
 * @_cmp	: Function to compare to node information.
 * @_print	: Function to print node information.
 *
 * Return new allocated entry on success and false otherwise.
 */
avl_tree_entry *avl_tree_create(alloc_fn _alloc, free_fn _free, cmp_fn _cmp,
								print_fn _print)
{
	avl_tree_entry *entry = NULL;

	// create entry
	entry = (avl_tree_entry *)malloc(sizeof(avl_tree_entry));
	if (!entry)
		return NULL;

	// initialize entry
	entry->root		= NULL;
	entry->_alloc	= _alloc;
	entry->_free	= _free;
	entry->_cmp		= _cmp;
	entry->_print	= _print;

	return entry;
}

/**
 * Destroy (free memory) for an AVL tree.
 */
void avl_tree_destroy(avl_tree_entry *entry)
{
	if (!entry || !entry->_free)
		return;

	__avl_tree_destroy(entry->root, entry->_free);
	free(entry);
}


/*****************************************************************************/

/**
 * Insert an entry to AVL tree. (O(log(n)))
 */
int avl_tree_insert(avl_tree_entry *entry, void *data)
{
	avl_tree_node *node;

	if (!entry || !data || !entry->_alloc || !entry->_cmp)
		goto error;

	//
	node = __avl_tree_insert(entry->root, data, entry->_alloc, entry->_cmp);
	if (!node)
		goto error;	// do not alter the root

// success
	entry->root = node;
	return 0;

error:
	return -1;
}

/**
 * Delete an entry from AVL tree. (O(log(n)))
 */
int avl_tree_delete(avl_tree_entry *entry, void *data)
{
	avl_tree_node *node;

	if (!entry || !data || !entry->_alloc || !entry->_free || !entry->_cmp)
		goto error;

	//
	node = __avl_tree_delete(entry->root, data, entry->_alloc, entry->_free,
							entry->_cmp);
	if (!node)
		goto error;	// do not alter the root

// success
	entry->root = node;
	return 0;

error:
	return -1;
}

/**
 * Lookup for an entry in an AVL tree. (O(log(n)))
 */
int avl_tree_lookup(avl_tree_entry *entry, void *data)
{

	if (!entry || !data || !entry->_cmp)
		return 0;

	//
	return __avl_tree_lookup(entry->root, data, entry->_cmp);
}


/*****************************************************************************/

/**
 * Print an AVL tree in pre-order. (left + root + right).
 */
void avl_tree_in_order_print(avl_tree_entry *entry)
{
	if (!entry || !entry->_print)
		return;

	if (!entry->root) {
		printf("AVL Tree is empty!\n");
		return;
	}

	__avl_tree_in_order_print(entry->root, entry->_print);
}

/**
 * Print an AVL tree in pre-order. (root + left + right).
 */
void avl_tree_pre_order_print(avl_tree_entry *entry)
{
	if (!entry || !entry->_print)
		return;

	if (!entry->root) {
		printf("AVL Tree is empty!\n");
		return;
	}

	__avl_tree_pre_order_print(entry->root, entry->_print);
}

/**
 * Print an AVL tree in post-order. (left + right + root).
 */
void avl_tree_post_order_print(avl_tree_entry *entry)
{
	if (!entry || !entry->_print)
		return;

	if (!entry->root) {
		printf("AVL Tree is empty!\n");
		return;
	}

	__avl_tree_post_order_print(entry->root, entry->_print);
}

void avl_tree_level_order_print(avl_tree_entry *entry)
{
	if (!entry || !entry->_print)
		return;

	if (!entry->root) {
		printf("AVL Tree is empty!\n");
		return;
	}

	__avl_tree_level_order_print(entry->root, entry->_print);
}
