#include <stdio.h>

#include <csnippets/module.h>

static void __init init(void)
{
    printf("Constructing\n");
}

static void __exit terminate(void)
{
    printf("Destructing...\n");
}

void module_func(void)
{
    printf("this is module_func speaking\n");
}

void module_func2(void)
{
    printf("this is module_func2 speaking\n");
}

