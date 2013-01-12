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
#ifdef USE_SELECT

#include <internal/socket_compat.h>
#include <csnippets/io_poll.h>
#include <csnippets/socket.h>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/time.h>
#endif

#ifndef MAX_EVENTS
#define MAX_EVENTS 1024
#endif

typedef struct fd_select {
	int fd;
	uint32_t events;
	uint32_t revents;
} fd_select_t;

struct pollev {
	size_t curr_size;
	fd_select_t *fds;
	fd_select_t **events;        /* Pointer to fds  */
};

static bool grow(struct pollev *pev, size_t s)
{
	alloc_grow(pev->fds, s * sizeof(fd_select_t),
	           return false);
	alloc_grow(pev->events, s * sizeof(fd_select_t *),
	           return false);
	return true;
}

struct pollev *pollev_init(void) {
	struct pollev *ev;

	xmalloc(ev, sizeof(struct pollev), return NULL);
	ev->curr_size = MAX_EVENTS;
	if (!grow(ev, ev->curr_size)) {
		free(ev);
		return NULL;
	}

	return ev;
}

void pollev_deinit(struct pollev *p)
{
	free(p->fds);
	free(p->events);
	free(p);
}

void pollev_add(struct pollev *pev, int fd, int bits)
{
	if (unlikely(!pev || fd < 0))
		return;

	if (fd >= pev->curr_size) {
		pev->curr_size *= 2;
		if (!grow(pev, pev->curr_size)) {
			pev->curr_size /= 2;
			return;
		}
	}

	pev->fds[fd].events = 0;
	if (bits & IO_READ)
		pev->fds[fd].events |= IO_READ;
	if (bits & IO_WRITE)
		pev->fds[fd].events |= IO_WRITE;

	pev->fds[fd].fd = fd;
	pev->fds[fd].revents = 0;
}

void pollev_del(struct pollev *pev, int fd)
{
	if (unlikely(!pev))
		return;

	if (fd > pev->curr_size) {
#ifdef _DEBUG_POLLEV
		eprintf("pollev_del(): fd %d is out of range (would overflow)\n", fd);
#endif
		return;
	}

	pev->fds[fd].events = 0;
	pev->fds[fd].revents = 0;
	pev->fds[fd].fd = -1;
}

int pollev_poll(struct pollev *pev, int timeout)
{
	int fd, maxfd, rc;
	static fd_set rfds, wfds, efds;
	static struct timeval tv;

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_ZERO(&efds);

	assert(timeout > 0 && "Would divide by a zero");
	memset(&tv, 0, sizeof(tv));
	tv.tv_sec  = timeout / 1000;
	tv.tv_usec = (timeout % 1000) * 1000;

	for (fd = 0, maxfd = 0; fd < pev->curr_size; fd++) {
		if (test_bit(pev->fds[fd].events, IO_READ))
			FD_SET(fd, &rfds);
		if (test_bit(pev->fds[fd].events, IO_WRITE))
			FD_SET(fd, &wfds);

		if (fd >= maxfd
		    && (test_bit(pev->fds[fd].events, IO_READ)
		        || test_bit(pev->fds[fd].events, IO_WRITE))) {
			maxfd = fd;
			/* Add it to watch-for-exceptions fd set  */
			FD_SET(fd, &efds);
		}
	}

	do
		rc = select(maxfd + 1, &rfds, &wfds, &efds, &tv);
	while (rc < 0 && s_error == s_EINTR);
	if (rc < 0)
		return rc;
	/* establish results and create the events array so that we keep compatibility
	 * with other interfaces.  */
	rc = 0;
	for (fd = 0; fd <= maxfd; fd++) {
		if (pev->fds[fd].fd < 0)
			continue;

		if (FD_ISSET(fd, &rfds))
			pev->fds[fd].revents |= IO_READ;
		if (FD_ISSET(fd, &wfds))
			pev->fds[fd].revents |= IO_WRITE;
		if (FD_ISSET(fd, &efds))
			pev->fds[fd].revents |= IO_ERR;
		if (FD_ISSET(fd, &rfds) || FD_ISSET(fd, &wfds)
		    || FD_ISSET(fd, &efds)) {
			pev->events[rc] = &pev->fds[fd];
			++rc;
		}
	}

	return rc;
}

__inline __const int pollev_active(struct pollev *pev, int index)
{
	if (unlikely(index < 0 || index > pev->curr_size))
		return -1;
	return pev->events[index]->fd;
}

__inline __const uint32_t pollev_revent(struct pollev *pev, int index)
{
	uint32_t r = 0;
	if (unlikely(index < 0 || index > pev->curr_size))
		return r;
	r = pev->events[index]->revents;
	pev->events[index]->revents = 0;
	return r;
}

#endif

