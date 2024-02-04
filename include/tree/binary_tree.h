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

	int							data;		// node data
	struct binary_tree_node_s	*left;		// left node
	struct binary_tree_node_s	*right;		// right node

} binary_tree_node;


/*****************************************************************************/

//
binary_tree_node *binary_tree_node_create(int data);
void binary_tree_node_destroy(binary_tree_node *node);


/*****************************************************************************/

//
binary_tree_node *binary_tree_insert(binary_tree_node *root, int data);
binary_tree_node *binary_tree_delete(binary_tree_node *root, int data);


/*****************************************************************************/

//
void binary_tree_in_order_print(binary_tree_node *root);
void binary_tree_pre_order_print(binary_tree_node *root);
void binary_tree_post_order_print(binary_tree_node *root);
void binary_tree_level_order_print(binary_tree_node *node);
void binary_tree_level_order_spiral_print(binary_tree_node *node);


/*****************************************************************************/

//
int binary_tree_depth_recursive(binary_tree_node *node);
int binary_tree_depth_iterative(binary_tree_node *node);


#endif	// BINARY_TREE_H
