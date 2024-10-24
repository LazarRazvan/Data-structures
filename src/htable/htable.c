/**
 * Generic singly linked list implementation.
 * Copyright (C) 2023 Lazar Razvan.
 */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "htable/htable.h"


/******************************************************************************/

/**
 * HTABLE
 *
 * kdlist_head_t         htable_node_t
 *
 * |  | --------------> |prev|htable_node_t|next| -> |prev|htable_node_t|next|
 *  --
 * |  | --------------> |prev|htable_node_t|next| -> |prev|htable_node_t|next|
 *  --
 * |  | --------------> |prev|htable_node_t|next| -> |prev|htable_node_t|next|
 *  --
 * |  | --------------> |prev|htable_node_t|next| -> |prev|htable_node_t|next|
 *  --
 *
 */

/******************************************************************************/

typedef struct htable_node_s {

	void			*key;		// htable entry key
	void			*value;		// htable entry value
	kdlist_node_t	node;		// htable linked list node

} htable_node_t;


/**
 * Alloc memory for a new node in hash table.
 */
static htable_node_t *__htable_node_create(void *key, void *value,
										alloc_key_cb alloc_key,
										free_key_cb free_key,
										alloc_value_cb alloc_value)
{
	htable_node_t *node;

	//
	node = malloc(sizeof(htable_node_t));
	if (!node)
		goto error;

	//
	node->key = alloc_key(key);
	if (!node->key)
		goto free_node;

	//
	node->value = alloc_value(value);
	if (!node->value)
		goto free_key;

	return node;

free_key:
	free_key(node->key);
free_node:
	free(node);
error:
	return NULL;
}

/**
 * Destroy memory for a node in hash table.
 */
static void __htable_node_destroy(htable_node_t *node, free_key_cb free_key,
								free_value_cb free_value)
{
	if (!node)
		return;

	//
	free_key(node->key);
	free_value(node->value);

	//
	free(node);
}


/******************************** PUBLIC API **********************************/

/**
 * Create a hash table.
 *
 * @capacity: Hash table capacity.
 * @hash	: Hash function.
 *
 * Return new allocated hash table on success and false otherwise.
 */
htable_t *htable_create(uint32_t capacity, hash_cb hash, cmp_cb cmp,
						alloc_key_cb alloc_key, free_key_cb free_key,
						alloc_value_cb alloc_value, free_value_cb free_value)
{
	htable_t *htable;

	//
	if (!hash || !cmp || !alloc_key || !free_key || !alloc_value || !free_value)
		goto error;

	//
	htable = malloc(sizeof(htable_t));
	if (!htable)
		goto error;

	//
	htable->buckets = malloc(capacity * sizeof(kdlist_head_t));
	if (!htable->buckets)
		goto free_htable;

	for (int i = 0; i < capacity; i++)
		kdlist_head_init(&htable->buckets[i]);

	//
	htable->size = 0;
	htable->capacity = capacity;

	//
	htable->__hash = hash;
	htable->__cmp = cmp;
	htable->__alloc_key = alloc_key;
	htable->__free_key = free_key;
	htable->__alloc_value = alloc_value;
	htable->__free_value = free_value;

	return htable;

free_htable:
	free(htable);
error:
	return NULL;
}

/**
 * Destroy (free) a hash table.
 */
void htable_destroy(htable_t *htable)
{
	htable_node_t *node;
	kdlist_node_t *it, *aux;
	kdlist_head_t *bucket_list;

	//
	if (!htable)
		return;

	for (int i = 0; i < htable->capacity; i++) {
		bucket_list = &htable->buckets[i];

		kdlist_for_each_safe(it, aux, bucket_list) {
			node = container_of(it, htable_node_t, node);
			__htable_node_destroy(node, htable->__free_key,
								htable->__free_value);
		}
	}

	//
	free(htable->buckets);

	//
	free(htable);
}


/*****************************************************************************/

/**
 * Insert an entry into hashtable.
 *
 * @htable	: Hash table data structure.
 * @key		: Hash table entry key.
 * @value	: Hash table entry value.
 *
 * Return 0 on success and <0 otherwise.
 */
int htable_insert(htable_t *htable, void *key, void *value)
{
	int rv = 0;
	uint32_t htable_idx;
	htable_node_t *node;

	// validate input
	if (!htable || !key || !value) {
		rv = -1; goto finish;
	}

	// validate available memory left
	if (htable->size == htable->capacity) {
		rv = -2; goto finish;
	}

	// compute and validate hash index
	htable_idx = htable->__hash(key);
	if (htable_idx >= htable->capacity) {
		rv = -3; goto finish;
	}

	// create new node for bucket list
	node = __htable_node_create(key, value, htable->__alloc_key,
								htable->__free_key,
								htable->__alloc_value);
	if (!node) {
		rv = -4; goto finish;
	}

	// insert data to bucket (if collision, add at the end of the list)
	kdlist_push_tail(&htable->buckets[htable_idx], &node->node);

finish:
	return rv;
}

/**
 * Delete an entry from hashtable.
 *
 * @htable	: Hash table data structure.
 * @key		: Hash table entry key to be deleted.
 *
 * Return 0 on success and <0 otherwise.
 */
int htable_delete(htable_t *htable, void *key)
{
	int rv = 0;
	uint32_t htable_idx;
	htable_node_t *node;
	kdlist_node_t *it, *aux;
	kdlist_head_t *bucket_list;

	// validate input
	if (!htable || !key) {
		rv = -1; goto finish;
	}

	// compute and validate hash index
	htable_idx = htable->__hash(key);
	if (htable_idx >= htable->capacity) {
		rv = -2; goto finish;
	}

	bucket_list = &htable->buckets[htable_idx];

	// empty buckets list
	if (kdlist_is_empty(bucket_list)) {
		rv = -3; goto finish;
	}

	// lookup into buckets list
	kdlist_for_each_safe(it, aux, bucket_list) {
		node = container_of(it, htable_node_t, node);

		if (htable->__cmp(key, node->key) == 0) {
			//
			it->next->prev = it->prev;
			it->prev->next = it->next;
			//
			__htable_node_destroy(node, htable->__free_key,
								htable->__free_value);
		}
	}

// not found
	rv = -4;

finish:
	return rv;
}


/*****************************************************************************/


/**
 * Print current entries in hashtable.
 *
 * @htable	: Hash table data structure.
 * @pkey	: Custom function for printing keys.
 * @pval	: Custom function for printing values.
 */
void htable_print(htable_t *htable, print_key_cb pkey, print_key_cb pval)
{
	htable_node_t *node;
	kdlist_node_t *it, *aux;
	kdlist_head_t *bucket_list;

	//
	if (!htable)
		return;

	//
	printf("Capacity: %u\n", htable->capacity);
	printf("Size    : %u\n", htable->size);

	//
	for (int i = 0; i < htable->capacity; i++) {
		bucket_list = &htable->buckets[i];
		//
		printf("    Bucket id: %d\n", i);

		//
		if (kdlist_is_empty(bucket_list)) {
			printf("Empty\n");
			continue;
		}

		kdlist_for_each_safe(it, aux, bucket_list) {
			node = container_of(it, htable_node_t, node);

			//
			pkey(node->key);
			pval(node->value);

		}
	}
}
