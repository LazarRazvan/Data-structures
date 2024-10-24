/**
 * Ring buffer config.
 * Copyright (C) 2024 Lazar Razvan.
 */

#ifndef RING_BUFFER_CONFIG_H
#define RING_BUFFER_CONFIG_H


/*****************************************************************************/

// ring buffer capacity
#define RING_BUFFER_CAPACITY					4


/*****************************************************************************/

#if 0
// TODO: Change ring buffer entries data type
typedef data_s {
	int		age;
	char	name[16];

} data_t;
#endif

typedef int data_t;

#endif	// RING_BUFFER_CONFIG_H

