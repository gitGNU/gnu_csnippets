## CSnippets

[![Build Status](https://secure.travis-ci.org/allanference/csnippets.png?branch=master)](http://travis-ci.org/allanference/csnippets)

A nice snippets of C code that's meant to take part of other software.

CSnippets can be used as static/shared library, jump to Installation for more information.

Current modules implemented:

     1. Asynchronous Tasks
     2. Asynchronous Events (This depends on Tasks to function)
     3. Asynchronous sockets (with epoll and select support)
     4. Dynamic Library Loading (currently works for Linux, it can load .so files and get callable function address)
     5. Dyanmic Stack (This can be used as standalone header see the comments in csnippets/stack.h)

and much more, glance at the directory csnippets/ to see included modules (This is where the headers go)

Glance at examples/ to see examples of each module.

### Origin

Some of the codes here is taken from other sources (rewritten or changed a bit to fit).
Send an email to <f.fallen45@gmail.com>  if you think your credit is missing.

Please see AUTHORS for more information.

### Mailing lists

The project currently have 2 mailing lists:

Development: https://lists.nongnu.org/mailman/listinfo/csnippets-dev

Bug reports: https://lists.nongnu.org/mailman/listinfo/csnippets-bugs

### Contribute

Contributions are always welcome, and if you're ready to do so, please follow those steps:

    1. Fork the repository.
    2. Add your improvements
    3. Push it to a branch in your fork.
    4. Send a pull request or notify me via e-mail <f.fallen45@gmail.com>

We have a mailing list now, so feel free to populate your patches there: https://lists.nongnu.org/mailman/listinfo/csnippets-dev
Another version of the GitHub repository can be browsered at: http://git.savannah.gnu.org/cgit/csnippets.git/

Also glance at HACKING.md for coding style.

### TODO and bug reports

Please report any bugs you encounter to the mailing list: https://lists.nongnu.org/mailman/listinfo/csnippets-bugs

### License

MIT (Also known as "The Expat License")

### Building

Before compiling, make sure you have:

      1. CMake 2.6 or later   (get it by your package manager or from the cmake website)
      2. gcc (GNU C Compiler) part of the GNU Compiler Collection (GCC)
      3. GNU Make

```sh
./build.sh
```
Run:
```
./build.sh -h
```
for a list of options and examples available.

or manually:
```sh
mkdir build && cd build
cmake ..
make
```

### Installation

To install with a custom prefix (other than /usr/local):
```sh
cmake .. -DCMAKE_INSTALL_PREFIX:PATH=/usr
```
To build a shared library use:
```sh
cmake .. -DUSE_STATIC_LIBS=OFF
```

Variables (i.e where the .a and headers go etc):
```sh
SET(BIN_INSTALL_DIR bin CACHE PATH "Where to install binaries to.")
SET(LIB_INSTALL_DIR lib CACHE PATH "Where to install libraries to.")
SET(INCLUDE_INSTALL_DIR include CACHE PATH "Where to install headers to.")
```

You can modify them when running cmake like:
```sh
cmake .. -DINCLUDE_INSTALL_DIR=myinclude
```

Finally run:
```
make all install
```

