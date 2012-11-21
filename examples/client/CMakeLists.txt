set(client_SOURCES ${modules_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/client.c
)

if(WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mthreads")
    set(client_DEFINITIONS ${modules_DEFINITIONS} -D_WIN32_WINNT=0x0600)
    set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--large-address-aware") # strip all debug information
elseif(APPLE)
    set(client_DEFINITIONS ${modules_DEFINITIONS} -D_REENTRANT) # enable thread safe code
else()
    #set(client_LIBRARIES ${modules_LIBRARIES} -ldl)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

