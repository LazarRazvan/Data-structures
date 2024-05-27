/**
 * AVL tree data structure implementation.
 * Copyright (C) 2024 Lazar Razvan.
 */

#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <stdint.h>
#include <stdbool.h>

/*****************************************************************************/

// avl tree function operations
typedef void* (*alloc_fn)(void *);
typedef void (*free_fn)(void *);
typedef int	(*cmp_fn)(void *, void *);
typedef void (*print_fn)(void *);


/*****************************************************************************/

// avl tree node
typedef struct avl_tree_node_s {

	void						*data;		// node information
	struct avl_tree_node_s		*left;		// left subtree
	struct avl_tree_node_s		*right;		// righ subtree
	int							height;		// node height

} avl_tree_node;

// avl tree entry
typedef struct avl_tree_entry_s {

	avl_tree_node				*root;		// avl tree root
	alloc_fn					alloc;		// data allocation function
	free_fn						free;		// data free function
	cmp_fn						cmp;		// data compare function
	print_fn					print;		// data print function

} avl_tree_entry;


/*****************************************************************************/

//
avl_tree_entry *avl_tree_create(alloc_fn alloc, free_fn free, cmp_fn cmp,
								print_fn print);
void avl_tree_destroy(avl_tree_entry *entry);


/*****************************************************************************/

//
int avl_tree_insert(avl_tree_entry *entry, void *data);


/*****************************************************************************/

//
void avl_tree_in_order_print(avl_tree_entry *entry);
void avl_tree_pre_order_print(avl_tree_entry *entry);
void avl_tree_post_order_print(avl_tree_entry *entry);
void avl_tree_level_order_print(avl_tree_entry *entry);


#endif	// AVL_TREE_H
