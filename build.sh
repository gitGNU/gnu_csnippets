#!/bin/bash

# A simple bash script to suit my needs for faster testing.
# This script was not made to be "optimized" but just to
# "do the job" as fast as possible.
#
# Feel free to improve it(as in add features/optimize etc.).

# Figure out cores
CORES=`grep processor /proc/cpuinfo | wc -l`
# Process to be passed to make -jN
MAKEOPT=$(($CORES + 1))
# arguments to pass to the executable generated
ARGS=
# The executable target to build.
TARGET=tasks
# the "make" command
MAKE=make
# the debugger
DBG=gdb
# build options
buildopt=

if [ ! -d build ]; then
    mkdir build
fi

run() {
    echo "Running $TARGET with $ARGS"
    if [ -d "$TARGET_DIR" ]; then
        cd $TARGET_DIR
    fi
    if [ "$1" = "$DBG" ]; then
        $1 --args $TARGET $ARGS
    else
        ./$TARGET $ARGS
    fi
    # return back to the old directory
    cd ..
}

__make() {
    $MAKE $1 $2 -j$MAKEOPT || exit
}

_make() {
    echo "Making Library"
    __make $1
    echo "Making module"
    __make module $1
    echo "Making $TARGET"
    __make $TARGET $1
}

case "$2" in
    -s) buildopt=-DUSE_SELECT_HANDLER=ON ;;
     *) buildopt=-DUSE_SELECT_HANDLER=OFF ;;
esac

cd build
cmake .. $buildopt

echo "Cleaning up stuff..."
$MAKE clean

case "$1" in
    -g) _make
        run $DBG
        ;;
    -a) _make
        run
        ;;
    -gv) _make V=1
        run $DBG
        ;;
    -av) _make V=1
        run
        ;;
    -v) _make V=1
        ;;
    *) _make
        ;;
esac

cd ..

