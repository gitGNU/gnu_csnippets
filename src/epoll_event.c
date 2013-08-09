/*
 * Copyright (c) 2012 Ahmed Samy <f.fallen45@gmail.com>
 * Licensed under MIT, see LICENSE.MIT for details.
 */
#ifdef USE_EPOLL
#if !defined(__linux) || !defined(linux) || !defined(__linux__)
#error "Epoll requires Linux"
#endif

#include <internal/socket_compat.h>

#include <csnippets/io_poll.h>
#include <csnippets/socket.h>

#include <sys/epoll.h>

typedef struct pollev {
	struct epoll_event *events;
	size_t size;
	int efd;
} pollev_t;

static inline short compute_revents(pollev_t *ev, int index)
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

pollev_t *pollev_init(void)
{
	pollev_t *ev;

	xmalloc(ev, sizeof(pollev_t), return NULL);
	if ((ev->efd = epoll_create1(0)) < 0) {
#ifdef _DEBUG_POLLEV
		perror("epoll_create1");
#endif
		free(ev);
		return NULL;
	}

	ev->size = 1024;
	xcalloc(ev->events, ev->size, sizeof(struct epoll_event),
	        free(ev); return NULL);
	return ev;
}

void pollev_deinit(pollev_t *pev)
{
	if (!pev)
		return;

	close(pev->efd);
	free(pev->events);
	free(pev);
}

bool pollev_add(pollev_t *pev, int fd, int bits)
{
	struct epoll_event ev;
	if (!pev)
		return false;

	if (fd >= pev->size) {
		size_t tmp = pev->size;
		pev->size = fd;
		alloc_grow(pev->events, sizeof(struct epoll_event),
		            pev->size = tmp; return false);
	}

	ev.events = EPOLLET | EPOLLPRI;
	if (bits & IO_READ)
		ev.events |= EPOLLIN;
	if (bits & IO_WRITE)
		ev.events |= EPOLLOUT;

	memset(&ev.data, 0, sizeof(ev.data));
	ev.data.fd = fd;
	if (epoll_ctl(pev->efd, EPOLL_CTL_ADD, fd, &ev) < 0) {
		eprintf("pollev_add(): epoll_ctl(%d) returned an error %d(%s)\n",
		        fd, errno, strerror(errno));
		return false;
	}

	return true;
}

bool pollev_del(pollev_t *pev, int fd)
{
	if (!pev)
		return false;

	if (epoll_ctl(pev->efd, EPOLL_CTL_DEL, fd, NULL) < 0) {
		eprintf("pollev_del(): epoll_ctl(%d) returned an error %d(%s)\n",
		        fd, S_error, strerror(S_error));
		return false;
	}

	return true;
}

int pollev_poll(pollev_t *pev, int timeout)
{
	int n;
	if (!pev)
		return -1;

	S_seterror(0);
	do
		n = epoll_wait(pev->efd, pev->events, pev->size, timeout);
	while (n == -1 && S_error == S_EINTR);
	return n;
}

__inline int pollev_activefd(pollev_t *pev, int index)
{
	return pev->events[index].data.fd;
}

short pollev_revent(pollev_t *ev, int index)
{
	if (!ev || (index < 0 || index > ev->size))
		return false;
	return compute_revents(ev, index);
}

bool pollev_ret(pollev_t *ev, int index, int *fd, short *revents)
{
	if (!ev || (index < 0 || index > ev->size))
		return false;
	*fd = ev->events[index].data.fd;
	*revents = compute_revents(ev, index);
	return true;
}

#endif

