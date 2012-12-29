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
#ifdef USE_EPOLL
#if !defined(__linux) || !defined(linux) || !defined(__linux__)
#error "Epoll requires Linux"
#endif

#ifndef MAX_EVENTS
#define MAX_EVENTS 1024
#endif

#include <internal/socket_compat.h>

#include <csnippets/io_poll.h>
#include <csnippets/socket.h>

#include <sys/epoll.h>
#include <unistd.h>   /* close() */

struct pollev {
	struct epoll_event *events;
	int epoll_fd;
};

struct pollev *pollev_init(void)
{
	struct pollev *ev;

	xmalloc(ev, sizeof(struct pollev), return NULL);
	if ((ev->epoll_fd = epoll_create1(0)) < 0) {
#ifdef _DEBUG_SOCKET
		perror("epoll_create1");
#endif
		free(ev);
		return NULL;
	}

	xcalloc(ev->events, MAX_EVENTS, sizeof(struct epoll_event),
			free(ev); return NULL);
	return ev;
}

void pollev_deinit(struct pollev *evs)
{
	if (unlikely(!evs))
		return;

	close(evs->epoll_fd);
	free(evs->events);
	free(evs);
}

void pollev_add(struct pollev *evs, int fd, int bits)
{
	struct epoll_event ev;
	if (unlikely(!evs))
		return;

	ev.events = EPOLLPRI;
	if (bits & IO_READ)
		ev.events |= EPOLLIN;
	if (bits & IO_WRITE)
		ev.events |= EPOLLOUT;

	memset(&ev.data, 0, sizeof(ev.data));
	ev.data.fd = fd;
	if (epoll_ctl(evs->epoll_fd, EPOLL_CTL_ADD, fd, &ev) < 0)
		eprintf("pollev_add(): epoll_ctl(%d) returned an error %d(%s)\n",
		        fd, errno, strerror(errno));
}

void pollev_del(struct pollev *evs, int fd)
{
	if (unlikely(!evs))
		return;

	if (epoll_ctl(evs->epoll_fd, EPOLL_CTL_DEL, fd, NULL) < 0)
		eprintf("pollev_del(): epoll_ctl(%d) returned an error %d(%s)\n",
		        fd, s_error, strerror(s_error));
}

int pollev_poll(struct pollev *evs)
{
	int n;
	if (unlikely(!evs))
		return -1;

	s_seterror(0);
	do
		n = epoll_wait(evs->epoll_fd, evs->events, MAX_EVENTS, -1);
	while (n == -1 && errno == s_EINTR);
	return n;
}

uint32_t pollev_revent(struct pollev *ev, int index)
{
	uint32_t events = ev->events[index].events;
	uint32_t r = 0;

	if (unlikely(events & EPOLLHUP || events & EPOLLERR)) {
		r |= IO_ERR;
		return r;
	}

	if (events & EPOLLIN)
		r |= IO_READ;
	if (events & EPOLLOUT)
		r |= IO_WRITE;
	return r;
}

__inline __const int pollev_active(struct pollev *evs, int index)
{
	return evs->events[index].data.fd;
}

#endif

