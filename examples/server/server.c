#include <csnippets/socket.h>
#include <csnippets/poll.h>

#include <unistd.h>
#include <pthread.h>

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

	conn_next(conn, echo_close, (struct buf *)0);
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

static void *start_thread(void *fptr)
{
	void (*rfptr) (void *) = fptr;
	if (rfptr)
		(*rfptr) (NULL);
	return NULL;
}

int main(int argc, char *argv[])
{
	int numthreads;
	if (!new_listener(argv[1], echo_start, NULL))
		fatal("failed to create new listener!\n");

	numthreads = argc > 3 ? atoi(argv[2]) : 0;
	if (numthreads > 0) {
		int i;
		pthread_t thrds[numthreads];
		for (i = 0; i < numthreads; ++i)
			if (pthread_create(&thrds[i], NULL, start_thread, conn_loop) != 0)
				return 1;
		struct pollfd pfd;
		pfd.fd = STDIN_FILENO;
		pfd.events = POLLIN;

		if (poll(&pfd, 1, -1) && pfd.revents & POLLIN) {
			printf("Keyboard interrupt\n");
			return 1;
		}
	} else
		conn_loop();
	return 0;
}

