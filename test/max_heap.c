/**
 * Max-Heap structure test using integers.
 * Copyright (C) 2025 Lazar Razvan.
 */

#include <time.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "heap/max_heap.h"


/*****************************************************************************/

// custom avl tree function operations
static void *__max_heap_alloc(void *data)
{
	void *d;

	d = malloc(1 * sizeof(int));
	if (!d)
		return NULL;

	*(int *)d = *(int *)data;

	return d;
}

static void __max_heap_free(void *data)
{
	free(data);
}

static int __max_heap_cmp(void *data1, void *data2)
{
	int val1 = *(int *)data1;
	int val2 = *(int *)data2;

	return val1 - val2;
}

static void __max_heap_print(void *data)
{
	int val = *(int *)data;

	printf("%d\n", val);
}


/*****************************************************************************/

int main()
{
	int *peek_val, last, max, rv;
	int values[] = {20, 5, 15, 10, 30, 2, 8};
	const int size = sizeof(values) / sizeof(values[0]);

	// Create Min-Heap
	max_heap_t *heap = max_heap_init(16, __max_heap_alloc, __max_heap_free,
									__max_heap_cmp, __max_heap_print);
	if (!heap) {
		fprintf(stderr, "Failed to init heap!\n");
		goto finish;
	}

	// Test insertion
	for (int i = 0; i < size; i++) {
		rv = max_heap_insert(heap, &values[i]);
		if (rv) {
			fprintf(stderr, "insert of %d failed (%d)\n", values[i], rv);
			goto free_heap;
		}

		// check minimum
		max = values[0];
		for (int j = 1; j <= i; j++) {
			if (values[j] > max)
				max = values[j];
		}

		peek_val = (int *)max_heap_peek(heap);
		if (!peek_val) {
			fprintf(stderr, "peek on insert failed\n");
			goto free_heap;
		}

		//
		if (*peek_val != max) {
			fprintf(stderr, "peek (%d) is not maximum (%d)\n", *peek_val, max);
			goto free_heap;
		}
	}

	// Print
	max_heap_print(heap);


	// Test deletion
	last = INT_MAX;
	for (int i = 0; i < size; i++) {
		peek_val = (int *)max_heap_peek(heap);
		if (!peek_val) {
			fprintf(stderr, "peek on insert failed\n");
			goto free_heap;
		}


		// ensure the peeked value is not less than the last (ascending)
		if (*peek_val > last) {
			fprintf(stderr, "peek (%d) smaller than previous (%d)\n", *peek_val,
					last);
			goto finish;
		}

		//
		last = *peek_val;

		// Delete top
		rv = max_heap_delete(heap);
		if (rv) {
			fprintf(stderr, "delete of %d failed (%d)\n", *peek_val, rv);
			goto finish;
		}
	}

free_heap:
	max_heap_destroy(heap);
finish:
	return 0;
}
