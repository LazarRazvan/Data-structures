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

// TODO: because bit of first page may be 1 ... underflow
static inline void *
__blk_2_buddy(void *addr, void *start, int order)
{
	return ((addr - start) ^ ((1ULL << order) * PAGE_SIZE)) + start;
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

static inline uint64_t
__blk_is_split(uint64_t mask, uint8_t bit, uint8_t lvl)
{
	// TODO: stop if allocated is found
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
 * TODO.
 */
static void *__blk_alloc(buddy_t *bud, uint8_t order_no)
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
		it = kdlist_pop_head(&bud->free_list[order_no]);
		if (it) {
			r_addr = (void *)it;
			goto finish;
		}

		// reset flag
		splitted = false;

		// split larger blocks
		for (int i = order_no + 1; i < BUDDY_ORDERS+1; i++) {
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

			// mark as spliited
			splitted = true;
			break;
		}
	}

//out_of_memory:
	return NULL;

finish:
	/******************************************************
	 * Mark block as allocated in tree
	 ******************************************************/
	assert(r_addr);
	__blk_set_allocated(&bud->tree_mask, r_addr, s_aligned,
						ORDER_2_LEVEL(order_no));

	return r_addr;
}

/**
 * TODO
 */
static void __blk_free(buddy_t *bud, void *blk, uint8_t level)
{
	uint8_t order;
	kdlist_node_t *node;
	void *bud1_addr, *bud2_addr, *s_aligned;

	/******************************************************
	 * Aligned starting address
	 ******************************************************/
	bud1_addr = blk;
	order = LEVEL_2_ORDER(level);
	s_aligned = PAGE_PTR_ALIGN(bud->mem);

	/******************************************************
	 * Clear allocated bit in tree and add to free list
	 ******************************************************/
	__blk_clr_allocated(&bud->tree_mask, bud1_addr, s_aligned, level);
	kdlist_push_tail(&bud->free_list[order], (kdlist_node_t *)bud1_addr);

	/******************************************************
	 * Loop trying to merge free blocks
	 ******************************************************/
	while (level) {
		order = LEVEL_2_ORDER(level);

		// stop if buddy is not free
		bud2_addr = __blk_2_buddy(bud1_addr, s_aligned, order);
		switch (__blk_get_status(bud->tree_mask, bud2_addr, s_aligned, level)) {
		case BLK_SPLIT:
		case BLK_ALLOCATED:
			goto finish;
		case BLK_FREE:
			break;
		default:
			assert(0);
		}

		//
		order = LEVEL_2_ORDER(level);

		// remove current and body from free list
		node = (kdlist_node_t *)bud1_addr;
		node->prev->next = node->next;
		node->next->prev = node->prev;
		//
		node = (kdlist_node_t *)bud2_addr;
		node->prev->next = node->next;
		node->next->prev = node->prev;

		// add merged block to next order free list
		bud1_addr = bud1_addr < bud2_addr ? bud1_addr : bud2_addr;

		kdlist_push_tail(&bud->free_list[order+1], (kdlist_node_t *)bud1_addr);

		// move to next level (higher order)
		level--;
	}

finish:
	return;
}


/******************************** PUBLIC API *********************************/

/**
 * Initialize the buddy alloator.
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
	if (!bud)
		goto error;

	//
	bud->tree_mask = 0;

	//
	for (int i = 0; i < BUDDY_ORDERS + 1; i++)
		kdlist_head_init(&bud->free_list[i]);

	//
	bud->mem = malloc(BUDDY_TOTAL_MEM);
	if (!bud->mem)
		goto free_bud;

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
 * Perform an allocation using the buddy allocator.
 */
void *buddy_alloc(buddy_t *bud, uint32_t bytes_no)
{
	int order_no = SIZE_2_ORDER(bytes_no);

	/******************************************************
	 * Validation
	 ******************************************************/
	if (!bud || order_no >= BUDDY_ORDERS+1)
		return NULL;

	/******************************************************
	 * Allocation
	 ******************************************************/
	return __blk_alloc(bud, order_no);
}

/**
 * Free a memory address.
 *
 * @addr: Address to be free.
 */
void buddy_free(buddy_t *bud, void *blk)
{
	void *s_aligned = PAGE_PTR_ALIGN(bud->mem);

	/******************************************************
	 * Validation (address in range and page aligned)
	 ******************************************************/
	if (!bud || !blk || blk < s_aligned || blk > s_aligned + BUDDY_MEM ||
		blk != PAGE_PTR_ALIGN(blk))
		return;

	/******************************************************
	 * Check block size (order/level)
	 ******************************************************/
	for (int8_t level = BUDDY_ORDERS; level >= 0; level--) {
		if (__blk_is_allocated(bud->tree_mask, blk, s_aligned, level)) {
			__blk_free(bud, blk, level);
			break;
		}
	}

	// TODO: add return
}

/**
 * Destroy memory used by the buddy allocator.
 */
void buddy_destroy(buddy_t *buddy)
{

	if (!buddy)
		return;

	//
	free(buddy->mem);
	free(buddy);
}

/**
 * Dump status of the buddy allocator.
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
