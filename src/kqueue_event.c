/*
 * Copyright (c) 2013 Ahmed Samy  <f.fallen45@gmail.com>
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
#ifdef USE_KQUEUE
#if !defined(BSD) || !defined(__unix__)		\
	|| !(defined(__APPLE__) && !defined(__MACH__))
#error Cannot use KQueue without a BSD system or an Apple with a Mach Kernel
#endif

#include <sys/time.h>
#include <time.h>

typedef struct pollev {
	struct kevent *events;
	size_t size;
	int kq;
} pollev_t;

pollev_t *pollev_init(void)
{
	pollev_t *ev;

	xmalloc(ev, sizeof(pollev_t), return NULL);
	ev->kq = kqueue();
	if (ev->kq < 0) {
		free(ev);
		return NULL;
	}

	ev->size = 1024;
	xmalloc(ev->events, sizeof(struct kevent) * ev->size,
		close(ev->kq); free(ev);
		return NULL
	       );
	return ev;
}

void pollev_deinit(pollev_t *ev)
{
	close(ev->kq);
	free(ev->events);
}

bool pollev_add(pollev_t *ev, int fd, int bits)
{
	struct kevent kev;
	struct timespec ts = {
		.tv_sec = 0,
		.tv_nsec = 0
	};

	if (fd >= ev->size) {
		size_t tmp = ev->size;
		ev->size = fd;
		alloc_grow(ev->events, sizeof(struct kevent),
			    ev->size = tmp; return false);
	}

	memset(&kev, 0, sizeof(kev));
	if (bits & IO_READ) {
		EV_SET(&kev, fd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, NULL);
		if (kevent(ev->kq, &kev, 1, NULL, 0, &ts) == -1)
			return false;
	}

	if (bits & IO_WRITE) {
		memset(&kev, 0, sizeof(kev));
		EV_SET(&kev, fd, EVFILT_WRITE, EV_ADD | EV_CLEAR,  0, 0, NULL);
		if (kevent(ev->kq, &kev, 1, NULL, 0, &ts) == -1)
			return false;
	}

	return true;
}

bool pollev_del(pollev_t *ev, int fd)
{
	return true;
}

int pollev_poll(pollev_t *ev, int timeout)
{
	struct timespec ts;

	ts.tv_sec = timeout / 1000;
	ts.tv_nsec = (timeout % 1000) * 1000000;

	return kevent(ev->kq, NULL, 0, ev->events, ev->size, &ts);
}

int pollev_activefd(pollev_t *ev, int index)
{
	return ev->events[i].ident;
}

short pollev_revent(pollev_t *ev, int index)
{
	short ret;

	switch (ev->events[i].filter) {
	case EVFILT_READ:
		ret = IO_READ;
		break;
	case EVFILT_WRITE:
		ret = IO_WRITE;
		break;
	default:
		return -1;
	}

	return ret;
}

bool pollev_ret(pollev_t *ev, int index, int *fd, short *revents)
{
	if (!ev || (index < 0 || index > ev->size))
		return false;

	*fd = ev->events[index].ident;
	*revents = pollev_revent(ev, index);
	return true;
}

#endif

