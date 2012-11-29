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
if(USE_SELECT_HANDLER)
    add_definitions(-DUSE_SELECT)
endif()

add_library(csnippets ${csnippets_SOURCES})
add_definitions(${csnippets_DEFINITIONS})
target_link_libraries(csnippets ${csnippets_LIBRARIES})
include_directories(csnippets ${csnippets_INCLUDE_DIRS})

set(this_library "-L. -Wl,--no-as-needed -lcsnippets ${csnippets_LIBRARIES}")
include(examples/server/CMakeLists.txt)
include(examples/client/CMakeLists.txt)
include(examples/modules/CMakeLists.txt)
include(examples/task/CMakeLists.txt)
include(examples/stack/CMakeLists.txt)

# Installation paths
SET(BIN_INSTALL_DIR bin CACHE PATH "Where to install binaries to.")
SET(LIB_INSTALL_DIR lib CACHE PATH "Where to install libraries to.")
SET(INCLUDE_INSTALL_DIR include CACHE PATH "Where to install headers to.")

# Installation
INSTALL(DIRECTORY csnippets DESTINATION ${INCLUDE_INSTALL_DIR})
INSTALL(TARGETS csnippets
    RUNTIME DESTINATION ${BIN_INSTALL_DIR}
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR})
