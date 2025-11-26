/**
 * Multiple consumer, multiple producer ring buffer implementation.
 * Copyright (C) 2025 Lazar Razvan.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>

#include "ring_buffer/mpmc_rbuffer.h"


/*****************************************************************************/

static inline uint32_t __next_pow2(uint32_t size)
{
	if (size < 2)
		return 1;

	size--;

	size |= size >> 1;
	size |= size >> 2;
	size |= size >> 4;
	size |= size >> 8;
	size |= size >> 16;

	size++;

	return size;
}


/*****************************************************************************/

/**
 * Initialize a mpmc ring buffer.
 *
 * @size	: Ring buffer size.
 *
 * Return mpmc ring buffer on success or NULL otherwise.
 */
mpmc_ring_buffer_t *mpmc_ring_buffer_init(uint32_t size)
{
	//
	mpmc_ring_buffer_t *r = NULL;

	r = malloc(sizeof(mpmc_ring_buffer_t));
	if (!r)
		goto fail;

	// align size to power of two for fast wrap-around index
	r->size = __next_pow2(size);

	//
	r->data = malloc(r->size * sizeof(void *));
	if (!r->data)
		goto free_r;

	// init
	r->mask = r->size - 1;

	atomic_init(&r->p_head, 0);
	atomic_init(&r->p_tail, 0);
	atomic_init(&r->c_head, 0);
	atomic_init(&r->c_tail, 0);

//success
	return r;

free_r:
	free(r);
fail:
	return NULL;
}

/**
 * Free mpmc ring buffer.
 *
 * @r	: Ring buffer to be freed.
 */
void mpmc_ring_buffer_deinit(mpmc_ring_buffer_t *r)
{
	if (!r)
		return;

	free(r->data);
	free(r);

	r = NULL;
}


/*****************************************************************************/

/*
 * Enqueue an element to a ring buffer.
 *
 * @r	: Ring buffer.
 * @obj	: Ring buffer object to be added.
 *
 * Return 0 on success and !0 on error.
 */
int mpmc_ring_buffer_enqueue(mpmc_ring_buffer_t *r, void *obj)
{
	uint32_t p_head, p_tail, c_tail, s_free, p_next;

	// update producer head
	while (1) {
		//
		p_head = atomic_load_explicit(&r->p_head, memory_order_relaxed);
		c_tail = atomic_load_explicit(&r->c_tail, memory_order_acquire);

		// check free space (wrap around safe)
		s_free = r->size - (p_head - c_tail);
		if (!s_free)
			return -1;

		// update producer head
		p_next = p_head + 1;

		if (atomic_compare_exchange_weak_explicit(&r->p_head, &p_head, p_next,
												memory_order_acquire,
												memory_order_relaxed))
			break;
	}

	// store data
	r->data[p_head & r->mask] = obj;
	atomic_thread_fence(memory_order_release);

	// update producer tail
	while (1) {
		p_tail = atomic_load_explicit(&r->p_tail, memory_order_acquire);
		if (p_tail == p_head) {
			atomic_store_explicit(&r->p_tail, p_next, memory_order_release);
			break;
		}
	}

	return 0;
}

/*
 * Dequeue an element from a ring buffer.
 *
 * @r	: Ring buffer.
 * @obj	: Ring buffer object to filled on success.
 *
 * Return 0 on success and !0 on error.
 */
int mpmc_ring_buffer_dequeue(mpmc_ring_buffer_t *r, void **obj)
{
	uint32_t c_head, p_tail, s_avail, c_next, c_tail;

	// update consumer head
	while (1) {
		//
		c_head = atomic_load_explicit(&r->c_head, memory_order_relaxed);
		p_tail = atomic_load_explicit(&r->p_tail, memory_order_acquire);

		// check available data (wrap around safe)
		s_avail = p_tail - c_head;
		if (!s_avail)
			return -1;

		// update consumer head
		c_next = c_head + 1;

		if (atomic_compare_exchange_weak_explicit(&r->c_head, &c_head, c_next,
												memory_order_acquire,
												memory_order_relaxed))
			break;
	}

	// load data
	*obj = r->data[c_head & r->mask];

	// update consumer tail
	while (1) {
		c_tail = atomic_load_explicit(&r->c_tail, memory_order_acquire);
		if (c_tail == c_head) {
			atomic_store_explicit(&r->c_tail, c_next, memory_order_release);
			break;
		}
	}

	return 0;
}


/*****************************************************************************/

/*
 * Enqueue multiple elements to a ring buffer.
 *
 * @r	: Ring buffer.
 * @obj	: Ring buffer objects to be added.
 * @n	: Number of objects to be added.
 *
 * Return 0 on success and !0 on error.
 */
int mpmc_ring_buffer_enqueue_bulk(mpmc_ring_buffer_t *r, void **obj, uint32_t n)
{
	uint32_t p_head, p_tail, c_tail, s_free, p_next;

	// update producer head
	while (1) {
		//
		p_head = atomic_load_explicit(&r->p_head, memory_order_relaxed);
		c_tail = atomic_load_explicit(&r->c_tail, memory_order_acquire);

		// check free space (wrap around safe)
		s_free = r->size - (p_head - c_tail);
		if (s_free < n)
			return -1;

		// update producer head
		p_next = p_head + n;

		if (atomic_compare_exchange_weak_explicit(&r->p_head, &p_head, p_next,
												memory_order_acquire,
												memory_order_relaxed))
			break;
	}

	// store data
	for (uint32_t i = 0; i < n; i++)
		r->data[(p_head + i) & r->maks] = obj[i];

	atomic_thread_fence(memory_order_release);

	// update producer tail
	while (1) {
		p_tail = atomic_load_explicit(&r->p_tail, memory_order_acquire);
		if (p_tail == p_head) {
			atomic_store_explicit(&r->p_tail, p_next, memory_order_release);
			break;
		}
	}

	return 0;
}

/*
 * Dequeue multiple elements from a ring buffer.
 *
 * @r	: Ring buffer.
 * @obj	: Ring buffer objects to filled on success.
 * @n	: Number of objects to be removed.
 *
 * Return 0 on success and !0 on error.
 */
int mpmc_ring_buffer_dequeue_bulk(mpmc_ring_buffer_t *r, void **obj, uint32_t n)
{
	uint32_t c_head, p_tail, s_avail, c_next, c_tail;

	// update consumer head
	while (1) {
		//
		c_head = atomic_load_explicit(&r->c_head, memory_order_relaxed);
		p_tail = atomic_load_explicit(&r->p_tail, memory_order_acquire);

		// check available data (wrap around safe)
		s_avail = p_tail - c_head;
		if (s_avail < n)
			return -1;

		// update consumer head
		c_next = c_head + n;

		if (atomic_compare_exchange_weak_explicit(&r->c_head, &c_head, c_next,
												memory_order_acquire,
												memory_order_relaxed))
			break;
	}

	// load data
	for (uint32_t i = 0; i < n; i++)
		obj[i] = r->data[(c_head + i) & r->maks];

	// update consumer tail
	while (1) {
		c_tail = atomic_load_explicit(&r->c_tail, memory_order_acquire);
		if (c_tail == c_head) {
			atomic_store_explicit(&r->c_tail, c_next, memory_order_release);
			break;
		}
	}

	return 0;
}
