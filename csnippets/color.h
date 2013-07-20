/*
 * Copyright (c) 2013 Ahmed Samy  <f.fallen45@gmail.com>
 * Licensed under MIT, see LICENSE.MIT for details. 
 */
#ifndef _COLOR_H
#define _COLOR_H

typedef union {
	struct {
		uint8_t r, g, b, a;
	} __packed;
	uint32_t value;
} color_t;

/* Converts the RGBA values to their respective hex color.
 *
 * Example usage:
 *	printf("0x%x\n", rgba_to_u32(0x12, 0x13, 0x14, 0xFF));
 * Output: 0x121314FF
 */
#define rgba_to_u32(__r, __g, __b, __a)		\
	__extension__ ({			\
		color_t __c = {			\
			.r = __r,		\
			.g = __g,		\
			.b = __b,		\
			.a = __a		\
		};				\
		swap_u32(__c.value);		\	/* ret */
	})

#endif

