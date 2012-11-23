set(stack_SOURCES ${stack_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/stack.c
)

add_executable(stack EXCLUDE_FROM_ALL ${stack_SOURCES})
target_link_libraries(stack -L../build -lcsnippets ${stack_LIBRARIES})

