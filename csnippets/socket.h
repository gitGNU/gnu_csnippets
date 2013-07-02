/*
 * Copyright (c) 2012 Ahmed Samy <f.fallen45@gmail.com>
 * Licensed under MIT, see LICENSE.MIT for details.
 *
 * TODO:
 *  * UDP support.
 *  * SSL support.
 *  * Documentation
 */
#ifndef _SOCKET_H
#define _SOCKET_H

#include <csnippets/typesafe_cb.h>

/* Forward declare conn, internal usage only.  */
typedef struct conn conn_t;

#define common_cb_cast(arg, expr)					\
	typesafe_cb_cast(bool (*) (conn_t *, void *),		\
			 bool (*) (conn_t *, __typeof__(arg)),	\
			 (expr))

/* Creates a listener and adds it to the poll queue. */
#define new_listener(service, fn, arg)				\
	_new_listener((service), common_cb_cast(arg, fn),	\
			(arg))
bool _new_listener(const char *service,
                  bool (*fn) (conn_t *, void *arg),
                  void *arg);

/* Creates a connection struct and adds it to the poll queue.
 * IPv6 is automatically used if available. */
#define new_conn(node, service, fn, arg)			\
	_new_conn((node), (service), common_cb_cast(arg, fn),	\
		   (arg))
bool _new_conn(const char *node, const char *service,
              bool (*fn) (conn_t *, void *arg),
              void *arg);

/* Like new_conn() but,  we don't look up nodes or services
 * here.
 * If the file descriptor is not a socket file descriptor,
 * (or if the fd has some error), a NULL is returned. */
#define new_conn_fd(fd, fn, arg)			\
	_new_conn_fd((fd), common_cb_cast(arg, fn),	\
			(arg))
conn_t *_new_conn_fd(int fd,
                         bool (*fn) (conn_t *, void *arg),
                         void *arg);
/* Close connected socket, and free memory. 
 * Fails if the socket file descriptor is already closed.  */
bool free_conn(conn_t *);

bool conn_read(conn_t *conn, void *data, size_t *len);
bool conn_write(conn_t *conn, const void *data, size_t len);
bool conn_writestr(conn_t *conn, const char *fmt, ...)
	__printf(2, 3);

/* Calls @next at next acitivty from the connnection,
 * if the next function returns false, the connection is
 * free'd and is no longer valid for use.
 *
 * It's not recommended to free the connection manually
 * in @next even if calling free_conn().
 */
#define conn_next(conn, next, arg)			\
	_conn_next(conn, common_cb_cast(arg, next),	\
		   (arg))
bool _conn_next(conn_t *,
               bool (*next) (conn_t *, void *arg),
               void *arg);

/* Close the connection, calling the next callback with the argument.  */
void next_close(conn_t *, void *arg);

/* Similar to setsockopt but for boolean values,
 * Saves a bit of writing for boolean options, when doing something
 * like that:
 *
 * \code
 *	int on = 1;
 *
 *	if (!conn_setopt(conn, SO_KEEPALIVE, &on, sizeof(on)))
 *		...
 * \endcode
 *
 * Instead:
 *	if (!conn_setopt_bool(conn, SO_KEEPALIVE, true))
 *		...
 */
#define conn_setopt_bool(conn, optname, enable)					\
	__extension__ ({							\
		int __enable = !!(enable);					\
		assert(conn_setopt(conn, optname, &(__enable), sizeof(int)));	\
		__enable;							\
	})

/* Wrappers for set/getsockopt().  */
bool conn_getopt(conn_t *, int optname, void *optval,
                 int *optlen);
bool conn_setopt(conn_t *, int optname, const void *optval,
                 int optlen);

/* Get this connection's name information,
 * Store host in @host, if numeric_host is specified, the
 * host will be returned as if it was an IP.
 *
 * The same for @host applies to @serv.
 */
bool conn_getnameinfo(conn_t *,
                      char *host, size_t hostlen,
                      char *serv, size_t servlen,
                      bool numeric_host,
                      bool numeric_serv);

/* This is the main loop.  */
void *conn_loop(void);

#endif    /* _SOCKET_H */

