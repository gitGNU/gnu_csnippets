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
# Executable
EXE=server
# Directory where the executable is.
EXE_DIR=
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
    echo "Running $EXE with $ARGS"
    if [ -d "$EXE_DIR" ]; then
        cd $EXE_DIR
    fi
    if [ "$1" = "$DBG" ]; then
        $1 --args $EXE $ARGS
    else
        ./$EXE $ARGS
    fi
    # return back to the old directory
    cd ..
}

_make() {
    echo "Making Library"
    $MAKE $1 -j$MAKEOPT || exit
    echo "Making executable"
    $MAKE $EXE $1 -j$MAKEOPT || exit
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
    -l) cd src && _make
        ;;
    -e) cd src && _make
        cd ../$EXE_DIR && _make
        cd ..
        ;;
    *) _make
        ;;
esac

cd ..

