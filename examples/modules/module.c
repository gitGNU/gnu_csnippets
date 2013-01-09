#include <stdio.h>

#define define_mod_func(name)	\
	void mod_ ##name(void) {	\
		printf("this is %s speaking\n", __func__); \
		return; \
	}

static void __init init(void)
{
	printf("Constructing\n");
}

static void __exit terminate(void)
{
	printf("Destructing...\n");
}

define_mod_func(fun1)
define_mod_func(fun2)
define_mod_func(testover9000)

