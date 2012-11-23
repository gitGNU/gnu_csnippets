set(server_SOURCES ${server_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/server.c
)

add_executable(server EXCLUDE_FROM_ALL ${server_SOURCES})
target_link_libraries(server ${this_library})

