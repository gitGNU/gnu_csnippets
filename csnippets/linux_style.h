/**
 * Various Linux stlye macros, when I say Linux style,
 * I mean the macro style used in the Linux kernel (__noreturn, __weak, ...)
 *
 * Defines shortcuts for some attributes if __GNUC__ (GNU C compiler) used.
 */
#ifndef _LINUX_STYLE_H
#define _LINUX_STYLE_H

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
/* Mark stuff as used/unused  */
#define __unused	__attribute__((unused))
#define __used		__attribute__((used))
#define __init		__attribute__((constructor))
#define __exit		__attribute__((destructor))
#define GCC_VERSION (__GNUC__ * 10000 \
                     + __GNUC_MINOR__ * 100 \
                     + __GNUC_PATCHLEVEL__)
#if GCC_VERSION < 40500 || !defined __clang__
#define __builtin_unreachable() do { fatal("Something went tottaly unexpected!\n"); } while (0)
#endif  /* GCC_VERSION */
#else   /* not __GNUC__ || __clang__ */
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
#define __builtin_unreachable() do { fatal("Something went tottaly unexpected!\n"); } while (0)
#define __init
#define __exit
#endif  /* __GNUC__ || __clang__ */

#endif    /* _LINUX_STYLE_H */

