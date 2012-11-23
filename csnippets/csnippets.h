/**
 * This file includes important headers and defines common
 * macros (mostly used as helpers).
 *
 * This file acts as a pre-compiled header.
 */
#ifndef __csnippets_h
#define __csnippets_h

#ifndef __cplusplus
/**
 * MSVC stuck in C89, shitty compiler (CL to be specific).
 */
#if defined _MSC_VER
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
#ifdef __cplusplus
#define __begin_header extern "C" {
#define __end_header   }
#else
#define __begin_header
#define __end_header
#endif

#endif  /* __csnippets_h */

