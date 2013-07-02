/*
 * Copyright (c) 2012 Ahmed Samy <f.fallen45@gmail.com>
 * Licensed under MIT, see LICENSE.MIT for details.
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

struct data {
	int fd;
	short events;
	short revents;
} __packed;

typedef struct pollev {
	struct data fds[FD_SETSIZE];     /* Array of fds to poll on  */
	struct data events[FD_SETSIZE];  /* Events occured.  */
} pollev_t;

static inline int
compute_revents(int fd, int sought, fd_set *rfds, fd_set *wfds, fd_set *efds)
{
	int happened = 0;
	if (FD_ISSET (fd, rfds))
		happened |= IO_READ & sought;

	if (FD_ISSET (fd, wfds))
		happened |= IO_WRITE & sought;

	if (FD_ISSET (fd, efds))
		happened |= IO_ERR & sought;

	return happened;
}

pollev_t *pollev_init(void)
{
	pollev_t *ev;

	xmalloc(ev, sizeof(pollev_t), return NULL);
	memset(ev->fds, 0, sizeof(ev->fds));
	memset(ev->events, 0, sizeof(ev->events));
	return ev;
}

void pollev_deinit(pollev_t *p)
{
	free(p);
}

void pollev_add(pollev_t *pev, int fd, int bits)
{
	if (!pev || fd < 0)
		return;

	if (fd >= FD_SETSIZE) {
#ifdef _DEBUG_POLLEV
		dbg("can't add more file descriptors to this set, the size would exceed the maximum number of file descriptors\n");
#endif
		return;
	}

	if (bits & IO_READ)
		pev->fds[fd].events |= IO_READ;
	if (bits & IO_WRITE)
		pev->fds[fd].events |= IO_WRITE;

	pev->fds[fd].fd = fd;
	pev->fds[fd].revents = 0;
}

void pollev_del(pollev_t *pev, int fd)
{
	int i;
	if (!pev)
		return;

	for (i = 0; i < FD_SETSIZE; ++i)
		if (pev->fds[i].fd == fd) {
			pev->fds[i].fd = -1;
			break;
		}
}

int pollev_poll(pollev_t *pev, int timeout)
{
	int maxfd, rc, i;
	static fd_set rfds, wfds, efds;
	struct timeval tv, *ptv;
	struct data *d;

	if (!pev)
		return -1;

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_ZERO(&efds);

	/* The following time out code is hacked off src/poll.c */
	if (timeout == 0) {
		ptv = &tv;
		tv.tv_sec = 0;
		tv.tv_usec = 0;
	} else if (timeout > 0) {
		ptv = &tv;
		tv.tv_sec  = timeout / 1000;
		tv.tv_usec = (timeout % 1000) * 1000;
	} else if (timeout == -1)
		ptv = NULL;
	else
		return -1;

	for (maxfd = -1, i = 0; i < FD_SETSIZE; ++i) {
		d = &pev->fds[i];
		if (d->fd <= 0)
			continue;
		if (d->events & IO_READ)
			FD_SET(d->fd, &rfds);
		if (d->events & IO_WRITE)
			FD_SET(d->fd, &wfds);
		if (d->events & (IO_READ | IO_WRITE)) {
			FD_SET(d->fd, &efds);
			if (d->fd > maxfd)
				maxfd = d->fd;
		}
	}

#ifdef _DEBUG_POLLEV
	dbg("polling on %d fds.\n", maxfd + 1);
#endif
	do
		rc = select(maxfd + 1, &rfds, &wfds, &efds, ptv);
	while (rc < 0 && S_error == S_EINTR);
	if (rc <= 0)
		return -1;

	/* establish results  */
	for (rc = 0, i = 0; i <= maxfd; ++i) {
		int happened = compute_revents(pev->fds[i].fd, pev->fds[i].events,
						&rfds, &wfds, &efds);
		if (happened) {
			pev->events[rc].revents = happened;
			pev->events[rc].fd = pev->fds[i].fd;
			++rc;
		}
	}

#ifdef _DEBUG_POLLEV
	dbg("Done.  %d fd(s) are ready.\n", rc);
#endif
	return rc;
}

__inline int pollev_activefd(pollev_t *pev, int index)
{
	if (index < 0 || index > FD_SETSIZE)
		return -1;
	return pev->events[index].fd;
}

__inline short pollev_revent(pollev_t *pev, int index)
{
	short r = 0;
	if (index < 0 || index > FD_SETSIZE)
		return r;
	r = pev->events[index].revents;
	pev->events[index].revents = 0;
	pev->events[index].fd = 0;
	return r;
}

bool pollev_ret(pollev_t *pev, int index, int *fd, short *revents)
{
	if (!pev || (index < 0 || index > FD_SETSIZE) || pev->events[index].revents == 0)
		return false;
	*fd = pev->events[index].fd;
	*revents = pev->events[index].revents;
	pev->events[index].revents = 0;
	pev->events[index].fd = 0;
	return true;
}

#endif

