cmake_minimum_required(VERSION 2.6)
project(csnippets)

set(VERSION "0.1")

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

