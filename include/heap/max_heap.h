/**
 * Max-Heap data structure implementation.
 * Copyright (C) 2025 Lazar Razvan.
 */

#ifndef MAX_HEAP_H
#define MAX_HEAP_H

#include <stdint.h>
#include <stdbool.h>

/*****************************************************************************/

// min-heap function operations
typedef void* (*alloc_fn)(void *);
typedef void (*free_fn)(void *);
typedef int	(*cmp_fn)(void *, void *);
typedef void (*print_fn)(void *);


/*****************************************************************************/

// avl tree node
typedef struct max_heap_s {

	//
	void						**data;		// data buffer
	int							size;		// data buffer current size
	int							capacity;	// data buffer capacity

	//
	alloc_fn					_alloc;		// data allocation function
	free_fn						_free;		// data free function
	cmp_fn						_cmp;		// data compare function
	print_fn					_print;		// data print function

} max_heap_t;


/*****************************************************************************/

//
max_heap_t *max_heap_init(int capacity, alloc_fn _alloc, free_fn _free,
						cmp_fn _cmp, print_fn _print);
void max_heap_destroy(max_heap_t *h);


/*****************************************************************************/

//
int max_heap_insert(max_heap_t *h, void *data);
int max_heap_delete(max_heap_t *h);
void *max_heap_peek(max_heap_t *h);


/*****************************************************************************/

//
void max_heap_print(max_heap_t *h);

#endif	// MAX_HEAP_H
