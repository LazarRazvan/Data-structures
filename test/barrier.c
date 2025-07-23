#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "synchronization/barrier.h"

#define NUM_THREADS 6
#define NUM_PHASES 5

barrier_t *barrier;

void *thread_func(void *arg) {
    int id = (int)(intptr_t)arg;
    for (int phase = 0; phase < NUM_PHASES; ++phase) {
        printf("[Thread %d] Reached barrier at phase %d\n", id, phase);

        barrier_wait(barrier);  // Synchronization point

        printf("[Thread %d] Passed barrier at phase %d\n", id, phase);

        usleep(10000 + (rand() % 10000));  // simulate work
    }
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    srand(time(NULL));

    barrier = barrier_create(NUM_THREADS);
    if (!barrier) {
        fprintf(stderr, "Failed to create barrier\\n");
        return 1;
    }

    for (int i = 0; i < NUM_THREADS; ++i)
        pthread_create(&threads[i], NULL, thread_func, (void *)(intptr_t)i);

    for (int i = 0; i < NUM_THREADS; ++i)
        pthread_join(threads[i], NULL);

    barrier_destroy(barrier);
    return 0;
}
