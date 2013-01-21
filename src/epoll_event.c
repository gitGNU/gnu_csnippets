/*
 * Copyright (c) 2012 Ahmed Samy <f.fallen45@gmail.com>
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

#include <internal/socket_compat.h>

#include <csnippets/io_poll.h>
#include <csnippets/socket.h>

#include <sys/epoll.h>

struct pollev {
	struct epoll_event *events;
	size_t curr_size;
	int efd;
};

struct pollev *pollev_init(void) {
	struct pollev *ev;

	xmalloc(ev, sizeof(struct pollev), return NULL);
	if ((ev->efd = epoll_create1(0)) < 0) {
#ifdef _DEBUG_POLLEV
		perror("epoll_create1");
#endif
		free(ev);
		return NULL;
	}

	ev->curr_size = 1024;
	xcalloc(ev->events, ev->curr_size, sizeof(struct epoll_event),
	        free(ev); return NULL);
	return ev;
}

void pollev_deinit(struct pollev *pev)
{
	if (unlikely(!pev))
		return;

	close(pev->efd);
	free(pev->events);
	free(pev);
}

void pollev_add(struct pollev *pev, int fd, int bits)
{
	struct epoll_event ev;
	if (unlikely(!pev))
		return;

	if (fd >= pev->curr_size)
		alloc_grow(pev->events, (pev->curr_size *= 2) * sizeof(struct epoll_event),
		           pev->curr_size /= 2; return);

	ev.events = EPOLLET | EPOLLPRI;
	if (bits & IO_READ)
		ev.events |= EPOLLIN;
	if (bits & IO_WRITE)
		ev.events |= EPOLLOUT;

	memset(&ev.data, 0, sizeof(ev.data));
	ev.data.fd = fd;
	if (epoll_ctl(pev->efd, EPOLL_CTL_ADD, fd, &ev) < 0)
		eprintf("pollev_add(): epoll_ctl(%d) returned an error %d(%s)\n",
		        fd, errno, strerror(errno));
}

void pollev_del(struct pollev *pev, int fd)
{
	if (unlikely(!pev))
		return;

	if (epoll_ctl(pev->efd, EPOLL_CTL_DEL, fd, NULL) < 0)
		eprintf("pollev_del(): epoll_ctl(%d) returned an error %d(%s)\n",
		        fd, S_error, strerror(S_error));
}

int pollev_poll(struct pollev *pev, int timeout)
{
	int n;
	if (unlikely(!pev))
		return -1;

	S_seterror(0);
	do
		n = epoll_wait(pev->efd, pev->events, pev->curr_size,
		               timeout);
	while (n == -1 && S_error == S_EINTR);
	return n;
}

__inline int pollev_activefd(struct pollev *pev, int index)
{
	return pev->events[index].data.fd;
}

short pollev_revent(struct pollev *ev, int index)
{
	uint32_t events = ev->events[index].events;
	short r = 0;

	if (events & EPOLLIN)
		r |= IO_READ;
	if (events & EPOLLOUT)
		r |= IO_WRITE;
	if (events & EPOLLERR || events & EPOLLHUP)
		r |= IO_ERR;
	return r;
}

#endif

