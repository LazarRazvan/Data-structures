/**
 * POC implementation of rcu (read-copy-update) mechanism.
 * Copyright (C) 2025 Lazar Razvan.
 */

#ifndef RCU_H
#define RCU_H

#include <stdint.h>
#include <pthread.h>
#include <stdatomic.h>

#include "utils.h"
#include "list/ksingly_linked_list.h"


/*****************************************************************************/

/**
 * Read-Copy-Update (RCU) Implementation
 * -------------------------------------
 * A userspace-friendly RCU implementation that supports safe concurrent access
 * to shared data by multiple readers with minimal synchronization overhead.
 *
 * RCU allows:
 *   - Lockless reads: multiple readers can access shared memory concurrently
 *   - Deferred reclamation: writers safely update or free memory after grace
 *   periods
 *
 * Core Components:
 *   - `rcu_ctx_t`: Per-domain RCU context holding thread states and callbacks
 *   - `rcu_register_thread()`: Registers a thread with the current RCU context
 *   - `rcu_read_lock()/rcu_read_unlock()`: Marks the read-side critical section
 *   - `rcu_call()`: Schedules a callback to be called after all readers finish
 *   - `rcu_synchronize()`: Waits for an RCU grace period to complete
 *   - `rcu_assign_pointer()` / `rcu_dereference()`: Safe publish/consume APIs
 *
 * Internals:
 *   - Readers are tracked via per-thread atomic counters aligned to cache lines
 *   - Writer waits for all counters to reach 0 before invoking deferred callbacks
 *   - Cache line alignment prevents false sharing between threads updating counters
 *   - A lock protects the callback list for scheduled deferred operations
 *
 * Requirements:
 *   - Requires C11 or equivalent atomic operations support
 *   - Threads must register with `rcu_register_thread()` before using RCU
 *
 * Use Case:
 *   - Shared read-mostly data structures (lists, trees, configs, etc.)
 *   - Replacement for mutexes where low-latency reads are critical
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

#define RCU_THREAD_PADDING			(CACHE_LINE_SIZE - (sizeof(int32_t)))

//
typedef void (*rcu_callback_t)(void *ptr);

//
typedef struct rcu_node_s {

	rcu_callback_t			fn;
	void					*ptr;
	kslist_node_t			node;

} rcu_node_t;


//
typedef struct thread_counter_s {

	atomic_int				cnt;
	char					_pad[RCU_THREAD_PADDING];

} __attribute__((aligned(CACHE_LINE_SIZE))) thread_counter_t;


//
typedef struct rcu_ctx_s {

	//
	int						size;
	pthread_mutex_t			lock;

	//
	kslist_head_t			cbs;

	//
	pthread_t				threads_idx[MAX_THREADS];
	thread_counter_t		threads_cnt[MAX_THREADS];

} rcu_ctx_t;


/*****************************************************************************/

// Create/Destroy
rcu_ctx_t *rcu_create(void);
void rcu_destroy(rcu_ctx_t *rcu);

// Register thread
int rcu_register_thread(rcu_ctx_t *rcu);

// Reader enter/exit
void rcu_read_lock(rcu_ctx_t *rcu, int thread_id);
void rcu_read_unlock(rcu_ctx_t *rcu, int thread_id);

// Read/Write
void rcu_assign_pointer(_Atomic(void *) *ptr, void *new_ptr);
void *rcu_dereference(_Atomic(void *) *ptr);

// Synchronize writer
void rcu_synchronize(rcu_ctx_t *rcu);

// Collect free objects
void rcu_call(rcu_ctx_t *rcu, rcu_callback_t fn, void *ptr);

// Cleanup free objects
void rcu_cleanup(rcu_ctx_t *rcu);


#endif	// RCU_H
