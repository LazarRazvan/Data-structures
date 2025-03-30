/**
 * Max-Heap data structure implementation.
 * Copyright (C) 2025 Lazar Razvan.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "heap/max_heap.h"

/*****************************************************************************/

#define PARENT_IDX(x)				(((x) - 1) / 2)
#define LEFT_IDX(x)					((2 * (x)) + 1)
#define RIGHT_IDX(x)				((2 * (x)) + 2)


/*****************************************************************************/

static inline bool __is_full(max_heap_t *h)
{
	return h->size == h->capacity;
}

static inline bool __is_empty(max_heap_t *h)
{
	return h->size == 0;
}

static inline void __swap(void **buf, int idx1, int idx2)
{
	void *tmp;

	//
	tmp = buf[idx1];
	buf[idx1] = buf[idx2];
	buf[idx2] = tmp;
}

static inline void __heapify_up(void **heap, cmp_fn heap_cmp, int idx)
{
	int pidx;

	// loop until parent is larger than current node
	pidx = PARENT_IDX(idx);

	while (idx > 0 && heap_cmp(heap[pidx], heap[idx]) < 0) {
		__swap(heap, pidx, idx);
		idx = pidx;
		pidx = PARENT_IDX(idx);
	}
}

static inline void __heapify_down(void **heap, cmp_fn heap_cmp, int idx, int size)
{
	int lidx, ridx, tmp;

	// loop until curent node (root) is larger than both children
	while (1) {
		lidx = LEFT_IDX(idx);
		ridx = RIGHT_IDX(idx);

		// stop if root is leaf
		if ((lidx >= size) && (ridx >= size))
			break;

		// compute largest node between node and its children
		tmp = idx;
		if ((lidx < size) && (heap_cmp(heap[tmp], heap[lidx]) < 0))
			tmp = lidx;

		if ((ridx < size) && (heap_cmp(heap[tmp], heap[ridx]) < 0))
			tmp = ridx;

		// stop if node is already larger than its children
		if (tmp == idx)
			break;

		// swap and continue
		__swap(heap, idx, tmp);
		idx = tmp;
	}
}


/*****************************************************************************/

/**
 * Create a max-heap entry.
 *
 * @size	: Max-Heap maximum size.
 * @_alloc	: Function to alloc data.
 * @_free	: Function to free data.
 * @_cmp	: Function to compare data.
 * @_print	: Function to print data.
 *
 * Return new allocated entry on success and false otherwise.
 */
max_heap_t *max_heap_init(int capacity, alloc_fn _alloc, free_fn _free,
						cmp_fn _cmp, print_fn _print)
{
	max_heap_t *entry = NULL;

	// validation
	if (capacity < 0 || !_alloc || !_free || !_cmp || !_print)
		goto error;

	// max-heap object
	entry = (max_heap_t *)malloc(sizeof(max_heap_t));
	if (!entry)
		goto error;

	// max-heap data
	entry->data = (void **)malloc(capacity * sizeof(void *));
	if (!entry->data)
		goto free_entry;

	// max-heap metadata
	entry->size		= 0;
	entry->capacity	= capacity;
	entry->_alloc	= _alloc;
	entry->_free	= _free;
	entry->_cmp		= _cmp;
	entry->_print	= _print;

//success:
	return entry;

free_entry:
	free(entry);
error:
	return NULL;
}

/**
 * Destroy (free memory) for a max-heap.
 */
void max_heap_destroy(max_heap_t *h)
{
	if (!h)
		return;

	//
	for (int i = 0; i < h->size; i++)
		h->_free(h->data[i]);

	//
	free(h->data);
	free(h);
}


/*****************************************************************************/

/**
 * Add new entry to max-heap.
 *
 * @h	: Max-Heap object.
 * @data: Data to be added to max-heap.
 *
 * Return 0 on success and <0 on error.
 */
int max_heap_insert(max_heap_t *h, void *data)
{
	void **heap;
	void *new_data;
	cmp_fn heap_cmp;
	int rv = 0, idx;
	alloc_fn heap_alloc;

	// validation
	if (!h || !data || __is_full(h)) {
		rv = -1; goto finish;
	}

	// init
	heap		= h->data;
	heap_cmp	= h->_cmp;
	heap_alloc	= h->_alloc;

	// data is added as last node
	new_data = heap_alloc(data);
	if (!new_data) {
		rv = -2; goto finish;
	}

	idx = h->size++;
	heap[idx] = new_data;

	// ensure max-heap properties
	__heapify_up(heap, heap_cmp, idx);

finish:
	return rv;
}

/**
 * Delete root from max-heap.
 *
 * @h	: Max-Heap object.
 *
 * Return 0 on success and <0 on error.
 */
int max_heap_delete(max_heap_t *h)
{
	int rv = 0;
	void **heap;
	cmp_fn heap_cmp;
	free_fn heap_free;

	// validation
	if (!h || __is_empty(h)) {
		rv = -1; goto finish;
	}

	// init
	heap		= h->data;
	heap_cmp	= h->_cmp;
	heap_free	= h->_free;

	// swap root with last node (and remove the last node)
	__swap(heap, 0, --h->size);
	heap_free(heap[h->size]);

	// ensure max-heap properties
	__heapify_down(heap, heap_cmp, 0, h->size);

finish:
	return rv;
}

/**
 * Peek top element from max-heap (smallest).
 *
 * @h	: Max-Heap object.
 *
 * Return object on success and NULL otherwise.
 */
void *max_heap_peek(max_heap_t *h)
{
	// validation
	if (!h || __is_empty(h))
		return NULL;

	return h->data[0];
}


/*****************************************************************************/

/**
 * Print max-heap.
 *
 * @h	: Max-Heap object.
 */
void max_heap_print(max_heap_t *h)
{
	// validation
	if (!h)
		return;

	//
	printf("heap_size     : %d\n", h->size);
	printf("heap_capacity : %d\n", h->capacity);

	//
	for (int i = 0; i < h->size; i++)
		h->_print(h->data[i]);
}
