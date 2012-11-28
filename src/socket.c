/*
 * Copyright (c) 2012 Allan Ference <f.fallen45@gmail.com>
 *
 * The first few static net_* functions are borrowed from ccan/net.
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
#include <csnippets/asprintf.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>    /* socklen_t */
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#endif
#include <csnippets/poll.h>
#include <time.h>

#ifdef _WIN32
static bool is_initialized = false;
#define SOCK_INIT() do { \
	WSADATA wsa; \
	if (!is_initialized) { \
		if (0 != WSAStartup(MAKEWORD(2, 2), &wsa)) { \
			WSACleanup(); \
			warning("WSAStartup() failed\n"); \
			return NULL; \
		} \
		is_initialized = true; \
	} \
} while (0)
static __exit __unused void __cleanup(void)
{
	is_initialized = false;
	WSACleanup();
}
#else
#define SOCK_INIT()
#endif

#if (EAGAIN == EWOULDBLOCK)
static __inline __const bool IsBlocking(void)
{
	return errno == EWOULDBLOCK;
}
#else
static __inline __const bool IsBlocking(void)
{
	return errno == EWOULDBLOCK || errno == EAGAIN;
}
#endif
#define IsAvail(sops, func) ((sops)->func != NULL ? true : false)
#define callop(sops, func, ...) (sops)->func (__VA_ARGS__)

/* Boring, polling specific functions  */
extern struct sock_events * sockset_init(void);
extern void                 sockset_deinit(struct sock_events *);
extern void                 sockset_add(struct sock_events *, int, int);
extern void                 sockset_del(struct sock_events *, int);
extern int                  sockset_poll(struct sock_events *);
extern int                  sockset_active(struct sock_events *, int);
extern uint32_t             sockset_revent(struct sock_events *, int);

static void add_connection(socket_t *socket, connection_t *conn)
{
	pthread_mutex_lock(&socket->conn_lock);

	list_add_tail(&socket->children, &conn->node);
	++socket->num_connections;

	pthread_mutex_unlock(&socket->conn_lock);
}

static void rm_connection(socket_t *socket, connection_t *conn)
{
	pthread_mutex_lock(&socket->conn_lock);

	list_del_from(&socket->children, &conn->node);
	--socket->num_connections;

	pthread_mutex_unlock(&socket->conn_lock);
}

static struct addrinfo *net_lookup(const char *hostname,
		const char *service,
		int family,
		int socktype)
{
	struct addrinfo hints;
	struct addrinfo *res;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = family;
	hints.ai_socktype = socktype;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;

	if (getaddrinfo(hostname, service, &hints, &res) != 0)
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

static void remove_fd(struct pollfd *pfd,
		const struct addrinfo **addr,
		socklen_t *slen,
		unsigned int *num,
		unsigned int i)
{
	memmove(pfd + i, pfd + i + 1, (*num - i - 1) * sizeof(pfd[0]));
	memmove(addr + i, addr + i + 1, (*num - i - 1) * sizeof(addr[0]));
	memmove(slen + i, slen + i + 1, (*num - i - 1) * sizeof(slen[0]));
	(*num)--;
}

static int net_connect(const struct addrinfo *addrinfo)
{
	int sockfd = -1, saved_errno;
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

		if (errno != EINPROGRESS) {
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
			errno = err;
			remove_fd(pfd, addr, slen, &num, i--);
		}
	}

got_one:
	sockfd = pfd[i].fd;

out:
	saved_errno = errno;
	for (i = 0; i < num; i++)
		if (pfd[i].fd != sockfd)
			close(pfd[i].fd);
	errno = saved_errno;
	return sockfd;
}

static bool __poll_on_client(socket_t *sock,
		connection_t *conn,
		void *events,
		uint32_t flags)
{
	if (unlikely(!conn))
		return false;

	conn->last_active = time(NULL);
	if (flags & EVENT_WRITE && conn->wbuff.size > 0) {
		int len = -1;
		do
			len = send(conn->fd, conn->wbuff.data, conn->wbuff.size, 0);
		while (len == -1 && errno == EINTR);
		if (len < 0 || len != conn->wbuff.size)
			__unreachable();

		if (IsAvail(&conn->ops, write))
			callop(&conn->ops, write, conn, &conn->wbuff);

		free(conn->wbuff.data);
		conn->wbuff.data = NULL;
		conn->wbuff.size = 0;
		return true;
	}

	if (flags & EVENT_READ) {
		bool err = !socket_read(conn, NULL, socket_get_read_size(conn));
		if (err) {
			if (IsBlocking())
				return true;
#ifdef _DEBUG_SOCKET
			eprintf("A wild error occured during read()\n");
#endif
			return false;
		}
	}

	return true;
}

static void *poll_on_client(void *client)
{
	connection_t *conn = (connection_t *)client;
	void *events;
	int i;

	events = sockset_init();
	if (!events)
		return NULL;

	sockset_add(events, conn->fd, EVENT_READ | EVENT_WRITE);
	while (1) {
		int ret = sockset_poll(events);
		for (i = 0; i < ret; i++)
			if (sockset_active(events, i) == conn->fd &&
			    !__poll_on_client(NULL, conn, events, sockset_revent(events, i))) {
				sockset_deinit(events);
				pthread_exit(NULL);
			}
	}
}

static void *poll_on_server(void *_socket)
{
	socket_t *socket = (socket_t *)_socket;
	struct sockaddr in_addr;
	socklen_t in_len = sizeof(in_addr);
	int in_fd;
	char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
	uint32_t bits;
	int ret, i, cfd;

	socket->events = sockset_init();
	if (!socket->events)
		return NULL;

	sockset_add(socket->events, socket->fd, EVENT_READ);
	while (1) {
		connection_t *conn = NULL;
		int ret = sockset_poll(socket->events);
		for (i = 0; i < ret; i++) {
			cfd = sockset_active(socket->events, i);
			bits = sockset_revent(socket->events, i);
			if (cfd != socket->fd) {
				list_for_each(&socket->children, conn, node) {
					if (conn->fd == cfd  && !__poll_on_client(socket, conn, socket->events, bits)) {
						rm_connection(socket, conn);
						connection_free(conn);
					}
				}
				continue;
			}

			if (!(bits & EVENT_READ))
				pthread_exit(NULL);

			while (1) {
				in_fd = accept(socket->fd, &in_addr, &in_len);
				if (in_fd == -1) {
					if (IsBlocking())
						break;
#ifdef _DEBUG_SOCKET
					else
						eprintf("an error occured during accept(): %d(%s)\n", errno, strerror(errno));
#endif
					continue;
				}

				conn = connection_create(in_fd);
				if (!conn) {
					close(in_fd);
					continue;
				}

				if (!set_nonblock(conn->fd) || !socket_set_read_size(conn, 2048)) {
					close(conn->fd);
					free(conn);
					continue;
				}

				conn->last_active = time(NULL);
				if (getnameinfo(&in_addr, in_len,
				                hbuf, sizeof hbuf,
				                sbuf, sizeof sbuf,
				                NI_NUMERICHOST | NI_NUMERICSERV) == 0) {
					strncpy(conn->host, hbuf, sizeof conn->host);
					strncpy(conn->port, sbuf, sizeof conn->port);
				}

				if (likely(socket->on_accept)) {
					(*socket->on_accept) (socket, conn);
					if (IsAvail(&conn->ops, connect))
						callop(&conn->ops, connect, conn);
				}

				if (unlikely(!socket->conn))   /* We don't always expect socket->conn to be null... */
					socket->conn = conn;

				sockset_add(socket->events, conn->fd, EVENT_READ | EVENT_WRITE);
				add_connection(socket, conn);
			}

		}
	}

	__unreachable();
}

socket_t *socket_create(void (*on_accept) (socket_t *, connection_t *))
{
	socket_t *ret;
	SOCK_INIT();
	xmalloc(ret, sizeof(socket_t), return NULL);

	ret->on_accept = on_accept;
	ret->conn = NULL;
	ret->fd = -1;
	return ret;
}

connection_t *connection_create(int fd)
{
	connection_t *ret;

	SOCK_INIT();
	xmalloc(ret, sizeof(*ret), return NULL);
	ret->fd = fd;
	ret->remote = NULL;

	ret->wbuff.size = 0;
	ret->wbuff.data = NULL;
	return ret;
}

int socket_connect(connection_t *conn, const char *addr, const char *service)
{
	struct addrinfo *address;
	pthread_t thread;
	int ret = -errno;

	address = net_lookup(addr, service, AF_INET, SOCK_STREAM);
	if (!address)
		return -errno;

	if ((conn->fd = net_connect(address)) < 0) {
		freeaddrinfo(address);
		return -errno;
	}

	if (!socket_set_read_size(conn, 2048)) {
		freeaddrinfo(address);
		return -errno;
	}

	conn->remote = (char *)addr;
	if (IsAvail(&conn->ops, connect))
		callop(&conn->ops, connect, conn);

	if ((ret = pthread_create(&thread, NULL, poll_on_client, (void *)conn)) != 0)
		eprintf("failed to create thread (%d): %s\n", ret, strerror(ret));

	return ret;
}

int socket_listen(socket_t *sock, const char *address, const char *service, long max_conns)
{
	int reuse_addr = 1;
	pthread_t thread;
	int ret = -1;
	struct addrinfo *addr;

	if (!sock)
		return false;

	addr = net_lookup(address, service, AF_INET, SOCK_STREAM);
	if (!addr)
		return -EHOSTUNREACH;

	if (sock->fd < 0)
		sock->fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
	if (sock->fd < 0) {
		freeaddrinfo(addr);
		return -1;
	}

	if (setsockopt(sock->fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(int)) != 0) {
		freeaddrinfo(addr);
		close(sock->fd);
		return -1;
	}

	if (bind(sock->fd, addr->ai_addr, addr->ai_addrlen) == -1
			|| listen(sock->fd, max_conns) == -1
			|| !set_nonblock(sock->fd)) {
		freeaddrinfo(addr);
		close(sock->fd);
		return -1;
	}

	list_head_init(&sock->children);
	if ((ret = pthread_create(&thread, NULL, poll_on_server, (void *)sock)) != 0)
		eprintf("failed to create thread (%d): %s\n", ret, strerror(ret));

	freeaddrinfo(addr);
	return ret;
}

int socket_write(connection_t *conn, const char *fmt, ...)
{
	char *data;
	int len, sent;
	va_list va;

	if (unlikely(!conn || conn->fd < 0))
		return -EINVAL;

	va_start(va, fmt);
	len = vasprintf(&data, fmt, va);
	va_end(va);

	if (!data || len < 0)
		return -ENOMEM;

	sent = send(conn->fd, data, len, 0);
	if (sent < 0) {
#ifdef _DEBUG_SOCKET
		eprintf("send(): returned %d, errno is %d, estring is %s\n", sent,
		        errno, strerror(errno));
#endif
		free(data);
		return -errno;
	}

	if (sent != len) {
		xrealloc(conn->wbuff.data, conn->wbuff.data,
		         (conn->wbuff.size + (len - sent)) * sizeof(char), free(data); return -1);
		memcpy(&conn->wbuff.data[conn->wbuff.size], &data[sent], len - sent);
		conn->wbuff.size += len - sent;
	} else {
		struct sk_buff on_stack = {
			.data = data,
			.size = len
		};
		if (IsAvail(&conn->ops, write))
			callop(&conn->ops, write, conn, &on_stack);
		if (data)
			free(data);
	}

	return sent;
}

int socket_bwrite(connection_t *conn, const unsigned char *bytes, size_t size)
{
	int sent;
	if (unlikely(!conn))
		return -1;

	sent = send(conn->fd, bytes, size, 0);
	if (sent < 0)
		return -1;

	if (sent != size) {
		xrealloc(conn->wbuff.data, conn->wbuff.data,
		         (conn->wbuff.size + (size - sent)) * sizeof(char), return -1);
		memcpy(&conn->wbuff.data[conn->wbuff.size], &bytes[sent], size - sent);
		conn->wbuff.size += size - sent;
	} else {
		struct sk_buff on_stack = {
			.data = (char *)bytes,
			.size = size
		};
		if (IsAvail(&conn->ops, write))
			callop(&conn->ops, write, conn, &on_stack);
	}

	return sent;
}

bool socket_set_read_size(connection_t *conn, int size)
{
	if (unlikely(!conn))
		return false;

	if (setsockopt(conn->fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(int)) != 0) {
#ifdef _DEBUG_SOCKET
		eprintf("socket_set_read_size(%p, %d): failed because setsockopt() returned -1, the errno was: %d,"
		        " the error string is: %s\n",
		        conn, size, errno, strerror(errno));
#endif
		return false;
	}

	return true;
}

int socket_get_read_size(connection_t *conn)
{
	int size = -1;
	socklen_t slen = sizeof(size);

	if (getsockopt(conn->fd, SOL_SOCKET, SO_RCVBUF, &size, &slen) != 0) {
#ifdef _DEBUG_SOCKET
		eprintf("socket_get_read_size(%p, %d): failed because getsockopt() returned -1, the errno was: %d,"
		        " the error string is: %s\n",
		        conn, size, errno, strerror(errno));
#endif
		return -1;
	}

	return size;
}

bool socket_set_send_size(connection_t *conn, int size)
{
	if (unlikely(!conn))
		return false;

	if (setsockopt(conn->fd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(int)) != 0) {
#ifdef _DEBUG_SOCKET
		eprintf("socket_set_send_size(%p, %d): failed because setsockopt() returned -1, the errno was: %d,"
		        " the error string is: %s\n",
		        conn, size, errno, strerror(errno));
#endif
		return false;
	}

	return true;
}

int socket_get_send_size(connection_t *conn)
{
	int size = -1;
	socklen_t slen = sizeof(size);

	if (getsockopt(conn->fd, SOL_SOCKET, SO_SNDBUF, &size, &slen) != 0) {
#ifdef _DEBUG_SOCKET
		eprintf("socket_get_send_size(%p, %d): failed because getsockopt() returned -1, the errno was: %d,"
		        " the error string is: %s\n",
		        conn, size, errno, strerror(errno));
#endif
		return -1;
	}

	return size;
}

bool socket_read(connection_t *conn, struct sk_buff *buff, size_t size)
{
	char *buffer;
	ssize_t count;

	if (unlikely(!conn))
		return false;

	if (!size)
		size = socket_get_read_size(conn);

	buffer = calloc(size, sizeof(char));
	if (!buffer)
		return false;

	errno = 0;
	count = recv(conn->fd, buffer, size, 0);
	if (count == -1) {
		if (IsBlocking()) {
			free(buffer);
			return false;
		}
	} else if (count == 0) {
		free(buffer);
		return false;
	}
	buffer[count] = '\0';

	struct sk_buff on_stack = {
		.data = buffer,
		.size = count
	};
	if (buff)
		*buff = on_stack;

	if (IsAvail(&conn->ops, read))
		callop(&conn->ops, read, conn, &on_stack);

	if (buffer) free(buffer);
	return true;
}

void socket_free(socket_t *socket)
{
	connection_t *conn, *next;
	if (!socket)
		return;

	list_for_each_safe(&socket->children, conn, next, node) {
		if (conn->fd > 0)
			close(conn->fd);
		list_del_from(&socket->children, &conn->node);
		free(conn);
	}

	if (socket->events)
		sockset_deinit(socket->events);
	pthread_mutex_destroy(&socket->conn_lock);
	free(socket);
}

void connection_free(connection_t *conn)
{
	if (unlikely(!conn || conn->fd < 0))
		return;

	if (IsAvail(&conn->ops, disconnect))
		callop(&conn->ops, disconnect, conn);

	close(conn->fd);
	free(conn);
}

bool socket_remove(socket_t *socket, connection_t *conn)
{
	if (!socket || !conn)
		return false;

	rm_connection(socket, conn);
	return true;
}

