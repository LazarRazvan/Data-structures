/**
 * Generic hash table implementation.
 * Copyright (C) 2024 Lazar Razvan.
 */

#ifndef HTABLE_H
#define HTABLE_H


#include <stdint.h>
#include <stdbool.h>

#include "list/kdoubly_linked_list.h"


/********************************* CALLBACKS *********************************/

//
typedef uint32_t (*hash_cb)(void *);
//
typedef int (*cmp_cb)(void *, void *);
//
typedef void * (*alloc_key_cb)(void *);
typedef void (*free_key_cb)(void *);
typedef void (*print_key_cb)(void *);
//
typedef void * (*alloc_value_cb)(void *);
typedef void (*free_value_cb)(void *);
typedef void (*print_value_cb)(void *);


/****************************** DATA STRUCTURE *******************************/

typedef struct htable_s {

	//
	uint32_t		capacity;		// total hash table capacity
	uint32_t		size;			// current size of hash table
	//
	kdlist_head_t	*buckets;		// hash table buckets
	//
	hash_cb			__hash;			// hash callback
	//
	cmp_cb			__cmp;			// compare callback
	//
	alloc_key_cb	__alloc_key;	// key allocation callback
	free_key_cb		__free_key;		// key free callback
	//
	alloc_value_cb	__alloc_value;	// value allocation callback
	free_value_cb	__free_value;	// value free callback

} htable_t;


/******************************** PUBLIC API *********************************/

//
htable_t *htable_create(uint32_t capacity, hash_cb hash, cmp_cb cmp,
						alloc_key_cb alloc_key, free_key_cb free_key,
						alloc_value_cb alloc_value, free_value_cb free_value);
void htable_destroy(htable_t *htable);

//
int htable_insert(htable_t *htable, void *key, void *value);
int htable_delete(htable_t *htable, void *key);

//
void htable_print(htable_t *htable, print_key_cb pkey, print_key_cb pval);

#endif	// HTABLE_H
