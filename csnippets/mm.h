/* Public Domain */
#ifndef _MEMORY_MANAGEMENT_H
#define _MEMORY_MANAGEMENT_H

/* Memory management functions, previously named as
 * "Auxiliars" or briefly "helpers".  */

/* It's been said, on Linux malloc never fails  */
#ifdef __linux
#define __check unlikely
#else
#define __check(x) (x)
#endif

#define xfree(p) do { if (p) free(p); p = NULL; } while (0)
#define __alloc_failure(s) do { warning("failed to allocate %ld bytes\n", (long) s); } while(0)
#define xmalloc(p, s, action) do {  \
	p = calloc(1, s); \
	if (__check(!p)) { \
		__alloc_failure(s); \
		action; \
	} \
} while (0)
#define xcalloc(p, l, s, action) do { \
	p = calloc(l, s); \
	if (__check(!p)) { \
		__alloc_failure((s) * (l)); \
		action; \
	} \
} while (0)
#define xrealloc(new_ptr, old_ptr, count, action) do { \
	(new_ptr) = realloc((old_ptr), count);  \
	if (__check(!new_ptr)) { \
		__alloc_failure((count)); \
		action; \
	} \
} while (0)
#define alloc_grow(ptr, count, action) xrealloc(ptr, ptr, count, action)

#endif  /*  _MEMORY_MANAGEMENT_H */

