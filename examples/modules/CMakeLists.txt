set(modules_SOURCES ${modules_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/modloader.c
)
set(module_INIT_SRC ${module_INIT_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/module.c    
)

add_library(module EXCLUDE_FROM_ALL SHARED ${module_INIT_SRC})
target_link_libraries(module -L../build -lcsnippets ${modules_LIBRARIES})

