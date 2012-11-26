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
#ifndef __linux
#error "Epoll requires Linux"
#endif

#define MAX_EVENTS 1024
#include <csnippets/socket.h>
#include <sys/epoll.h>
#include <unistd.h>   /* close() */

struct sock_events {
    struct epoll_event *events;
    int epoll_fd;
};

uint32_t sockset_revent(struct sock_events *ev, int index)
{
    uint32_t events = ev->events[index].events;
    uint32_t r = 0;

    if (events & EPOLLIN)
        r |= EVENT_READ;
    else if (events & EPOLLOUT)
        r |= EVENT_WRITE;
    return r;
}

__inline __const int sockset_active(struct sock_events *evs, int index)
{
    return evs->events[index].data.fd;;
}

void sockset_add(struct sock_events *evs, int fd, int bit)
{
    struct epoll_event ev;
    if (unlikely(!evs))
        return;

    if (bit & EVENT_READ)
        ev.events |= EPOLLIN;
    if (bit & EVENT_WRITE)
        ev.events |= EPOLLOUT;

    ev.data.fd = fd;
    if (epoll_ctl(evs->epoll_fd, EPOLL_CTL_ADD, fd, &ev) < 0)
        eprintf("sockset_add(): epoll_ctl(%d) returned an error %d(%s)\n",
                fd, errno, strerror(errno));
}

struct sock_events *sockset_init(void)
{
    struct sock_events *ev;

    xmalloc(ev, sizeof(struct sock_events), return NULL);
    if ((ev->epoll_fd = epoll_create1(0)) < 0) {
        perror("epoll_create1");
        free(ev);
        return NULL;
    }

    xcalloc(ev->events, MAX_EVENTS, sizeof(struct epoll_event),
            free(ev); return NULL);
    return ev;
}

void sockset_deinit(struct sock_events *evs)
{
    if (unlikely(!evs))
        return;

    free(evs->events);
    free(evs);
}

void sockset_del(struct sock_events *evs, int fd)
{
    if (unlikely(!evs))
        return;

    if (epoll_ctl(evs->epoll_fd, EPOLL_CTL_DEL, fd, NULL) < 0)
        eprintf("sockset_del(): epoll_ctl(%d) returned an error %d(%s)\n",
                fd, errno, strerror(errno));
}

int sockset_poll(struct sock_events *evs)
{
    int n;
    if (unlikely(!evs))
        return -1;

    do
        n = epoll_wait(evs->epoll_fd, evs->events, MAX_EVENTS, -1);
    while (n == -1 && errno == EINTR);
    return n;
}

int sockset_poll_and_get_fd(struct sock_events *evs, int desired_fd)
{
    int n, index;
    uint32_t flags;
    if (unlikely(!evs))
        return -1;
    do
        n = epoll_wait(evs->epoll_fd, evs->events, MAX_EVENTS, -1);
    while (n == -1 && errno == EINTR);
    return n;
}

#endif

