/*
 * This code is licensed under BSD-MIT By Rusty Russell, see also:
 * https://github.com/rustyrussell/ccan/
 */

#ifndef _ASPRINTF_H
#define _ASPRINTF_H

#include <stdarg.h>

_BEGIN_DECLS

/**
 * asprintf() - allocates *strp and returns the
 * length of the string.
 */
extern int __printf(2, 3) asprintf(char **strp, const char *fmt, ...);
/**
 * Works the same as asprintf() and is used by asprintf()
 */
extern int vasprintf(char **strp, const char *fmt, va_list va);

_END_DECLS

#endif  /* _ASPRINTF_H */

