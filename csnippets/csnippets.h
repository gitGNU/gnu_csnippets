/**
 * This file includes important headers and defines common
 * macros (mostly used as helpers).
 *
 * This file acts as a pre-compiled header.
 */
#ifndef _CSNIPPETS_H
#define _CSNIPPETS_H

#ifndef __cplusplus
/* MSVC stuck in C89, shitty compiler (CL to be specific). */
#if defined _MSC_VER || (defined __STDC_VERSION__ && __STDC_VERSION__ < 199901L)
typedef enum _Bool {
	false = 0,
	true  = 1
} bool;
#else
#include <stdbool.h>
#endif  /* _MSC_VER */
#endif  /* __cplusplus */

/* Comon headers.  */
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

/* This does NOT include the one from libc,
 * but csnippets/string.h if -I is passed to the compiler,
 * In order to solve this, i've added a #include_next in csnippets/string.h
 * which solves this problem.
 *
 * If some compilers do not support #include_next, please file a bug report.
 * See also: http://gcc.gnu.org/onlinedocs/cpp/Wrapper-Headers.html
 */
#include <string.h>

#include <csnippets/compiler.h>
#include <csnippets/error.h>
#include <csnippets/mm.h>

/* countof() returns the count of elements in an array. */
#define countof(array) (sizeof((array)) / sizeof((array)[0]))

static __const_inline bool test_bit(uint32_t bits, uint32_t bit)
{
	return (bits & bit) == bit;
}

static __const_inline bool swap_u32(uint32_t val)
{
	val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
	return (val << 16) | ((val >> 16) & 0xFFFF);
}

/* C++ compatiblity.  */
#if !defined(_BEGIN_DECLS) && !defined(_END_DECLS)
#ifdef __cplusplus
#define _BEGIN_DECLS extern "C" {
#define _END_DECLS   }
#else
#define _BEGIN_DECLS
#define _END_DECLS
#endif
#endif

#endif  /* _CSNIPPETS_H */

