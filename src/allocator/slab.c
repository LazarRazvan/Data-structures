/**
 * POC implementation of slab cache allocator.
 * Copyright (C) 2025 Lazar Razvan.
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "allocator/slab.h"


/*****************************************************************************/

#define BITS_OF(type)				(sizeof(type) * 8)
#define BITMAPS_FULL(o)				((o) / BITS_OF(uint64_t))
#define BITMAPS_BITS_LEFT(o)		((o) % BITS_OF(uint64_t))


/*****************************************************************************/

static inline void *__addr_2_slab(void *ptr)
{
	return (void *)((uintptr_t)ptr & ~(SLAB_BLK_SIZE - 1));
}

static inline bool __slab_is_addr_in_range(slab_t *slab, void *ptr)
{
	void *s_addr = (void *)&slab->mem;

	return ((ptr >= s_addr) && (ptr < s_addr + SLAB_MEM_SIZE));
}

static inline bool __slab_is_addr_aligned(slab_t *slab, void *ptr, size_t size)
{
	void *s_addr = (void *)&slab->mem;

	return (!((uintptr_t)ptr % SLAB_OBJ_ALIGNMENT)) && !((ptr - s_addr) % size);
}

static inline void *__slab_obj_addr(slab_t *slab, size_t bitmap_idx,
									size_t bit_idx, size_t obj_size_align)
{
	void *s_addr = (void *)&slab->mem;

	return (s_addr + (((bitmap_idx * BITS_OF(uint64_t)) + bit_idx) *
		obj_size_align));
}


/*****************************************************************************/

static void __slab_dump(slab_t *slab, uint8_t _full, uint8_t _bits)
{
	int i, j;
	uint64_t *bitmap = slab->bitmap;

	//
	printf("    SLAB [%p] count: %ld\n", slab, slab->count);

	//
	for (i = 0; i < _full; i++) {
		for (j = 0; j < 64; j++) {
			printf("        obj %ld : %s\n",
					(i * BITS_OF(uint64_t)) + j,
					bitmap[i] & (1ULL << j) ? "USED" : "FREE");
		}
	}

	//
	for (j = 0; j < _bits; j++) {
		printf("        obj %ld : %s\n",
				(i * BITS_OF(uint64_t)) + j,
				bitmap[i] & (1ULL << j) ? "USED" : "FREE");
	}
}


/*****************************************************************************/

static inline bool __slab_is_free(slab_t *slab)
{
	return (slab->count == 0);
}

static inline bool __slab_is_full(slab_t *slab, size_t obj_per_slab)
{
	return (slab->count == obj_per_slab);
}

static inline bool __slab_is_partial(slab_t *slab, size_t obj_per_slab)
{
	return !__slab_is_free(slab) && !__slab_is_full(slab, obj_per_slab);
}


/*****************************************************************************/

static slab_t * __slab_create(size_t bitmap_size)
{
	slab_t *slab = NULL;

	// ensure page aligned slabs
	if (posix_memalign((void **)&slab, PAGE_SIZE, SLAB_BLK_SIZE)) {
		SLAB_ERR("Unable to create new slab entry!\n");
		goto finish;
	}

#if DBG_ENABLE
	assert((uintptr_t)((void *)&slab->mem) % SLAB_MEM_OFFSET == 0);
#endif

	slab->bitmap = calloc(bitmap_size, sizeof(uint64_t));
	if (!slab->bitmap) {
		SLAB_ERR("Unable to create new slab metadata!\n");
		goto free_slab;
	}

	slab->count = 0;
	slab->magic = SLAB_MAGIC;

//success
	return slab;

free_slab:
	free(slab);
	slab = NULL;
finish:
	return slab;
}


static void __slab_destroy(slab_t *slab)
{
	free(slab->bitmap);
	free(slab);
}


/*****************************************************************************/

static void * __slab_alloc(slab_t *slab, size_t obj_size, size_t obj_size_align,
						uint8_t _full, uint8_t _bits)
{
	int i;
	void *ptr = NULL;
	uint64_t _bit, _bitmap, _masked, mask;

#if DBG_ENABLE
	size_t obj_per_slab = _full * BITS_OF(uint64_t) + _bits;

	assert(__slab_is_free(slab) || __slab_is_partial(slab, obj_per_slab));
#endif

	/**
	 * Lookup into fully specified bitmaps for a free object.
	 */
	for (i = 0; i < _full; i++) {
		_bitmap = slab->bitmap[i];

		if (_bitmap == 0xFF)
			continue;

		_bit = __builtin_ctzll(~_bitmap);
		slab->bitmap[i] = _bitmap | (1ULL << _bit);

		goto finish;
	}

	/**
	 * Lookup into remaining bits. Since a free an object was not found above,
	 * it is mandatory to have remaining bits and a free object within.
	 */
#if DBG_ENABLE
	assert(_bits);
#endif

	_bitmap = slab->bitmap[i];
	_masked = (1ULL << _bits) - 1;
	mask = _bitmap & _masked;

#if DBG_ENABLE
	assert(mask != _masked);
#endif

	_bit = __builtin_ctzll(~mask);
	slab->bitmap[i] = _bitmap | (1ULL << _bit);

finish:
	slab->count++;

	ptr = __slab_obj_addr(slab, i, _bit, obj_size_align);
#if DBG_ENABLE
	assert((uintptr_t)ptr % SLAB_OBJ_ALIGNMENT == 0);
#endif

#if RED_ZONE
	uint64_t *red_zone = ptr + obj_size;
	*red_zone = RED_ZONE_MAGIC;
#endif

	return ptr;
}

static void __slab_free(slab_t *slab, void *ptr, size_t obj_size,
						size_t obj_size_align)
{
	void *slab_mem;
	uint64_t _bitmap, _masked, obj_idx, bitmap_idx, bit_idx;

	//
	slab_mem = (void *)&slab->mem;

	//
	if (!__slab_is_addr_aligned(slab, ptr, obj_size_align)) {
		SLAB_ERR("Unalgined %p free address!\n", ptr);
		assert(0);
	}

	//
	obj_idx		= ((ptr - slab_mem) / obj_size_align);
	bitmap_idx	= obj_idx / BITS_OF(uint64_t);
	bit_idx		= obj_idx % BITS_OF(uint64_t);

	//
	_bitmap = slab->bitmap[bitmap_idx];
	_masked = (1ULL << bit_idx);

	//
	if (!(_bitmap & _masked)) {
		SLAB_ERR("Address %p already free!\n", ptr);
		assert(0);
	}

#if RED_ZONE
	uint64_t *red_zone = ptr + obj_size;

	if (*red_zone != RED_ZONE_MAGIC) {
		SLAB_ERR("Red zone corruption for address %p\n", ptr);
		assert(0);
	}

#endif

	slab->bitmap[bitmap_idx] = _bitmap & ~_masked;
	slab->count--;
}


/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * Initialize slab cache.
 *
 * @obj_size	: Slab cache object size.
 * @obj_name	: Slab object name.
 *
 * Return slab cache on success or NULL otherwise.
 */
slab_cache_t * slab_cache_create(size_t obj_size, char *obj_name)
{
	slab_cache_t *slab_cache = NULL;

	//
	if ((obj_size > SLAB_OBJ_MAX_SIZE) || (obj_size < SLAB_OBJ_MIN_SIZE)) {
		SLAB_ERR("Invalid object size!\n");
		goto finish;
	}

	if (!obj_name || strlen(obj_name) >= SLAB_OBJ_MAX_NAME) {
		SLAB_ERR("Invalid object name!\n");
		goto finish;
	}

	//
	slab_cache = malloc(sizeof(slab_cache_t));
	if (!slab_cache) {
		SLAB_ERR("Unable to create slab_cache!\n");
		goto finish;
	}

	//
	slab_cache->obj_size 		= obj_size;
#if RED_ZONE
	obj_size += RED_ZONE_SIZE;
#endif
	slab_cache->obj_size_align	= ALIGN(obj_size, SLAB_OBJ_ALIGNMENT);
	slab_cache->obj_per_slab	= SLAB_MEM_SIZE / slab_cache->obj_size_align;

	slab_cache->full_bytes		= BITMAPS_FULL(slab_cache->obj_per_slab);
	slab_cache->left_bits		= BITMAPS_BITS_LEFT(slab_cache->obj_per_slab);

	//
	memcpy(slab_cache->obj_name, obj_name, strlen(obj_name));

	//
	for (int i = 0; i < MAX_CPUS; i++) {
		kdlist_head_init(&slab_cache->_cpu[i].full_slabs_head);
		kdlist_head_init(&slab_cache->_cpu[i].partial_slabs_head);
		kdlist_head_init(&slab_cache->_cpu[i].free_slabs_head);
	}


finish:
	return slab_cache;
}


/**
 * Destroy slab cache.
 *
 * @slab_cache	: Slab cache to be destroyed.
 */
void slab_cache_destroy(slab_cache_t *slab_cache)
{
	slab_t *slab;
	kdlist_node_t *it, *aux;

	//
	if (!slab_cache)
		goto finish;

	//
	for (int i = 0; i < MAX_CPUS; i++) {
		kdlist_for_each_safe(it, aux, &slab_cache->_cpu[i].full_slabs_head) {
			slab = kdlist_entry(it, slab_t, node);
			__slab_destroy(slab);
		}

		kdlist_for_each_safe(it, aux, &slab_cache->_cpu[i].partial_slabs_head) {
			slab = kdlist_entry(it, slab_t, node);
			__slab_destroy(slab);
		}

		kdlist_for_each_safe(it, aux, &slab_cache->_cpu[i].free_slabs_head) {
			slab = kdlist_entry(it, slab_t, node);
			__slab_destroy(slab);
		}
	}

	//
	free(slab_cache);

finish:
	return;
}

/**
 * Shrink slab cache.
 *
 * @slab_cache	: Slab cache to be destroyed.
 * @cpu_id		: Core id.
 */
void slab_cache_shrink(slab_cache_t *slab_cache, int cpu_id)
{
	slab_t *slab;
	kdlist_node_t *it, *aux;

	//
	if (!slab_cache)
		goto finish;

	//
	kdlist_for_each_safe(it, aux, &slab_cache->_cpu[cpu_id].free_slabs_head) {
		slab = kdlist_entry(it, slab_t, node);
		__slab_destroy(slab);
	}

finish:
	return;
}


/*****************************************************************************/

/**
 * Allocate slab cache object.
 *
 * @slab_cache	: slab cache to be used for allocation.
 * @cpu_id		: Core id.
 *
 * Return object address on success and NULL otherwise.
 */
void * slab_cache_alloc(slab_cache_t *slab_cache, int cpu_id)
{
	slab_t *slab;
	void *obj = NULL;
	kdlist_node_t *it;
	uint8_t _full, _bits;
	kdlist_head_t *partial_slabs, *full_slabs, *free_slabs;
	size_t obj_size, obj_per_slab, obj_size_align, bitmap_size;

	//
	if (!slab_cache)
		goto finish;

#if DBG_ENABLE
	assert(cpu_id >= 0 && cpu_id < MAX_CPUS);
#endif

	//
	_full			= slab_cache->full_bytes;
	_bits			= slab_cache->left_bits;
	obj_size		= slab_cache->obj_size;
	obj_per_slab	= slab_cache->obj_per_slab;
	obj_size_align	= slab_cache->obj_size_align;

	//
	full_slabs		= &slab_cache->_cpu[cpu_id].full_slabs_head;
	free_slabs		= &slab_cache->_cpu[cpu_id].free_slabs_head;
	partial_slabs	= &slab_cache->_cpu[cpu_id].partial_slabs_head;

	/**
	 * Lookup into partial slabs. If any partial slab found, perform the
	 * allocation and move the slab to full slabs list, if required.
	 */
//partial_slabs_lookup:
	kdlist_for_each(it, partial_slabs) {
		slab = kdlist_entry(it, slab_t, node);

		obj = __slab_alloc(slab, obj_size, obj_size_align, _full, _bits);
		if (!obj)
			goto free_slabs_lookup;

		//
		if (__slab_is_full(slab, obj_per_slab)) {
			kdlist_delete(it);
			kdlist_push_tail(full_slabs, it);
		}

		goto finish;
	}

	/**
	 * Lookup into free slabs. If any free slab found, perform the
	 * allocation and move the slab either to partial or full list.
	 */
free_slabs_lookup:
	kdlist_for_each(it, free_slabs) {
		slab = kdlist_entry(it, slab_t, node);

		obj = __slab_alloc(slab, obj_size, obj_size_align, _full, _bits);
		if (!obj)
			goto new_slab;

		//
		kdlist_delete(it);

		if (__slab_is_full(slab, obj_per_slab))
			kdlist_push_tail(full_slabs, it);
		else
			kdlist_push_tail(partial_slabs, it);

		goto finish;
	}

	/**
	 * No available slabs found. Create a new slab, perform the allocation
	 * and move the slab either to partial or full list.
	 */
new_slab:
	bitmap_size = (obj_per_slab + BITS_OF(uint64_t) - 1) / BITS_OF(uint64_t);
	slab = __slab_create(bitmap_size);
	if (!slab) {
		goto finish;
	}

	//
	obj = __slab_alloc(slab, obj_size, obj_size_align, _full, _bits);
	if (!obj) {
		SLAB_ERR("Unable to allocate new object!\n");
		goto finish;
	}

	if (__slab_is_full(slab, obj_per_slab))
		kdlist_push_tail(full_slabs, &slab->node);
	else
		kdlist_push_tail(partial_slabs, &slab->node);

finish:
	return obj;
}


/**
 * Free object from slab cache.
 *
 * @slab_cache	: Slab cache to be used for free.
 * @ptr			: Object address to be free.
 * @cpu_id		: Core id.
 */
void slab_cache_free(slab_cache_t *slab_cache, void *ptr, int cpu_id)
{
	slab_t *slab;
	bool full_slab;
	kdlist_head_t *partial_slabs, *free_slabs;
	size_t obj_size, obj_size_align, obj_per_slab;

	//
	if (!slab_cache || !ptr)
		goto finish;

	//
	obj_size		= slab_cache->obj_size;
	obj_per_slab	= slab_cache->obj_per_slab;
	obj_size_align	= slab_cache->obj_size_align;

	//
	free_slabs		= &slab_cache->_cpu[cpu_id].free_slabs_head;
	partial_slabs	= &slab_cache->_cpu[cpu_id].partial_slabs_head;

	/**
	 * Compute slab address from the given pointer and validate it. It is
	 * certain that each each slab is page aligned.
	 *
	 * After validation and mark object as free check if moving slab to partial
	 * or free list is required.
	 */
	slab = __addr_2_slab(ptr);

	//
	if (slab->magic != SLAB_MAGIC) {
		SLAB_ERR("Invalid %p free address!\n", ptr);
		assert(0);
	}

#if DBG_ENABLE
	assert(__slab_is_addr_in_range(slab, ptr));
#endif

	full_slab = __slab_is_full(slab, obj_per_slab);

	//
	__slab_free(slab, ptr, obj_size, obj_size_align);

	//
	if (full_slab) {
		kdlist_delete(&slab->node);
		if (__slab_is_free(slab))
			kdlist_push_tail(free_slabs, &slab->node);
		else
			kdlist_push_tail(partial_slabs, &slab->node);
	} else {
		if (__slab_is_free(slab)) {
			kdlist_delete(&slab->node);
			kdlist_push_tail(free_slabs, &slab->node);
		}
	}
finish:
	return;
}


/*****************************************************************************/

/**
 * Dump slab cache.
 */
void slab_cache_dump(slab_cache_t *slab_cache)
{
	slab_t *slab;
	kdlist_node_t *it;

	//
	if (!slab_cache)
		goto finish;

	//
	printf("============ SLAB CACHE [%p] ============\n", slab_cache);

	printf("object name       : %s\n", slab_cache->obj_name);
	printf("object size       : %lu\n", slab_cache->obj_size);
	printf("objects per slab  : %lu\n", slab_cache->obj_per_slab);

	for (int i = 0; i < MAX_CPUS; i++) {
		printf("[cpu %d] full slabs:\n", i);
		kdlist_for_each(it, &slab_cache->_cpu[i].full_slabs_head) {
			slab = kdlist_entry(it, slab_t, node);
			__slab_dump(slab, slab_cache->full_bytes, slab_cache->left_bits);
		}

		printf("[cpu %d] partial slabs:\n", i);
		kdlist_for_each(it, &slab_cache->_cpu[i].partial_slabs_head) {
			slab = kdlist_entry(it, slab_t, node);
			__slab_dump(slab, slab_cache->full_bytes, slab_cache->left_bits);
		}

		printf("[cpu %d] free slabs:\n", i);
		kdlist_for_each(it, &slab_cache->_cpu[i].free_slabs_head) {
			slab = kdlist_entry(it, slab_t, node);
			__slab_dump(slab, slab_cache->full_bytes, slab_cache->left_bits);
		}
	}

	printf("=========================================\n");

finish:
}
