set(modules_SOURCES ${modules_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/modloader.c
)
set(module_INIT_SRC ${module_INIT_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/module.c    
)

if(WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mthreads")
    set(modules_DEFINITIONS ${modules_DEFINITIONS} -D_WIN32_WINNT=0x0600)
    set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--large-address-aware") # strip all debug information
elseif(APPLE)
    set(modules_DEFINITIONS ${modules_DEFINITIONS} -D_REENTRANT) # enable thread safe code
else()
    set(modules_LIBRARIES ${modules_LIBRARIES} -ldl)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

add_library(module EXCLUDE_FROM_ALL SHARED ${module_INIT_SRC})
target_link_libraries(module -L../build -lcsnippets ${modules_LIBRARIES})

