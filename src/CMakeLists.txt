#
# Copyright (c) 2012 Ahmed Samy <f.fallen45@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

set(csnippets_LIBRARIES "")
set(csnippets_DEFINITIONS "")
set(csnippets_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/..")
set(csnippets_SOURCES ${csnippets_SOURCES}
	${CMAKE_CURRENT_LIST_DIR}/asprintf.c
	${CMAKE_CURRENT_LIST_DIR}/event.c
	${CMAKE_CURRENT_LIST_DIR}/socket.c
	${CMAKE_CURRENT_LIST_DIR}/select_event.c
	${CMAKE_CURRENT_LIST_DIR}/epoll_event.c
	${CMAKE_CURRENT_LIST_DIR}/string.c
	${CMAKE_CURRENT_LIST_DIR}/error.c
	${CMAKE_CURRENT_LIST_DIR}/list.c
	${CMAKE_CURRENT_LIST_DIR}/rwlock.c
	${CMAKE_CURRENT_LIST_DIR}/task.c
	${CMAKE_CURRENT_LIST_DIR}/module.c
	${CMAKE_CURRENT_LIST_DIR}/poll.c
	${CMAKE_CURRENT_LIST_DIR}/htable.c
	${CMAKE_CURRENT_LIST_DIR}/hash.c
	${CMAKE_CURRENT_LIST_DIR}/rbtree.c
	${CMAKE_CURRENT_LIST_DIR}/csnippets.c
)
set(csnippets_PRE_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/../csnippets/csnippets.h")

set(BUILD_COMMIT "devel" CACHE "Git commit string (intended for releases)" STRING)
set(BUILD_REVISION "0" CACHE "Git revision string (intended for releases)" STRING)

set(USE_COLORS "ON" CACHE "Use colorful terminal messages (for errors)" BOOL)
if(USE_COLORS)
	set(csnippets_DEFINITIONS ${csnippets_DEFINITIONS} -DUSE_COLORS)
endif()

# default build type
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()

# gcc compile flags
set(WARNS_FLAGS "-Wall -Wno-sign-compare ")
set(CMAKE_C_FLAGS		    "${CMAKE_C_FLAGS} ${WARNS_FLAGS} ${ARCH_FLAGS} -pipe -include ${csnippets_PRE_INCLUDE}")
set(CMAKE_C_FLAGS_DEBUG		    "-O0 -g3 -ggdb")
set(CMAKE_C_FLAGS_RELWITHDEBINFO    "-O1 -g3 -ggdb -fno-omit-frame-pointer")
set(CMAKE_C_FLAGS_RELEASE	    "-O2")
set(CMAKE_C_FLAGS_PERFORMANCE	    "-Ofast -mmmx -msse -msse2")
set(CMAKE_C_FLAGS_MINSIZEREL	    "-Os")

message(STATUS "Build type: " ${CMAKE_BUILD_TYPE})
add_definitions(-D"BUILD_TYPE=\\\"${CMAKE_BUILD_TYPE}\\\"")
message(STATUS "Build commit: ${BUILD_COMMIT}")
add_definitions(-D"BUILD_COMMIT=\\\"${BUILD_COMMIT}\\\"")
message(STATUS "Build revision: ${BUILD_REVISION}")
add_definitions(-D"BUILD_REVISION=\\\"${BUILD_REVISION}\\\"")

find_package(LibIntl)
if(WIN32)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mthreads")
	set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--large-address-aware") # strip all debug information
	set(csnippets_LIBRARIES "-lws2_32")
elseif(APPLE)
	set(csnippets_DEFINITIONS ${csnippets_DEFINITIONS} -D_REENTRANT) # enable thread safe code
elseif(UNIX)
	find_package(DL REQUIRED)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
	set(csnippets_DEFINITIONS "${csnippets_DEFINITIONS} -D_DEBUG_POLLEV -D_DEBUG_SOCKET -D_DEBUG_EVENTS -D_DEBUG_TASKS -D_DEBUG_MODULES -D_DEBUG")
	set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--no-as-needed")
	message(STATUS "Debug information: ON")
else()
	message(STATUS "Debug information: OFF")
	set(csnippets_DEFINITIONS ${csnippets_DEFINITIONS} -DNDEBUG) # NDEBUG disable asserations
	set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--as-needed -Wl,-s") # strip all debug information
endif()

set(csnippets_INCLUDE_DIRS ${csnippets_INCLUDE_DIRS} ${DL_INCLUDE_DIR})
set(csnippets_LIBRARIES ${csnippets_LIBRARIES} ${LIBINTL_LIBRARIES} -pthread ${DL_LIBRARIES})
if(LIBINTL_FOUND)
	set(csnippets_DEFINITIONS "${csnippets_DEFINITIONS} -DHAVE_LIBINTL_H")
endif()

