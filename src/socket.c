/*
 * Copyright (c) 2012 Ahmed Samy <f.fallen45@gmail.com>
 *
 * The first few static net_* functions are hacked off ccan/net.
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
#include <csnippets/socket.h>
#include <csnippets/io_poll.h>   /* IO event-based polling.  */
#include <csnippets/asprintf.h>  /* Needed in conn_writestr  */
#include <csnippets/poll.h>      /* Fake poll(2) enviroment that is cross-platform.  (Part of gnulib) */
#include <csnippets/list.h>

#include <internal/socket_compat.h>  /* Internal definitions, for compatibility with various platforms.  */

struct sk_buff {
	char *data;
	size_t size;
};

struct listener {
	int fd;

	bool (*fn) (struct conn *, void *arg);
	void *arg;

	struct list_node node;
};

struct conn {
	int fd;

	bool (*next) (struct conn *, void *);
	void *argp;

	bool (*fn) (struct conn *, void *);
	void *farg;
	bool in_progress;

	struct sockaddr sa;
	int sa_len;

	struct sk_buff wb;
	struct list_node node;
};

static struct pollev *io_events;
static LIST_HEAD(conns);      /* XXX if many connections, searching will be slow!  */
static LIST_HEAD(listeners);

static struct conn *find_conn(int fd)
{
	struct conn *ret;

	list_for_each(&conns, ret, node)
		if (ret->fd == fd)
			return ret;
	return NULL;
}

static struct listener *find_listener(int fd)
{
	struct listener *ret;

	list_for_each(&listeners, ret, node)
		if (ret->fd == fd)
			return ret;
	return NULL;
}

static bool do_write(struct conn *conn, const void *data, size_t len)
{
	int t_bytes = 0, r_bytes, n = -1;
	struct sk_buff *skb;
	if (unlikely(!conn))
		return false;

	r_bytes = len;
	skb = &conn->wb;

	might_bug();
	while (t_bytes < len) {
		if (!skb->data)
			do
				n = send(conn->fd, data + t_bytes, r_bytes, 0);
			while (n == -1 && S_error == S_EINTR);

		if (n < 0) {
			if ((IsBlocking() && r_bytes > 0) || skb->data) {
				if (!skb->data) {
					skb->size = r_bytes + 128;
					xmalloc(skb->data, sizeof(char) * skb->size,
					        return -1);
				} else {
					skb->size += r_bytes;
					alloc_grow(skb->data, sizeof(char) * skb->size,
					           return -1);
				}

				memcpy(skb->data + (skb->size - r_bytes), data + t_bytes, r_bytes);
				return false;
			}

			break;
		}

		t_bytes += n;
		r_bytes -= n;
	}

	return true;
}

static bool do_write_queue(struct conn *conn)
{
	int t_bytes = 0, r_bytes, n;
	struct sk_buff *skb;

	if (unlikely(!conn || !conn->wb.data))
		return true;

	skb = &conn->wb;
	r_bytes = skb->size;

	might_bug();
	while (t_bytes < skb->size) {
		do
			n = send(conn->fd, skb->data + t_bytes, r_bytes, 0);
		while (n == -1 && S_error == S_EINTR);
		if (n == -1) {
			if (IsBlocking() && r_bytes > 0) {
				/* Not everything has been sent yet...  Store for next.  */
				memmove(skb->data, skb->data + t_bytes, r_bytes);
				skb->size = r_bytes;
				return false;
			}
			break;
		}

		t_bytes += n;
		r_bytes -= n;
	}

	skb->size = 0;
	free(skb->data);
	skb->data = NULL;

	return true;
}

static __init __used void __sock_startup(void)
{
#ifdef _WIN32
	/* Initialise windows sockets */
	WSADATA wsa;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsa)) {
		WSACleanup();
		eprintf("WSAStartup(2, 2) failed\n");
		/* We shall abort after an error like this has occured,
		 * the API won't work at all.  */
		abort();
	}
#endif

	/* Initialise polling  */
	io_events = pollev_init();
	if (!io_events) {
		/* Unstoppable error, we have to abort.
		 * Polling won't work */
		eprintf("%s:%d: %s: initialising input and output events has failed due to OOM (Out of memory!)\n",
		        __FILE__, __LINE__, __func__);
		abort();
	}
}

static __exit __used void __sock_cleanup(void)
{
#ifdef _WIN32
	WSACleanup();
#endif
	if (io_events)
		pollev_deinit(io_events);
}

static struct addrinfo *
net_lookup(const char *node, const char *service,
           int family, int socktype) {
	struct addrinfo hints;
	struct addrinfo *res;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = family;
	hints.ai_socktype = socktype;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;

	if (getaddrinfo(node, service, &hints, &res) != 0)
		return NULL;

	return res;
}

static bool set_nonblock(int fd)
{
	long flags;
#ifndef _WIN32
	flags = fcntl(fd, F_GETFL);
	if (flags == -1)
		return false;

	if (!(flags & O_NONBLOCK))
		flags |= O_NONBLOCK;
	return fcntl(fd, F_SETFL, flags) == 0;
#else
	flags = 1;
	return ioctlsocket(fd, FIONBIO, (u_long *)flags) == 0;
#endif
}

/* We only handle IPv4 and IPv6 */
#define MAX_PROTOS 2

static void remove_fd(
        struct pollfd *pfd,
        const struct addrinfo **addr,
        socklen_t *slen,
        unsigned int *num,
        unsigned int i
)
{
	memmove(pfd  + i, pfd  + i + 1, (*num - i - 1) * sizeof(pfd [0]));
	memmove(addr + i, addr + i + 1, (*num - i - 1) * sizeof(addr[0]));
	memmove(slen + i, slen + i + 1, (*num - i - 1) * sizeof(slen[0]));
	(*num)--;
}

static int net_connect(const struct addrinfo *addrinfo)
{
	int sockfd = -1, saved_error;
	unsigned int i, num;
	const struct addrinfo *ipv4 = NULL, *ipv6 = NULL;
	const struct addrinfo *addr[MAX_PROTOS];
	socklen_t slen[MAX_PROTOS];
	struct pollfd pfd[MAX_PROTOS];

	for (; addrinfo; addrinfo = addrinfo->ai_next) {
		switch (addrinfo->ai_family) {
		case AF_INET:
			if (!ipv4)
				ipv4 = addrinfo;
			break;
		case AF_INET6:
			if (!ipv6)
				ipv6 = addrinfo;
			break;
		}
	}

	num = 0;
	/* We give IPv6 a slight edge by connecting it first. */
	if (ipv6) {
		addr[num] = ipv6;
		slen[num] = sizeof(struct sockaddr_in6);
		pfd[num].fd = socket(AF_INET6, ipv6->ai_socktype,
		                     ipv6->ai_protocol);
		if (pfd[num].fd != -1)
			num++;
	}

	if (ipv4) {
		addr[num] = ipv4;
		slen[num] = sizeof(struct sockaddr_in);
		pfd[num].fd = socket(AF_INET, ipv4->ai_socktype,
		                     ipv4->ai_protocol);
		if (pfd[num].fd != -1)
			num++;
	}

	for (i = 0; i < num; i++) {
		if (!set_nonblock(pfd[i].fd)) {
			remove_fd(pfd, addr, slen, &num, i--);
			continue;
		}
		/* Connect *can* be instant. */
		if (connect(pfd[i].fd, addr[i]->ai_addr, slen[i]) == 0)
			goto got_one;

		if (S_error != S_EINPROGRESS) {
			/* Remove dead one. */
			remove_fd(pfd, addr, slen, &num, i--);
		}
		pfd[i].events = POLLOUT;
	}

	while (num && poll(pfd, num, -1) != -1) {
		for (i = 0; i < num; i++) {
			int err;
			socklen_t errlen = sizeof(err);
			if (!pfd[i].revents)
				continue;
			if (getsockopt(pfd[i].fd, SOL_SOCKET, SO_ERROR, &err, &errlen) != 0)
				goto out;
			if (err == 0)
				goto got_one;

			/* Remove dead one. */
			S_seterror(err);
			remove_fd(pfd, addr, slen, &num, i--);
		}
	}

got_one:
	sockfd = pfd[i].fd;

out:
	saved_error = S_error;
	for (i = 0; i < num; i++)
		if (pfd[i].fd != sockfd)
			S_close(pfd[i].fd);
	S_seterror(saved_error);
	return sockfd;
}

bool _new_listener(const char *service,
                  bool (*fn) (struct conn *, void *arg),
                  void *arg)
{
	int reuse_addr;
	struct addrinfo *addr;
	struct listener *ret;
	int fd;

	addr = net_lookup(NULL, service, AF_UNSPEC, SOCK_STREAM);
	if (!addr)
		return false;

	fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
	if (fd < 0) {
		freeaddrinfo(addr);
		return false;
	}

	set_nonblock(fd);
	reuse_addr = 1; /* ON */
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(int)) != 0
	    || bind(fd, addr->ai_addr, addr->ai_addrlen) != 0
	    || listen(fd, 2048) != 0) {
		freeaddrinfo(addr);
		S_close(fd);
		return false;
	}
	freeaddrinfo(addr);

	xmalloc(ret, sizeof(*ret),
	        S_close(fd); return false);
	list_add_tail(&listeners, &ret->node);
	pollev_add(io_events, fd, IO_READ);

	ret->fd  = fd;
	ret->fn  = fn;
	ret->arg = arg;
	return true;
}

bool _new_conn(const char *node, const char *service,
              bool (*fn) (struct conn *, void *arg),
              void *arg)
{
	struct addrinfo *addr;
	struct conn *conn;
	int fd;

	addr = net_lookup(node, service, AF_UNSPEC, SOCK_STREAM);
	if (!addr)
		return false;

	fd = net_connect(addr);
	conn = new_conn_fd(fd, fn, arg);
	if (!conn)
		S_close(fd);
	else {
		conn->sa = *addr->ai_addr;
		conn->sa_len = addr->ai_addrlen;
	}

	freeaddrinfo(addr);
	return !!conn;
}

bool free_conn(struct conn *conn)
{
	bool retval = false;
	if (unlikely(!conn))
		return retval;

	list_del(&conn->node);
	pollev_del(io_events, conn->fd);
	if (S_close(conn->fd) == 0)
		retval = true;

	free(conn);
	return retval;
}

struct conn *_new_conn_fd(int fd,
                         bool (*fn) (struct conn *, void *arg),
                         void *arg)
{
	struct conn *ret;
	if (unlikely(fd < 0))
		return NULL;

	xmalloc(ret, sizeof(*ret), return NULL);
	ret->fd    = fd;
	ret->fn    = fn;
	ret->farg  = arg;
	ret->in_progress = true;

	list_add_tail(&conns, &ret->node);
	pollev_add(io_events, ret->fd, IO_READ | IO_WRITE);
	return ret;
}

bool conn_read(struct conn *conn, void *data, size_t *len)
{
	ssize_t count;
	if (unlikely(!conn))
		return false;

	S_seterror(0);
	do
		count = recv(conn->fd, data, *len, 0);
	while (count == -1 && S_error == S_EINTR);
	if (count <= 0)
		*len = 0;
	else
		*len = count;
	return !(count <= 0 && !IsBlocking());
}

bool conn_write(struct conn *conn, const void *data, size_t len)
{
	if (unlikely(!conn))
		return false;
	return do_write(conn, data, len);
}

bool conn_writestr(struct conn *conn, const char *fmt, ...)
{
	char *ptr;
	va_list ap;
	int len;
	bool ret;
	if (unlikely(!conn))
		return false;

	va_start(ap, fmt);
	len = vasprintf(&ptr, fmt, ap);
	va_end(ap);

	if (!ptr)
		return false;
	ret = do_write(conn, ptr, len);
	free(ptr);
	return ret;
}

bool _conn_next(struct conn *c,
               bool (*next) (struct conn *, void *arg),
               void *arg)
{
	if (unlikely(!c))
		return false;

	c->next = next;
	c->argp = arg;
	return true;
}

void next_close(struct conn *conn, void *arg)
{
	if (unlikely(!conn))
		return;

	if (conn->next)
		conn->next(conn, arg);
	assert(free_conn(conn));
}

bool conn_getopt(struct conn *conn, int optname, void *optval,
                 int *optlen)
{
	if (unlikely(!conn))
		return false;
	return getsockopt(conn->fd, SOL_SOCKET, optname,
	                  optval, (socklen_t *)optlen) == 0;
}

bool conn_setopt(struct conn *conn, int optname, const void *optval,
                 int optlen)
{
	if (unlikely(!conn))
		return false;
	return setsockopt(conn->fd, SOL_SOCKET, optname,
	                  optval, (socklen_t)optlen) == 0;
}

bool conn_getnameinfo(struct conn *conn,
                      char *host, size_t hostlen,
                      char *serv, size_t servlen,
                      bool numeric_host,
                      bool numeric_serv)
{
	int flags = 0;
	if (unlikely(!conn))
		return false;
	if (numeric_host)
		flags |= NI_NUMERICHOST;
	if (numeric_serv)
		flags |= NI_NUMERICSERV;
	return getnameinfo((const struct sockaddr *)&conn->sa, conn->sa_len,
	                   host, hostlen,
	                   serv, servlen,
	                   flags) == 0;
}

void *conn_loop(void)
{
	struct conn *conn;
	struct listener *li;

	while (!list_empty(&listeners) || !list_empty(&conns)) {
		int nfds, i;

		/* 1 second time out  */
		nfds = pollev_poll(io_events, 1000);
		if (nfds < 0)
			continue;

		for (i = 0; i < nfds; ++i) {
			int fd, revent;

			fd = pollev_active(io_events, i);
			if (fd < 0)
				continue;
			revent = pollev_revent(io_events, i);
			if ((li = find_listener(fd))) {
				if (test_bit(revent, IO_ERR)) {
#ifdef _DEBUG_SOCKET
					eprintf("Closing listener %d (error occured)\n", fd);
#endif
					list_del(&li->node);
					pollev_del(io_events, li->fd);
					S_close(li->fd);
					free(li);
					continue;
				}

				while (1) {
					struct sockaddr in_addr;
					socklen_t in_len = sizeof(in_addr);
					int in_fd;

					S_seterror(0);
					do
						in_fd = accept(li->fd, &in_addr, &in_len);
					while (in_fd == -1 && S_error == S_EINTR);
					if (in_fd == -1) {
						if (IsBlocking())
							break;
						continue;
					}

					set_nonblock(in_fd);
					conn = new_conn_fd(in_fd, NULL, NULL);
					if (!conn) {
						S_close(in_fd);
						break;
					}

					conn->sa = in_addr;
					conn->sa_len = in_len;
					if (likely(li->fn) && !li->fn(conn, li->arg))
						assert(free_conn(conn));
				}
			} else if ((conn = find_conn(fd))) {
				if (test_bit(revent, IO_ERR)) {
#ifdef _DEBUG_SOCKET
					eprintf("Closing %d (error occured)\n", fd);
#endif
					free_conn(conn);
					continue;
				}

				if (test_bit(revent, IO_WRITE)) {
					if (conn->in_progress) {
						int err = 0;
						socklen_t errlen = sizeof(err);

						if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &errlen) == 0
						    && err == 0) {
							conn->in_progress = false;
							if (conn->fn && !conn->fn(conn, conn->farg))
								assert(free_conn(conn));
						} else /* Disconnected?  */
							assert(free_conn(conn));
						continue;
					} else if (conn->wb.data) {
#ifdef _DEBUG_SOCKET
						eprintf("sending incomplete data...\n");
#endif
						if (!do_write_queue(conn) && !IsBlocking())
							assert(free_conn(conn));
						continue;
					}
				}

				if (conn->next && !conn->next(conn, conn->argp))
					assert(free_conn(conn));
			}
		}
	}

	dbg("Reached end of conn_loop()\n");
	return NULL;
}

