/*
 * Copyright (c) 2012 Allan Ference  <f.fallen45@gmail.com>
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
#ifndef _SOCKET_COMPAT_H
#define _SOCKET_COMPAT_H

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>	 /* socklen_t */

#define s_close(fd) closesocket((fd))
#define s_error     WSAGetLastError()
#define s_seterror(e)  WSASetLastError((e))

#define s_EAGAIN	WSAEWOULDBLOCK
#define s_EBLOCK	WSAEWOULDBLOCK
#define s_EINTR		WSAEINTR
#define s_EINPROGRESS	WSAEINPROGRESS

#define IsBlocking() (s_error == s_EBLOCK)
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

#define s_close(fd)  close((fd))
#define s_error      errno
#define s_seterror(e) errno = (e)

#define s_EAGAIN     EAGAIN
#define s_EBLOCK     EWOULDBLOCK
#define s_EINTR      EINTR
#define s_EINPROGRESS EINPROGRESS

#define IsBlocking() (s_error == EWOULDBLOCK || s_error == EAGAIN)
#endif

#endif    /* _SOCKET_COMPAT_H  */

