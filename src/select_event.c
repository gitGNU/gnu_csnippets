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

/* Hacked off compute_revents in src/poll.c */
static int
compute_revents(int fd, int sought, fd_set *rfds, fd_set *wfds, fd_set *efds)
{
	int happened = 0;

	if (FD_ISSET(fd, rfds)) {
		int r;
		int socket_errno;

# if defined __MACH__ && defined __APPLE__
		/* There is a bug in Mac OS X that causes it to ignore MSG_PEEK
		 for some kinds of descriptors.  Detect if this descriptor is a
		 connected socket, a server socket, or something else using a
		 0-byte recv, and use ioctl(2) to detect hang up.  */
		r = recv (fd, NULL, 0, MSG_PEEK);
		socket_errno = (r < 0) ? errno : 0;
		if (r == 0 || socket_errno == S_ENOTSOCK)
			ioctl (fd, FIONREAD, &r);
#else
		char data[64];
		r = recv (fd, data, sizeof (data), MSG_PEEK);
		socket_errno = (r < 0) ? S_error : 0;
#endif
		/* If the event happened on an unconnected server socket,
		 that's fine. */
		if (r > 0 || ( /* (r == -1) && */ socket_errno == S_ENOTCONN))
			happened |= IO_READ & sought;

		/* Distinguish hung-up sockets from other errors.  */
		else if (socket_errno == S_ESHUTDOWN || socket_errno == S_ECONNRESET
		         || socket_errno == S_ECONNABORTED || socket_errno == S_ENETRESET)
			happened |= IO_ERR;

		/* some systems can't use recv() on non-socket, including HP NonStop */
		else if (/* (r == -1) && */ socket_errno == S_ENOTSOCK)
			happened |= IO_READ & sought;

		else
			happened |= IO_ERR;
	}

	if (FD_ISSET (fd, wfds))
		happened |= IO_WRITE & sought;

	if (FD_ISSET (fd, efds))
		happened |= IO_ERR & sought;

	return happened;
}

static bool grow(struct pollev *pev, size_t s)
{
	alloc_grow(pev->fds, s * sizeof(fd_select_t),
	           return false);
	alloc_grow(pev->events, s * sizeof(fd_select_t *),
	           return false);
	return true;
}

struct pollev *pollev_init(void)
{
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

	if (fd > pev->curr_size) {
		size_t tmp = pev->curr_size;
		pev->curr_size = fd;
		if (!grow(pev, pev->curr_size)) {
			pev->curr_size = tmp;
			return;
		}
	}

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
	int fd, maxfd, rc, i;
	static fd_set rfds, wfds, efds;
	static struct timeval tv;

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_ZERO(&efds);

	assert(timeout > 0 && "Would divide by a zero");
	memset(&tv, 0, sizeof(tv));
	tv.tv_sec  = timeout / 1000;
	tv.tv_usec = (timeout % 1000) * 1000;

	for (maxfd = 0, i = 0; i < pev->curr_size; ++i) {
		fd = pev->fds[i].fd;
		if (test_bit(pev->fds[i].events, IO_READ))
			FD_SET(fd, &rfds);
		if (test_bit(pev->fds[i].events, IO_WRITE))
			FD_SET(fd, &wfds);

		if (fd >= maxfd
		    && (pev->fds[i].events & (IO_READ | IO_WRITE))) {
			maxfd = fd;
			/* Add it to watch-for-exceptions fd set  */
			FD_SET(fd, &efds);
		}
	}

	dbg("polling on %d fds.\n", maxfd + 1);
	do
		rc = select(maxfd + 1, &rfds, &wfds, &efds, &tv);
	while (rc < 0 && S_error == S_EINTR);
	if (rc <= 0)
		return rc;

	/* establish results and create the events array so that we keep compatibility
	 * with other interfaces.  */
	for (rc = 0, i = 0; i <= maxfd; ++i) {
		if (pev->fds[i].fd >= 0) {
			int happened = compute_revents(pev->fds[i].fd, pev->fds[i].events,
							&rfds, &wfds, &efds);
			if (happened) {
				pev->fds[i].revents = happened;
				pev->events[rc++] = &pev->fds[i];
			}
		}
	}

	dbg("Done.  %d fd(s) are ready.\n", rc);
	return rc;
}

__inline int pollev_active(struct pollev *pev, int index)
{
	if (unlikely(index < 0 || index > pev->curr_size))
		return -1;
	return pev->events[index]->fd;
}

__inline uint32_t pollev_revent(struct pollev *pev, int index)
{
	uint32_t r = 0;
	if (unlikely(index < 0 || index > pev->curr_size))
		return r;
	r = pev->events[index]->revents;
	pev->events[index]->revents = 0;
	return r;
}

#endif

