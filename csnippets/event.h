/*
 * Copyright (c) 2012 Ahmed Samy <f.fallen45@gmail.com>
 * Licensed under MIT, see LICENSE.MIT for details.
 */
#ifndef _EVENT_H
#define _EVENT_H

#include <csnippets/task.h>

_BEGIN_DECLS

typedef struct event event_t;

/**
 * Initialize events thread
 *
 * Creates a thread that runs indepdently and can be stopped
 * via calling events_stop().
 */
extern void events_init(void);
/**
 * Stop events thread, this adds any running event to the task queue.
 *
 */
extern void events_stop(void);
/**
 * Add an event to the event list.
 *
 * @param event, create it with event_create().
 */
extern void events_add(event_t *event);
/**
 * Create an event, NOTE: This does NOT add it to the list.
 * You must add it manually via events_add().
 *
 * Example usage:
 *    events_add(event_create(10, my_func, my_param));
 */
extern event_t *event_create(int delay, task_routine start, void *p);

_END_DECLS
#endif  /* _EVENT_H */

