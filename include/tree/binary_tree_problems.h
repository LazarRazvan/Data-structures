/**
 * Binary tree data structure problems.
 * Copyright (C) 2024 Lazar Razvan.
 */

#ifndef BINARY_TREE_PROBLEMS_H
#define BINARY_TREE_PROBLEMS_H

#include <stdbool.h>

#include "tree/binary_tree.h"


/*****************************************************************************/

//
binary_tree_node *binary_tree_by_traversals(int *in, int in_len, int *level,
											int level_len);
//
bool binary_tree_is_sum_tree(binary_tree_node *root);
//
bool binary_tree_cousins(binary_tree_node *root, int a, int b);
//
bool binary_tree_divide_two_halves(binary_tree_node *root);
//
bool binary_tree_is_perfect(binary_tree_node *root);
//
bool binary_tree_are_duplicate(binary_tree_node *r1, binary_tree_node *r2);
//
bool binary_tree_are_mirror(binary_tree_node *r1, binary_tree_node *r2);
//
bool binary_tree_is_foldable(binary_tree_node *root);


#endif	// BINARY_TREE_PROBLEMS_H
