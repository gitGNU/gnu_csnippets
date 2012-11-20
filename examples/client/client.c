#include <csnippets/socket.h>

#include <signal.h>

static connection_t *conn = NULL;

static void on_read(connection_t *s, const struct sk_buff *buff)
{
    printf("[%d]: %s", s->fd, buff->data);
}

static void on_disconnect(connection_t *s)
{
    printf("%s disconnected\n", s->ip);
    exit(EXIT_SUCCESS);
}

static void on_connect(connection_t *s)
{
    s->on_read = on_read;
    s->on_disconnect = on_disconnect;

    printf("Connected to %s\n", s->remote);
}

static void __noreturn signal_handle(int sig)
{
    connection_free(conn);
    printf("Terminated\n");
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
    int err;

    log_init();
    conn = connection_create(&on_connect);
    if (!conn)
        return 1;

    conn->auto_read = true;
    err = socket_connect(conn, argc > 1 ? argv[1] : "127.0.0.1",
                argc > 2 ? argv[2] : "1337");
    if (err != 0) {
        perror("socket_connect()");
        return err;
    }

    signal(SIGINT, signal_handle);
    signal(SIGTERM, signal_handle);

    char buffer[1024];
    while (fgets(buffer, sizeof buffer, stdin))
        socket_write(conn, buffer);

    return 0;
}

