/**
 * POC implementation of read-write lock mechanism.
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

#include "synchronization/rwlock.h"


/*****************************************************************************/

/**
 * Create a new read-write lock.
 *
 * Return the lock on success or NULL on error.
 */
rwlock_t *rwlock_create(void)
{
	rwlock_t *rwlock = NULL;

	//
	if (posix_memalign((void **)&rwlock, CACHE_LINE_SIZE, (sizeof(rwlock_t))))
		goto finish;

	//
	atomic_store_explicit(&rwlock->cnt, 0, memory_order_release);
	atomic_store_explicit(&rwlock->writers, 0, memory_order_release);

finish:
	return rwlock;
}


/**
 * Free memory for a read-write lock.
 */
void rwlock_destroy(rwlock_t *rwlock)
{
	free(rwlock);
}


/*****************************************************************************/

/**
 * Wait and acquire a read lock.
 *
 * @rwlock	: Read-write lock.
 */
void rwlock_read_lock(rwlock_t *rwlock)
{
	int c;

	while (1) {
		// prevent new reader if there is a wating writer
		if (atomic_load_explicit(&rwlock->writers, memory_order_acquire)) {
			sched_yield();
			continue;
		}

		do {
			// wait for writer to finish
			while ((c = atomic_load_explicit(&rwlock->cnt, memory_order_acquire)) < 0)
				sched_yield();
		} while (!atomic_compare_exchange_weak(&rwlock->cnt, &c, c + 1));

		break;
	}
}

/**
 * Try to acquire a read lock.
 *
 * @rwlock	: Read-write lock.
 */
int rwlock_read_trylock(rwlock_t *rwlock)
{
	int c;

	// stop if any writer is waiting
	if (atomic_load_explicit(&rwlock->writers, memory_order_acquire))
		return -1;

	do {
		// stop if any writer holds the lock
		c = atomic_load_explicit(&rwlock->cnt, memory_order_acquire);
		if (c < 0)
			return -1;
	} while (!atomic_compare_exchange_weak(&rwlock->cnt, &c, c + 1));

	return 0;
}

/**
 * Release a read lock.
 *
 * @rwlock	: Read-write lock.
 */
void rwlock_read_unlock(rwlock_t *rwlock)
{
	atomic_fetch_sub(&rwlock->cnt, 1);
}


/*****************************************************************************/

/**
 * Wait and acquire a write lock.
 *
 * @rwlock	: Read-write lock.
 */
void rwlock_write_lock(rwlock_t *rwlock)
{
	int x = 0;

	// new waiting writer
	atomic_fetch_add(&rwlock->writers, 1);

	// wait for readers to finish
	while (!atomic_compare_exchange_weak(&rwlock->cnt, &x, -1)) {
		x = 0;
		sched_yield();
	}

	// waiting writer serviced
	atomic_fetch_sub(&rwlock->writers, 1);
}

/**
 * Try to acquire a write lock.
 *
 * @rwlock	: Read-write lock.
 */
int rwlock_write_trylock(rwlock_t *rwlock)
{
	int x = 0;

	// new waiting writer
	atomic_fetch_add(&rwlock->writers, 1);

	// writers already waiting
	if (atomic_load_explicit(&rwlock->writers, memory_order_acquire)) {
		atomic_fetch_sub(&rwlock->writers, 1);
		return -1;
	}

	// readers in progress
	if (!atomic_compare_exchange_strong(&rwlock->cnt, &x, -1)) {
		atomic_fetch_sub(&rwlock->writers, 1);
		return -1;
	}

	atomic_fetch_sub(&rwlock->writers, 1);
	return 0;
}

/**
 * Release a write lock.
 *
 * @rwlock	: Read-write lock.
 */
void rwlock_write_unlock(rwlock_t *rwlock)
{
	atomic_store_explicit(&rwlock->cnt, 0, memory_order_release);
}
