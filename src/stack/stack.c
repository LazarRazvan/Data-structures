/**
 * Stack implementation with doubly linked list.
 * Copyright (C) 2024 Lazar Razvan.
 *
 * The implementation may be improved to create a pool of nodes instead of
 * allocating a node each time
 */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

#include "stack/stack.h"


/*****************************************************************************/

//
typedef struct stack_entry_s {

	void			*data;		// stack entry data
	kdlist_node_t	node;		// singly linked list links

} stack_entry_t;


/*****************************************************************************/

//
static stack_entry_t *__stack_entry_create(void *data)
{
	stack_entry_t *entry;

	// alloc space
	entry = (stack_entry_t *)malloc(sizeof(stack_entry_t));
	if (!entry)
		return NULL;

	// store information
	entry->data = data;

	return entry;
}


static void __stack_entry_destroy(stack_entry_t *entry)
{
	free(entry);
}


/*****************************************************************************/

/**
 * Create a stack data structure.
 *
 * Return stack head on success and NULL otherwise.
 */
stack_t *stack_create(void)
{
	stack_t *stack;

	// create doubly linked list head
	stack = (stack_t *)malloc(sizeof(stack_t));
	if (!stack)
		return NULL;

	// initialize list head
	kdlist_head_init(stack);

	return stack;
}


/**
 * Destroy (free space) for a stack data structure.
 */
void stack_destroy(stack_t *stack)
{
	stack_t *it, *aux;
	stack_entry_t *entry;

	// free entries space
	kdlist_for_each_safe(it, aux, stack) {
		entry = container_of(it, stack_entry_t, node);
		assert(entry);

		//
		__stack_entry_destroy(entry);
	}

	// free head
	free(stack);
}


/**
 * Check if a stack is empty.
 *
 * Return true if stack is empty and false otherwise.
 */
bool stack_is_empty(stack_t *stack)
{
	return kdlist_is_empty(stack);
}

/*****************************************************************************/

/**
 * Add an element to stack.
 *
 * @stack	: Stack data structure.
 * @data	: Element to be added to stack.
 *
 * Return 0 on success and <0 otherwise.
 */
int stack_push(stack_t *stack, void *data)
{
	stack_entry_t *entry = NULL;

	// validation
	if (!stack)
		return -1;

	// create stack entry
	entry = __stack_entry_create(data);
	if (!entry)
		return -2;

	// update stack head
	kdlist_push_head(stack, &entry->node);

	return 0;
}

/**
 * Remove an element from stack.
 *
 * @stack	: Stack data structure.
 *
 * Return element on success and NULL otherwise.
 */
void *stack_pop(stack_t *stack)
{
	void *data;
	stack_t *slot;
	stack_entry_t *entry = NULL;

	// validation
	if (!stack)
		return NULL;

	// pop head
	slot = kdlist_pop_head(stack);
	if (!slot)
		return NULL;

	// get data
	entry = container_of(slot, stack_entry_t, node);
	assert(entry);

	data = entry->data;

	// free node memory
	__stack_entry_destroy(entry);

	return data;
}

/**
 * Return the head element of the stack without removing it.
 *
 * @stack	: Stack data structure.
 *
 * Return element on success and NULL otherwise.
 */
void *stack_top(stack_t *stack)
{
	stack_t *slot;
	stack_entry_t *entry = NULL;

	// validation
	if (!stack)
		return NULL;

	// pop head
	slot = kdlist_get_head(stack);
	if (!slot)
		return NULL;

	// get data
	entry = container_of(slot, stack_entry_t, node);
	assert(entry);

	return entry->data;
}

