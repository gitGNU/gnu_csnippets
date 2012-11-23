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

struct sock_events {
    fd_set active_fd_set;
    size_t maxfd;
};

void *sockset_init(int fd)
{
    struct sock_events *ev = malloc(sizeof(struct sock_events));
    if (!ev)
        return NULL;

    FD_ZERO(&ev->active_fd_set);
    FD_SET(fd, &ev->active_fd_set);

    ev->maxfd = fd;
    return ev;
}

void sockset_deinit(void *p)
{
    free(p);
}

void sockset_add(void *p, int fd)
{
    struct sock_events *evs = (struct sock_events *)p;
    if (unlikely(!evs))
        return;
    FD_SET(fd, &evs->active_fd_set);
    if (fd > evs->maxfd)
        evs->maxfd = fd;
}

void sockset_del(void *p, int fd)
{
    struct sock_events *evs = (struct sock_events *)p;
    if (unlikely(!evs))
        return;
    FD_CLR(fd, &evs->active_fd_set);
}

int sockset_poll(socket_t *sock, int desired_fd, connection_t **conn)
{
    int n;
    int fd = 0;
    struct sock_events *evs = sock->events;
    connection_t *ret;
    fd_set rset;

    if (unlikely(!evs))
        return -1;

    rset = evs->active_fd_set;
    n = select(evs->maxfd + 1, &rset, NULL, NULL, NULL);
    if (n < 0)
        return -1;

    if (FD_ISSET(desired_fd, &rset)) {
        return 1;
    } else {
        list_for_each(&sock->children, ret, node) {
            if (++fd > evs->maxfd)
                break;

            if (FD_ISSET(ret->fd, &rset)) {
                *conn = ret;
                return 0;
            }
        }
    }

    return -1;
}

int sockset_poll_and_get_fd(void *events, int desired_fd)
{
    struct sock_events *evs = (struct sock_events *)events;
    int n, fd;
    fd_set rset;
    if (unlikely(!evs))
        return -1;

    rset = evs->active_fd_set;
    n = select(desired_fd + 1, &rset, NULL, NULL, NULL);
    if (n < 0)
        return -1;

    if (FD_ISSET(desired_fd, &rset))
        return 0;
    return -1;
}

#endif

