/*
 * Copyright (c) 2013 Ahmed Samy  <f.fallen45@gmail.com>
 * Licensed under MIT, see LICENSE.MIT for details.
 */
#ifndef _BUFFER_H
#define _BUFFER_H

typedef struct buffer buffer_t;
/* See bseek() for more information.  */
typedef enum seektype {
	seek_start,
	seek_curr,
	seek_end
} seektype_t;

/* balloc(int size) - Allocate buffer of @size
 *
 * Returns a malloc'd buffer.
 */
extern buffer_t *balloc(int bsize);

/* balloc_fp(const char *f) - Allocate buffer and store
 * the file data to the buffer's memory.
 *
 * Returns a malloc'd version of buffer_t with
 * all data set.
 */
extern buffer_t *balloc_fp(const char *f);

/* bfree(bp) - Free @b's memory.  */
extern void      bfree(buffer_t *bp);

/* bwrite(bp, const char *f) - Write @bp's data into
 * the file @f
 *
 * This writes the data the user has stored in memory
 * to the file @f.
 *
 * NB: This does not free the buffers memory after
 * a successfull write, you need to call bfree
 * on the buffer in order to free the memory.
 * This was intended for flexible reasons.
 *
 * Returns false on failure, see errno.
 */
extern bool	bwrite(buffer_t *bp, const char *f);


/* btell(bp) - Get the current read position of a buffer.
 *
 * Returns the current read position of a buffer.
 */
extern int      btell(buffer_t *bp);

/* bgetc(bp) - Get an unsigned character from a buffer
 *
 * Returns an unsigned character at the current read position
 * of the buffer.
 */
extern uint8_t  bgetc(buffer_t *bp);

/* bget16(bp) - Get an unsigned short from a buffer.
 *
 * Returns an unsigned short integer at the current read position
 * of the buffer.
 */
extern uint16_t bget16(buffer_t *bp);

/* bget32(bp) - Get an unsigned long from a buffer.
 *
 * Returns an unsigned long integer at the current read position
 * of the buffer.
 */
extern uint32_t bget32(buffer_t *bp);

/* baddc(bp, byte) - Add byte at current write position of the buffer.
 *
 * Adds the byte @byte at the current write pos.
 */
extern void     baddc(buffer_t *bp, uint8_t byte);

/* badd16(bp, val) - Add val at current write position of the buffer.
 *
 * Adds @val at the current write pos.
 */
extern void	badd16(buffer_t *bp, uint16_t val);

/* badd32(bp, val) - Add val at current write position of the buffer.
 *
 * Adds @val at the current write pos.
 */
extern void	badd32(buffer_t *bp, uint32_t val);

/* baddbytes(bp, bytes, size) - Add @bytes of size @size.
 *
 * This is an extension function.
 */
extern void	baddbytes(buffer_t *bp, uint8_t *bytes, size_t size);

/* bskip(bp, size) - Skip data of @size
 *
 * Increment the buffer read position with size
 * This is used to escape some unneeded data.
 *
 * Example Usage:
 *	bskip(bp, 4); skip a u32
 *	bskip(bp, 2); skip a u16
 *	etc.
 */
extern void     bskip(buffer_t *bp, int size);

/* bseek(bp, pos, write/read, type) - Seek to @pos
 *
 * Moves the read/write position to @pos.
 * If the type is:
 *  - seek_start  The read/write pos is set to @pos
 *  - seek_curr   This function works the same as bskip
 *		  (add @pos to the read/write pos)
 *  - seek_end    Seeks from the end of the buffer to @pos.
 */
extern void	bseek(buffer_t *bp, int pos, bool move_readpos, seektype_t type);

#endif

