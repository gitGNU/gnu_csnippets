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

cmake_minimum_required(VERSION 2.6)
project(csnippets)

set(VERSION "0.1")

# CMAKE_CURRENT_LIST_DIR cmake 2.6 compatibility
if(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 6)
	get_filename_component(CMAKE_CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)
endif(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 6)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
include(src/CMakeLists.txt)

option(USE_SELECT_HANDLER "Use select handler instead of epoll (enabled by default on windows)" OFF)
add_definitions(-D"VERSION=\\"${VERSION}\\"")

set(SOCKET_INTERFACE "Select")
set(SOCKET_INTERFACE_DEF "-DUSE_SELECT")
if(USE_SELECT_HANDLER OR WIN32)
	# do nothing, it's the default!
elseif(UNIX)
	# TODO: Epoll is Linux specific, we cannot just assign it to
	# every UNIX-based system.  Instead select can be used as "generic"
	# interface or the corresponding interface.
	set(SOCKET_INTERFACE "Epoll")
	set(SOCKET_INTERFACE_DEF "-DUSE_EPOLL")
else()
	message(STATUS "No poll interface specified,  falling back to default (${SOCKET_INTERFACE} interface)")
endif()
message(STATUS "Using interface: ${SOCKET_INTERFACE}")
add_definitions(${SOCKET_INTERFACE_DEF})

option(USE_STATIC_LIBS "Don't use shared libraries" ON)
if(USE_STATIC_LIBS)
	if(NOT APPLE)
		set(CMAKE_C_LINK_FLAGS "-static-libgcc")
	endif()
	add_library(csnippets ${csnippets_SOURCES})
	message(STATUS "Link to static libraries: ON")
else()
	add_library(csnippets SHARED ${csnippets_SOURCES})
	message(STATUS "Link to static libraries: OFF")
endif()

add_definitions(${csnippets_DEFINITIONS})
target_link_libraries(csnippets ${csnippets_LIBRARIES})
include_directories(csnippets ${csnippets_INCLUDE_DIRS})

if(USE_STATIC_LIBS)
	set(this_library "-L. -Wl,--no-as-needed -lcsnippets ${csnippets_LIBRARIES}")
else()
	set(this_library "-L. -lcsnippets ${csnippets_LIBRARIES}")
endif()

include(examples/server/CMakeLists.txt)
include(examples/client/CMakeLists.txt)
include(examples/modules/CMakeLists.txt)
include(examples/task/CMakeLists.txt)
include(examples/stack/CMakeLists.txt)

# Installation paths
set(BIN_INSTALL_DIR	bin	CACHE PATH "Where to install binaries to.")
set(LIB_INSTALL_DIR	lib	CACHE PATH "Where to install libraries to.")
set(INCLUDE_INSTALL_DIR include CACHE PATH "Where to install headers to.")

# Installation
install(DIRECTORY csnippets DESTINATION ${INCLUDE_INSTALL_DIR})
install(TARGETS csnippets
	RUNTIME DESTINATION ${BIN_INSTALL_DIR}
	LIBRARY DESTINATION ${LIB_INSTALL_DIR}
	ARCHIVE DESTINATION ${LIB_INSTALL_DIR})

