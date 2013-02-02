/* Socket compatibility with other Platforms.
 * This still Needs more work.
 *
 * This should never be included in an exported header,
 * only in a source file or an internal header (like this one).
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

#define S_close(fd)	closesocket((fd))
#define S_error		WSAGetLastError()
#define S_seterror(e)	WSASetLastError((e))

#define S_EAGAIN	WSAEWOULDBLOCK     /* Windows socket does not have WSAEAGAIN it seems, define it to would block instead (Same as some POSIX-based systems).  */
#define S_EBLOCK	WSAEWOULDBLOCK
#define S_EINTR		WSAEINTR
#define S_EINPROGRESS	WSAEINPROGRESS
#define S_ENOTCONN	WSAENOTCONN
#define S_ESHUTDOWN	WSAESHUTDOWN
#define S_ECONNRESET	WSAECONNRESET
#define S_ECONNABORTED	WSAECONNABORTED
#define S_ENETRESET	WSAENETRESET
#define S_ENOTSOCK	WSAENOTSOCK
#define S_ETIMEDOUT	WSAETIMEDOUT
#define S_EBADF		WSAEBADF
#elif defined(__unix__)
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define S_close(fd)	close((fd))
#define S_error		errno
#define S_seterror(e)	errno = (e)

#define S_EAGAIN	EAGAIN
#define S_EBLOCK	EWOULDBLOCK
#define S_EINTR		EINTR
#define S_EINPROGRESS	EINPROGRESS
#define S_ENOTCONN	ENOTCONN
#define S_ESHUTDOWN	ESHUTDOWN
#define S_ECONNRESET	ECONNRESET
#define S_ECONNABORTED	ECONNABORTED
#define S_ENETRESET	ENETRESET
#define S_ENOTSOCK	ENOTSOCK
#define S_ETIMEDOUT	ETIMEDOUT
#define S_EBADF		EBADF
#endif
#define IsBlocking() (S_error == S_EBLOCK || S_error == S_EAGAIN)

#endif    /* _SOCKET_COMPAT_H  */

