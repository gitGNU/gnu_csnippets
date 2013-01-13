#include <csnippets/socket.h>

#define BUFFER_SIZE 1024

struct buf {
	char bytes[BUFFER_SIZE];
	size_t used;
};

static bool echo_close(struct conn *conn, struct buf *buf)
{
	free(buf);
	return true;
}

static bool echo_write(struct conn *conn, struct buf *buf)
{
	if (!conn_write(conn, buf->bytes, buf->used)) {
		free(buf);
		return false;
	}

	conn_next(conn, echo_close, NULL);
	next_close(conn, buf);
	return true;
}

static bool echo_read(struct conn *conn, struct buf *buf)
{
	buf->used = BUFFER_SIZE;
	if (!conn_read(conn, buf->bytes, &buf->used)) {
		free(buf);
		return false;
	}

	printf("%*s\n", buf->used, buf->bytes);
	return conn_next(conn, echo_write, buf);
}

static bool echo_start(struct conn *conn, void *unused)
{
	struct buf *buf = malloc(sizeof(*buf));
	if (!buf)
		return false;

	return conn_next(conn, echo_read, buf);
}

int main(int argc, char **argv)
{
	if (!new_conn(argv[1], argv[2], echo_start, NULL))
		fatal("failed to create a conn to %s %s!\n", argv[1], argv[2]);

	conn_loop ();
	return 0;
}

