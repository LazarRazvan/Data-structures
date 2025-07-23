/**
 * POC implementation of barrier mechanism.
 * Copyright (C) 2025 Lazar Razvan.
 */

#ifndef BARRIER_H
#define BARRIER_H

#include <stdint.h>
#include <stdatomic.h>

#include "utils.h"


/*****************************************************************************/

/**
 * Barrier Synchronization Mechanism
 * ----------------------------------
 * This is a lightweight, reusable thread barrier implementation using atomic
 * operations. A barrier is a synchronization primitive that blocks
 * participating threads until all have reached the barrier point, at which
 * time they are all released to continue.
 *
 * - `size`: Total number of threads that must call `barrier_wait()` before unblocking.
 * - `count`: Atomic counter incremented by each thread arriving at the barrier.
 * - `state`: Atomic phase counter used to distinguish between successive barrier rounds.
 *
 *
 * The barrier is safe for multiple reuse cycles. Cache line alignment is
 * applied to avoid false sharing. This implementation is suitable for
 * low-latency, non-blocking synchronization in tightly looped multithreaded
 * workloads.
 */


/*****************************************************************************/

//
// CONFIG
//
#define DBG_ENABLE					1
#define ERR_ENABLE					1


/*****************************************************************************/

//
#define CACHE_LINE_SIZE				64


/*****************************************************************************/

//
typedef struct barrier_s {

	int						size;
	atomic_int				count;
	atomic_int				state;

} __attribute__((aligned(CACHE_LINE_SIZE))) barrier_t;


/*****************************************************************************/

// Create/Destroy
barrier_t *barrier_create(int size);
void barrier_destroy(barrier_t *barrier);

// Wait
void barrier_wait(barrier_t *barrier);

#endif	// BARRIER_H

