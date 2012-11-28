/*
 * Copyright (c) 2012 Allan Ference <f.fallen45@gmail.com>
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
#if defined USE_SELECT

#include <csnippets/socket.h>
#ifdef _WIN32
#include <winsock2.h>
#endif
#ifndef MAX_EVENTS
#define MAX_EVENTS 1024
#endif

typedef struct fd_select {
	int fd;
	char read : 2;    /* first bit: in use */
	char write : 2;   /* second bit: pending data */
} fd_select_t;

struct sock_events {
	size_t maxfd;
	fd_select_t fds[MAX_EVENTS], **events;
};

struct sock_events *sockset_init(void)
{
	struct sock_events *ev;

	xmalloc(ev, sizeof(struct sock_events), return NULL);
	xcalloc(ev->events, MAX_EVENTS, sizeof(fd_select_t *),
			free(ev); return NULL);
	return ev;
}

void sockset_deinit(struct sock_events *p)
{
	free(p->events);
	free(p);
}

void sockset_add(struct sock_events *evs, int fd, int bits)
{
	if (unlikely(!evs))
		return;

	if (fd > MAX_EVENTS) {
#ifdef _DEBUG_SOCKET
		eprintf("sockset_add(): fd %d is out of range\n", fd);
#endif
		return;
	}

	if (bits & EVENT_READ)
		evs->fds[fd].read |= 1;
	if (bits & EVENT_WRITE)
		evs->fds[fd].write |= 1;
	evs->fds[fd].fd = fd;
}

void sockset_del(struct sock_events *p, int fd)
{
	struct sock_events *evs = (struct sock_events *)p;
	if (unlikely(!evs))
		return;

	if (fd > MAX_EVENTS) {
#ifdef _DEBUG_SOCKET
		eprintf("sockset_del(): fd %d is out of range\n", fd);
#endif
		return;
	}

	evs->fds[fd].read = 0;
	evs->fds[fd].write = 0;
}

int sockset_poll(struct sock_events *evs)
{
	int fd, i, maxfd, numfds;
	fd_set rfds, wfds, efds;

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_ZERO(&efds);

	for (fd = 0, maxfd = 0; fd < MAX_EVENTS; ++fd) {
		if (evs->fds[fd].read)
			FD_SET(fd, &rfds);
		if (evs->fds[fd].write)
			FD_SET(fd, &wfds);
		if (evs->fds[fd].read || evs->fds[fd].write) {
			FD_SET(fd, &efds);
			if (fd > maxfd)
				maxfd = fd;
		}
	}

	errno = 0;
	numfds = select(maxfd + 1, &rfds, &wfds, &efds, NULL);
	if (numfds == -1)
		return -1;

	for (fd = 0; fd <= maxfd; ++fd) {
		if (FD_ISSET(fd, &efds)) {
#ifdef _DEBUG_SOCKET
			eprintf("XXX ignoring fd %d with OOB data\n", fd);
#endif
			continue;
		}

		if (FD_ISSET(fd, &rfds))
			evs->fds[fd].read |= 2;
		else
			evs->fds[fd].read &= 1;
		if (FD_ISSET(fd, &wfds))
			evs->fds[fd].write |= 2;
		else
			evs->fds[fd].write &= 1;
	}

	for (fd = 0, i = 0; fd <= maxfd; ++fd)
		if (FD_ISSET(fd, &rfds) || FD_ISSET(fd, &wfds))
			evs->events[i++] = &evs->fds[fd];
	return i;
}

__inline __const int sockset_active(struct sock_events *evs, int index)
{
	return evs->events[index]->fd;
}

uint32_t sockset_revent(struct sock_events *evs, int index)
{
	uint32_t r;
	int fd = evs->events[index]->fd;

	if (evs->fds[fd].read & 0x02)
		r |= EVENT_READ;
	if (evs->fds[fd].write & 0x02)
		r |= EVENT_WRITE;

	evs->fds[fd].read &= 0x01;
	evs->fds[fd].write &= 0x01;
	return r;
}

#endif

