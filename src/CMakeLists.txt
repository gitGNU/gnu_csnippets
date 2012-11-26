set(src_LIBRARIES "")
set(src_DEFINITIONS "")
set(src_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/..")
set(src_SOURCES ${src_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/asprintf.c
    ${CMAKE_CURRENT_LIST_DIR}/config.c
    ${CMAKE_CURRENT_LIST_DIR}/event.c
    ${CMAKE_CURRENT_LIST_DIR}/map.c
    ${CMAKE_CURRENT_LIST_DIR}/socket.c
    ${CMAKE_CURRENT_LIST_DIR}/socket_select.c
    ${CMAKE_CURRENT_LIST_DIR}/socket_epoll.c
    ${CMAKE_CURRENT_LIST_DIR}/strmisc.c
    ${CMAKE_CURRENT_LIST_DIR}/error.c
    ${CMAKE_CURRENT_LIST_DIR}/list.c
    ${CMAKE_CURRENT_LIST_DIR}/rwlock.c
    ${CMAKE_CURRENT_LIST_DIR}/task.c
    ${CMAKE_CURRENT_LIST_DIR}/module.c
)

set(BUILD_COMMIT "devel" CACHE "Git commit string (intended for releases)" STRING)
set(BUILD_REVISION "0" CACHE "Git revision string (intended for releases)" STRING)

# default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()

# gcc compile flags
set(WARNS_FLAGS "-Wall -Wextra -Wno-unused-parameter -Wno-unused-but-set-variable -Wno-unused-variable -Wno-sign-compare")
set(CMAKE_C_FLAGS                   "${CMAKE_C_FLAGS} ${WARNS_FLAGS} ${ARCH_FLAGS} -pipe -include ${CMAKE_CURRENT_LIST_DIR}/../csnippets/csnippets.h")
set(CMAKE_C_FLAGS_DEBUG             "-O0 -g3 -ggdb")
set(CMAKE_C_FLAGS_RELWITHDEBINFO    "-O1 -g3 -ggdb -fno-omit-frame-pointer")
set(CMAKE_C_FLAGS_RELEASE           "-O2")
set(CMAKE_C_FLAGS_PERFORMANCE       "-Ofast -mmmx -msse -msse2")
set(CMAKE_C_FLAGS_MINSIZEREL        "-Os")

message(STATUS "Build type: " ${CMAKE_BUILD_TYPE})
add_definitions(-D"BUILD_TYPE=\\\"${CMAKE_BUILD_TYPE}\\\"")
message(STATUS "Build commit: ${BUILD_COMMIT}")
add_definitions(-D"BUILD_COMMIT=\\\"${BUILD_COMMIT}\\\"")
message(STATUS "Build revision: ${BUILD_REVISION}")
add_definitions(-D"BUILD_REVISION=\\\"${BUILD_REVISION}\\\"")

find_package(DL REQUIRED)
find_package(LibIntl)

if(WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mthreads -D__use_select")
    set(src_DEFINITIONS ${src_DEFINITIONS} -D_WIN32_WINNT=0x0600)
    set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--large-address-aware") # strip all debug information
    set(src_LIBRARIES "-lws2_32")
    set(src_SOURCES ${src_SOURCES} ${CMAKE_CURRENT_LIST_DIR}/module_windows.c)
elseif(APPLE)
    set(src_DEFINITIONS ${src_DEFINITIONS} -D_REENTRANT) # enable thread safe code
endif()

if(NOT WIN32)
    set(src_SOURCES ${src_SOURCES} ${CMAKE_CURRENT_LIST_DIR}/module_unix.c)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(src_DEFINITIONS "${src_DEFINITIONS} -D_DEBUG_SOCKET -D_DEBUG_EVENTS -D_DEBUG_TASKS -D_DEBUG_MODULES")
    set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--no-as-needed")
    message(STATUS "Debug information: ON")
else()
    message(STATUS "Debug information: OFF")
    set(src_DEFINITIONS ${src_DEFINITIONS} -DNDEBUG) # NDEBUG disable asserts
    set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--as-needed -Wl,-s") # strip all debug information
endif()


set(src_INCLUDE_DIRS ${src_INCLUDE_DIRS} ${DL_INCLUDE_DIR})
set(src_LIBRARIES ${src_LIBRARIES} ${LIBINTL_LIBRARIES} -lpthread ${DL_LIBRARIES})
if(LIBINTL_FOUND)
    set(src_DEFINITIONS "${src_DEFINITIONS} -DHAVE_LIBINTL_H")
endif()

