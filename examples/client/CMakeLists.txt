# CMAKE_CURRENT_LIST_DIR cmake 2.6 compatibility
if(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 6)
    get_filename_component(CMAKE_CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)
endif(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 6)

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

