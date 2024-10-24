/**
 * Buddy allocator test.
 * Copyright (C) 2024 Lazar Razvan.
 */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "allocator/buddy.h"


/*****************************************************************************/

static uint32_t __generate_random_size(uint32_t min, uint32_t max)
{
    return min + (rand() % (max - min + 1));
}

static uint32_t __generate_order_size(uint8_t order)
{
	uint32_t min, max;

	switch (order) {
	case 0:  // Order 0: < 1 page
		min = 1;
		max = PAGE_SIZE - 1;
		break;
	case 1:  // Order 2: between 1 and 2 pages
		min = PAGE_SIZE;
		max = 2 * PAGE_SIZE - 1;
		break;
	case 2:  // Order 2: between 2 and 4 pages
		min = 2 * PAGE_SIZE;
		max = 4 * PAGE_SIZE - 1;
		break;
	case 3:  // Order 3: between 4 and 8 pages
		min = 4 * PAGE_SIZE;
		max = 8 * PAGE_SIZE - 1;
		break;
	case 4:  // Order 4: between 8 and 16 pages
		min = 8 * PAGE_SIZE;
		max = 16 * PAGE_SIZE - 1;
		break;
	case 5:  // Order 5: between 16 and 32 pages
		min = 16 * PAGE_SIZE;
		max = 32 * PAGE_SIZE - 1;
		break;
	default:
		min = 1;
		max = PAGE_SIZE - 1;
		break;
	}

	return __generate_random_size(min, max);
}


/*****************************************************************************/

static void __buddy_max_slots_for_order(buddy_t *buddy, uint8_t order)
{
	void *arr[32];
	uint32_t size;

	//
	srand(time(NULL));

	// alloc
	for (int i = 0; i <= ((1 << (BUDDY_ORDERS - order)) - 1); i++) {
		size = __generate_order_size(order);

		arr[i] = buddy_alloc(buddy, size);
		if (!arr[i])
			goto failed;
	}

	// free
	for (int i = 0; i <= ((1 << (BUDDY_ORDERS - order)) - 1); i++)
		buddy_free(buddy, arr[i]);

//success:
    printf("Running max slots for order %u... SUCCESS\n", order);
	return;

failed:
    printf("Running max slots for order %u... FAILED\n", order);
	assert(0);
}

static void __buddy_exceed_slots_for_order(buddy_t *buddy, uint8_t order)
{
	void *xtra;
	void *arr[32];

	//
	srand(time(NULL));

	// alloc
	for (int i = 0; i <= ((1 << (BUDDY_ORDERS - order)) - 1); i++) {
		arr[i] = buddy_alloc(buddy, __generate_order_size(order));
		if (!arr[i])
			goto failed;
	}

	// extra alloc that should fail
	xtra = buddy_alloc(buddy, 1);
	if (xtra)
		goto failed;

	// free
	for (int i = 0; i <= ((1 << (BUDDY_ORDERS - order)) - 1); i++)
		buddy_free(buddy, arr[i]);

//success:
    printf("Running exceed slots for order %u... SUCCESS\n", order);
	return;

failed:
    printf("Running exceed slots for order %u... FAILED\n", order);
	assert(0);
}


/*****************************************************************************/

int main()
{
	buddy_t *buddy;

	// create buddy allocator
	buddy = buddy_init();
	if (!buddy)
		return -1;

	// test max slots
	for (int order = 0; order <= BUDDY_ORDERS; order++)
		__buddy_max_slots_for_order(buddy, order);

	// test exceed slots
	for (int order = 0; order <= BUDDY_ORDERS; order++)
		__buddy_exceed_slots_for_order(buddy, order);

	// free buddy allocator
	buddy_destroy(buddy);

	return 0;
}
