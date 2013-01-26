/*
 * Copyright (c) 2012 Ahmed Samy <f.fallen45@gmail.com>.
 * Licensed under MIT, see LICENSE.MIT for details.
 */
#include <csnippets/task.h>

#include <pthread.h>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  cond  = PTHREAD_COND_INITIALIZER;
static LIST_HEAD(tasks);
static bool running = false;
static pthread_t self;

static void *tasks_thread(void __unused *unused)
{
	task_t *task = NULL, *next = NULL;

	while (running) {
		pthread_mutex_lock(&mutex);
		if (list_empty(&tasks)) {
			pthread_cond_wait(&cond, &mutex);
			if (!running)
				break;
		}

		if (!list_empty(&tasks)) {
			task = list_top(&tasks, task_t, node);
			if (!task) {
				pthread_mutex_unlock(&mutex);
				continue;
			}
			list_del_from(&tasks, &task->node);
		}

		pthread_mutex_unlock(&mutex);

		(*task->start_routine) (task->param);
		free(task);
	}

	/* Execute any task waiting */
	list_for_each_safe(&tasks, task, next, node) {
		pthread_mutex_lock(&mutex);
		list_del_from(&tasks, &task->node);
		pthread_mutex_unlock(&mutex);

		(*task->start_routine) (task->param);
		free(task);
	}

	return NULL;
}

void tasks_init(void)
{
	pthread_attr_t attr;
	int rc;

	rc = pthread_attr_init(&attr);
	if (rc != 0)
		fatal("failed to initialize thread attributes\n");

	rc = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	if (rc != 0)
		fatal("failed to setdetachstate");

	running = true;
	rc = pthread_create(&self, &attr, &tasks_thread, NULL);
	if (rc != 0)
		fatal("failed to create thread");
}

void tasks_stop(void)
{
	pthread_mutex_lock(&mutex);
	running = false;
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);

	pthread_join(self, NULL);
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);
}

task_t *task_create(task_routine routine, void *param)
{
	task_t *task;
	if (!routine)
		return NULL;

	xmalloc(task, sizeof(task_t), return NULL);

	task->start_routine = routine;
	task->param = param;
	return task;
}

void tasks_add(task_t *task)
{
	bool empty = false;
	if (!task)
		return;

	pthread_mutex_lock(&mutex);
	if (running) {
		empty = list_empty(&tasks);
		list_add_tail(&tasks, &task->node);
	}
#ifdef _DEBUG_TASKS
	else
		warning("attempting to add a task to a terminated task queue\n");
#endif

	pthread_mutex_unlock(&mutex);
	if (empty)
		pthread_cond_signal(&cond);
}

