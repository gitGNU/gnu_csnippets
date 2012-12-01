#include <csnippets/task.h>
#include <csnippets/event.h>
#include <csnippets/config.h>

#include <unistd.h>

static void config_parse_event(void *filename)
{
    struct config *c, *p = NULL;
    struct def *def = NULL;

    c = config_parse((char *)filename);
    if (!c)
        fatal("failed to load config!");

    for (p = c; p; p = p->next) {
        eprintf("Section %s\n", p->section);
        for (def = c->def; def; def = def->next)
            eprintf("Key [%s] -> Value [%s]\n", def->key, def->value);
    }

    config_free(c);
}

void test(void *p)
{
    eprintf("test()\n");
    events_add(event_create(1, test, NULL));
}

int main(int argc, char **argv)
{
    tasks_init();
    events_init();

    eprintf("Giving some time for the threads to run...\n");
    sleep(2);    /* wait for both threads to run */

    eprintf("Adding task to test()\n");
    tasks_add(task_create(test, NULL));

    eprintf("Creating an event for reading configuration.\n");
    events_add(event_create(2, config_parse_event,
                argc > 1 ? (void *)argv[1] : (void *)"config_test"));

    eprintf("Adding an event (with the test() function as the routine)\n");
    events_add(event_create(2, test, NULL));

    eprintf("Waiting a bit for the tasks to execute...\n");
    sleep(5);

    eprintf("Adding another event (with the test() function as the routine)\n");
    events_add(event_create(3, test, NULL));

    eprintf("Stopping both threads\n");
    events_stop();
    tasks_stop();

    eprintf("Done\n");
    return 0;
}

