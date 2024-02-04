/**
 * Stack implementation with doubly linked list.
 * Copyright (C) 2024 Lazar Razvan.
 *
 * The implementation may be improved to create a pool of nodes instead of
 * allocating a node each time
 */
#ifndef STACK_H
#define STACK_H


#include "list/kdoubly_linked_list.h"


/*****************************************************************************/

//
typedef kdlist_node_t stack_t;


/*****************************************************************************/

//
stack_t *stack_create(void);
void stack_destroy(stack_t *stack);
bool stack_is_empty(stack_t *stack);


/*****************************************************************************/

//
int stack_push(stack_t *stack, void *data);
void *stack_pop(stack_t *stack);
void *stack_top(stack_t *stack);

#endif // STACK_H
