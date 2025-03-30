/**
 * Min-Heap structure test using integers.
 * Copyright (C) 2025 Lazar Razvan.
 */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "heap/min_heap.h"


/*****************************************************************************/

// custom avl tree function operations
static void *__min_heap_alloc(void *data)
{
	void *d;

	d = malloc(1 * sizeof(int));
	if (!d)
		return NULL;

	*(int *)d = *(int *)data;

	return d;
}

static void __min_heap_free(void *data)
{
	free(data);
}

static int __min_heap_cmp(void *data1, void *data2)
{
	int val1 = *(int *)data1;
	int val2 = *(int *)data2;

	return val1 - val2;
}

static void __min_heap_print(void *data)
{
	int val = *(int *)data;

	printf("%d\n", val);
}


/*****************************************************************************/

int main()
{
	int *peek_val, last, min, rv;
	int values[] = {20, 5, 15, 10, 30, 2, 8};
	const int size = sizeof(values) / sizeof(values[0]);

	// Create Min-Heap
	min_heap_t *heap = min_heap_init(16, __min_heap_alloc, __min_heap_free,
									__min_heap_cmp, __min_heap_print);
	if (!heap) {
		fprintf(stderr, "Failed to init heap!\n");
		goto finish;
	}

	// Test insertion
	for (int i = 0; i < size; i++) {
		rv = min_heap_insert(heap, &values[i]);
		if (rv) {
			fprintf(stderr, "insert of %d failed (%d)\n", values[i], rv);
			goto free_heap;
		}

		// check minimum
		min = values[0];
		for (int j = 1; j <= i; j++) {
			if (values[j] < min)
				min = values[j];
		}

		peek_val = (int *)min_heap_peek(heap);
		if (!peek_val) {
			fprintf(stderr, "peek on insert failed\n");
			goto free_heap;
		}

		//
		if (*peek_val != min) {
			fprintf(stderr, "peek (%d) is not minim (%d)\n", *peek_val, min);
			goto free_heap;
		}
	}

	// Print
	min_heap_print(heap);


	// Test deletion
	last = -1;
	for (int i = 0; i < size; i++) {
		peek_val = (int *)min_heap_peek(heap);
		if (!peek_val) {
			fprintf(stderr, "peek on insert failed\n");
			goto free_heap;
		}


		// ensure the peeked value is not less than the last (ascending)
		if (*peek_val < last) {
			fprintf(stderr, "peek (%d) larger than previous (%d)\n", *peek_val,
					last);
			goto finish;
		}

		//
		last = *peek_val;

		// Delete top
		rv = min_heap_delete(heap);
		if (rv) {
			fprintf(stderr, "delete of %d failed (%d)\n", *peek_val, rv);
			goto finish;
		}
	}

free_heap:
	min_heap_destroy(heap);
finish:
	return 0;
}
