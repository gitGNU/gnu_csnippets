/**
 * Various linux stlye macros, when I say linux style,
 * I mean the macro style used in the Linux kernel (__noreturn, __weak, ...)
 *
 * Defines shortcuts for some attributes if __GNUC__ (GNU C compiler) used.
 */
#ifndef _LINUX_STYLE_H
#define _LINUX_STYLE_H

#ifdef __GNUC__
#define __noreturn      __attribute__((noreturn))
#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)
#define __inline__      inline __attribute__((always_inline))
#define __warn_unused   __attribute__((warn_unused_result))
#define __weak          __attribute__((weak))
#define __printf(nfmt, narg) \
    __attribute__((format(__printf__, nfmt, narg)))
#define __const         __attribute__((const))
#define __unused        __attribute__((unused))
#define GCC_VERSION (__GNUC__ * 10000 \
                            + __GNUC_MINOR__ * 100 \
                            + __GNUC_PATCHLEVEL__)
#if GCC_VERSION >= 40500
#define __unreachable() __builtin_unreachable();
#else
#define __unreachable() do { fatal("Something went tottaly unexpected!\n"); } while (0)
#endif  /* GCC_VERSION */
#else   /* not __GNUC__ */
#define __attribute__(x)
#define __noreturn
#define likely(x) (x)
#define unlikely(x) (x)
#define __inline__ inline
#define __warn_unused
#define __weak
#define __printf(nfmt, narg)
#define __const
#define __unused
#define __unreachable()
#endif  /* __GNUC__ */

#endif    /* _LINUX_STYLE_H */

