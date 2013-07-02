#ifndef _ATOMIC_H
#define _ATOMIC_H

#if __GNUC__ || __INTEL_COMPILER
#define atomic_xadd(P, V)	__sync_fetch_and_add((P), (V))
#define cmpxchg(P, O, N)	__sync_val_compare_and_swap((P), (O), (N))
#define atomic_ref(P)		__sync_add_and_fetch((P), 1)
#define atomic_deref(P)		__sync_sub_and_fetch((P), 1)
/* Compile read-write barrier */
#define barrier()		asm volatile("":::"memory")

#if (__i386__ || __i386 || __amd64__ || __amd64)
/* Pause instruction to prevent excess processor bus usage */
#define cpu_relax()		asm volatile("pause\n":::"memory")
#else
#define cpu_relax()
#endif

#endif

#endif   /* _ATOMIC_H */

