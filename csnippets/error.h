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
#ifndef _ERROR_H
#define _ERROR_H

#if HAVE_LIBINTL_H
#include <libintl.h>
#define _(x) gettext((x))
#else
#define _(x) x
#endif

#ifdef USE_COLORS
/* Color output  */
#define COL_RED		"\033[31m"
#define COL_PURPLE	"\033[35m"
#define COL_ORANGE	"\033[33m"
#define COL_BLANK	"\033[0m"	/* Resets colors  */
#else
#define COL_RED		""
#define COL_GREEN	""
#define COL_BLUE	""
#define COL_BLANK	""
#endif

#define LOG_FATAL	COL_RED	   "FATAL: " COL_BLANK
#define LOG_WARNING	COL_ORANGE "WARNING: " COL_BLANK
#define LOG_NOTICE	COL_PURPLE "NOTICE: " COL_BLANK
#define LOG_DEBUG	COL_PURPLE "DEBUG: "

/* shortcuts. */
#define elog(str, args...)	dolog(LOG_NOTICE str, ##args)
#define die			fatal
#define fatal(str, args...)	error_nret(LOG_FATAL str, ##args)
#define warning(str, args...)	dolog(LOG_WARNING str, ##args)
#define eprintf(str, args...)	dolog(str, ##args)
#ifdef _DEBUG
#define might_bug()		dolog(LOG_WARNING " %s is not well-tested and might behave incorrectly, please report any bugs you encounter\n", __func__)
#define dbg(fmt, args...)	dolog(LOG_DEBUG "%s:%d " fmt COL_BLANK, __func__, __LINE__, ##args)
#else
#define might_bug()
#define dbg(fmt, args...)
#endif

/* Initialize log file.  */
extern void log_init(const char *tofile);

/* error_nret() - Log a formatted string.
 *
 * NOTE: This function doesn't return, that means the program will
 * exit after.
 */
extern void __noreturn __printf(1, 2) error_nret(const char *str, ...);
/* dolog() - Log a formatted string.
 *
 * This function doesn't exit.
 */
extern void __printf(1, 2) dolog(const char *str, ...);
/* set_verbose_level() Set verbose level
 *
 * 0 - be quiet.
 * 1 - be annoying.  (This is the default)
 */
extern void set_verbose_level(int level);

#endif  /* _ERROR_H */

