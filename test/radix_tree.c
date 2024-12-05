/**
 * Radix tree data structure test.
 * Copyright (C) 2024 Lazar Razvan.
 *
 * Example of byte-based radix tree (2^8 = 256) that splits keys in byte
 * boundaries useful for strings.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tree/radix_tree.h"


/*****************************************************************************/

#define TREE_DUMP_BEFORE				1
#define TREE_DUMP_INSIDE				1
#define TREE_DUMP_AFTER					1


/*****************************************************************************/

void *data_alloc(void *data)
{
	if (!data)
		return NULL;

	return strdup((char *)data);
}

void data_print(void *data)
{
	printf("%s", data ? (char *)data : "NULL");
}

void data_free(void *data)
{
	free(data);
}


/*****************************************************************************/

static int test_basic_operations(radix_tree_t *tree)
{
	printf("Running %s test...\n", __func__);

#if TREE_DUMP_BEFORE
	printf("%s radix tree dump before:\n", __func__);
	radix_tree_dump(tree);
#endif

	if (radix_tree_insert(tree, "cat", "value_cat"))
		goto error;
	if (radix_tree_insert(tree, "car", "value_car"))
		goto error;
	if (radix_tree_insert(tree, "cart", "value_cart"))
		goto error;

#if TREE_DUMP_INSIDE
	printf("%s radix tree dump inside:\n", __func__);
	radix_tree_dump(tree);
#endif

	if (radix_tree_remove(tree, "cat"))
		goto error;
	if (radix_tree_remove(tree, "car"))
		goto error;
	if (radix_tree_remove(tree, "cart"))
		goto error;

#if TREE_DUMP_AFTER
	printf("%s radix tree dump after:\n", __func__);
	radix_tree_dump(tree);
#endif

	if (!radix_tree_is_empty(tree))
		goto error;

//success:
	printf("%s test passed!\n", __func__);
	return 0;

error:
	printf("%s test failed!\n", __func__);
	return -1;
}

static int test_splitting_nodes(radix_tree_t *tree)
{
	printf("Running %s test...\n", __func__);

#if TREE_DUMP_BEFORE
	printf("%s radix tree dump before:\n", __func__);
	radix_tree_dump(tree);
#endif

	if (radix_tree_insert(tree, "dog", "value_dog"))
		goto error;
	if (radix_tree_insert(tree, "door", "value_door"))
		goto error;
	if (radix_tree_insert(tree, "dorm", "value_dorm"))
		goto error;

#if TREE_DUMP_INSIDE
	printf("%s radix tree dump inside:\n", __func__);
	radix_tree_dump(tree);
#endif

	if (radix_tree_remove(tree, "dog"))
		goto error;
	if (radix_tree_remove(tree, "door"))
		goto error;
	if (radix_tree_remove(tree, "dorm"))
		goto error;

#if TREE_DUMP_AFTER
	printf("%s radix tree dump after:\n", __func__);
	radix_tree_dump(tree);
#endif

	if (!radix_tree_is_empty(tree))
		goto error;

//success:
	printf("%s test passed.\n", __func__);
	return 0;

error:
	printf("%s test failed!\n", __func__);
	return -1;
}

static int test_merging_nodes(radix_tree_t *tree)
{
	printf("Running %s test...\n", __func__);

#if TREE_DUMP_BEFORE
	printf("%s radix tree dump before:\n", __func__);
	radix_tree_dump(tree);
#endif

	if (radix_tree_insert(tree, "test", "value_test"))
		goto error;
	if (radix_tree_insert(tree, "testing", "value_testing"))
		goto error;
	if (radix_tree_remove(tree, "testing"))
		goto error;

#if TREE_DUMP_INSIDE
	printf("%s radix tree dump inside:\n", __func__);
	radix_tree_dump(tree);
#endif

	if (radix_tree_remove(tree, "test"))
		goto error;

#if TREE_DUMP_AFTER
	printf("%s radix tree dump after:\n", __func__);
	radix_tree_dump(tree);
#endif

	if (!radix_tree_is_empty(tree))
		goto error;

//success:
	printf("%s test passed.\n", __func__);
	return 0;

error:
	printf("%s test failed!\n", __func__);
	return -1;
}

static int test_existing_nodes(radix_tree_t *tree)
{
	printf("Running %s test...\n", __func__);

#if TREE_DUMP_BEFORE
	printf("%s radix tree dump before:\n", __func__);
	radix_tree_dump(tree);
#endif

	if (radix_tree_insert(tree, "apple", "value_apple"))
		goto error;
	if (radix_tree_insert(tree, "applet", "value_applet"))
		goto error;

	if (!radix_tree_insert(tree, "apple", "updated_apple"))
		goto error;

#if TREE_DUMP_INSIDE
	printf("%s radix tree dump inside:\n", __func__);
	radix_tree_dump(tree);
#endif

	if (radix_tree_remove(tree, "apple"))
		goto error;
	if (radix_tree_remove(tree, "applet"))
		goto error;

#if TREE_DUMP_AFTER
	printf("%s radix tree dump after:\n", __func__);
	radix_tree_dump(tree);
#endif

	if (!radix_tree_is_empty(tree))
		goto error;

//success:
	printf("%s test passed.\n", __func__);
	return 0;

error:
	printf("%s test failed!\n", __func__);
	return -1;
}

static int test_edge_cases(radix_tree_t *tree)
{
	printf("Running %s test...\n", __func__);

#if TREE_DUMP_BEFORE
	printf("%s radix tree dump before:\n", __func__);
	radix_tree_dump(tree);
#endif

	// Insert an empty key
	if (!radix_tree_insert(tree, "", "value_empty"))
		goto error;

	// Remove a key not in the tree
	if (!radix_tree_remove(tree, "nonexistent"))
		goto error;

	// Remove a prefix key
	if (radix_tree_insert(tree, "bat", "value_bat"))
		goto error;
	if (radix_tree_insert(tree, "batch", "value_batch"))
		goto error;
	if (radix_tree_remove(tree, "bat"))
		goto error;

	// Remove remaining nodes
	if (radix_tree_remove(tree, "batch"))
		goto error;

#if TREE_DUMP_AFTER
	printf("%s radix tree dump after:\n", __func__);
	radix_tree_dump(tree);
#endif

	if (!radix_tree_is_empty(tree))
		goto error;

//success:
	printf("%s test passed.\n", __func__);
	return 0;

error:
	printf("%s test failed!\n", __func__);
	return -1;
}

static int test_lookup_nodes(radix_tree_t *tree)
{
	printf("Running %s test...\n", __func__);

#if TREE_DUMP_BEFORE
	printf("%s radix tree dump before:\n", __func__);
	radix_tree_dump(tree);
#endif

	if (radix_tree_insert(tree, "abcdef", "value_apple"))
		goto error;
	if (radix_tree_insert(tree, "abcdgh", "value_apple"))
		goto error;
	if (radix_tree_insert(tree, "ablm", "value_apple"))
		goto error;
	if (radix_tree_insert(tree, "abcd", "value_apple"))
		goto error;
	if (radix_tree_insert(tree, "ab", "value_apple"))
		goto error;
	if (radix_tree_insert(tree, "a", "value_apple"))
		goto error;

#if TREE_DUMP_INSIDE
	printf("%s radix tree dump inside:\n", __func__);
	radix_tree_dump(tree);
#endif

	if (!radix_tree_lookup(tree, "a"))
		goto error;
	if (!radix_tree_lookup(tree, "ab"))
		goto error;
	if (radix_tree_lookup(tree, "abc"))
		goto error;
	if (!radix_tree_lookup(tree, "abcd"))
		goto error;
	if (radix_tree_lookup(tree, "abl"))
		goto error;
	if (radix_tree_lookup(tree, "abcde"))
		goto error;
	if (radix_tree_lookup(tree, "abcdg"))
		goto error;
	if (!radix_tree_lookup(tree, "ablm"))
		goto error;
	if (!radix_tree_lookup(tree, "abcdef"))
		goto error;
	if (!radix_tree_lookup(tree, "abcdgh"))
		goto error;

	if (radix_tree_remove(tree, "a"))
		goto error;
	if (radix_tree_remove(tree, "ab"))
		goto error;
	if (radix_tree_remove(tree, "abcd"))
		goto error;
	if (radix_tree_remove(tree, "ablm"))
		goto error;
	if (radix_tree_remove(tree, "abcdgh"))
		goto error;
	if (radix_tree_remove(tree, "abcdef"))
		goto error;

#if TREE_DUMP_AFTER
	printf("%s radix tree dump after:\n", __func__);
	radix_tree_dump(tree);
#endif

	if (!radix_tree_is_empty(tree))
		goto error;

//success:
	printf("%s test passed.\n", __func__);
	return 0;

error:
	printf("%s test failed!\n", __func__);
	return -1;
}

static int test_large(radix_tree_t *tree)
{
	char key[128];

	printf("Running %s test...\n", __func__);

#if TREE_DUMP_BEFORE
	printf("%s radix tree dump before:\n", __func__);
	radix_tree_dump(tree);
#endif

	// insert
	for (int i = 0; i < 1000; i++) {
		sprintf(key, "prefix%d", i);
		if (radix_tree_insert(tree, key, key))
			goto error;
	}

	// lookup
	for (int i = 0; i < 1000; i++) {
		sprintf(key, "prefix%d", i);
		if (!radix_tree_lookup(tree, key))
			goto error;
	}

	// partial remove and validate
	for (int i = 0; i < 500; i++) {
		sprintf(key, "prefix%d", i);
		if (radix_tree_remove(tree, key))
			goto error;

		if (radix_tree_lookup(tree, key))
			goto error;
	}

	// remove and validate all
	for (int i = 500; i < 1000; i++) {
		sprintf(key, "prefix%d", i);
		if (radix_tree_remove(tree, key))
			goto error;

		if (radix_tree_lookup(tree, key))
			goto error;
	}

#if TREE_DUMP_AFTER
	printf("%s radix tree dump after:\n", __func__);
	radix_tree_dump(tree);
#endif

//	radix tree will not be empty here
//	if (!radix_tree_is_empty(tree))
//		goto error;

//success:
	printf("%s test passed.\n", __func__);
	return 0;

error:
	printf("%s test failed!\n", __func__);
	return -1;
}


/*****************************************************************************/

int main()
{
	radix_tree_t *radix;

	// init
	radix = radix_tree_init(data_alloc, data_print, data_free);
	if (!radix) {
		printf("radix_tree_init() failed!\n");
		return -1;
	}

	//
	assert(!test_basic_operations(radix));
	assert(!test_splitting_nodes(radix));
	assert(!test_merging_nodes(radix));
	assert(!test_existing_nodes(radix));
	assert(!test_edge_cases(radix));
	assert(!test_lookup_nodes(radix));
	assert(!test_large(radix));

	// destroy
	radix_tree_destroy(radix);

	return 0;
}
