Compiling modules under GNU/Linux
=================================

Turn a module into a .so:
```sh
gcc -ggdb3 -fPIC -c -Wall module.c -o module.o
gcc -ggdb3 -shared module.o -o module.so
```

And you should be set.

