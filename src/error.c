/*
 * Copyright (c) 2012 Ahmed Samy  <f.fallen45@gmail.com>
 * Licensed under MIT, see LICENSE.MIT for details.
 */
#include <csnippets/error.h>
#include <csnippets/asprintf.h>

/* The following extern may break compilation on some compilers,
 * or other libc, this one is from GLIBC (GNU libc).
 */
extern char *program_invocation_short_name;
static int verbose = 0;
panic_fn g_panic;

/* A short name instead of a longass one.  */
#define prog	program_invocation_short_name
#define __dolog(s, args...) do { \
		fprintf(stderr, _(s), ##args); \
		if (verbose) \
			fprintf(stdout, _(s), ##args); \
	} while(0)

void log_init(const char *tofile)
{
	(void) freopen(tofile, "w", stderr);
	verbose = 1;
}

void __noreturn error_nret(const char *str, ...)
{
	va_list ap;
	char *buff;

	va_start(ap, str);
	vasprintf(&buff, str, ap);
	va_end(ap);

	__dolog(_("%s: %s\n%s: error is not recoverable, terminating now...\n"),
			prog, buff, prog);
	g_panic(buff);
	free(buff);
	exit(EXIT_FAILURE);
}

void dolog(const char *str, ...)
{
	va_list va;
	char *buff;

	va_start(va, str);
	vasprintf(&buff, str, va);
	va_end(va);

	__dolog(_("%s: %s"), prog, buff);
	free(buff);
}

__inline void set_verbose_level(int level)
{
	verbose = !!level;
}

__inline void set_panic_callback(panic_fn fptr)
{
	g_panic = fptr;
}

