/**
 * Binary tree data structure implementation.
 * Copyright (C) 2024 Lazar Razvan.
 */

#ifndef BINARY_TREE_H
#define BINARY_TREE_H

#include <stdint.h>


/*****************************************************************************/

// binary tree node
typedef struct binary_tree_node_s {

	void						*data;		// node data
	struct binary_tree_node_s	*left;		// left node
	struct binary_tree_node_s	*right;		// right node

} binary_tree_node;


/*****************************************************************************/

// node data allocation callback
typedef void *(*alloc_cb)(void *);

// node data free callback
typedef void (*free_cb)(void *);

// node data print callback
typedef void (*print_cb)(void *);


/*****************************************************************************/

//
binary_tree_node *binary_tree_node_create(void *data, alloc_cb cb);
void binary_tree_node_destroy(binary_tree_node *node, free_cb cb);

//
void binary_tree_in_order_print(binary_tree_node *root, print_cb cb);
void binary_tree_pre_order_print(binary_tree_node *root, print_cb cb);
void binary_tree_post_order_print(binary_tree_node *root, print_cb cb);
void binary_tree_level_order_print(binary_tree_node *node, print_cb cb);

#endif	// BINARY_TREE_H
