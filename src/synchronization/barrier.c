/**
 * POC implementation of barrier mechanism.
 * Copyright (C) 2025 Lazar Razvan.
 */

#include <sched.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "synchronization/barrier.h"


/*****************************************************************************/

/**
 * Create a new barrier.
 *
 * Return the barrier on success or NULL on error.
 */
barrier_t *barrier_create(int size)
{
	barrier_t *barrier = NULL;

	//
	if (posix_memalign((void **)&barrier, CACHE_LINE_SIZE, (sizeof(barrier_t))))
		goto finish;

	//
	barrier->size = size;
	//
	atomic_store_explicit(&barrier->count, 0, memory_order_release);
	atomic_store_explicit(&barrier->state, 0, memory_order_release);

finish:
	return barrier;
}


/**
 * Free memory for a barrier.
 */
void barrier_destroy(barrier_t *barrier)
{
	free(barrier);
}


/*****************************************************************************/

/**
 * Barrier wait.
 *
 * @barrier	: Barrier.
 */
void barrier_wait(barrier_t *barrier)
{
	int crt_idx, crt_state;

	// read current state
	crt_state = atomic_load_explicit(&barrier->state, memory_order_acquire);

	// per-thread increment
	crt_idx = atomic_fetch_add_explicit(&barrier->count, 1, memory_order_release);

	if (crt_idx == barrier->size - 1) {
		// last thread reset count and update state
		atomic_store_explicit(&barrier->count, 0, memory_order_release);
		atomic_fetch_add_explicit(&barrier->state, 1, memory_order_release);
	} else {
		// waiting for last thread
		while (atomic_load_explicit(&barrier->state, memory_order_acquire) == crt_state)
			sched_yield();
	}
}
