#include <csnippets/task.h>
#include <csnippets/event.h>

#include <unistd.h>

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

