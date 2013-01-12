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
#include <csnippets/error.h>
#include <csnippets/asprintf.h>

/* The following extern may break compilation on some compilers,
 * or other libc, this one is from GLIBC (GNU libc).
 */
extern char *program_invocation_short_name;
static int verbose = 0;

/* A short name instead of a longass one.  */
#define prog	program_invocation_short_name
#define __dolog(s, args...) do { \
		fprintf(stderr, _(s), ##args); \
		if (verbose > 0) \
			fprintf(stdout, _(s), ##args); \
	} while(0)

void log_init(void)
{
	(void) freopen(LOG_FILE, "w", stderr);
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
	free(buff);
	exit(EXIT_FAILURE);
}

void dolog(const char *str, ...)
{
	va_list va;
	char *buff;

	va_start(va, str);
	(void) vasprintf(&buff, str, va);
	va_end(va);

	__dolog(_("%s: %s"), prog, buff);
	free(buff);
}

void set_verbose_level(int level)
{
	verbose = !!level;
}

