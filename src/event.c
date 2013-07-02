/*
 * Copyright (c) 2012 Ahmed Samy <f.fallen45@gmail.com>.
 * Licensed under MIT, see LICENSE.MIT for details.
 */
#include <csnippets/list.h>
#include <csnippets/event.h>

#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>

typedef struct event {
	int64_t delay;
	struct task *task;
	struct list_node node;
} event_t;

static pthread_mutex_t mutex        = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  cond         = PTHREAD_COND_INITIALIZER;
static bool            running = false;
static pthread_t       self;
static LIST_HEAD(events);

static void *events_thread(void __unused *unused)
{
	event_t *event = NULL, *next = NULL;
	struct timespec ts;
	struct timeval tv;

	while (running) {
		pthread_mutex_lock(&mutex);
		if (list_empty(&events)) {
			pthread_cond_wait(&cond, &mutex);
			if (!running) {
				pthread_mutex_unlock(&mutex);
				break;
			}
		}
		event = list_top(&events, event_t, node);
		if (!event) {
			pthread_mutex_unlock(&mutex);
			continue;
		}
		list_del(&event->node);

		gettimeofday(&tv, NULL);
		ts.tv_sec  = tv.tv_sec;
		ts.tv_nsec = tv.tv_usec * 1000;
		ts.tv_sec += event->delay;

		pthread_cond_timedwait(&cond, &mutex, &ts);
		pthread_mutex_unlock(&mutex);

		tasks_add(event->task);
		free(event);
	}

	/* if we have any remaining events, add them to tasks  */
	list_for_each_safe(&events, event, next, node) {
		pthread_mutex_lock(&mutex);
		list_del(&event->node);
		pthread_mutex_unlock(&mutex);

		tasks_add(event->task);
		free(event);
	}

	return NULL;
}

void events_init(void)
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
	rc = pthread_create(&self, &attr, &events_thread, NULL);
	if (rc != 0)
		fatal("failed to create thread");
}

void events_stop(void)
{
	pthread_mutex_lock(&mutex);
	running = false;
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);

	pthread_join(self, NULL);
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);
}

event_t *event_create(int delay, task_routine start, void *p)
{
	event_t *event;
	if (!start || delay < 0)
		return NULL;

	xmalloc(event, sizeof(event_t), return NULL);
	event->delay = delay;
	event->task = task_create(start, p);

	if (!event->task) {
		free(event);
		return NULL;
	}

	return event;
}

void events_add(event_t *event)
{
	bool empty = false;
	if (!event)
		return;

	pthread_mutex_lock(&mutex);
	if (running) {
		empty = list_empty(&events);
		list_add_tail(&events, &event->node);
	}
#ifdef _DEBUG_EVENTS
	else
		warning("attempting to add an event to a terminated event queue\n");
#endif

	pthread_mutex_unlock(&mutex);
	if (empty)
		pthread_cond_signal(&cond);
}

