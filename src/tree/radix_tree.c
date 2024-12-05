/**
 * Radix tree data structure implementation.
 * Copyright (C) 2024 Lazar Razvan.
 *
 * Example of byte-based radix tree (2^8 = 256) that splits keys in byte
 * boundaries useful for strings.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>

#include "tree/radix_tree.h"

/*****************************************************************************/

#define PADDING					"                                             "


/*****************************************************************************/

//
// PREFIX
//

/**
 * Compute longest common prefix for two strings.
 */
static inline int
__common_prefix(char *s1, char *s2)
{
	int i = 0;

	//
	assert(s1 && s2);

	//
	while (s1[i] && s2[i] && s1[i] == s2[i])
		i++;

	return i;
}


/*****************************************************************************/

//
// NODE
//

/**
 * Get number of children for a given node.
 */
static inline uint16_t
__node_children(radix_tree_node_t *node)
{
	uint16_t cnt = 0;

	if (!node)
		return 0;

	for (int i = 0; i < RADIX; i++) {
		if (node->children[i])
			cnt++;
	}

	return cnt;
}

/**
 * Create a node in radix tree.
 */
static inline radix_tree_node_t *
__node_create(char *prefix, void *data, alloc_fn _alloc)
{
	radix_tree_node_t *node = NULL;

	//
	node = (radix_tree_node_t *)malloc(sizeof(radix_tree_node_t));
	if (!node)
		goto error;

	//
	node->data = NULL;
	node->prefix = NULL;
	memset(node->children, 0, sizeof(node->children));

	//
	if (prefix) {
		node->prefix = strdup(prefix);
		if (!node->prefix)
			goto node_free;
	}

	//
	if (data) {
		node->data = _alloc(data);
		if (!node->data)
			goto prefix_free;
	}

	return node;

prefix_free:
	free(node->prefix);
node_free:
	free(node);
error:
	return NULL;
}

/**
 * Free a node from radix tree.
 */
static inline void
__node_destroy(radix_tree_node_t *node, free_fn _free)
{
	if (!node)
		return;

	//
	_free(node->data);
	free(node->prefix);
	free(node);
}

/**
 * Remove obosolete paths for a node.
 */
static inline void
__node_cleanup(radix_tree_node_t *node, free_fn _free)
{
	radix_tree_node_t *it;

	if (!node)
		return;

	for (int i = 0; i < RADIX; i++) {
		it = node->children[i];
		if (!it)
			continue;

		if (__node_children(it) == 0 && !it->data) {
			node->children[i] = NULL;
			__node_destroy(it, _free);
		}
	}
}

/**
 * Split a node in radix tree based on prefix len.
 */
static inline int
__node_split(radix_tree_node_t *node, int prefix_len, alloc_fn _alloc)
{
	int child_index;
	radix_tree_node_t *child_node;

	//
	assert(node->prefix);
	assert(strlen(node->prefix) > prefix_len);

	// child index
	child_index = (unsigned char)*(node->prefix + prefix_len);

	// create child node
	child_node = __node_create(node->prefix + prefix_len, NULL, _alloc);
	if (!child_node)
		return -1;

	// move data from current node to the child node
	child_node->data = node->data;

	// move children from current node to the child node
	memcpy(child_node->children, node->children, sizeof(node->children));

	// reset current node (prefix and children)
	node->data = NULL;
	node->prefix[prefix_len] = '\0';
	memset(node->children, 0, sizeof(node->children));

	// add child node
	node->children[child_index] = child_node;

	return 0;
}

/**
 * Merge node in radix tree (only if it has one children and both node and
 * his child are not user defined nodes).
 */
static inline int
__node_merge(radix_tree_node_t *node, free_fn _free)
{
	int i;
	radix_tree_node_t *child = NULL;

	//
	if (__node_children(node) != 1 || node->data)
		return 0;

	//
	for (i = 0; i < RADIX; i++) {
		if (node->children[i]) {
			child = node->children[i];
			break;
		}
	}

	//
	if (child->data)
		return 0;

	// update prefix
	node->prefix = (char *)realloc(node->prefix, strlen(node->prefix) +
												strlen(child->prefix) + 1);
	if (!node->prefix)
		return -1;

	strcat(node->prefix, child->prefix);

	// inherit children of the child
	memcpy(node->children, child->children, sizeof(child->children));

	// free child node
	__node_destroy(child, _free);

	return 0;
}


/*****************************************************************************/

//
// HELPERS
//

/**
 * Insert a node in radix tree.
 */
static inline int
__radix_tree_insert(radix_tree_node_t *node, char *key, void *data,
					alloc_fn _alloc)
{
	int index, prefix_len;
	radix_tree_node_t *new_node, *it;

	//
	it = node;

	//
	while (*key) {
		index = (unsigned char)*key;

		// path not found (create node with remaining key as prefix)
		if (!it->children[index]) {
			new_node = __node_create(key, data, _alloc);
			if (!new_node)
				return -1;

			it->children[index] = new_node;
			// success
			goto finish;
		}

		//
		it = it->children[index];

		// compare prefix
		prefix_len = __common_prefix(it->prefix, key);
		assert(prefix_len);

		// shorther prefix, split current node
		if (prefix_len < strlen(it->prefix)) {
			if (__node_split(it, prefix_len, _alloc))
				return -2;
		}

		// node already found, set data if empty and return error otherwise
		if (prefix_len == strlen(key)) {
			if (it->data)
				return -3;	// already existing

			// node become user defined node
			it->data = _alloc(data);
			if (!it->data)
				return -3;

			return 0;
		}

		// longer prefix, continue
		key += prefix_len;
	}

finish:
	return 0;
}

/**
 * Remove a node from radix tree. It can only be removed if it is not an
 * intermediate node (data is set).
 */
static inline int
__radix_tree_remove(radix_tree_node_t *node, char *key, free_fn _free)
{
	int index, prefix_len;
	radix_tree_node_t *it;

	//
	index = (unsigned char)*key;
	it = node->children[index];

	// path not found
	if (!it)
		return -1;

	//
	prefix_len = __common_prefix(it->prefix, key);
	assert(prefix_len);

	// shorther prefix, error
	if (prefix_len < strlen(it->prefix))
		return -2;

	// node found
	if (prefix_len == strlen(key)) {
		// intermediate node, error
		if (!it->data)
			return -3;

		//
		if (__node_children(it) == 0) {
			// no children, remove it from tree and try compacting the tree
			node->children[index] = NULL;
			__node_destroy(it, _free);

			goto compact_tree;
		} else {
			// there are children, only free data
			_free(it->data);
			it->data = NULL;

			goto finish;
		}
	}

	// continue recursion
	if (__radix_tree_remove(it, key + prefix_len, _free))
		return -4;

compact_tree:
	// cleanup empty paths in root (not user defined nodes)
	if (!node->prefix) {
		__node_cleanup(node, _free);

		goto finish;
	}

	return __node_merge(node, _free);

finish:
	return 0;
}

/**
 * Key lookup in radix tree.
 */
static inline void *
__radix_tree_lookup(radix_tree_node_t *node, char *key)
{
	int index, prefix_len;
	radix_tree_node_t *it;

	//
	it = node;

	//
	while (*key) {
		index = (unsigned char)*key;

		// key not found
		if (!it->children[index])
			return NULL;

		//
		it = it->children[index];
		prefix_len = __common_prefix(it->prefix, key);
		assert(prefix_len);

		// shorter prefix, key not found
		if (prefix_len < strlen(it->prefix))
			return NULL;

		//
		if (prefix_len == strlen(key))
			return it->data;

		// continue
		key += prefix_len;
	}

	return NULL;
}

/**
 * Free memory for radix tree.
 */
static inline void
__radix_tree_destroy(radix_tree_node_t *node, free_fn _free)
{
	if (!node)
		return;

	//
	for (int i = 0; i < RADIX; i++) {
		if (node->children[i])
			__radix_tree_destroy(node->children[i], _free);
	}

	//
	__node_destroy(node, _free);
}

/**
 * Dump radix tree.
 */
static inline void
__radix_tree_dump(radix_tree_node_t *node, print_fn _print, int level)
{
	if (!node)
		return;

	//
	printf("%.*s%s(", level, PADDING, node->prefix);
	_print(node->data);
	printf(")\n");

	//
	for (int i = 0; i < RADIX; i++) {
		if (!node->children[i])
			continue;

		__radix_tree_dump(node->children[i], _print, level + strlen(node->prefix));
	}
}


/*****************************************************************************/

//
// PUBLIC API
//

/**
 * Initialize a radix tree.
 *
 * @_alloc	: Data allocation function.
 * @_print	: Data print function.
 * @_free	: Data free function.
 *
 * Return radix tree pointer on success and NULL on error.
 */
radix_tree_t *radix_tree_init(alloc_fn _alloc, print_fn _print,
							free_fn _free)
{
	radix_tree_t *tree = NULL;

	//
	if (!_alloc || !_print || !_free)
		goto error;

	//
	tree = (radix_tree_t *)malloc(sizeof(radix_tree_t));
	if (!tree)
		goto error;

	//
	tree->root = __node_create(NULL, NULL, _alloc);
	if (!tree->root)
		goto tree_free;

	//
	tree->_alloc = _alloc;
	tree->_print = _print;
	tree->_free = _free;

	return tree;

tree_free:
	free(tree);
error:
	return NULL;
}

/**
 * Destroy radix tree.
 *
 * @tree: Radix tree data structure.
 */
void radix_tree_destroy(radix_tree_t *tree)
{
	if (!tree)
		return;

	//
	assert(tree->_free);
	__radix_tree_destroy(tree->root, tree->_free);

	//
	free(tree);
}

/**
 * Create a node in radix tree.
 *
 * @key: Key to be added in the radix tree.
 *
 * Return 0 on success and <0 otherwise.
 */
int radix_tree_insert(radix_tree_t *tree, char *key, void *data)
{
	if (!tree || !tree->root || !tree->_alloc || !key || !strlen(key) || !data)
		return -1;

	return __radix_tree_insert(tree->root, key, data, tree->_alloc);
}

/**
 * Remove a node from radix tree.
 *
 * @key: Key to be added in the radix tree.
 *
 * Return 0 on success and <0 otherwise.
 */
int radix_tree_remove(radix_tree_t *tree, char *key)
{
	if (!tree || !tree->root || !tree->_free || !key || !strlen(key))
		return -1;

	return __radix_tree_remove(tree->root, key, tree->_free);
}

/**
 * Check the tree is empty (no other node than root).
 *
 * Return true if empty and false otherwise.
 */
bool radix_tree_is_empty(radix_tree_t *tree)
{
	if (!tree || !tree->root)
		return false;

	return __node_children(tree->root) == 0;
}

/**
 * Lookup for a key in radix tree.
 *
 * @key: Lookup key.
 *
 * Return key data if found and NULL otherwise.
 */
void *radix_tree_lookup(radix_tree_t *tree, char *key)
{
	if (!tree || !tree->root || !key || !strlen(key))
		return NULL;

	return __radix_tree_lookup(tree->root, key);
}

/**
 * Dump entries in a radix tree.
 */
void radix_tree_dump(radix_tree_t *tree)
{
	if (!tree || !tree->root || !tree->_print)
		return;

	for (int i = 0; i < RADIX; i++)
		__radix_tree_dump(tree->root->children[i], tree->_print, 0);
}
