set(tasks_SOURCES ${tasks_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/tasks.c
)

add_executable(tasks EXCLUDE_FROM_ALL ${tasks_SOURCES})
target_link_libraries(tasks ${this_library})

