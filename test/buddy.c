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

static void
__buddy_exceed_slots_for_order(buddy_t *buddy, uint8_t order)
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
		if (buddy_free(buddy, arr[i]))
			goto failed;

//success:
    printf("[SUCCESS] Exceed slots for order %u\n", order);
	return;

failed:
    printf("[FAILED] Exceed slots for order %u\n", order);
	assert(0);
}

static void
__buddy_exceed_slots_custom_orders(buddy_t *buddy, uint32_t *size,
							uint8_t len)
{
	void *arr[32];
	void *xtra;

	// alloc
	for (int i = 0; i < len; i++) {
		arr[i] = buddy_alloc(buddy, size[i]);
		if (!arr[i]) {
			printf("Failed on size %u\n", size[i]);
			goto failed;
		}
	}

	// extra alloc that should fail
	xtra = buddy_alloc(buddy, 1);
	if (xtra) {
		printf("xtra = %p\n", xtra);
		goto failed;
	}

	// free
	for (int i = 0; i < len; i++)
		if (buddy_free(buddy, arr[i]))
			goto failed;

//success:
    printf("[SUCCESS] Exceed slots for config [");
	for (int i = 0; i < len; i++)
		printf("%u ", size[i]);

	printf("]\n");
	return;

failed:
    printf("[FAILED] Exceed slots for config [");
	for (int i = 0; i < len; i++)
		printf("%u ", size[i]);

	printf("]\n");
	assert(0);
}

static void
__buddy_invalid_free(buddy_t *buddy)
{
	void *addr;

	// 1 page allocation (order 0)
	addr = buddy_alloc(buddy, 1);
	if (!addr)
		goto failed;

	// free one page before starting address
	if (!buddy_free(buddy, addr - PAGE_SIZE))
		goto failed;

	// free inside page
	for (int i = 1; i < PAGE_SIZE; i++)
		if (!buddy_free(buddy, addr + i))
			goto failed;

	// free all other order 0 pages
	for (int i = 1; i < (1 << BUDDY_ORDERS); i++)
		if (!buddy_free(buddy, addr + i * PAGE_SIZE))
			goto failed;

	// free allocated address
	if (buddy_free(buddy, addr))
		goto failed;

	// double free
	if (!buddy_free(buddy, addr))
		goto failed;

// success:
	printf("[SUCCESS] Invalid free memory\n");
	return;

failed:
    printf("[FAILED] Invalid free memory\n");
	assert(0);
}

/*****************************************************************************/

int main()
{
	buddy_t *buddy;
	uint32_t size[32];

	//
	srand(time(NULL));

	// create buddy allocator
	buddy = buddy_init();
	if (!buddy)
		return -1;

	// test exceed slots
	for (int order = 0; order <= BUDDY_ORDERS; order++)
		__buddy_exceed_slots_for_order(buddy, order);

	// Pattern: [1, 1, 2, 1, 1, 2, ..] - Filling 32 pages exactly
	size[0] = __generate_order_size(0);  // 1 page
	size[1] = __generate_order_size(0);  // 1 page
	size[2] = __generate_order_size(1);  // 2 pages
	size[3] = __generate_order_size(0);  // 1 page
	size[4] = __generate_order_size(0);  // 1 page
	size[5] = __generate_order_size(1);  // 2 pages
	size[6] = __generate_order_size(0);  // 1 page
	size[7] = __generate_order_size(0);  // 1 page
	size[8] = __generate_order_size(1);  // 2 pages
	size[9] = __generate_order_size(0);  // 1 page
	size[10] = __generate_order_size(0); // 1 page
	size[11] = __generate_order_size(1); // 2 pages
	size[12] = __generate_order_size(0); // 1 page
	size[13] = __generate_order_size(0); // 1 page
	size[14] = __generate_order_size(1); // 2 pages
	size[15] = __generate_order_size(0); // 1 page
	size[16] = __generate_order_size(0); // 1 page
	size[17] = __generate_order_size(1); // 2 pages
	size[18] = __generate_order_size(0); // 1 page
	size[19] = __generate_order_size(0); // 1 page
	size[20] = __generate_order_size(1); // 2 pages
	size[21] = __generate_order_size(0); // 1 page
	size[22] = __generate_order_size(0); // 1 page
	size[23] = __generate_order_size(1); // 2 pages
	__buddy_exceed_slots_custom_orders(buddy, size, 24);

	// Pattern: [16, 8, 4, 2, 1] - Filling 32 pages
	size[0] = __generate_order_size(4);  // 16 pages
	size[1] = __generate_order_size(3);  // 8 pages
	size[2] = __generate_order_size(2);  // 4 pages
	size[3] = __generate_order_size(1);  // 2 pages
	size[4] = __generate_order_size(0);  // 1 page
	size[5] = __generate_order_size(0);  // 1 page
	__buddy_exceed_slots_custom_orders(buddy, size, 6);

	// Pattern: [8, 8, 4, 4, 2, 2, 1, 1, 1, 1] - 32 pages
	size[0] = __generate_order_size(3);  // 8 pages
	size[1] = __generate_order_size(3);  // 8 pages
	size[2] = __generate_order_size(2);  // 4 pages
	size[3] = __generate_order_size(2);  // 4 pages
	size[4] = __generate_order_size(1);  // 2 pages
	size[5] = __generate_order_size(1);  // 2 pages
	size[6] = __generate_order_size(0);  // 1 page
	size[7] = __generate_order_size(0);  // 1 page
	size[8] = __generate_order_size(0);  // 1 page
	size[9] = __generate_order_size(0);  // 1 page
	__buddy_exceed_slots_custom_orders(buddy, size, 10);

	// Pattern: [4, 4, 4, 4, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1] - 32 pages
	size[0] = __generate_order_size(2);  // 4 pages
	size[1] = __generate_order_size(2);  // 4 pages
	size[2] = __generate_order_size(2);  // 4 pages
	size[3] = __generate_order_size(2);  // 4 pages
	size[4] = __generate_order_size(1);  // 2 pages
	size[5] = __generate_order_size(1);  // 2 pages
	size[6] = __generate_order_size(1);  // 2 pages
	size[7] = __generate_order_size(1);  // 2 pages
	size[8] = __generate_order_size(0);  // 1 page
	size[9] = __generate_order_size(0);  // 1 page
	size[10] = __generate_order_size(0); // 1 page
	size[11] = __generate_order_size(0); // 1 page
	size[12] = __generate_order_size(0); // 1 page
	size[13] = __generate_order_size(0); // 1 page
	size[14] = __generate_order_size(0); // 1 page
	size[15] = __generate_order_size(0); // 1 page
	__buddy_exceed_slots_custom_orders(buddy, size, 16);

	// Pattern: [8, 2, 8, 2, 4, 1, 1, 4, 1, 1] - 32 pages total
	size[0] = __generate_order_size(3);  // 8 pages
	size[1] = __generate_order_size(1);  // 2 pages (forces split of an 8-page block)
	size[2] = __generate_order_size(3);  // 8 pages
	size[3] = __generate_order_size(1);  // 2 pages (split remaining 8-page block)
	size[4] = __generate_order_size(2);  // 4 pages (forces a 4-page split)
	size[5] = __generate_order_size(0);  // 1 page
	size[6] = __generate_order_size(0);  // 1 page (fills small slots from splits)
	size[7] = __generate_order_size(2);  // 4 pages
	size[8] = __generate_order_size(0);  // 1 page
	size[9] = __generate_order_size(0);  // 1 page
	__buddy_exceed_slots_custom_orders(buddy, size, 10);

	// Pattern: [8, 4, 8, 4, 2, 2, 1, 1, 1, 1] - 32 pages total
	size[0] = __generate_order_size(3);  // 8 pages
	size[1] = __generate_order_size(2);  // 4 pages (forces split)
	size[2] = __generate_order_size(3);  // 8 pages
	size[3] = __generate_order_size(2);  // 4 pages (another forced split)
	size[4] = __generate_order_size(1);  // 2 pages
	size[5] = __generate_order_size(1);  // 2 pages (splits final free 4-page block)
	size[6] = __generate_order_size(0);  // 1 page
	size[7] = __generate_order_size(0);  // 1 page
	size[8] = __generate_order_size(0);  // 1 page
	size[9] = __generate_order_size(0);  // 1 page
	__buddy_exceed_slots_custom_orders(buddy, size, 10);

	// Pattern: [1, 1, 2, 4, 4, 8, 8, 4] - 32 pages total
	size[0] = __generate_order_size(0);  // 1 page
	size[1] = __generate_order_size(0);  // 1 page
	size[2] = __generate_order_size(1);  // 2 pages
	size[3] = __generate_order_size(2);  // 4 pages
	size[4] = __generate_order_size(2);  // 4 pages
	size[5] = __generate_order_size(3);  // 8 pages
	size[6] = __generate_order_size(3);  // 8 pages
	size[7] = __generate_order_size(2);  // 4 pages
	__buddy_exceed_slots_custom_orders(buddy, size, 8);

	// invalid free
	__buddy_invalid_free(buddy);

	// free buddy allocator
	buddy_destroy(buddy);

	return 0;
}
