/**
 * Radix tree data structure implementation.
 * Copyright (C) 2024 Lazar Razvan.
 *
 * Example of byte-based radix tree (2^8 = 256) that splits keys in byte
 * boundaries useful for strings.
 */

#ifndef RADIX_TREE_H
#define RADIX_TREE_H


#include <stdbool.h>
#include <stdint.h>


/*****************************************************************************/

// Radix order (byte-based)
#define RADIX				(1 << 8)

// Max word length allowed
#define MAX_WORD_LEN		128


/*****************************************************************************/

// radix tree data operations
typedef void* (*alloc_fn)(void *);
typedef void (*free_fn)(void *);
typedef void (*print_fn)(void *);

// radix tree node
typedef struct radix_tree_node {

	char					*prefix;				// prefix
	void					*data;					// data
	struct radix_tree_node 	*children[RADIX];		// children

} radix_tree_node_t;

// radix tree entry
typedef struct radix_tree {

	radix_tree_node_t		*root;					// root
	alloc_fn				_alloc;					// data allocation
	print_fn				_print;					// data print
	free_fn					_free;					// data free

} radix_tree_t;


/*****************************************************************************/

//
radix_tree_t *radix_tree_init(alloc_fn _alloc, print_fn _print,
							free_fn _free);
void radix_tree_destroy(radix_tree_t *tree);

//
int radix_tree_insert(radix_tree_t *tree, char *key, void *data);
int radix_tree_remove(radix_tree_t *tree, char *key);

//
void *radix_tree_lookup(radix_tree_t *tree, char *key);

//
bool radix_tree_is_empty(radix_tree_t *tree);

//
void radix_tree_dump(radix_tree_t *tree);


#endif	// RADIX_TREE_H
