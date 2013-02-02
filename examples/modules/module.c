#include <stdio.h>

void init(int argc, char **argv)
{
	printf("module: init\n");
	return;
}

int deinit(void)
{
	printf("module: bye\n");
	return 0;
}

