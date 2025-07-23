/**
 * RCU mechanism test.
 * Copyright (C) 2025 Lazar Razvan.
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdatomic.h>

#include "synchronization/rcu.h"


/*****************************************************************************/

#define NUM_READERS						16
#define NUM_WRITERS						4
#define ITERATIONS						100


/*****************************************************************************/

//
typedef struct {
	int									version;
	char								data[64];
} shared_object_t;

//
rcu_ctx_t *rcu_ctx;
_Atomic(shared_object_t *) shared_ptr = NULL;


/*****************************************************************************/

void cleanup_callback(void *ptr)
{
	shared_object_t *obj = (shared_object_t *)ptr;
	printf("[RCU CLEANUP] Freeing version: %d\n", obj->version);
	free(obj);
}


/*****************************************************************************/

void *reader_thread(void *arg)
{
	int thread_id;
	shared_object_t *obj;
	intptr_t id = (intptr_t)arg;

	// register thread
	thread_id = rcu_register_thread(rcu_ctx);

	for (int i = 0; i < ITERATIONS; ++i) {
		rcu_read_lock(rcu_ctx, thread_id);

		//
		obj = (shared_object_t *)rcu_dereference((_Atomic(void *) *)&shared_ptr);
		if (!obj) {
		//	printf("Thread %d read: null\n", thread_id);
			rcu_read_unlock(rcu_ctx, thread_id);
			usleep(10); // simulate work

			continue;
		}

		//printf("Thread %d read: version %d, data %s\n", thread_id, obj->version,
		//		obj->data);

		rcu_read_unlock(rcu_ctx, thread_id);
		usleep(10); // simulate work
	}

	printf("[READER %ld] Done.\n", id);
	return NULL;
}


/*****************************************************************************/

void *writer_thread(void *arg)
{
	intptr_t id = (intptr_t)arg;
	shared_object_t *new_obj, *old_obj;

	// register thread
	rcu_register_thread(rcu_ctx);

	for (int i = 0; i < ITERATIONS / 50; ++i) {
		new_obj = malloc(sizeof(shared_object_t));
		assert(new_obj);

		//
		new_obj->version = id * 1000 + i;
		snprintf(new_obj->data, sizeof(new_obj->data), "Writer %ld, Iteration %d", id, i);

		// make sure value is exchanged
		old_obj = (shared_object_t *)rcu_dereference((_Atomic(void *) *)&shared_ptr);

		do {
			//old_obj = atomic_load_explicit(&shared_ptr, memory_order_relaxed);
			old_obj = (shared_object_t *)rcu_dereference((_Atomic(void *) *)&shared_ptr);
		} while (!atomic_compare_exchange_weak_explicit(
			&shared_ptr,
			&old_obj,
			new_obj,
			memory_order_release,
			memory_order_relaxed));

		//
		if (old_obj) {
			rcu_call(rcu_ctx, cleanup_callback, old_obj);
		}

		//
		rcu_synchronize(rcu_ctx);

		usleep(100); // slower writes
	}

	printf("[WRITER %ld] Done.\n", id);
	return NULL;
}


/*****************************************************************************/

int main()
{
	pthread_t readers[NUM_READERS];
	pthread_t writers[NUM_WRITERS];

	rcu_ctx = rcu_create();

	for (intptr_t i = 0; i < NUM_READERS; ++i)
		pthread_create(&readers[i], NULL, reader_thread, (void *)i);

	for (intptr_t i = 0; i < NUM_WRITERS; ++i)
		pthread_create(&writers[i], NULL, writer_thread, (void *)i);

	for (int i = 0; i < NUM_READERS; ++i)
		pthread_join(readers[i], NULL);

	for (int i = 0; i < NUM_WRITERS; ++i)
		pthread_join(writers[i], NULL);

	rcu_synchronize(rcu_ctx);
	rcu_cleanup(rcu_ctx);
	rcu_destroy(rcu_ctx);

	free(shared_ptr);

	printf("[MAIN] All threads done. Test completed.\n");
	return 0;
}
