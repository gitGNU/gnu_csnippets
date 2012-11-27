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

#include "list.h"

#include <time.h>
#include <pthread.h>

typedef struct socket socket_t;
typedef struct connection connection_t;

struct sk_buff {
	char *data;             /* The data read of this fd.  */
	size_t size;            /* length of data.  */
};

struct sock_events;
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
struct sock_operations {
	/* Called when we've have connected.  This is the root of the connection.
	 * It should be used to setup other callbacks!  */
	void (*connect) (connection_t *self);

	/* Called when we've disconnected.   */
	void (*disconnect) (connection_t *self);

	/* Called when anything is read.  */
	void (*read) (connection_t *self, const struct sk_buff *buff);

	/* Called when this connection writes something (on successfull write operations only) */
	void (*write) (connection_t *self, const struct sk_buff *buff);
};

struct socket {
	int fd;                         /* Socket file descriptor.  */

	struct list_head children;      /* The list of conn */
	connection_t *conn;             /* The head connection */
	unsigned int num_connections;   /* Current active connections */
	pthread_mutex_t conn_lock;      /* The connection lock, for adding new connections,
                                       removing dead ones, incrementing number of active connections */

	struct sock_events *events;     /* Internal usage  */

	/**
	 * on_accept() this callback is called whenever a new connection
	 * is accepted.  self is this socket, conn is obviously the
	 * accepted connection.
	 *
	 * See below for more information on what to do when this function is called.
	 */
	void (*on_accept) (socket_t *self, connection_t *conn);
};

struct connection {
	int fd;              /* The socket file descriptor */
	char host[1025];     /* The hostname of this connection */
	char port[32];       /* The port we're connected to */
	char *remote;        /* Who did we connect to?  Or who did we come from?  */
	time_t last_active;  /* The timestamp of last activity.  Useful for PING PONG. */

	struct sk_buff wbuff;    /* "write buffer" this is changed whenever data has been been sent.
                                If the data was successfully sent over the connection, on_write() will be
                                called.  */

	struct sock_operations ops;  /* operations  */
	struct list_node node;   /* The node */
};

#define EVENT_READ  0x01   /* There's data to be read on this connection.  */
#define EVENT_WRITE 0x02   /* We're free to send incomplete data.  */

/** socket_create() - Create and prepare a socket for listening.
 *
 * @return a malloc'd socket or NULL on failure.
 */
extern socket_t *socket_create(void (*on_accept) (socket_t *, connection_t *));

/**
 * Free socket and all of it's connections if any.
 *
 * @socket a socket returned by socket_create().
 */
extern void socket_free(socket_t *socket);

/**
 * Create a connection
 *
 * @on_connect, the callback to use for on_connect
 */
extern connection_t *connection_create(int fd);

/**
 * Close & Free the connection
 *
 * Note: Consider calling socket_remove() before calling this function, see below.
 * Note: That the above note applies only if this connection is part of
 *       a listening socket, otherwise don't.
 *
 * @conn a socket created by connection_create()
 */
extern void connection_free(connection_t *conn);

/**
 * socket_connect() - connect to a server.
 *
 * @conn a malloc'd connection that has atleast the on_connect callback setup.
 * @addr the address the server is listening on.
 * @service port or a register service.
 */
extern int socket_connect(connection_t *conn, const char *addr,
                          const char *service);
/** socket_listen() - Listen on this socket.
 *
 * @address can be NULL if independent
 * @port port to listen on.
 */
extern int socket_listen(socket_t *socket, const char *address,
                         const char *service, long max_conns);

/**
 * socket_write() - Write string on socket connection
 *
 * @param conn a connection created by socket_connect() or from the listening socket.
 * @param data the data to send
 * @return errno (negatively!) on failure, 0 on success.
 *
 * This function stores the data in conn->squeue, and then
 * calls conn->on_write later on if connection ever becomes available
 * for writing.
 */
extern int socket_write(connection_t *conn, const char *fmt, ...);

/**
 * socket_bwrite() - Write byte array on socket connection
 *
 * @conn the socket connection
 * @bytes the byte array to send
 * @size the size of `bytes'.
 *
 * The behavior is same as socket_write().
 */
extern int socket_bwrite(connection_t *conn, const uint8_t *bytes, size_t size);

/** socket_set_read_size() - set maximum read size
 *
 * returns true on success, false otherwise
 */
extern bool socket_set_read_size(connection_t *conn, int size);

/** socket_get_read_size() - get maximum read size
 *
 * returns the maximum read size for the connection, -1 on failure.
 */
extern int socket_get_read_size(connection_t *conn);

/** socket_set_send_size() - set maximum write/send size
 *
 * returns true on success, false otherwise
 */
extern bool socket_set_send_size(connection_t *conn, int size);

/** socket_get_send_size() - get maximum send size
 *
 * returns the maximum send size for the connection, -1 on failure.
 */
extern int socket_get_send_size(connection_t *conn);

/**
 * socket_read() - read from a socket
 *
 * @conn a connected socket
 * @size how many characters to read?
 *
 * on successfull read, this function returns true and does the following:
 *   * calls conn->on_read if not NULL.
 *   * sets the data read to `buff'.
 * on failure, this function returns false.
 *
 * This function heap allocates memory for the data read, this means
 * buff->data must be free'd later on to avoid memory leak issues.
 */
extern bool socket_read(connection_t *conn, struct sk_buff *buff, size_t size);

/**
 * socket_remove() - remove connection from listening socket
 *
 * returns true on success, false otherwise
 */
extern bool __const socket_remove(socket_t *socket, connection_t *conn);

#endif    /* _SOCKET_H */

