/* Public Domain, see LICENSE.CCO.
 *
 * Defines shortcuts for compiler specific attributes and some builtins.
 * This header is pre-included by csnippets.h.  This header should be used
 * for compiler-specific compatibilities as well, not just shortcuts.
 */
#ifndef _COMPILER_H
#define _COMPILER_H

#if defined __GNUC__ || defined __clang__
#define __noreturn	__attribute__((noreturn))
#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)
#define __warn_unused	__attribute__((warn_unused_result))
#define __weak		__attribute__((weak))
#define __printf(nfmt, narg) \
	__attribute__((__format__(__printf__, nfmt, narg)))
#define __packed	__attribute__((packed))
#define __fconst	__attribute__((__const__))
#define __noinline	__attribute__((noinline))
#define __const_inline	inline __attribute__((always_inline, __const__))
#define __noclone	__attribute__((noclone))

#ifndef __unused
#define __unused	__attribute__((unused))
#endif

#define __used		__attribute__((used))
#define __cold		__attribute__((cold))
#define __init		__attribute__((constructor))
#define __exit		__attribute__((destructor))
#define __construct	__init
#define __destruct	__exit
#define GCC_VERSION (__GNUC__ * 10000 \
                     + __GNUC_MINOR__ * 100 \
                     + __GNUC_PATCHLEVEL__)
#if GCC_VERSION < 40500 || !defined __clang__
#define __builtin_unreachable() do { fatal("Something went tottaly unexpected!\n"); } while (0)
#endif  /* GCC_VERSION */
#else   /* not __GNUC__ or __clang__ */
#define __attribute__(x)
#define __noreturn
#define likely(x) (x)
#define unlikely(x) (x)
#define __warn_unused
#define __weak
#define __printf(nfmt, narg)
#define __packed
#define __fconst
#define __noinline
#define __const_inline
#define __noclone
#define __unused
#define __used
#define __cold
#define __init
#define __exit
#define __construct
#define __destruct
#define __builtin_unreachable() do { fatal("Something went tottaly unexpected!\n"); } while (0)
#warning "Some features might not work since your compiler is not supported"
#endif  /* __GNUC__ || __clang__ */

#endif    /* _COMPILER_H */

