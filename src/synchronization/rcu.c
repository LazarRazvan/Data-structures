/**
 * POC implementation of rcu (read-copy-update) mechanism.
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

#include "synchronization/rcu.h"


/*****************************************************************************/

/**
 * Create a rcu mechanism context.
 *
 * Return context on success or NULL on error.
 */
rcu_ctx_t *rcu_create(void)
{
	rcu_ctx_t *rcu = NULL;

	//
	// ensure CACHE_LINE_SIZE align to prevent false sharing
	if (posix_memalign((void **)&rcu, CACHE_LINE_SIZE, (sizeof(rcu_ctx_t))))
		goto finish;

	//
	rcu->size = 0;
	pthread_mutex_init(&rcu->lock, NULL);
	//
	kslist_head_init(&rcu->cbs);
	//
	memset(rcu->threads_idx, 0, MAX_THREADS * sizeof(pthread_t));
	memset(rcu->threads_cnt, 0, MAX_THREADS * sizeof(thread_counter_t));

finish:
	return rcu;
}


/**
 * Free memort for a rcu mechanism context.
 */
void rcu_destroy(rcu_ctx_t *rcu)
{
	rcu_node_t *_node;
	kslist_head_t *_it, *_tmp;

	//
	if (kslist_is_empty(&rcu->cbs))
		goto free_rcu;

	//
	kslist_for_each_safe(_tmp, _it, &rcu->cbs) {
		_node = kslist_entry(_tmp, rcu_node_t, node);
		_node->fn(_node->ptr);

		free(_node);
	}

free_rcu:
	free(rcu);
}


/*****************************************************************************/

/**
 * Register thread to current rcu context.
 *
 * @rcu			: Rcu context.
 *
 * Return slot_id to be used in subsequent calls.
 */
int rcu_register_thread(rcu_ctx_t *rcu)
{
	int id;
	pthread_t self;

	//
	self = pthread_self();

	//
	pthread_mutex_lock(&rcu->lock);

	for (int i = 0; i < rcu->size; i++) {
		if (pthread_equal(rcu->threads_idx[i], self)) {
			pthread_mutex_unlock(&rcu->lock);
			return i;
		}
	}

	id = rcu->size;
	rcu->threads_idx[id] = self;
	rcu->size++;

	//
	pthread_mutex_unlock(&rcu->lock);

	return id;
}



/*****************************************************************************/

/**
 * Mark reader start section.
 *
 * @rcu			: Rcu context.
 * @thread_id	: Reader id.
 */
void rcu_read_lock(rcu_ctx_t *rcu, int thread_id)
{
#if DBG_ENABLE
	assert(thread_id >= 0 && thread_id < rcu->size);
#endif

	atomic_fetch_add(&rcu->threads_cnt[thread_id].cnt, 1);
}


/**
 * Mark reader end section.
 *
 * @rcu			: Rcu context.
 * @thread_id	: Reader id.
 */
void rcu_read_unlock(rcu_ctx_t *rcu, int thread_id)
{
#if DBG_ENABLE
	assert(thread_id >= 0 && thread_id < rcu->size);
#endif

	atomic_fetch_sub(&rcu->threads_cnt[thread_id].cnt, 1);
}


/*****************************************************************************/

/**
 * Write pointer.
 *
 * @ptr		: Pointer to update.
 * @new_ptr	: New updated pointer.
 */
void rcu_assign_pointer(_Atomic(void *) *ptr, void *new_ptr)
{
	atomic_store_explicit(ptr, new_ptr, memory_order_release);
}


/**
 * Read pointer.
 *
 * @ptr		: Pointer to read.
 * @new_ptr	: New updated pointer.
 */
void *rcu_dereference(_Atomic(void *) *ptr)
{
	return atomic_load_explicit(ptr, memory_order_acquire);
}


/*****************************************************************************/

/**
 * Writer section to wait for all reader.
 *
 * @rcu			: Rcu context.
 */
void rcu_synchronize(rcu_ctx_t *rcu)
{
	int i;
	bool any_reader;

	while (1) {
		any_reader = false;

		for (i = 0; i < rcu->size; i++) {
			if (atomic_load(&rcu->threads_cnt[i].cnt) > 0) {
				any_reader = true; break;
			}
		}

		if (!any_reader)
			break;

		sched_yield();
	}
}


/*****************************************************************************/

/**
 * Collect unused objects.
 *
 * @rcu	: Rcu context.
 */
void rcu_call(rcu_ctx_t *rcu, rcu_callback_t fn, void *ptr)
{
	rcu_node_t *node;

	//
	node = malloc(sizeof(rcu_node_t));
	if (!node)
		return;

	//
	node->fn	= fn;
	node->ptr	= ptr;

	//
	pthread_mutex_lock(&rcu->lock);
	kslist_push_head(&rcu->cbs, &node->node);
	pthread_mutex_unlock(&rcu->lock);
}


/*****************************************************************************/

/**
 * Cleanup unused objects.
 *
 * @rcu	: Rcu context.
 */
void rcu_cleanup(rcu_ctx_t *rcu)
{
	rcu_node_t *_node;
	kslist_head_t *_it, *_tmp;

	//
	pthread_mutex_lock(&rcu->lock);
	_it = kslist_get_head(&rcu->cbs);
	kslist_head_init(&rcu->cbs);;
	pthread_mutex_unlock(&rcu->lock);

	//
	_tmp = _it;
	while (_it) {
		_it = _it->next;

		//
		_node = kslist_entry(_tmp, rcu_node_t, node);
		_node->fn(_node->ptr);

		//
		free(_node);
		_tmp = _it;
	}
}
