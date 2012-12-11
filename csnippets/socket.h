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
 *
 * TODO:
 *  * UDP support.
 *  * SSL support.
 */
#ifndef _SOCKET_H
#define _SOCKET_H

#include <csnippets/list.h>

#include <time.h>
#include <pthread.h>

struct sk_buff {
	/** The data read from a recv() syscall  */
	void *data;
	/** length of @data.  */
	size_t size;
};

/**
 * \code
 *     struct sock_operationrs sops = {
 *          .connect = my_connect,
 *          .disconnect = my_disconnect,
 *          ...
 *     };
 * \endcode
 *
 *     when initialising the connected socket:
 * \code
 *     mconn->ops = sops;
 * \endcode
 */
struct conn;
struct sock_operations {
	/* Called when we've have connected.  */
	void (*connect) (struct conn *self);

	/* Called when we've disconnected.   */
	void (*disconnect) (struct conn *self);

	/* Called when anything is read.  */
	void (*read) (struct conn *self, const struct sk_buff *buff);

	/* Called when this connection writes something (on successfull write operations only) */
	void (*write) (struct conn *self, const struct sk_buff *buff);
};

struct listener {
	int fd;                         /* Socket file descriptor.  */

	struct list_head children;      /* The list of conn */
	unsigned int num_connections;   /* Current active connections */

	/* The connection lock, for adding new connections,
	 * removing dead ones, incrementing number of active connections */
	pthread_mutex_t conn_lock;

	struct sock_events *events;     /* Internal usage  */
	void (*on_accept) (struct listener *self, struct conn *conn);
};

struct conn {
	int fd;			/* The socket file descriptor */
	char host[1025];	/* The hostname of this connection */
	char port[32];		/* The port we're connected to */
	char remote[1025];	/* Who did we connect to?  Or who did we come from?  */
	time_t last_active;	/* The timestamp of last activity.  Useful for PING PONG. */

	/* "write buffer" this is changed whenever data has been been sent.
	 * If the data was successfully sent over the connection, ops.write will be
	 * called.  */
	struct sk_buff wbuff;
	struct sock_operations ops;  /* operations  */
	struct list_node node;	     /* next & prevvious conn */
};

#define EVENT_READ  0x01   /* There's data to be read on this connection.  */
#define EVENT_WRITE 0x02   /* We're free to send incomplete data.  */

/**
 * socket_create() - Create and prepare a socket for listening.
 *
 * @return a malloc'd socket or NULL on failure.
 */
extern struct listener *socket_create(void (*on_accept) (struct listener *, struct conn *));

/**
 * socket_free() - Free socket and all of it's connections if any.
 *
 * @socket a socket returned by socket_create().
 */
extern void socket_free(struct listener *socket);

/**
 * conn_create() - Create a connection
 *
 * fd can be -1 if not known, but then call socket_connect() to do the rest.
 */
extern struct conn *conn_create(int fd);

/**
 * conn_free() - Close & Free the connection
 *
 * Note: Consider calling socket_remove() before calling this function, see below.
 * Note: That the above note applies only if this connection is part of
 *       a listening socket, otherwise don't.
 *
 * @conn a socket created by conn_create()
 */
extern void conn_free(struct conn *conn);

/**
 * socket_connect() - connect to a server.
 *
 * @conn a malloc'd connection (the connection should have sock_operationrs setup) 
 * @addr the address the server is listening on.
 * @service port or a register service.
 */
extern int socket_connect(struct conn *conn, const char *addr,
                          const char *service);
/**
 * socket_listen() - Listen on this socket for incoming connections.
 *
 * @address can be NULL if independent
 * @port to listen on.
 */
extern int socket_listen(struct listener *socket, const char *address,
                         const char *service, long max_conns);
/**
 * socket_write() - Write string on socket connection
 *
 * @conn a connection created by socket_connect() or from the listening socket.
 * @data the data to send
 * @return errno (negatively!) on failure, 0 on success.
 *
 * This function stores the data in conn->squeue, and then
 * calls conn->on_write later on if connection ever becomes available
 * for writing.
 */
extern int socket_write(struct conn *conn, const void *data, size_t len);

/* Write a formatted string */
extern int socket_writestr(struct conn *conn, const char *fmt, ...);

/**
 * socket_set_read_size() - set maximum read size
 *
 * returns true on success, false otherwise
 */
extern bool socket_set_read_size(struct conn *conn, int size);

/**
 * socket_get_read_size() - get maximum read size
 *
 * returns the maximum read size for the connection, -1 on failure.
 */
extern int socket_get_read_size(struct conn *conn);

/**
 * socket_set_send_size() - set maximum write/send size
 *
 * returns true on success, false otherwise
 */
extern bool socket_set_send_size(struct conn *conn, int size);

/**
 * socket_get_send_size() - get maximum send size
 *
 * returns the maximum send size for the connection, -1 on failure.
 */
extern int socket_get_send_size(struct conn *conn);

/**
 * socket_read() - read from a socket
 *
 * @conn a connected socket
 * @size how many characters to read?
 *
 * on successfull read, this function returns true and does the following:
 *   * calls conn->ops.read if not NULL.
 *   * sets the data read to `buff'.
 * on failure, this function returns false.
 *
 * This function heap allocates memory for the data read, we free it later on if the user didn't.
 */
extern bool socket_read(struct conn *conn, struct sk_buff *buff, size_t size);

#endif    /* _SOCKET_H */

