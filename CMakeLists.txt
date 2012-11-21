cmake_minimum_required(VERSION 2.6)
project(csnippets)

set(VERSION "0.1")

# CMAKE_CURRENT_LIST_DIR cmake 2.6 compatibility
if(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 6)
    get_filename_component(CMAKE_CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)
endif(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 6)

include(src/CMakeLists.txt)
include(examples/server/CMakeLists.txt)
include(examples/client/CMakeLists.txt)
include(examples/modules/CMakeLists.txt)

option(USE_SELECT_HANDLER "Use select handler instead of epoll (enabled by default on windows)" OFF)
add_definitions(-D"VERSION=\\"${VERSION}\\"")
if(USE_SELECT_HANDLER)
    add_definitions(-D__use_select)
endif()

set(this_library "-L../build -lcsnippets")

add_library(csnippets ${src_SOURCES})
target_link_libraries(csnippets ${src_LIBRARIES})

add_executable(client EXCLUDE_FROM_ALL ${client_SOURCES})
target_link_libraries(client ${this_library} ${client_LIBRARIES})

add_executable(server EXCLUDE_FROM_ALL ${server_SOURCES})
target_link_libraries(server ${this_library} ${server_LIBRARIES})

add_executable(modloader EXCLUDE_FROM_ALL ${modules_SOURCES})
target_link_libraries(modloader ${this_library} ${modules_LIBRARIES})

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
