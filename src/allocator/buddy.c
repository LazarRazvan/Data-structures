/**
 * POC implementation of buddy system allocator.
 * Copyright (C) 2024 Lazar Razvan.
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

#include "allocator/buddy.h"


/*****************************************************************************/

/**
 * Calculate the buddy address for a block, relative to the allocator's start
 * address. Shifts `addr` relative to `start` before XORing to ensure the buddy
 * stays within the memory range. Without alignment to `start`, the computed
 * buddy might fall outside the allocator's range.
 */
static inline void *
__blk_2_buddy(void *addr, void *start, int order)
{
#if 0
	return (addr ^ ((1ULL << order) * PAGE_SIZE))
#else
	return ((addr - start) ^ ((1ULL << order) * PAGE_SIZE)) + start;
#endif
}

/*****************************************************************************/

static inline uint8_t
__blk_in_lvl_2_index(void *blk, void *start, uint8_t lvl)
{
	return (blk - start) / SIZE_OF_LEVEL(lvl);
}

static inline uint8_t
__blk_in_lvl_2_bit(void *blk, void *start, uint8_t lvl)
{
	return (1 << lvl) - 1 + __blk_in_lvl_2_index(blk, start, lvl);
}


/*****************************************************************************/

static inline int
__blk_in_lvl_aligned(void *blk, void *start, uint8_t lvl)
{
	uintptr_t r = blk - start;

	//
	return (r == ALIGN((uintptr_t)r, (1 << LEVEL_2_ORDER(lvl)) * PAGE_SIZE));
}


/*****************************************************************************/

static inline uint64_t
__blk_is_split(uint64_t mask, uint8_t bit, uint8_t lvl)
{
	if (lvl == BUDDY_ORDERS)
		return (mask & (1ULL << bit));

	return ((mask & (1ULL << bit)) ||
			(__blk_is_split(mask, 2 * bit + 1, lvl + 1)) ||
			(__blk_is_split(mask, 2 * bit + 2, lvl + 1)));
}

static inline uint64_t
__blk_is_allocated(uint64_t mask, void *blk, void *start, uint8_t lvl)
{
	return (mask & (1ULL << __blk_in_lvl_2_bit(blk, start, lvl)));
}

static inline void
__blk_set_allocated(uint64_t *mask, void *blk, void *start, uint8_t lvl)
{
	*mask |= (1ULL << __blk_in_lvl_2_bit(blk, start, lvl));
}

static inline void
__blk_clr_allocated(uint64_t *mask, void *blk, void *start, uint8_t lvl)
{
	*mask &= ~(1ULL << __blk_in_lvl_2_bit(blk, start, lvl));
}

static inline blk_status_e
__blk_get_status(uint64_t mask, void *blk, void *start, uint8_t lvl)
{
	if (__blk_is_allocated(mask, blk, start, lvl))
		return BLK_ALLOCATED;

	if (__blk_is_split(mask, __blk_in_lvl_2_bit(blk, start, lvl), lvl))
		return BLK_SPLIT;

	return BLK_FREE;
}


/*****************************************************************************/

/**
 * Allocate a memory block.
 *
 * @bud		: Buddy data structure.
 * @order	: Block order to be allocated.
 *
 * Allocate a memory block by querying the free list for the required order.
 * If the free list is empty, attempt to split larger blocks (higher orders)
 * until two blocks of the required order are created.
 *
 * Once a suitable memory block is found, remove it from the free list and mark
 * the corresponding bit as allocated.
 *
 * Return block memory address on success and NULL otherwise.
 */
static void *__blk_alloc(buddy_t *bud, uint8_t order)
{
	bool splitted;
	kdlist_node_t *it;
	void *r_addr, *bud1_addr, *bud2_addr, *s_aligned;

	/******************************************************
	 * Returned block address and aligned starting address
	 ******************************************************/
	r_addr = NULL;
	s_aligned = PAGE_PTR_ALIGN(bud->mem);

	/******************************************************
	 * Loop until find a block of given order and try to
	 * split larger blocks
	 ******************************************************/
	splitted = true;

	//
	while (splitted) {
		// check if our order is fulfilled
		it = kdlist_pop_head(&bud->free_list[order]);
		if (it) {
			r_addr = (void *)it;
			goto finish;
		}

		// reset flag
		splitted = false;

		// split larger blocks
		for (int i = order + 1; i < BUDDY_ORDERS+1; i++) {
			// no empty block of given order to be splitted
			if (kdlist_is_empty(&bud->free_list[i]))
				continue;

			// empty block
			it = kdlist_pop_head(&bud->free_list[i]);

			// compute starting address for splitted blocks
			bud1_addr = (void *)it;
			bud2_addr = __blk_2_buddy(bud1_addr, s_aligned, i - 1);

			// split in two smaller blocks
			kdlist_push_tail(&bud->free_list[i-1], (kdlist_node_t *)bud1_addr);
			kdlist_push_tail(&bud->free_list[i-1], (kdlist_node_t *)bud2_addr);

			//
			BUDDY_DBG("Split |%p(%u)| -> |%p(%u)|%p(%u)|\n", bud1_addr, i,
					bud1_addr, i-1, bud2_addr, i-1);

			// mark as spliited
			splitted = true;
			break;
		}
	}

//out_of_memory:
	BUDDY_DBG("No memory left for order %d!\n", order);
	return NULL;

finish:
	/******************************************************
	 * Mark block as allocated in tree
	 ******************************************************/
	assert(r_addr);
	__blk_set_allocated(&bud->tree_mask, r_addr, s_aligned,
						ORDER_2_LEVEL(order));

	BUDDY_DBG("Found address %p for order %d!\n", r_addr, order);
	return r_addr;
}

/**
 * Free a memory block.
 *
 * @bud	: Buddy data structure.
 * @blk	: Block address to be freed.
 * @lvl	: Block level in bit mask tree.
 *
 * Loop through the addresses to attempt merging buddies. For each address,
 * compute the address of its corresponding buddy. If the buddy is either split
 * or allocated, stop the iteration and add the free address to the free list
 * for the current order. If the buddy is free, remove it from the free list
 * and merge the two blocks into a block of the next higher order.
 *
 * When a block is freed, clear the corresponding allocated bit.
 *
 * Return 0 on success and <0 otherwise.
 */
static int __blk_free(buddy_t *bud, void *blk, uint8_t lvl)
{
	uint8_t order;
	kdlist_node_t *node;
	void *bud1_addr, *bud2_addr, *s_aligned;

	// TODO improve implementation

	/******************************************************
	 * Aligned starting address
	 ******************************************************/
	bud1_addr = blk;
	order = LEVEL_2_ORDER(lvl);
	s_aligned = PAGE_PTR_ALIGN(bud->mem);

	/******************************************************
	 * Clear allocated bit in tree and add to free list
	 ******************************************************/
	__blk_clr_allocated(&bud->tree_mask, bud1_addr, s_aligned, lvl);
	kdlist_push_tail(&bud->free_list[order], (kdlist_node_t *)bud1_addr);

	/******************************************************
	 * Loop trying to merge free blocks
	 ******************************************************/
	while (lvl) {
		order = LEVEL_2_ORDER(lvl);

		// stop if buddy is not free
		bud2_addr = __blk_2_buddy(bud1_addr, s_aligned, order);
		switch (__blk_get_status(bud->tree_mask, bud2_addr, s_aligned, lvl)) {
		case BLK_SPLIT:
		case BLK_ALLOCATED:
			goto finish;
		case BLK_FREE:
			break;
		default:
			assert(0);
		}

		//
		order = LEVEL_2_ORDER(lvl);

		// remove current and body from free list
		node = (kdlist_node_t *)bud1_addr;
		kdlist_delete(node);
		//
		node = (kdlist_node_t *)bud2_addr;
		kdlist_delete(node);

		// add merged block to next order free list
		bud1_addr = bud1_addr < bud2_addr ? bud1_addr : bud2_addr;

		kdlist_push_tail(&bud->free_list[order+1], (kdlist_node_t *)bud1_addr);

		//
		BUDDY_DBG("Merge |%p(%u)|%p(%u)| -> |%p(%u)|\n", bud1_addr, order,
				bud2_addr, order, bud1_addr, order+1);

		// move to next level (higher order)
		lvl--;
	}

finish:
	return 0;
}


/******************************** PUBLIC API *********************************/

/**
 * Initialize the buddy allocator.
 */
buddy_t *buddy_init(void)
{
	buddy_t *bud = NULL;

	/******************************************************
	 * Memory allocation and initialization
	 *
	 * Allocate BUDDY_XTRA bytes to ensure the buddy start
	 * address can be aligned to a page boundary without
	 * exceeding the allocated memory.
	 ******************************************************/
	bud = (buddy_t *)malloc(sizeof(buddy_t));
	if (!bud) {
		BUDDY_ERR("Unable to allocate buddy data structure!\n");
		goto error;
	}

	//
	bud->tree_mask = 0;

	//
	for (int i = 0; i < BUDDY_ORDERS + 1; i++)
		kdlist_head_init(&bud->free_list[i]);

	//
	bud->mem = malloc(BUDDY_TOTAL_MEM);
	if (!bud->mem) {
		BUDDY_ERR("Unable to allocate buddy memory!\n");
		goto free_bud;
	}

	/******************************************************
	 * Entire memory is a single block of max order
	 ******************************************************/
	kdlist_push_tail(&bud->free_list[BUDDY_ORDERS],
					(kdlist_node_t *)PAGE_PTR_ALIGN(bud->mem));

// success
	return bud;

free_bud:
	free(bud);
error:
	return NULL;
}

/**
 * Allocate a memory block of custom size.
 *
 * @bud		: Buddy data structure.
 * @bytes	: Block size in bytes.
 *
 * Return block address on success and NULL otherwise.
 */
void *buddy_alloc(buddy_t *bud, uint32_t bytes)
{
	int order = SIZE_2_ORDER(bytes);

	/******************************************************
	 * Validation
	 ******************************************************/
	if (!bud || order >= BUDDY_ORDERS+1) {
		BUDDY_ERR("Unable to validate %d bytes allocation request!\n", bytes);
		return NULL;
	}

	/******************************************************
	 * Allocation
	 ******************************************************/
	return __blk_alloc(bud, order);
}

/**
 * Free a memory block.
 *
 * @bud	: Buddy data structure.
 * @blk	: Block address to be freed.
 *
 * To determine the size (order) of the memory block, loop through all levels
 * of the tree, starting from the highest level (smallest orders), and check if
 * the block's address is marked as allocated at each level.
 *
 * Return 0 on success and <0 otherwise.
 */
int buddy_free(buddy_t *bud, void *blk)
{
	void *s_aligned = PAGE_PTR_ALIGN(bud->mem);

	/******************************************************
	 * Validation (address in range and page aligned)
	 ******************************************************/
	if (!bud || !blk || blk < s_aligned || blk > s_aligned + BUDDY_MEM ||
		blk != PAGE_PTR_ALIGN(blk)) {
		BUDDY_ERR("Unable to validate %p addr free request!\n", blk);
		goto error;
	}

	/******************************************************
	 * Check block size (order/level)
	 ******************************************************/
	for (int8_t level = BUDDY_ORDERS; level >= 0; level--) {
		// skip if not aligned in level
		if (!__blk_in_lvl_aligned(blk, s_aligned, level))
			continue;

		// check if allocated in level
		if (__blk_is_allocated(bud->tree_mask, blk, s_aligned, level))
			return __blk_free(bud, blk, level);
	}

	// invalid memory address
	BUDDY_ERR("Address %p double free or corruption!\n", blk);

error:
	return -1;
}

/**
 * Free memory used by buddy allocator.
 *
 * @bud	: Buddy data structure.
 */
void buddy_destroy(buddy_t *bud)
{

	if (!bud)
		return;

	//
	free(bud->mem);
	free(bud);
}

/**
 * Dump status of the buddy allocator.
 *
 * @bud	: Buddy data structure.
 */
void buddy_dump(buddy_t *bud)
{
	kdlist_node_t *it;

	//
	if (!bud)
		return;

	/******************************************************
	 * Unaligned starting address
	 ******************************************************/
	printf("Memory start %p: \n", bud->mem);

	/******************************************************
	 * Free list
	 ******************************************************/
	for (int order = 0; order < BUDDY_ORDERS+1; order++) {
		printf("ORDER %d (%d page(s)): ", order, (1 << order));

		//
		if (kdlist_is_empty(&bud->free_list[order])) {
			printf("EMPTY\n");
			continue;
		}

		//
		kdlist_for_each(it, &bud->free_list[order]) {
			printf("%p ->", it);
		}

		printf("\n");
	}

	/******************************************************
	 * Tree
	 ******************************************************/
	int bit;

	printf("mask: %lu\n", bud->tree_mask);

	for (int level = 0; level < BUDDY_ORDERS+1; level++) {
		printf("LEVEL %d\n", level);
		for (int blk = 0; blk < (1 << level); blk++) {
			bit = (1 << level) - 1 + blk;

			if (bud->tree_mask & (1ULL<<bit))
				printf("%c(%d)|", 'A', bit);
			else
				if (__blk_is_split(bud->tree_mask, bit, level))
					printf("%c(%d)|", 'S', bit);
				else
					printf("%c(%d)|", 'F', bit);

		}
		printf("\n");
	}
}
