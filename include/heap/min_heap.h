/**
 * Min-Heap data structure implementation.
 * Copyright (C) 2025 Lazar Razvan.
 */

#ifndef MIN_HEAP_H
#define MIN_HEAP_H

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
typedef struct min_heap_s {

	//
	void						**data;		// data buffer
	int							size;		// data buffer current size
	int							capacity;	// data buffer capacity

	//
	alloc_fn					_alloc;		// data allocation function
	free_fn						_free;		// data free function
	cmp_fn						_cmp;		// data compare function
	print_fn					_print;		// data print function

} min_heap_t;


/*****************************************************************************/

//
min_heap_t *min_heap_init(int capacity, alloc_fn _alloc, free_fn _free,
						cmp_fn _cmp, print_fn _print);
void min_heap_destroy(min_heap_t *h);


/*****************************************************************************/

//
int min_heap_insert(min_heap_t *h, void *data);
int min_heap_delete(min_heap_t *h);
void *min_heap_peek(min_heap_t *h);


/*****************************************************************************/

//
void min_heap_print(min_heap_t *h);

#endif	// MIN_HEAP_H
