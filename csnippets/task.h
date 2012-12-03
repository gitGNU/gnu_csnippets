/*
 * Copyright (c) 2012 Allan Ference  <f.fallen45@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef _TASK_H
#define _TASK_H

#include <csnippets/list.h>

_BEGIN_DECLS

/** The start routine of a task.
 *  This will be called via the thread created by tasks_init().
 */
typedef void (*task_routine) (void *);

typedef struct {
	task_routine start_routine;    /* See above */
	void *param;                   /* The param to call the function (start_routine) with.  */
	struct list_node node;         /* The next and prev task */
} task_t;

/**
 * Initialize tasks thread.
 *
 * This is where all the tasks are executed.  The thread
 * runs indepdent (detached) but can be stopped via tasks_stop().
 *
 * if tasks_stop() is called when there are tasks still waiting,
 * the tasks will be executed before we exit.
 */
extern void tasks_init(void);
/**
 * Returns true if the tasks thread was initialized.
 */
extern bool tasks_running(void);
/**
 * Stops the tasks the thread, this means every waiting every
 * will be executed and memory will be free'd.
 */
extern void tasks_stop(void);
/**
 * Add a task to the task list.
 *
 * If the tasks thread was not initialized this function will throw
 * a warning on console, and will do nothing.
 */
extern void tasks_add(task_t *task);
/**
 * Create a task, NOTE: This does NOT add it to the queue.
 *
 * Returns a malloc'd task with task_routine routine and param param.
 *
 * Add it with task_add(...);
 *
 * Example Usage:
 *     task_add(task_create(my_task, my_param));
 */
extern task_t *task_create(task_routine routine, void *param);

_END_DECLS
#endif  /* _TASK_H */

