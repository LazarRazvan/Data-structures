/**
 * Binary tree data structure problems.
 * Copyright (C) 2024 Lazar Razvan.
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

#include "queue/queue.h"
#include "tree/binary_tree_problems.h"


/*****************************************************************************/

/**
 * Create the level order traversal for left subtree.
 */
static
int *__left_tree_lvl_order(int *in, int in_len, int *lvl, int lvl_len,
							int root_idx, int *llvl_len)
{
	int *llvl;

	// alloc space
	llvl = (int *)malloc(in_len * sizeof(int));
	if (!llvl)
		return NULL;

	// fill in the level order array for left subtree
	*llvl_len = 0;
	for (int i = 1; i < lvl_len; i++) {
		for (int j = 0; j < root_idx; j++) {
			if (lvl[i] == in[j]) {
				llvl[(*llvl_len)++] = lvl[i];
			}
		}
	}

	return llvl;
}


/**
 * Create the level order traversal for right subtree.
 */
static
int *__right_tree_lvl_order(int *in, int in_len, int *lvl, int lvl_len,
							int root_idx, int *rlvl_len)
{
	int *rlvl;

	// alloc
	rlvl = (int *)malloc(in_len * sizeof(int));
	if (!rlvl)
		return NULL;

	// fill in the level order array for right subtree
	*rlvl_len = 0;
	for (int i = 1; i < lvl_len; i++) {
		for (int j = root_idx + 1; j < in_len; j++) {
			if (lvl[i] == in[j]) {
				rlvl[(*rlvl_len)++] = lvl[i];
			}
		}
	}

	return rlvl;
}


/**
 * Create each subtree based on left and right subtrees level order traversal.
 *
 * Ex:
 * 	in[]  =  {4, 8, 10, 12, 14, 20, 22};
 * 	lvl[] = {20, 8, 22, 4, 12, 10, 14};
 *
 * 	- in level order traversal, the root is always the first
 * 	- in in-order traversal, left subtree is always left side of root and right
 * 	subtree is always right side of root.
 *
 * 	Root = 20;
 *
 * 	llvl[] = {8, 4, 12, 10, 14}
 * 	rlvl[] = {20}
 *
 * 	And so on until the tree is created.
 */
static
binary_tree_node *__binary_tree_by_traversals(int *in, int in_len, int *lvl,
											int lvl_len)
{
	binary_tree_node *node;
	int *llvl, *rlvl;					// arrays of left/right level order
	int root_idx, llvl_len, rlvl_len;	// length of above arrays

	// stop recursion if level order list is empty
	if (!lvl_len)
		return NULL;

	// create current root node (first element in level order list)
	node = binary_tree_node_create(lvl[0]);
	assert(node);

	// find root index in in-order traversal
	root_idx = -1;
	for (int i = 0; i < in_len; i++) {
		if (in[i] == lvl[0]) {
			root_idx = i; break;
		}
	}
	//
	assert(root_idx != -1);

	// create left subtree level order traversal
	llvl = __left_tree_lvl_order(in, in_len, lvl, lvl_len, root_idx, &llvl_len);

	// create right subtree level order traversal
	rlvl = __right_tree_lvl_order(in, in_len, lvl, lvl_len, root_idx, &rlvl_len);

	// recursion to create the subtrees
	node->left = __binary_tree_by_traversals(in, in_len, llvl, llvl_len);
	node->right = __binary_tree_by_traversals(in, in_len, rlvl, rlvl_len);

	// free memory
	free(llvl);
	free(rlvl);

	return node;
}


/**
 * Create a binary tree based on in- order traversal list and level order
 * traversal list.
 *
 * @in		: In order traversal list.
 * @in_len	: In order traversal list len.
 * @lvl		: Level order traversal list.
 * @lvl_len	: Level order traversal list len.
 *
 * Return binary tree root node on success and false otherwise.
 */
binary_tree_node *binary_tree_by_traversals(int *in, int in_len, int *lvl,
											int lvl_len)
{
	bool validation;

	// validate traversals (same len and same elements)
	if (in_len != lvl_len)
		return NULL;

	//
	for (int i = 0; i < in_len; i++) {
		validation = false;
		for (int j = 0; j < lvl_len; j++) {
			if (in[i] == lvl[j]) {
				validation = true;
				break;
			}
		}

		if (!validation)
			return NULL;
	}

	return __binary_tree_by_traversals(in, in_len, lvl, lvl_len);
}


/*****************************************************************************/

/**
 * Get the sum of all nodes for a tree
 */
static
int __binary_tree_sum(binary_tree_node *root)
{
	if (!root)
		return 0;

	return (__binary_tree_sum(root->left) + __binary_tree_sum(root->right) +
			root->data);
}

/**
 * Check if a given binary tree is a sum tree. A binary tree is sum tree if the
 * value of the root is equal to the sum of left subtree and right subtree
 * nodes values.
 *
 * @root: Binary tree root.
 *
 * Return true if the binary tree is a sum tree and false otherwise.
 */
bool binary_tree_is_sum_tree(binary_tree_node *root)
{
	int left_sum, right_sum;

	// empty nodes are considered to be sumtree
	if (!root)
		return true;

	// do not compute calculation for leafs
	if (binary_tree_node_is_leaf(root))
		return true;

	// compute left and right sum for current node
	left_sum = __binary_tree_sum(root->left);
	right_sum = __binary_tree_sum(root->right);

	return (root->data == left_sum + right_sum &&
		binary_tree_is_sum_tree(root->left) &&
		binary_tree_is_sum_tree(root->right));
}


/*****************************************************************************/

/**
 * Check if two node are cousins (are on the same level and have different
 * parents).
 *
 * Two nodes are cousins if the parents are different and are on the same
 * level.
 *
 * @root: Binary tree root.
 * @a	: First node.
 * @b	: Second node.
 *
 * Return true if node a and b are cousins and false otherwise.
 */
bool binary_tree_cousins(binary_tree_node *root, int a, int b)
{
	unsigned int size;
	binary_tree_node *crt;
	queue_t *queue = NULL;
	binary_tree_node *parent_a = NULL;
	binary_tree_node *parent_b = NULL;

	//
	if (!root)
		return false;

	// create queue
	queue = queue_create(128);
	assert(queue);

	// add root to queue
	assert(!queue_enqueue(queue, root));

	while (!queue_is_empty(queue)) {

		// loop to dequeue all nodes on the same level
		size = queue_size(queue);
		for (int i = 0; i < size; i++) {
			crt = queue_dequeue(queue);
			assert(crt);

			// add children in queue and check parents
			if (crt->left) {
				if (crt->left->data == a)
					parent_a = crt;

				if (crt->left->data == b)
					parent_b = crt;

				assert(!queue_enqueue(queue, crt->left));
			}

			if (crt->right) {
				if (crt->right->data == a)
					parent_a = crt;

				if (crt->right->data == b)
					parent_b = crt;

				assert(!queue_enqueue(queue, crt->right));
			}
		}

		// if any parent is found, stop iteration
		if (parent_a || parent_b)
			break;
	}

	// destroy queue
	queue_destroy(queue);

	return (parent_a && parent_b && (parent_a != parent_b));
}


/*****************************************************************************/

/**
 * Check if by removing an edge in a binary tree, it can be divided in two
 * halves (two trees with same number of nodes).
 *
 * @root: Binary tree root.
 *
 * Return true if binary tree can be divided in two halves by removing an
 * edge and false otherwise.
 */
bool binary_tree_divide_two_halves(binary_tree_node *root)
{
	queue_t *queue = NULL;
	int tree_size, subtree_size;

	//
	if (!root)
		return false;

	// get size of entire tree
	tree_size = binary_tree_size(root);

	//
	queue = queue_create(128);
	assert(queue);

	// add root to queue
	assert(!queue_enqueue(queue, root));

	// check if length of each subtree is half the size of total tree
	while (!queue_is_empty(queue)) {
		root = queue_dequeue(queue);
		assert(root);

		//
		subtree_size = binary_tree_size(root);

		// two halves condition
		if (subtree_size == tree_size - subtree_size)
			return true;

		//
		if (root->left)
			assert(!queue_enqueue(queue, root->left));

		if (root->right)
			assert(!queue_enqueue(queue, root->right));
	}

	// destroy queue
	queue_destroy(queue);

	return false;
}


/*****************************************************************************/

static
bool __binary_tree_is_perfect(binary_tree_node *root, int tree_depth,
							int crt_depth)
{

	// validate leaf level
	if (binary_tree_node_is_leaf(root))
		return tree_depth == crt_depth;

	// internal nodes must have two children
	if (!root->left || !root->right)
		return false;

	return (__binary_tree_is_perfect(root->left, tree_depth, crt_depth + 1) &&
			__binary_tree_is_perfect(root->right, tree_depth, crt_depth + 1));
}


/**
 * Check if a binary tree is a perfect tree. A perfect tree is a tree where
 * all internal nodes have two children and all leaves are on the same
 * level.
 *
 * If h is the height (number of nodes from root to leaves) than the tree must
 * have 2^h - 1 nodes.
 *
 * @root: Binary tree root.
 *
 * Return true if the binary tree is perfect and false otherwise.
 */
bool binary_tree_is_perfect(binary_tree_node *root)
{

	//
	if (!root)
		return false;

	return __binary_tree_is_perfect(root, binary_tree_depth_iterative(root), 1);
}
