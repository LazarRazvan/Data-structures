/**
 * Ring buffer implementation.
 * Copyright (C) 2024 Lazar Razvan.
 *
 * Note that this implementation it is concurrency-unsafe.
 */

#ifndef RING_BUFFER_H
#define RING_BUFFER_H


#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "ring_buffer/rbuffer_config.h"


/*****************************************************************************/

typedef struct ring_buffer_s {

	data_t 				data[RING_BUFFER_CAPACITY];		// rbuffer memory
	uint32_t			head;							// rbuffer head
	uint32_t			tail;							// rbuffer tail
	uint32_t			size;							// rbuffer current size

} ring_buffer_t;


/*****************************************************************************/

typedef void (*ring_buffer_print_cb)(data_t *);


/*****************************************************************************/

// initialize ring buffer
#define RING_BUFFER_INIT\
	{					\
		.head = 0,		\
		.tail = 0,		\
		.size = 0,		\
	};

// declare and initialize head
#define RING_BUFFER(name)	\
	ring_buffer_t name = RING_BUFFER_INIT(data)


/*****************************************************************************/

static inline bool ring_buffer_is_full(ring_buffer_t *r)
{
	return (r->size == RING_BUFFER_CAPACITY);
}

static inline bool ring_buffer_is_empty(ring_buffer_t *r)
{
	return (r->size == 0);
}


/*****************************************************************************/

//
void ring_buffer_init(ring_buffer_t *r);

//
bool ring_buffer_push(ring_buffer_t *r, data_t *data);
bool ring_buffer_pop(ring_buffer_t *r, data_t *data);
void ring_buffer_print(ring_buffer_t *r, ring_buffer_print_cb cb);


#endif	// RING_BUFFER_H

