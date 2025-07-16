#ifndef UTILS_H
#define UTILS_H


/*****************************************************************************/
//
// Size
//
#define _1KiB				(1024)
#define _1MiB				(1024 * _1KiB)
#define _1GiB				(1024 * _1MiB)

//
#define PAGE_SIZE			(4 * _1KiB)

//
#define MEMBER_SIZE(t, m)	sizeof(((t *)0)->m)


/*****************************************************************************/
//
// Align
//
#define ALIGN(x,a)			(((x) + (a) - 1) & ~((a) - 1))
#define ALIGN_PTR(x,a)		(void *)ALIGN((uintptr_t)(x), a)

//
#define PAGE_ALIGN(x)		ALIGN(x, PAGE_SIZE)
#define PAGE_PTR_ALIGN(x)	ALIGN_PTR(x, PAGE_SIZE)


/*****************************************************************************/
//
// Utils
//
#define IS_POWER_2(x)		(((x) & ((x) - 1)) == 0)
#define ARRAY_SIZE(x)		(sizeof(x) / sizeof((x)[0]))


/*****************************************************************************/

#endif	// UTILS_H
