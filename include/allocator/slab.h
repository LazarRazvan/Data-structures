/**
 * POC implementation of slab system allocator.
 * Copyright (C) 2025 Lazar Razvan.
 */

#ifndef SLAB_H
#define SLAB_H

#include <stdint.h>

#include "utils.h"
#include "list/kdoubly_linked_list.h"

/*****************************************************************************/

//
// CONFIG
//
#define DBG_ENABLE					1
#define ERR_ENABLE					1
#define RED_ZONE					1


/*****************************************************************************/

/**
 * Slab Allocator Overview
 * ------------------------
 * This slab allocator manages memory in chunks called "slabs", each backed
 * by a memory region of page size (4096 bytes).
 *
 * Each slab contains:
 *   - Metadata (bitmap, count, list node, padding)
 *   - A series of aligned objects (32B, 64B, etc.)
 *   - Optional red zones for buffer overflow detection (enabled via macro)
 *
 * The object region is aligned to SLAB_OBJ_ALIGNMENT (e.g., 64B) to ensure
 * efficient cache usage and to avoid false sharing.
 *
 * ┌──────────────────────────────┐
 * │          slab_t              │
 * ├──────────────────────────────┤
 * │ uint64_t magic               │ // metadata
 * │ uint64_t *bitmap             │
 * │ size_t    count              │
 * │ kdlist_node_t node           │
 * ├──────────────────────────────┤
 * │ uint8_t _pad[]               │ // padding to align `mem[]` start
 * ├──────────────────────────────┤
 * │ uint8_t mem[]                │ // start of aligned objects
 * │ ┌──────────────┐             │
 * │ │   object 0   │ ← aligned   │
 * │ ├──────────────┤             │
 * │ │ red zone     │ ← if enabled│
 * │ ├──────────────┤             │
 * │ │   object 1   │             │
 * │ ├──────────────┤             │
 * │ │ red zone     │             │
 * │ ├──────────────┤             │
 * │ │    ...       │             │
 * │ ├──────────────┤             │
 * │ │ object N-1   │             │
 * │ ├──────────────┤             │
 * │ │ red zone     │             │
 * │ └──────────────┘             │
 * └──────────────────────────────┘
 *
 * Alignment guarantees:
 * ---------------------
 *   - slab itself is page-aligned (posix_memalign)
 *   - mem[] starts at SLAB_OBJ_ALIGNMENT boundary
 *   - All objects are SLAB_OBJ_ALIGNMENT-aligned
 *
 * Red Zone Support (optional):
 * ----------------------------
 *   - Red zones are placed *after* each object
 *   - They are filled with a known byte pattern (RED_ZONE_MAGIC)
 *   - On free, the red zone is validated for corruption
 *   - Used for detecting buffer overruns during testing/debug
 *   - Enabled by defining `SLAB_ENABLE_REDZONE` macro
 */

/*****************************************************************************/

//
// Print macros
//
#if DBG_ENABLE
    #define SLAB_DBG(fmt, ...)	printf("DBG: %s: " fmt, __func__, ##__VA_ARGS__)
#else
    #define SLAB_DBG(fmt, ...)
#endif

#if ERR_ENABLE
    #define SLAB_ERR(fmt, ...)	printf("ERR: %s: " fmt, __func__, ##__VA_ARGS__)
#else
    #define SLAB_ERR(fmt, ...)
#endif


/*****************************************************************************/

#define SLAB_MAGIC					0xDEADBEEFCAFEBABEULL
#define RED_ZONE_MAGIC				0xDEDEDEDEDEDEDEDEULL

/*****************************************************************************/

//
// Max cpus
//
#define MAX_CPUS					16

//
// Alignemnt (cache line)
//
#define SLAB_OBJ_ALIGNMENT			64

//
// Memory
//
#define RED_ZONE_SIZE				(sizeof(uint64_t))
#define SLAB_BLK_SIZE				PAGE_SIZE
#define SLAB_MEM_SIZE				(SLAB_BLK_SIZE - sizeof(slab_t))

//
#define SLAB_OBJ_MAX_SIZE			SLAB_MEM_SIZE
#define SLAB_OBJ_MIN_SIZE			4

//
#define SLAB_OBJ_MAX_CNT			SLAB_MEM_SIZE / SLAB_OBJ_MIN_SIZE

//
// Name
//
#define SLAB_OBJ_MAX_NAME			64


_Static_assert(IS_POWER_2(SLAB_OBJ_ALIGNMENT), "Obj align is not power of 2");
_Static_assert(IS_POWER_2(SLAB_BLK_SIZE), "Block size is not power of 2");


/*****************************************************************************/

#define SLAB_METADATA_SIZE			\
			(sizeof(uint64_t) + sizeof(uint64_t*) + sizeof(size_t) + \
			sizeof(kdlist_node_t))

#define SLAB_MEM_OFFSET				(ALIGN(SLAB_METADATA_SIZE, SLAB_OBJ_ALIGNMENT))
#define SLAB_PAD_SIZE				(SLAB_MEM_OFFSET - SLAB_METADATA_SIZE)


/*****************************************************************************/

//
typedef struct slab_s {

	uint64_t						magic;
	uint64_t						*bitmap;
	size_t							count;

	//
	kdlist_node_t					node;

	// padding to ensure alignment
	uint8_t							_pad[SLAB_PAD_SIZE];

	//
	uint8_t							mem[0];

} slab_t;

//
typedef struct per_cpu_slab_s {

	kdlist_head_t					full_slabs_head;
	kdlist_head_t					partial_slabs_head;
	kdlist_head_t					free_slabs_head;

} per_cpu_slab_t;

//
typedef struct slab_cache_s {

	//
	uint8_t							full_bytes;
	uint8_t							left_bits;

	//
	size_t							obj_size;
	size_t							obj_per_slab;
	size_t							obj_size_align;

	//
	char							obj_name[SLAB_OBJ_MAX_NAME];

	//
	per_cpu_slab_t					_cpu[MAX_CPUS];

} slab_cache_t;


/*****************************************************************************/

// Initialize slab cache
slab_cache_t *slab_cache_create(size_t obj_size, char *obj_name);

// Destroy slab cache
void slab_cache_destroy(slab_cache_t *slab_cache);

// Shrink slab cache
void slab_cache_shrink(slab_cache_t *slab_cache, int cpu_id);

// Allocate slab cache object
void *slab_cache_alloc(slab_cache_t *slab_cache, int cpu_id);

// Free slab cache object
void slab_cache_free(slab_cache_t *slab_cache, void *ptr, int cpu_id);

// Dump slab cache
void slab_cache_dump(slab_cache_t *slab_cache);

#endif	// SLAB_H
