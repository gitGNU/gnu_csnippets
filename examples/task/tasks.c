/*
 * Copyright (c) 2012 Allan Ference  <f.fallen45@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
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
        eprint("Section %s\n", p->section);
        for (def = c->def; def; def = def->next)
            eprint("Key [%s] -> Value [%s]\n", def->key, def->value);
    }

    config_free(c);
}

void test(void *p)
{
    eprint("test()\n");
}

int main(int argc, char **argv)
{
    log_init();

    tasks_init();
    events_init();

    eprint("Giving some time for the threads to run...\n");
    sleep(2);    /* wait for both threads to run */

    eprint("Adding task to test()\n");
    tasks_add(task_create(test, NULL));

    eprint("Creating an event for reading configuration.\n");
    events_add(event_create(2, config_parse_event,
                argc > 1 ? (void *)argv[1] : (void *)"config_test"));

    eprint("Adding an event (with the test() function as the routine)\n");
    events_add(event_create(2, test, NULL));

    eprint("Waiting a bit for the tasks to execute...\n");
    sleep(5);

    eprint("Adding another event (with the test() function as the routine)\n");
    events_add(event_create(3, test, NULL));

    eprint("Stopping both threads\n");
    events_stop();
    tasks_stop();

    eprint("Done\n");
    return 0;
}

