/**
 * Ring buffer implementation.
 * Copyright (C) 2023 Lazar Razvan.
 *
 * Note that this implementation it is concurrency-unsafe.
 */

#include <stdio.h>
#include <string.h>

#include "ring_buffer/rbuffer.h"


/*****************************************************************************/

/**
 * Initialize ring buffer.
 *
 * @r		: Ring buffer.
 * @data	: Ring buffer memory.
 * @capacity: Ring buffer capacity.
 */
void ring_buffer_init(ring_buffer_t *r)
{
	r->head = 0;
	r->tail = 0;
	r->size = 0;

	//
	memset(r->data, 0, RING_BUFFER_CAPACITY * sizeof(data_t));
}

/**
 * Push an entry to ring buffer.
 *
 * @r	: Ring buffer.
 * @data: Data to be added to ring buffer.
 *
 * Return true on success and false on error.
 */
bool ring_buffer_push(ring_buffer_t *r, data_t *data)
{
	if (ring_buffer_is_full(r))
		return false;

	//
	r->data[r->tail] = *data;

	//
	r->tail = (r->tail + 1) % RING_BUFFER_CAPACITY;
	r->size++;

	return true;
}

/**
 * Pop an entry from ring buffer.
 *
 * @r	: Ring buffer.
 * @data: Data to be retrieved from ring buffer.
 *
 * Return true on success and set data or NULL on error.
 */
bool ring_buffer_pop(ring_buffer_t *r, data_t *data)
{
	if (ring_buffer_is_empty(r))
		return NULL;

	//
	*data = r->data[r->head];

	//
	r->head = (r->head + 1) % RING_BUFFER_CAPACITY;
	r->size--;

	//
	return data;
}

/**
 * Print entries in a ring buffer.
 *
 * @r	: Ring buffer.
 * @cb	: Ring buffer data print.
 */
void ring_buffer_print(ring_buffer_t *r, ring_buffer_print_cb cb)
{
	uint32_t index;

	printf("RING BUFFER_PRINT:\n");

	//
	if (ring_buffer_is_empty(r)) {
		printf("EMPTY\n");
		return;
	}

	//
	for (int i = 0; i < r->size; i++) {
		index = (r->head + i) % RING_BUFFER_CAPACITY;
		printf("[%d]: ", index);
		cb(&r->data[index]);
		printf("\n");
	}
}

