/* Public domain */
#ifndef _MATH_H
#define _MATH_H

/* Common math functions.  */
#ifndef min
#define min(a, b)				\
	__extension__ ({			\
		__typeof__(a) _a = (a);		\
		__typeof__(b) _b = (b);		\
		_a < _b ? _a : _b;		\
	})
#endif
#ifndef max
#define max(a, b)				\
	__extension__ ({			\
		__typeof__(a) _a = (a);		\
		__typeof__(b) _b = (b);		\
		_a > _b ? _a : _b;		\
	})
#endif

#endif  /* _MATH_H */

