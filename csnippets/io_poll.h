/*
 * Copyright (c) 2012 Ahmed Samy  <f.fallen45@gmail.com>
 * Licensed under MIT, see LICENSE.MIT for details.
 */
#ifndef _IO_POLL_H
#define _IO_POLL_H

struct pollev;

#define IO_READ		0x0001   /* There's data to be read.  */
#define IO_WRITE	0x0002   /* We're free to send incomplete data.  */
#define IO_ERR		0x0004   /* An error occured in this fd.  */

/**
 * pollev_init() - allocate pollev structure and members,
 *
 * This structure is private for each interface;  Since
 * The members differ for each one.
 */
struct pollev *pollev_init(void);

/**
 * pollev_deinit() - deinitialize and free memory
 *
 * This free's the pointer passed if valid, does nothing if
 * invalid.  Note this does not close any file descriptors
 * assiocated, unless otherwise stated in the interface
 * manual page.
 */
void pollev_deinit(struct pollev *);

/* Add file descriptor @fd of IO bits to the poll queue.
 *
 * Bits should be something like:
 *	IO_READ | IO_WRITE or just one of them.
 */
void pollev_add(struct pollev *, int fd, int bits);

/* Delete @fd from the poll queue. */
void pollev_del(struct pollev *, int fd);

/**
 * pollev_poll() - poll on every file descriptor added.
 *
 * This function ignores system call interrupts and blocks
 * until a timeout, or forever if timeout is -1.
 *
 * Returns the number of file descriptors that were active
 * at that time.
 *
 * Usage example:
 *
 *	int nfds, i;
 *	...
 *
 *	nfds = pollev_poll(events, 1000);
 *	for (i = 0; i < nfds; i++) {
 *		int revents, fd;
 *
 *		fd = pollev_activefd(events, i);
 *		revent = pollev_revent(events, i);
 *
 *		if (revent & IO_READ)
 *			...
 *	}
 */
int pollev_poll(struct pollev *, int timeout);

/**
 * pollev_activefd() - get the current active file descriptor
 *
 * Get the active file descriptor of @index.
 *
 * See pollev_poll() for more information.
 */
int pollev_activefd(struct pollev *, int index) __fconst;

/**
 * pollev_revent() - get the returned events
 *
 * Returns the returned events from the active
 * file descriptor from @index (must be same as the fd's)
 *
 * See pollev_poll() for more information.
 */
short pollev_revent(struct pollev *, int index) __fconst;

#endif

