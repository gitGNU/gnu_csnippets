/*
 * Copyright (c) 2012-2013 Ahmed Samy <f.fallen45@gmail.com>
 * Licensed under MIT, see LICENSE.MIT for details.
 */
/**
 * This module does not depend on any other modules, therefore
 * it can be placed on top of any project and used safely.
 *
 * Note that it uses the xmalloc, xrealloc, ... macros therefore
 * a replacement or an implementation is needed for this to compile.
 */
#ifndef _STACK_H
#define _STACK_H

/**
 * This stack does not actually act as stacks in C, it was made
 * to be used on "stack" and not heap allocated but of course can be
 * heap-allocated.  This is sometimes referenced as "Dynamic Array"
 *
 * 'ptr' is dynamicly allocated of course depending on the size
 * needed, see stack_push().
 */
typedef struct stack {
	void **ptr;     /* the internel array */
	size_t size;
} stack_t;

#define INITIAL_SIZE 10
#define SIZE_INCREMENT 2

#define stack_foreach(stack, out)	\
	for ((out) = &(stack).ptr[0]; (out) < &(stack).ptr[(stack).size]; ++(out))

/**
 * Initialize stack `s'.  Allocates memory of size `size` if > 0
 * otherwise uses INITIAL_SIZE
 *
 * This does NOT allocate memory for the stack itself, it's intended to
 * be used like this:
 *
 *    stack_t stack;
 *    if (!stack_init(&stack, 0))
 *        ...
 *  But as note, it's not a *MUST* to use it on stack only.
 *
 * \sa stack_free().
 */
extern bool stack_init(stack_t *s, size_t size);

/**
 * Free memory used.
 * Note: This does not free the stack itself.
 *
 * \sa stack_push().
 */
extern void stack_free(stack_t *s, bool freeAll);


/**
 * Preserve some memory of size `new_size'.
 * Does not free previous memory.
 * This is called whenever memory is needed (Internal use).
 */
extern bool stack_grow(stack_t *s, int newSize);

/**
 * Push item `ptr' on this stack
 *
 * `where' can be -1 if we have to figure out the place ourselves.
 * Specifiying where is good when the user know where to place (saves some cycles).
 *
 * constructor can be NULL if not needed.
 *
 * \returns -1 on failure or pos of where the item is placed.
 * \sa stack_pop(), stack_top(), stack_remove().
 */
extern int stack_push(stack_t *s, void *ptr, int where, void (*constructor) (void *));

/**
 * Pop an item from the top stack.
 *
 * The user must free the pointer himself (and null terminate if possible).
 *
 * \sa stack_top()
 */
extern void *stack_pop(stack_t *s);

/**
 * Get an item off the top of the stack.
 *
 * This keeps the pointer on the stack.
 *
 * \sa stack_remove(), stack_pop()
 */
extern void *stack_top(stack_t *s);

/**
 * Remove an item from the stack.
 *
 * If compare_function is specified, it's used instead.
 * This free's the pointer `ptr' unless destructor is specified.
 * duplicate is useful when the user knows the item can be duplicated.
 *
 * \sa stack_push().
 */
bool stack_remove(stack_t *s, void *ptr, bool (*compare_function) (const void *, const void *),
                         void (*destructor) (void *), bool duplicate);

#endif  /* _STACK_H */

