#!/bin/bash

# A simple bash script to suit my needs for faster testing.
# This script was not made to be "optimized" but just to
# "do the job" as fast as possible.
#
# Feel free to improve it(as in add features/optimize etc.).

# Figure out cores
CORES=`grep processor /proc/cpuinfo | wc -l`
# Process to be passed to $MAKE
MAKEOPT=$(($CORES + 1))
# arguments to pass to the executable generated
ARGS=1337
# The target to build after building the static library.
TARGET=server
# the "make" command, change this to "mingw32-make" if using the MINGW toolchain.
MAKE=make
# the debugger
DBG=gdb
# build options
buildopt=
# whether we should clean before building
clean_before_build=no

# If this is the GIT version, define build commit and revision.
# Otherwise, don't do anything.  We might need the
# gen-version.sh script for releases, this will help
# get the tag name and version associated.
# Or alternatively, we could write an else case here, and
# define those strings to something informative.
#
# Those are intended for feature-checking in the near future.
if [ -d .git ]; then
	buildopt="-DBUILD_COMMIT=`git describe --dirty --always` \
			-DBUILD_REVISION=`git rev-list --all | wc -l`"
fi

run() {
	echo "Running $TARGET with $ARGS"
	if [ "$1" = "$DBG" ]; then
		$1 --args $TARGET $ARGS
	else
		./$TARGET $ARGS
	fi
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
	-c) clean_before_build=yes ;;
	*) buildopt=-DUSE_SELECT_HANDLER=OFF ;;
esac

mkdir -p build
cd build
cmake .. $buildopt || exit

if [ "$clean_before_build" = "yes" ]; then
	echo "Cleaning up stuff..."
	$MAKE clean
fi

case "$1" in
	-g)  _make
		run $DBG ;;
	-a)  _make
		run ;;
	-gv) _make V=1
		run $DBG ;;
	-av) _make V=1
		run ;;
	-v)  _make V=1 ;;
	*)   _make ;;
esac

