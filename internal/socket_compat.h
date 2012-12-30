/* Socket compatibility with other Platforms.
 * This still Needs more work.
 *
 * I probably misused some things as well.
 */
#ifndef _SOCKET_COMPAT_H
#define _SOCKET_COMPAT_H

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

#define s_close(fd)	closesocket((fd))
#define s_error		WSAGetLastError()
#define s_seterror(e)	WSASetLastError((e))

#define s_EAGAIN	WSAEWOULDBLOCK     /* Windows socket does not have WSAEAGAIN it seems, define it to would block instead (Same as some POSIX-based platforms).  */
#define s_EBLOCK	WSAEWOULDBLOCK
#define s_EINTR		WSAEINTR
#define s_EINPROGRESS	WSAEINPROGRESS
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define s_close(fd)	close((fd))
#define s_error		errno
#define s_seterror(e)	errno = (e)

#define s_EAGAIN	EAGAIN
#define s_EBLOCK	EWOULDBLOCK
#define s_EINTR		EINTR
#define s_EINPROGRESS	EINPROGRESS
#endif
#define IsBlocking() (s_error == s_EBLOCK || s_error == s_EAGAIN)

#endif    /* _SOCKET_COMPAT_H  */

