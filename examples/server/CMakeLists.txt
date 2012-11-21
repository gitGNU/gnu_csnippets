set(server_SOURCES ${server_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/server.c
)

if(WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mthreads")
    set(server_DEFINITIONS ${src_DEFINITIONS} -D_WIN32_WINNT=0x0600)
    set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--large-address-aware") # strip all debug information
elseif(APPLE)
    set(server_DEFINITIONS ${src_DEFINITIONS} -D_REENTRANT) # enable thread safe code
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

