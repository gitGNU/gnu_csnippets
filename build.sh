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
# arguments to pass to $TARGET (see below) generated
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
# Are we cross-compiling?
cross_build=no
# build type (Release with debug info, debug only, release only)
# Release with debug info (This is what we fallback to in cmake, if noone specified).
# Valid ones are:
#    Debug,
#    RelWithDebInfo
#    Release
build_type=RelWithDebInfo

buildopt="$buildopt -DCMAKE_BUILD_TYPE=$build_type"
# If this is the GIT version, define build commit and revision.
# We might need the gen-version.sh script instead of all this magic,
# this will help get the tag name and version associated.
#
# Or alternatively, we could write an else case here, and
# define those strings to something informative.
#
# Those are intended for feature-checking in the near future.
if [ -d .git ]; then
	really_inside=yes
	git --help >/dev/null  # Is GIT actually installed at this site?
	test $? -eq 0 || really_inside=no
	if [ "$really_inside" = "yes" ]; then
		buildopt="$buildopt -DBUILD_COMMIT=`git describe --dirty --always` \
			   -DBUILD_REVISION=`git rev-list --all | wc -l`"
	fi
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
	-s) buildopt="$buildopt -DUSE_SELECT_HANDLER=ON" ;;
	-c) clean_before_build=yes ;;
	-cb) cross_build=yes ;;
	*) buildopt="$buildopt -DUSE_SELECT_HANDLER=OFF" ;;
esac

if [ "$cross_build" = "yes" ]; then
	if [ -d build ]; then
		# CMake does not allow major changes like these if there's
		# already a cached version with these variables set, so clean it up.
		rm -rf build
	fi
	buildopt="$buildopt -DCMAKE_SYSTEM_NAME=Windows\
		   -DCMAKE_RC_COMPILER=/usr/bin/i486-mingw32-windres\
		   -DCMAKE_C_COMPILER=/usr/bin/i486-mingw32-gcc\
		   -DCMAKE_CXX_COMPILER=/usr/bin/i486-mingw32-g++"
fi

if [ "$clean_before_build" = "yes" ]; then
	if [ -d build ]; then
		echo "Removing build directory..."
		rm -rf build
	fi
fi

mkdir -p build
cd build

echo Generating Makefile with $buildopt
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

