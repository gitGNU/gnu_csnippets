/**
 * This file includes important headers and defines common
 * macros (mostly used as helpers).
 *
 * This file acts as a pre-compiled header.
 */
#ifndef _CSNIPPETS_H
#define _CSNIPPETS_H

#ifndef __cplusplus
/**
 * MSVC stuck in C89, shitty compiler (CL to be specific).
 */
#if defined _MSC_VER || (defined __STDC_VERSION__ && __STDC_VERSION__ < 199901L)
typedef enum _Bool {
	false = 0,
	true  = 1
} bool;
#else 
#include <stdbool.h>
#endif  /* _MSC_VER */
#endif  /* __cplusplus */
/**
 * Comon headers.
 *
 * I don't like spamming every header with inclusion of those ones
 */
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <csnippets/linux_style.h>
#include <csnippets/error.h>
#include <csnippets/compat.h>
#include <csnippets/mm.h>

/*
 * countof() returns the count of array, i.e
 * const char *my_array[] = { ... };
 * int count = countof(my_array);
 */
#define countof(array) (sizeof((array)) / sizeof((array)[0]))

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

#if HAVE_LIBINTL_H
#include <libintl.h>
#define _(x) gettext((x))
#else
#define _(x) x
#endif

#endif  /* _CSNIPPETS_H */

