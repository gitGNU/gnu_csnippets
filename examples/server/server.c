#include <csnippets/socket.h>

#include <signal.h>
#include <unistd.h>

static struct listener *socket = NULL;  /* global for signal */

/**
 * Callbacks, self-explained
 */

static void on_read(struct conn *s, const struct sk_buff *buff)
{
    eprintf("(read)[%d][%zd]: %s\n", s->fd, buff->size, (char *)buff->data);
}

static void on_write(struct conn *s, const struct sk_buff *buff)
{
    eprintf("(write)[%d][%zd]: %s\n", s->fd, buff->size, (char *)buff->data);
}

static void on_disconnect(struct conn *s)
{
    eprintf("%s disconnected\n", s->host);
}

static void on_connect(struct conn *s)
{
    socket_writestr(s, "hi %s\n", s->host);
}

static struct sock_operations sops = {
    .write        = on_write,
    .read         = on_read,
    .connect      = on_connect,
    .disconnect   = on_disconnect
};

static void on_accept(struct listener *s, struct conn *n)
{
    n->ops = sops;
    eprintf("Accepted connection from %s\n", n->host);
}

static void __noreturn signal_handle(int sig)
{
    socket_free(socket);
    eprintf("Terminated\n");
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
    int err;

    socket = socket_create(on_accept);
    if (!socket)
        return 1;

    err = socket_listen(socket, argc > 1 ? argv[1] : NULL,
                argc > 2 ? argv[2] : "1337",
                argc > 3 ? atoi(argv[3]) : 10000);
    if (err) {
        perror("socket_listen()");
        return 1;
    }

    signal(SIGINT, signal_handle);
    signal(SIGTERM, signal_handle);
    while (1) {
        sleep(5);
	pthread_mutex_lock(&socket->conn_lock);
        eprintf("%d connections ATM\n", socket->num_connections);
	pthread_mutex_unlock(&socket->conn_lock);
    }

    socket_free(socket);
    return 0;
}

