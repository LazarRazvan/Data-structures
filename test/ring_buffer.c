/**
 * Ring buffer test.
 * Copyright (C) 2022 Lazar Razvan.
 *
 * Test for ring buffer implementation.
 */

#include <stdio.h>

#include "ring_buffer/rbuffer.h"


/*****************************************************************************/

#define RING_BUFFER_CAPACITY				4


/*****************************************************************************/

void my_print(data_t *data)
{
	printf("%d", *(int *)data);
}


/*****************************************************************************/

int main()
{
	int value;
	ring_buffer_t r;
	int test_arr[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

	// initialize ring buffer
	ring_buffer_init(&r);

	// try adding all elements in ring buffer
	for (int i = 0; i < 16; i++) {
		printf("Adding %d to ring buffer...\n", test_arr[i]);

		if (ring_buffer_push(&r, &test_arr[i]) == false)
			printf("ERROR!\n");
	}

	// print ring buffer
	ring_buffer_print(&r, my_print);

	// pop two element from ring buffer
	for (int i = 0; i < 2; i++) {
		printf("Extracting element from ring buffer ...\n");
		if (ring_buffer_pop(&r, &value) == false)
			printf("ERROR!\n");
		else
			printf("Element %d extracted!\n", value);
	}

	// print ring buffer
	ring_buffer_print(&r, my_print);

	// try adding all remaining elements
	for (int i = 4; i < 16; i++) {
		printf("Adding %d to ring buffer...\n", test_arr[i]);

		if (ring_buffer_push(&r, &test_arr[i]) == false)
			printf("ERROR!\n");
	}

	// print ring buffer
	ring_buffer_print(&r, my_print);

	// pop five elements from ring buffer
	for (int i = 0; i < 5; i++) {
		printf("Extracting element from ring buffer ...\n");
		if (ring_buffer_pop(&r, &value) == false)
			printf("ERROR!\n");
		else
			printf("Element %d extracted!\n", value);
	}

	// print ring buffer
	ring_buffer_print(&r, my_print);

// success
	return 0;
}
