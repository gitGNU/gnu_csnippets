/*
 * Copyright (c) 2012 Ahmed Samy  <f.fallen45@gmail.com>
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
 * This function ignores any system call interrupt and blocks
 * until anything happens on file descriptors at this time.
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
 *		fd = pollev_active(events, i);
 *		revent = pollev_revent(events, i);
 *
 *		if (revent & IO_READ)
 *			...
 *	}
 */
int pollev_poll(struct pollev *, int timeout);

/**
 * pollev_active() - get the current active file descriptor
 *
 * Get the active file descriptor of @index.
 *
 * See pollev_poll() for more information.
 */
int pollev_active(struct pollev *, int index) __fconst;

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

