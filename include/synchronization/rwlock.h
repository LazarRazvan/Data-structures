/**
 * POC implementation of read-write lock mechanism.
 * Copyright (C) 2025 Lazar Razvan.
 */

#ifndef RWLOCK_H
#define RWLOCK_H

#include <stdint.h>
#include <stdatomic.h>

#include "utils.h"


/*****************************************************************************/

/**
 * Read-Write Lock (RWLock) Implementation
 * ---------------------------------------
 * This lightweight read-write lock allows multiple readers to hold the lock
 * concurrently, while writers are granted exclusive access. It is implemented
 * using two atomic integers:
 *
 *   - `cnt`: Represents the lock state. A positive value indicates the number
 *            of active readers; -1 indicates an active writer.
 *   - `writers`: A counter of threads waiting to acquire the write lock.
 *                This prevents writer starvation by blocking new readers when
 *                writers are pending.
 *
 * The implementation uses atomic operations and spin-waiting with `sched_yield()`
 * for thread coordination. The structure is aligned to cache line size to
 * prevent false sharing.
 *
 * Suitable for low-latency scenarios where critical sections are short and
 * spinning is acceptable. Does not use blocking or OS-level synchronization.
 */

/*****************************************************************************/

//
// CONFIG
//
#define DBG_ENABLE					1
#define ERR_ENABLE					1


/*****************************************************************************/

//
#define MAX_THREADS					64

//
#define CACHE_LINE_SIZE				64


/*****************************************************************************/

//
typedef struct rwlock_s {

	atomic_int				cnt;
	atomic_int				writers;

} __attribute__((aligned(CACHE_LINE_SIZE))) rwlock_t;


/*****************************************************************************/

// Create/Destroy
rwlock_t *rwlock_create(void);
void rwlock_destroy(rwlock_t *rwlock);

// Reader lock/unlock
void rwlock_read_lock(rwlock_t *rwlock);
int rwlock_read_trylock(rwlock_t *rwlock);
void rwlock_read_unlock(rwlock_t *rwlock);

// Writer lock/unlock
void rwlock_write_lock(rwlock_t *rwlock);
int rwlock_write_trylock(rwlock_t *rwlock);
void rwlock_write_unlock(rwlock_t *rwlock);

#endif	// RWLOCK_H
