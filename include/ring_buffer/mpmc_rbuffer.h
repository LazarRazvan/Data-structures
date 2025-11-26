/**
 * Multiple consumer, multiple producer ring buffer implementation.
 * Copyright (C) 2025 Lazar Razvan.
 */

#ifndef MPMPC_RING_BUFFER_H
#define MPMC_RING_BUFFER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


/*****************************************************************************/

typedef struct mpmc_ring_buffer_s {

	void 				**data;							// rbuffer memory

	uint32_t			size;							// rbuffer size
	uint32_t			mask;							// rbuffer mask

	_Atomic uint32_t	p_head;							// rbuffer producer head
	_Atomic uint32_t	p_tail;							// rbuffer producer tail

	_Atomic uint32_t	c_head;							// rbuffer producer head
	_Atomic uint32_t	c_tail;							// rbuffer producer tail

} mpmc_ring_buffer_t;


/*****************************************************************************/

//
mpmc_ring_buffer_t *mpmc_ring_buffer_init(uint32_t size);
void mpmc_ring_buffer_deinit(mpmc_ring_buffer_t *r);

//
int mpmc_ring_buffer_enqueue_bulk(mpmc_ring_buffer_t *r, void **o, uint32_t n);
int mpmc_ring_buffer_dequeue_bulk(mpmc_ring_buffer_t *r, void **obj, uint32_t n);

#endif	// MPMC_RING_BUFFER_H

