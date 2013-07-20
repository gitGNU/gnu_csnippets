/*
 * Copyright (c) 2013 Ahmed Samy  <f.fallen45@gmail.com>
 * Licensed under MIT, see LICENSE.MIT for details.
 */
#include <csnippets/buffer.h>

typedef struct buffer {
	int wpos, rpos, size;
	uint8_t *data;
} buffer_t;

static __inline uint16_t unpack_u16(unsigned char *addr)
{
	return addr[1] << 8 | addr[0];
}

static __inline uint32_t unpack_u32(unsigned char *addr)
{
	return unpack_u16(addr + 2) << 16 | unpack_u16(addr);
}

static __inline void pack_u16(unsigned char *addr, uint16_t value)
{
	addr[1] = value >> 8;
	addr[0] = (uint8_t)value;
}

static __inline void pack_u32(unsigned char *addr, uint32_t value)
{
	pack_u16(addr + 2, value >> 16);
	pack_u16(addr,     (uint16_t)value);
}

static bool stat_file(const char *f, int *size)
{
	struct stat st;
	if (stat(f, &st) != 0)
		return false;

	*size = st.st_size;
	return true;
}

static void bresize(buffer_t *p, int nsize)
{
	uint8_t *n;
	if (!(n = malloc(nsize)))
		return;

	p->size = nsize;
	memcpy(n, p->data, p->wpos);

	if (p->data)
		free(p->data);

	p->data = n;
	memset(p->data, 0, p->size - p->wpos);
}

uint32_t bget32(buffer_t *bp)
{
	uint32_t tmp = unpack_u32(&bp->data[bp->rpos]);
	bp->rpos += 4;
	return tmp;
}

uint16_t bget16(buffer_t *bp)
{
	uint16_t tmp = unpack_u16(&bp->data[bp->rpos]);
	bp->rpos += 2;
	return tmp;
}

uint8_t bgetc(buffer_t *bp)
{
	uint8_t tmp = bp->data[bp->rpos];
	++bp->rpos;
	return tmp;
}

int btell(buffer_t *bp)
{
	return bp->rpos;
}

void baddc(buffer_t *buffer, uint8_t byte)
{
	if (buffer->wpos+1 > buffer->size)
		bresize(buffer, buffer->size + 1);

	buffer->data[buffer->wpos] = byte;
	++buffer->wpos;
}

void badd16(buffer_t *buffer, uint16_t val)
{
	if (buffer->wpos + 2 > buffer->size)
		bresize(buffer, buffer->size + 2);

	pack_u16(&buffer->data[buffer->wpos], val);
	buffer->wpos += 2;
}

void badd32(buffer_t *buffer, uint32_t val)
{
	if (buffer->wpos + 4 > buffer->size)
		bresize(buffer, buffer->size + 4);

	pack_u32(&buffer->data[buffer->wpos], val);
	buffer->wpos += 4;
}

void baddbytes(buffer_t *buffer, uint8_t *bytes, size_t size)
{
	size_t i;
	if (buffer->wpos + size > buffer->size)
		bresize(buffer, buffer->size + size);

	for (i = 0; i < size; ++i)
		buffer->data[buffer->wpos++] = bytes[i];
}

void bskip(buffer_t *buffer, int size)
{
	if (buffer->rpos + size > buffer->size)
		return;
	buffer->rpos += size;
}

void bseek(buffer_t *buffer, int pos, bool move_readpos, seektype_t type)
{
	int *writePos;
	if (move_readpos)
		writePos = &buffer->rpos;
	else
		writePos = &buffer->wpos;

	switch (type) {
	case seek_start:
		*writePos = pos;
		break;
	case seek_curr:
		if (!move_readpos && buffer->wpos + pos > buffer->size)
			bresize(buffer, buffer->size + (pos * 2));
		*writePos += pos;
		break;
	case seek_end:
		*writePos -= pos;
		break;
	}
}

buffer_t *balloc(int bsize)
{
	buffer_t *b;
	if (!(b = malloc(sizeof(*b))))
		abort();

	if (!(b->data = malloc(bsize)))
		abort();

	memset(b->data, 0, bsize);
	b->size  = bsize;
	b->wpos  = b->rpos = 0;
	return b;
}

buffer_t* balloc_fp(const char* f)
{
	FILE *fp;
	buffer_t *res;
	int fsize;

	if (!stat_file(f, &fsize))
		return NULL;

	fp = fopen(f, "rb");
	if (!fp)
		return NULL;

	res = balloc(fsize);
	if (!res) {
		fclose(fp);
		return NULL;
	}

	if (fread(res->data, 1, res->size, fp) != res->size) {
		bfree(res);
		fclose(fp);
		return NULL;
	}

	fclose(fp);
	return res;
}

bool bwrite(buffer_t *buffer, const char *f)
{
	FILE *fp;

	fp = fopen(f, "wb");
	if (!fp)
		return false;

	if (fwrite(buffer->data, 1, buffer->size, fp) != buffer->size)
		return false;

	/* Flush n close.  */
	fclose(fp);
	return true;
}

void bfree(buffer_t *buffer)
{
	free(buffer->data);
	free(buffer);
}

