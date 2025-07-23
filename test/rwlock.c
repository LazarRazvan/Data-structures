/**
 * read-write lock mechanism test.
 * Copyright (C) 2025 Lazar Razvan.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

#include "synchronization/rwlock.h"


/*****************************************************************************/

#define NUM_READERS						5
#define NUM_WRITERS						2
#define READS_PER_THREAD				10
#define WRITES_PER_THREAD				5


/*****************************************************************************/

static rwlock_t *rwlock;
static int shared_data = 0;


/*****************************************************************************/

void *reader_thread(void *arg)
{
	int id = (int)(intptr_t)arg;

	for (int i = 0; i < READS_PER_THREAD; ++i) {
		rwlock_read_lock(rwlock);
		printf("[Reader %d] Read shared_data = %d\n", id, shared_data);
		rwlock_read_unlock(rwlock);
		usleep(10000); // simulate some processing
	}

	return NULL;
}

void *writer_thread(void *arg)
{
	int id = (int)(intptr_t)arg;

	for (int i = 0; i < WRITES_PER_THREAD; ++i) {
		rwlock_write_lock(rwlock);
		shared_data += 1;
		printf("[Writer %d] Wrote shared_data = %d\n", id, shared_data);
		rwlock_write_unlock(rwlock);
		usleep(20000); // simulate some processing
	}

	return NULL;
}


/*****************************************************************************/

int main() {
	pthread_t readers[NUM_READERS];
	pthread_t writers[NUM_WRITERS];

	rwlock = rwlock_create();
	if (!rwlock) {
		perror("rwlock_create failed");
		return 1;
	}

	for (int i = 0; i < NUM_READERS; ++i) {
		pthread_create(&readers[i], NULL, reader_thread, (void *)(intptr_t)i);
	}

	for (int i = 0; i < NUM_WRITERS; ++i) {
		pthread_create(&writers[i], NULL, writer_thread, (void *)(intptr_t)i);
	}

	for (int i = 0; i < NUM_READERS; ++i) {
		pthread_join(readers[i], NULL);
	}

	for (int i = 0; i < NUM_WRITERS; ++i) {
		pthread_join(writers[i], NULL);
	}

	rwlock_destroy(rwlock);
	return 0;
}
