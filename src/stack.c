/*
 * Copyright (c) 2013 Ahmed Samy  <f.fallen45@gmail.com>
 * Licensed under MIT, see LICENSE.MIT for details.
 */
#include "stack.h"

bool stack_init(stack_t *s, size_t size)
{
	if (size == 0)
		size = INITIAL_SIZE;
	s->ptr = calloc(size, sizeof(void *));
	if (!s->ptr)
		return false;
	s->size = size;
	return true;
}

void stack_free(stack_t *s, bool freeAll)
{
	int i;

	for (i = 0; i < s->size; ++i)
		if (s->ptr[i])
			free(s->ptr[i]);

	free(s->ptr);
	s->size = 0;
	s->ptr = NULL;
}

bool stack_grow(stack_t *s, int new_size)
{
	void *tmp;

	tmp = realloc(s->ptr, new_size * sizeof(void *));
	if (!tmp)
		return false;

	s->ptr = tmp;
	s->size = new_size;
	return true;
}

int stack_push(stack_t *s, void *ptr, int where, void (*constructor) (void *))
{
	int place = where;

	if (place < 0) {
		/* Find the first empty place.  */
		for (place = 0; place < s->size && s->ptr[place]; ++place);
		/* If there's no space left, reallocate  */
		if (place == s->size && s->ptr[place] != NULL
		    && !stack_grow(s, s->size * SIZE_INCREMENT))
			return -1;
	} else if (place > s->size && !stack_grow(s, place * SIZE_INCREMENT))
		return -1;

	s->ptr[place] = ptr;
	if (constructor)
		(*constructor) (ptr);
	return place;
}

void *stack_pop(stack_t *s)
{
	return s ? s->ptr[--s->size] : NULL;
}

void *stack_top(stack_t *s)
{
	return s ? s->ptr[s->size - 1] : NULL;
}

bool stack_remove(stack_t *s, void *ptr, bool (*compare_function) (const void *, const void *),
                         void (*destructor) (void *), bool duplicate)
{
	int i;
	bool r = false;

	for (i = 0; i < s->size; ++i) {
		if (!compare_function) {
			r = !!(s->ptr[i] == ptr);
			if (r) {
				if (!destructor)
					free(s->ptr[i]);
				else
					(*destructor) (s->ptr[i]);
				s->ptr[i] = NULL;
			}
		} else {
			r = (*compare_function) (s->ptr[i], ptr);
			if (r) {
				if (!destructor)
					free(s->ptr[i]);
				else
					(*destructor) (s->ptr[i]);
				s->ptr[i] = NULL;
			}
		}

		if (!duplicate && r)
			break;
	}

	return r;
}

