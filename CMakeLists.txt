cmake_minimum_required(VERSION 2.6)
project(csnippets)

set(VERSION "0.1")

include(src/CMakeLists.txt)
include(examples/CMakeLists.txt)

option(USE_SELECT_HANDLER "Use select handler instead of epoll (enabled by default on windows)" OFF)
add_definitions(-D"VERSION=\\"${VERSION}\\"")
if(USE_SELECT_HANDLER)
    add_definitions(-D__use_select)
endif()
add_library(csnippets ${src_SOURCES})

target_link_libraries(csnippets ${src_LIBRARIES})

add_custom_target(run COMMAND server DEPENDS server WORKING_DIRECTORY ${CMAKE_PROJECT_DIR})
add_executable(server ${examples_SOURCES})
target_link_libraries(server ${examples_LIBRARIES})

