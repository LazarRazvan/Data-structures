/**
 * POC implementation of buddy system allocator.
 * Copyright (C) 2024 Lazar Razvan.
 */

#ifndef BUDDY_H
#define BUDDY_H

#include <stdint.h>

#include "utils.h"
#include "list/kdoubly_linked_list.h"


/*****************************************************************************/

//
// Print config
//
#define DBG_ENABLE					0
#define ERR_ENABLE					0

//
// Print macros
//
#if DBG_ENABLE
    #define BUDDY_DBG(fmt, ...)	printf("DBG: %s: " fmt, __func__, ##__VA_ARGS__)
#else
    #define BUDDY_DBG(fmt, ...)
#endif

#if ERR_ENABLE
    #define BUDDY_ERR(fmt, ...)	printf("ERR: %s: " fmt, __func__, ##__VA_ARGS__)
#else
    #define BUDDY_ERR(fmt, ...)
#endif


/*****************************************************************************/

//
// Buddy (32 pages in total + xtra to ensure alignemnt)
//
#define BUDDY_ORDERS		5
#define BUDDY_MEM			((1 << BUDDY_ORDERS) * PAGE_SIZE)
#define BUDDY_XTRA			(PAGE_SIZE - 1)

//
// Buddy total memory
//
#define BUDDY_TOTAL_MEM		(BUDDY_MEM + BUDDY_XTRA)


/*****************************************************************************/

//
// Convert memory size to order
//
#define SIZE_2_PAGES(x)		((PAGE_ALIGN(x)) / (PAGE_SIZE))
#define PAGES_2_ORDER(x)	((x) == 1 ? 0 : 32 - __builtin_clz((x) - 1))
#define SIZE_2_ORDER(x)		(PAGES_2_ORDER(SIZE_2_PAGES(x)))

//
// Convert from order to tree level
//
#define ORDER_2_LEVEL(x)	(BUDDY_ORDERS - (x))
#define LEVEL_2_ORDER(x)	(ORDER_2_LEVEL(x))

//
// Tree levels
//
#define LEN_OF_LEVEL(n)		(1ULL << (n))
#define SIZE_OF_LEVEL(n)	(BUDDY_MEM / LEN_OF_LEVEL(n))


//
// Make sure tree mask is not exceeded by updating 'BUDDY_ORDERS'
//

#if ((1ULL << (BUDDY_ORDERS + 1)) - 1) > 64
	#error "BUDDY_ORDERS exceeds the number of bits available in tree_mask!"
#endif


/*****************************************************************************/

typedef enum blk_status_s {

	BLK_FREE = 0,
	BLK_SPLIT,
	BLK_ALLOCATED,

} blk_status_e;


/*****************************************************************************/

typedef struct buddy_s {

	void						*mem;
	kdlist_head_t				free_list[BUDDY_ORDERS+1];
	uint64_t					tree_mask;

} buddy_t;


/*****************************************************************************/

// Initialize buddy allocator
buddy_t *buddy_init(void);

// Destroy buddy allocator
void buddy_destroy(buddy_t *buddy);

// Alloc memory
void *buddy_alloc(buddy_t *buddy, uint32_t bytes_no);

// Free memory
int buddy_free(buddy_t *buddy, void *addr);

// Dump buddy allocator info
void buddy_dump(buddy_t *buddy);

#endif	// BUDDY_H
