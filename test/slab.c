#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#include "allocator/slab.h"

/*****************************************************************************/

// Single core
#define SLAB_CACHE_ALLOC(cache)			slab_cache_alloc(cache, 0)
#define SLAB_CACHE_FREE(cache,ptr)		slab_cache_free(cache, ptr, 0)

/*****************************************************************************/

static inline uint64_t now_ns() {
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}


static int check_pattern(void *ptr, uint8_t pattern, size_t size) {
    uint8_t *p = ptr;
    for (size_t i = 0; i < size; ++i) {
        if (p[i] != pattern)
            return 0;
    }
    return 1;
}


/*****************************************************************************/

static int test1_no = 1024;
static int test1_size[] = {4, 8, 16, 32, 64, 128, 256, 512, 1024};


static void __slab_test1(void)
{
	void **ptrs;
	int obj_size;
	char pattern;
	slab_cache_t *slab_cache;

	printf("================= TEST1 =================\n");

	ptrs = malloc(test1_no * sizeof(void *));
	if (!ptrs) {
		fprintf(stderr, "Fail to alloc pointers memory!\n");
		assert(0);
	}

	//
	for (int i = 0; i < ARRAY_SIZE(test1_size); i++) {
		obj_size = test1_size[i];

		printf("Creating %d objects of size %d...\n", test1_no, obj_size);

		// create slab cache
		slab_cache = slab_cache_create(obj_size, "struct test1");
		if (!slab_cache) {
			fprintf(stderr, "Fail to create slab cache!\n");
			assert(0);
		}

		// alloc
		for (int j = 0; j < test1_no; j++) {
			ptrs[j] = SLAB_CACHE_ALLOC(slab_cache);
			assert(ptrs[j]);
		}

		// write
		for (int j = 0; j < test1_no; j++) {
			pattern = i % 255;
			assert(memset(ptrs[j], pattern, obj_size));
		}

		// validate
		for (int j = 0; j < test1_no; j++) {
			pattern = i % 255;
			assert(check_pattern(ptrs[j], pattern, obj_size));
		}

		// free
		for (int j = 0; j < test1_no; j++) {
			SLAB_CACHE_FREE(slab_cache, ptrs[j]);
		}

		// destroy slab cache
		slab_cache_destroy(slab_cache);
	}

	free(ptrs);
	printf("SUCCESS\n");
}

/*****************************************************************************/

#define TEST2_PATTERN			0xAB

static int test2_no = 1024 * 1024;
static int test2_size[] = {4, 8, 16, 32, 64, 128, 256, 512, 1024};


static void __slab_test2(void)
{
	void **ptrs;
	int obj_size;
	slab_cache_t *slab_cache;
	int slab_reuse , malloc_reuse;

	printf("================= TEST2 =================\n");

	ptrs = malloc(test2_no * sizeof(void *));
	if (!ptrs) {
		fprintf(stderr, "Fail to alloc pointers memory!\n");
		assert(0);
	}

	//
	for (int i = 0; i < ARRAY_SIZE(test2_size); i++) {
		slab_reuse = 0;
		malloc_reuse = 0;
		obj_size = test2_size[i];

		printf("Creating %d objects of size %d...\n", test2_no, obj_size);

		//
		// Slab
		//

		// create slab cache
		slab_cache = slab_cache_create(obj_size, "struct test2");
		if (!slab_cache) {
			fprintf(stderr, "Fail to create slab cache!\n");
			assert(0);
		}

		// alloc + write pattern
		for (int j = 0; j < test2_no; j++) {
			ptrs[j] = SLAB_CACHE_ALLOC(slab_cache);
			assert(ptrs[j]);

			assert(memset(ptrs[j], TEST2_PATTERN, obj_size));
		}

		// free
		for (int j = 0; j < test2_no; j++) {
			SLAB_CACHE_FREE(slab_cache, ptrs[j]);
		}

		// alloc + validate
		for (int j = 0; j < test2_no; j++) {
			ptrs[j] = SLAB_CACHE_ALLOC(slab_cache);
			assert(ptrs[j]);

			if (check_pattern(ptrs[j], TEST2_PATTERN, obj_size))
				slab_reuse++;
		}

		// destroy slab cache
		slab_cache_destroy(slab_cache);

		//
		// Malloc/Free
		//

		// malloc + write
		for (int j = 0; j < test2_no; j++) {
			ptrs[j] = malloc(obj_size);
			assert(ptrs[j]);

			assert(memset(ptrs[j], TEST2_PATTERN, obj_size));
		}

		// free
		for (int j = 0; j < test2_no; j++) {
			free(ptrs[j]);
		}

		// malloc + validate
		for (int j = 0; j < test2_no; j++) {
			ptrs[j] = malloc(obj_size);
			assert(ptrs[j]);

			if (check_pattern(ptrs[j], TEST2_PATTERN, obj_size))
				malloc_reuse++;
		}

		printf("Slab reuse  :     %d/%d (%.2f%%)\n", slab_reuse, test2_no, 100.0 * slab_reuse/test2_no);
		printf("Malloc reuse:     %d/%d (%.2f%%)\n", malloc_reuse, test2_no, 100.0 * malloc_reuse/test2_no);
	}

	free(ptrs);
}


/*****************************************************************************/

#define TEST3_PATTERN			0xAA

static int test3_no = 1024 * 1024 * 4;
static int test3_size[] = {4, 8, 16, 32, 64, 128, 256, 512, 1024};

static void __slab_test3(void)
{
	void **ptrs;
	int obj_size;
	slab_cache_t *slab_cache;
	uint64_t slab_start, slab_alloc, slab_write, slab_free, slab_reuse;
	uint64_t malloc_start, malloc_alloc, malloc_write, malloc_free, malloc_reuse;

	printf("================= TEST2 =================\n");

	ptrs = malloc(test3_no * sizeof(void *));
	if (!ptrs) {
		fprintf(stderr, "Fail to alloc pointers memory!\n");
		assert(0);
	}

	//
	for (int i = 0; i < ARRAY_SIZE(test3_size); i++) {
		obj_size = test3_size[i];

		printf("Creating %d objects of size %d...\n", test3_no, obj_size);

		//
		// Slab
		//

		// create slab cache
		slab_cache = slab_cache_create(obj_size, "struct test3");
		if (!slab_cache) {
			fprintf(stderr, "Fail to create slab cache!\n");
			assert(0);
		}

		slab_start = now_ns();

		// alloc
		for (int j = 0; j < test3_no; j++) {
			ptrs[j] = SLAB_CACHE_ALLOC(slab_cache);
			assert(ptrs[j]);
		}
		slab_alloc = now_ns();

		// write
		for (int j = 0; j < test3_no; j++) {
			assert(memset(ptrs[j], TEST3_PATTERN, obj_size));
		}
		slab_write = now_ns();

		// free
		for (int j = 0; j < test3_no; j++) {
			SLAB_CACHE_FREE(slab_cache, ptrs[j]);
		}
		slab_free = now_ns();

		// reuse
		for (int j = 0; j < test3_no; j++) {
			ptrs[j] = SLAB_CACHE_ALLOC(slab_cache);
			assert(ptrs[j]);

			((char *)ptrs[j])[0]++;
		}
		slab_reuse = now_ns();

		// destroy slab cache
		slab_cache_destroy(slab_cache);

		//
		// Malloc/Free
		//

		malloc_start = now_ns();

		// alloc
		for (int j = 0; j < test3_no; j++) {
			ptrs[j] = malloc(obj_size);
			assert(ptrs[j]);
		}
		malloc_alloc = now_ns();

		// write
		for (int j = 0; j < test3_no; j++) {
			assert(memset(ptrs[j], TEST3_PATTERN, obj_size));
		}
		malloc_write = now_ns();

		// free
		for (int j = 0; j < test3_no; j++) {
			free(ptrs[j]);
		}
		malloc_free = now_ns();

		// reuse
		for (int j = 0; j < test3_no; j++) {
			ptrs[j] = malloc(obj_size);
			assert(ptrs[j]);

			((char *)ptrs[j])[0]++;
		}
		malloc_reuse = now_ns();

		// free
		for (int j = 0; j < test3_no; j++) {
			free(ptrs[j]);
		}

		//
		printf("Alloc\n");
		printf("Slab  : %lu ns\n", slab_alloc - slab_start);
		printf("Malloc: %lu ns\n", malloc_alloc - malloc_start);

		printf("Write\n");
		printf("Slab  : %lu ns\n", slab_write - slab_alloc);
		printf("Malloc: %lu ns\n", malloc_write - malloc_alloc);

		printf("Free\n");
		printf("Slab  : %lu ns\n", slab_free - slab_write);
		printf("Malloc: %lu ns\n", malloc_free - malloc_write);

		printf("Reuse\n");
		printf("Slab  : %lu ns\n", slab_reuse - slab_free);
		printf("Malloc: %lu ns\n", malloc_reuse - malloc_free);
	}

	free(ptrs);
}


/*****************************************************************************/

#if RED_ZONE

static void __slab_test4(void)
{
	void **ptrs;
	int obj_size;
	slab_cache_t *slab_cache;

	printf("================= TEST1 =================\n");

	ptrs = malloc(test1_no * sizeof(void *));
	if (!ptrs) {
		fprintf(stderr, "Fail to alloc pointers memory!\n");
		assert(0);
	}

	//
	for (int i = 0; i < ARRAY_SIZE(test1_size); i++) {
		obj_size = test1_size[i];

		printf("Creating %d objects of size %d...\n", test1_no, obj_size);

		// create slab cache
		slab_cache = slab_cache_create(obj_size, "struct test1");
		if (!slab_cache) {
			fprintf(stderr, "Fail to create slab cache!\n");
			assert(0);
		}

		// alloc + corruption
		for (int j = 0; j < test1_no; j++) {
			ptrs[j] = SLAB_CACHE_ALLOC(slab_cache);
			assert(ptrs[j]);

			*(uint8_t*)(ptrs[j] + obj_size) = 0x12;
		}

		// free
		for (int j = 0; j < test1_no; j++) {
			SLAB_CACHE_FREE(slab_cache, ptrs[j]);
		}

		// destroy slab cache
		slab_cache_destroy(slab_cache);
	}

	free(ptrs);
	printf("SUCCESS\n");
}

#endif // RED_ZONE

/*****************************************************************************/

int main() {
	__slab_test1();
	__slab_test2();
	__slab_test3();
#if RED_ZONE
	__slab_test4();
#endif

	return 0;
}
